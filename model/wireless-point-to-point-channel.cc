/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 * Author: Ben Newton (Adapted from point-to-point-channel.cc)
 */

#include "wireless-point-to-point-channel.h"
#include "wireless-point-to-point-net-device.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/mobility-model.h"

#include "ns3/mpi-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WirelessPointToPointChannel");

NS_OBJECT_ENSURE_REGISTERED (WirelessPointToPointChannel);

TypeId 
WirelessPointToPointChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WirelessPointToPointChannel")
    .SetParent<Channel> ()
    .SetGroupName ("WirelessPointToPoint")
    .AddConstructor<WirelessPointToPointChannel> ()
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay"
                   " model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&WirelessPointToPointChannel::m_delayModel),
                   MakePointerChecker<PropagationDelayModel> ())
    .AddAttribute ("Delay", "Propagation delay through the channel", //This is actually the minimum delay of any wp2p connection. distributed-simulator-impl uses it.  
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&WirelessPointToPointChannel::m_delay),
                   MakeTimeChecker ())
    /*.AddTraceSource ("TxRxWirelessPointToPoint",
                     "Trace source indicating transmission of packet "
                     "from the WirelessPointToPointChannel, used by the Animation "
                     "interface.",
                     MakeTraceSourceAccessor (&WirelessPointToPointChannel::m_txrxWirelessPointToPoint),
                     "ns3::WirelessPointToPointChannel::TxRxAnimationCallback")*/
  ;
  return tid;
}

//
// By default, you get a channel that 
// has an "infitely" fast transmission speed and zero delay.
WirelessPointToPointChannel::WirelessPointToPointChannel()
  :
    Channel ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
WirelessPointToPointChannel::Attach (Ptr<WirelessPointToPointNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT (device != 0);
  //m_nodes.insert(device->GetNode());
  m_deviceList.push_back(device); 
}

bool
WirelessPointToPointChannel::TransmitStart (
  Ptr<Packet> p,
  Ptr<WirelessPointToPointNetDevice> src,
  Time txTime)
{
  NS_LOG_FUNCTION (this <<" ifIndex=" << src->GetIfIndex()+1 << "src=" << src );
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  if(m_alignmentMap.find(src) != m_alignmentMap.end())
    {
      //get device currently aligned with 
      Ptr<WirelessPointToPointNetDevice> dst = m_alignmentMap[src]; 
      Ptr<MobilityModel> srcMob = src->GetNode()->GetObject<MobilityModel> ();
      Ptr<MobilityModel> dstMob = dst->GetNode()->GetObject<MobilityModel> ();
      Time delay = m_delayModel->GetDelay (srcMob, dstMob);
           
      uint32_t dstNode = dst->GetObject<NetDevice>()->GetNode()->GetId();
      uint32_t n1SystemId = 0;
      uint32_t n2SystemId = 0;

      if (MpiInterface::IsEnabled()){
        n1SystemId = src->GetNode()->GetSystemId();
        n2SystemId = dst->GetNode()->GetSystemId();
      }
      
      if (n1SystemId != n2SystemId)
      {
        #ifdef NS3_MPI
        Time rxTime = Simulator::Now () + txTime + delay;
        MpiInterface::SendPacket (p, rxTime, dst->GetNode()->GetId(), dst->GetIfIndex());
        #else
          NS_FATAL_ERROR("Can't use distributed simulator without MPI compiled in");
        #endif
      }
      else
      {
      
      //later remove txTime below?? don't remember why todo
      Simulator::ScheduleWithContext (dstNode,
                                      txTime + delay, 
                                      &WirelessPointToPointNetDevice::Receive,
                                      dst, p); 
      }
    }
  else
    {
      //not aligned with another terminal
      //drop??  should we report it dropped? todo
      return false;
    }
  return true;

  // Call the tx anim callback on the net device.  fix this todo?
  //m_txrxWirelessPointToPoint (p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
}

uint32_t 
WirelessPointToPointChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_deviceList.size();
}

Ptr<WirelessPointToPointNetDevice>
WirelessPointToPointChannel::GetWirelessPointToPointDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (i < m_deviceList.size());
  return m_deviceList[i];
}

Ptr<NetDevice>
WirelessPointToPointChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetWirelessPointToPointDevice (i);
}

void
WirelessPointToPointChannel::SetPropagationDelayModel(Ptr<PropagationDelayModel> delayModel) //is this used? todo
{
  m_delayModel = delayModel;
}
 
  //used by toplogy control to know which links to not include in the topology 
bool WirelessPointToPointChannel::IsOneWayConnection(long unsigned int nodeId1, long unsigned int nodeId2)
{
  typedef std::map<std::pair<Ptr<Node>, Ptr<WirelessPointToPointNetDevice> >, Ptr<Node> >::iterator iter;
  for(iter i=m_oneWayConnectionMap.begin(); i != m_oneWayConnectionMap.end(); i++)
    {
      if((i->first.first->GetId() == nodeId1 && i->second->GetId() == nodeId2) || 
         (i->first.first->GetId() == nodeId2 && i->second->GetId() == nodeId1) )  //check both directions
        {
          return true;
        }
    }
  return false;
}

