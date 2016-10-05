/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) University of North Carolina at Chapel Hill
 * Authors: Ben Newton and Neil Davis
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wireless-point-to-point-module.h"


#include "ns3/mpi-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WPPP-Distributed");

//===========================================================================
//Set position of a node
//===========================================================================
static void
SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}

//===========================================================================
//Set velocity of a node
//===========================================================================
static void
SetVelocity (Ptr<Node> node, Vector velocity)
{
  Ptr<ConstantVelocityMobilityModel> mobility = 
    node->GetObject<ConstantVelocityMobilityModel> ();
  mobility->SetVelocity (velocity);
}

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;
  bool tracing = false;
  bool nullmsg = false;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("nullmsg", "Enable the use of null-message synchronization", 
                nullmsg);

  cmd.Parse (argc,argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough, otherwise IP addresses 
  // soon become an issue
  if (nWifi > 250 || nCsma > 250)
    {
      std::cout << "Too many wifi or csma nodes, no more than 250 each." 
                << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  // Sequential fallback values
  uint32_t systemCount = 2;

#ifdef NS3_MPI

  // Distributed simulation setup; by default use granted time window algorithm.
  if(nullmsg) 
    {
      GlobalValue::Bind ("SimulatorImplementationType",
                         StringValue ("ns3::NullMessageSimulatorImpl"));
    } 
  else 
    {
      GlobalValue::Bind ("SimulatorImplementationType",
                         StringValue ("ns3::DistributedSimulatorImpl"));
    }

  MpiInterface::Enable (&argc, &argv);

  systemCount = MpiInterface::GetSize ();

  // Check for valid distributed parameters.
  // Must have 2 and only 2 Logical Processors (LPs)
  if (systemCount != 2)
    {
      std::cout << "This simulation requires 2 and only 2 logical processors." 
                << std::endl;
      return 1;
    }

#endif // NS3_MPI

  uint32_t system_one = 0;
  
  uint32_t system_two = systemCount - 1;
  
  NodeContainer p2pNodes;

  Ptr<Node> p2pNode1 = CreateObject<Node> (system_one); 
  Ptr<Node> p2pNode2 = CreateObject<Node> (system_two); 
  p2pNodes.Add (p2pNode1);
  p2pNodes.Add (p2pNode2);

  WirelessPointToPointHelper wirelessPointToPoint;
  wirelessPointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  // Below needs to be set to lowst value depending on smallest node separation
  wirelessPointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms")); 
  wirelessPointToPoint.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  NetDeviceContainer p2pDevices;
  p2pDevices = wirelessPointToPoint.Install (p2pNodes, 3);

  InternetStackHelper stack;
  stack.Install (p2pNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (p2pDevices);

  //explicitly connect devices
  Ptr<WirelessPointToPointNetDevice> dev1 = 
    DynamicCast<WirelessPointToPointNetDevice>(p2pNodes.Get(0)->GetDevice(1)); 
  //node0(link1)-> node1
  dev1->Connect(p2pNodes.Get(0), dev1, p2pNodes.Get(1));
  
  Ptr<WirelessPointToPointNetDevice> dev2 = 
    DynamicCast<WirelessPointToPointNetDevice>(p2pNodes.Get(1)->GetDevice(1)); 
  //node1(link1)-> node0
  dev2->Connect(p2pNodes.Get(1), dev2, p2pNodes.Get(0));

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install(p2pNodes);
  
  SetPosition (p2pNodes.Get(0), Vector (0.01,0.0,0.0));
  SetPosition (p2pNodes.Get(1), Vector (190000.0,0.0,0.0));
  SetVelocity (p2pNodes.Get(0), Vector (0.0,0.0,0.0));
  SetVelocity (p2pNodes.Get(1), Vector (0.0,0.0,0.0));


  UdpEchoServerHelper echoServer (9);
  
  ApplicationContainer serverApps = echoServer.Install (p2pNodes.Get(1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
 
  UdpEchoClientHelper echoClient (interfaces.GetAddress(3), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps = 
    echoClient.Install (p2pNodes.Get(0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
  Simulator::Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();

#ifdef NS3_MPI
  // Exit the MPI execution environment
  MpiInterface::Disable ();
#endif
  
  return 0;
}
