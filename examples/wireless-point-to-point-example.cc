/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 * Author: Ben Newton
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wireless-point-to-point-module.h"
#include "ns3/applications-module.h"

#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WirelessPointToPointExample");

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
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (2);

  WirelessPointToPointHelper wirelessP2P;
  wirelessP2P.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  wirelessP2P.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  unsigned int devicesPerNode = 3;

  NetDeviceContainer devices;
  devices = wirelessP2P.Install (nodes, devicesPerNode); 

  InternetStackHelper stack;
  stack.Install (nodes);
  
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  //explicitly connect devices
  Ptr<WirelessPointToPointNetDevice> dev1 = 
    DynamicCast<WirelessPointToPointNetDevice>(nodes.Get(0)->GetDevice(1)); 
  //node0(link1)-> node1
  dev1->Connect(nodes.Get(0), dev1, nodes.Get(1));
  
  Ptr<WirelessPointToPointNetDevice> dev2 = 
    DynamicCast<WirelessPointToPointNetDevice>(nodes.Get(1)->GetDevice(1)); 
  //node1(link1)-> node0
  dev2->Connect(nodes.Get(1), dev2, nodes.Get(0));

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install(nodes);
  
  SetPosition (nodes.Get(0), Vector (0.01,0.0,0.0));
  SetPosition (nodes.Get(1), Vector (190000.0,0.0,0.0));
  SetVelocity (nodes.Get(0), Vector (0.0,0.0,0.0));
  SetVelocity (nodes.Get(1), Vector (0.0,0.0,0.0));

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  //below could choose one of 3 interfaces(3,4,5) on other node
  UdpEchoClientHelper echoClient (interfaces.GetAddress (3), 9); 
  
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