void 
WirelessPointToPointChannel::Connect(Ptr<Node> localNode, 
                                     Ptr<WirelessPointToPointNetDevice> dev, 
                                     Ptr<Node> remoteNode) 
{
  NS_LOG_FUNCTION(this << localNode << dev << remoteNode);
  //this is where the simulation seconds could be displayed!!!
  //std::cout << Simulator::Now ().GetSeconds() << std::endl;
  //std::cout << "Connect " << localNode->GetId() << "<->" 
  //          << remoteNode->GetId() << std::endl;
  m_connectionMap[std::make_pair(localNode, dev)] = remoteNode;
  
  /////////////////////TODO/OPTIMIZE this , search only one way connections??
  //see if other side was already in connection map
  typedef std::map<std::pair<Ptr<Node>, Ptr<WirelessPointToPointNetDevice> >, 
                   Ptr<Node> >::iterator iter;
  for(iter i = m_connectionMap.begin(); i != m_connectionMap.end(); i++) 
    {
      if(i->first.first == remoteNode && i->second == localNode)
        {
          //other side of connection already exists, add to alignment map
          m_alignmentMap[i->first.second] = dev;
          m_alignmentMap[dev] = i->first.second;

          //update neighbors for pyvis
          //dev->GetObject<Neighbor>()->SetDevice((Ptr<NetDevice>)i->first.second);
          //i->first.second->GetObject<Neighbor>()->SetDevice((Ptr<NetDevice>)dev);
          //could do something to display half connected?? todo
          localNode->GetObject<Ipv4>()->GetRoutingProtocol()->
            NotifyInterfaceUp(localNode->GetObject<Ipv4>()->
                              GetInterfaceForDevice(dev));

          //can we just use RemoteNode below instead? 
          Ptr<Node> remNode = i->first.second->GetNode();
          remNode->GetObject<Ipv4>()->GetRoutingProtocol()->
            NotifyInterfaceUp(remNode->GetObject<Ipv4>()->
                              GetInterfaceForDevice(i->first.second));

          //above is not realistic, and will need to be re-implemented!!!!! 
          //todo ??   
          //std::cout << "now up fully" << localNode->GetId() << "<-->" << remoteNode->GetId() << std::endl;
          
          //remove the one way connection
          m_oneWayConnectionMap.erase(std::make_pair(remNode, i->first.second));
          return;
        }
    }
  //if not found this is a one way connection currently
  m_oneWayConnectionMap[std::make_pair(localNode, dev)] = remoteNode;
}

void 
WirelessPointToPointChannel::Disconnect(Ptr<Node> localNode,
                                        Ptr<WirelessPointToPointNetDevice> dev,
                                        Ptr<Node> remoteNode)
{
  //std::cout << "Disconnect " << localNode->GetId() << remoteNode->GetId() 
  //<< std::endl;
  m_connectionMap.erase(std::make_pair(localNode, dev));
  
  //see if the other side is still in connection map
  typedef std::map<std::pair<Ptr<Node>, Ptr<WirelessPointToPointNetDevice> >, 
                   Ptr<Node> >::iterator iter;
  for(iter i = m_connectionMap.begin(); i != m_connectionMap.end(); i++)
    {
      if(i->first.first == remoteNode && i->second == localNode)
        {
          //if other side of connection still exists, notify down.  
          m_alignmentMap.erase(dev);
          m_alignmentMap.erase(i->first.second);
          //update neighbors for PyVis
          //dev->GetObject<Neighbor>()->SetDevice(NULL);
          //i->first.second->GetObject<Neighbor>()->SetDevice(NULL);
          localNode->GetObject<Ipv4>()->GetRoutingProtocol()->
            NotifyInterfaceDown(localNode->GetObject<Ipv4>()->
                                GetInterfaceForDevice(dev));
          //can we just use remoteNode below? todo
          Ptr<Node> remNode = i->first.second->GetNode(); 
          remNode->GetObject<Ipv4>()->GetRoutingProtocol()->
            NotifyInterfaceDown(remNode->GetObject<Ipv4>()->
                                GetInterfaceForDevice(i->first.second));
          //above is not realistic, and will need to be re-implemented!!!!! 
          //todo ?? 
          
          m_oneWayConnectionMap[std::make_pair(remNode, i->first.second)] = 
            localNode;
          return;
        }
    }
  //if not found this is a one way connection currently, so erase.  
  m_oneWayConnectionMap.erase(std::make_pair(localNode, dev)); 
}

  //This is actually the minimum delay of any wp2p connection.  
  //the Delay attribute is obtained by distributed-simulator-impl
  //this method isn't needed 
  /*Time
  WirelessPointToPointChannel::GetDelay (void) const
  {
    return m_delay;  
    }*/

} // namespace ns3
