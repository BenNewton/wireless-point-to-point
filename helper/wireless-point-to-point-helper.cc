/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 * Author: Ben Newton (adapted from point-to-point-helper.cc)
 */

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/wireless-point-to-point-net-device.h"
#include "ns3/wireless-point-to-point-channel.h"
#include "ns3/queue.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/mpi-module.h"

#include "ns3/trace-helper.h"
#include "wireless-point-to-point-helper.h"

#include "ns3/mobility-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WirelessPointToPointHelper");

WirelessPointToPointHelper::WirelessPointToPointHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::WirelessPointToPointNetDevice");
  m_channelFactory.SetTypeId ("ns3::WirelessPointToPointChannel");
}

void 
WirelessPointToPointHelper::SetQueue (std::string type,
                              std::string n1, const AttributeValue &v1,
                              std::string n2, const AttributeValue &v2,
                              std::string n3, const AttributeValue &v3,
                              std::string n4, const AttributeValue &v4)
{
  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void
WirelessPointToPointHelper::SetPropagationDelay (std::string type,
                                                 std::string n0, 
                                                 const AttributeValue &v0,
                                                 std::string n1, 
                                                 const AttributeValue &v1,
                                                 std::string n2, 
                                                 const AttributeValue &v2,
                                                 std::string n3, 
                                                 const AttributeValue &v3,
                                                 std::string n4, 
                                                 const AttributeValue &v4,
                                                 std::string n5, 
                                                 const AttributeValue &v5,
                                                 std::string n6, 
                                                 const AttributeValue &v6,
                                                 std::string n7, 
                                                 const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_propagationDelay = factory;
}

void 
WirelessPointToPointHelper::SetDeviceAttribute (std::string n1, 
                                                const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
WirelessPointToPointHelper::SetChannelAttribute (std::string n1, 
                                                 const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

void 
WirelessPointToPointHelper::EnablePcapInternal (std::string prefix, 
                                                Ptr<NetDevice> nd, 
                                                bool promiscuous, 
                                                bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type 
  // WirelessPointToPointNetDevice.
  //
  Ptr<WirelessPointToPointNetDevice> device = 
    nd->GetObject<WirelessPointToPointNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("WirelessPointToPointHelper::EnablePcapInternal(): Device " 
                   << device 
                   << " not of type ns3::WirelessPointToPointNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, 
                                                     PcapHelper::DLT_PPP);
  pcapHelper.HookDefaultSink<WirelessPointToPointNetDevice> (device, 
                                                             "PromiscSniffer", 
                                                             file);
}

void 
WirelessPointToPointHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type 
  // WirelessPointToPointNetDevice.
  //
  Ptr<WirelessPointToPointNetDevice> device = 
    nd->GetObject<WirelessPointToPointNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("WirelessPointToPointHelper::EnableAsciiInternal(): Device " 
                   << device 
                   << " not of type ns3::WirelessPointToPointNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to 
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create 
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy 
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = 
        asciiTraceHelper.CreateFileStream (filename);

      //
      // The MacRx trace source provides our "r" event.
      //
      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<WirelessPointToPointNetDevice> (device, "MacRx", theStream);

      //
      // The "+", '-', and 'd' events are driven by trace sources actually in 
      // the transmit queue.
      //
      Ptr<Queue> queue = device->GetQueue ();
      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue> (queue, 
                                                                    "Enqueue", 
                                                                    theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue> (queue, "Drop",
                                                                 theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue> (queue, 
                                                                    "Dequeue", 
                                                                    theStream);

      // PhyRxDrop trace source for "d" event
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<WirelessPointToPointNetDevice> (device, "PhyRxDrop", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to providd a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for 
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the 
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static 
  // functions that are always there waiting for just such a case.
  //
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  oss << "/NodeList/" << nd->GetNode ()->GetId () << "/DeviceList/" << deviceid 
      << "/$ns3::WirelessPointToPointNetDevice/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid 
      << "/$ns3::WirelessPointToPointNetDevice/TxQueue/Enqueue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid 
      << "/$ns3::WirelessPointToPointNetDevice/TxQueue/Dequeue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid 
      << "/$ns3::WirelessPointToPointNetDevice/TxQueue/Drop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid 
      << "/$ns3::WirelessPointToPointNetDevice/PhyRxDrop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer 
WirelessPointToPointHelper::Install (NodeContainer c, int devicesPerNode)
{
  NetDeviceContainer container;

  Ptr<WirelessPointToPointChannel> channel = 
    m_channelFactory.Create<WirelessPointToPointChannel> (); 
  Ptr<PropagationDelayModel> delay = 
    m_propagationDelay.Create<PropagationDelayModel> ();
  channel->SetPropagationDelayModel (delay);

  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
      for(int j=0; j<devicesPerNode; j++)
        {
          Ptr<WirelessPointToPointNetDevice> device = 
            m_deviceFactory.Create<WirelessPointToPointNetDevice> ();
          device->SetAddress (Mac48Address::Allocate ());

          if (MpiInterface::IsEnabled())
           {
            Ptr<MpiReceiver> mpiRec = CreateObject<MpiReceiver>();
            mpiRec->SetReceiveCallback (MakeCallback(&WirelessPointToPointNetDevice::Receive, device));
            device->AggregateObject (mpiRec);
          }

          node->AddDevice (device);
          
          Ptr<Queue> queue = m_queueFactory.Create<Queue> ();
          device->SetQueue (queue);
          
          device->Attach (channel);
          container.Add (device);
        }
    }
  return container;
}

} // namespace ns3
