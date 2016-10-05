/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 * Author: Ben Newton (adapted from point-to-point-helper.h)
 */
#ifndef WIRELESS_POINT_TO_POINT_HELPER_H
#define WIRELESS_POINT_TO_POINT_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"

#include "ns3/trace-helper.h"

namespace ns3 {

class Queue;
class NetDevice;
class Node;

/**
 * \brief Build a set of WirelessPointToPointNetDevice objects
 *
 * Normally we eschew multiple inheritance, however, the classes 
 * PcapUserHelperForDevice and AsciiTraceUserHelperForDevice are
 * "mixins".
 */
class WirelessPointToPointHelper : public PcapHelperForDevice,
                                   public AsciiTraceHelperForDevice
{
public:
  /**
   * Create a WirelessPointToPointHelper to make life easier when creating 
   * point to point networks.
   */
  WirelessPointToPointHelper ();
  virtual ~WirelessPointToPointHelper () {}

  /**
   * Each point to point net device must have a queue to pass packets through.
   * This method allows one to set the type of the queue that is automatically
   * created when the device is created and attached to a node.
   *
   * \param type the type of queue
   * \param n1 the name of the attribute to set on the queue
   * \param v1 the value of the attribute to set on the queue
   * \param n2 the name of the attribute to set on the queue
   * \param v2 the value of the attribute to set on the queue
   * \param n3 the name of the attribute to set on the queue
   * \param v3 the value of the attribute to set on the queue
   * \param n4 the name of the attribute to set on the queue
   * \param v4 the value of the attribute to set on the queue
   *
   * Set the type of queue to create and associated to each
   * WirelessPointToPointNetDevice created through
   * WirelessPointToPointHelper::Install.
   */
  void SetQueue (std::string type,
                 std::string n1 = "", 
                 const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", 
                 const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", 
                 const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", 
                 const AttributeValue &v4 = EmptyAttributeValue ());

  /**
   * \param name the name of the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * Configure a propagation delay for this channel.
   */
  void SetPropagationDelay (std::string name,
                            std::string n0 = "", 
                            const AttributeValue &v0 = EmptyAttributeValue (),
                            std::string n1 = "", 
                            const AttributeValue &v1 = EmptyAttributeValue (),
                            std::string n2 = "", 
                            const AttributeValue &v2 = EmptyAttributeValue (),
                            std::string n3 = "", 
                            const AttributeValue &v3 = EmptyAttributeValue (),
                            std::string n4 = "", 
                            const AttributeValue &v4 = EmptyAttributeValue (),
                            std::string n5 = "", 
                            const AttributeValue &v5 = EmptyAttributeValue (),
                            std::string n6 = "", 
                            const AttributeValue &v6 = EmptyAttributeValue (),
                            std::string n7 = "", 
                            const AttributeValue &v7 = EmptyAttributeValue ());

  /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::WirelessPointToPointNetDevice created
   * by WirelessPointToPointHelper::Install
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each Channel created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attribute on each ns3::WirelessPointToPointChannel created
   * by WirelessPointToPointHelper::Install
   */
  void SetChannelAttribute (std::string name, const AttributeValue &value);

  /**
   * \param c a set of nodes
   * \return a NetDeviceContainer for nodes
   *
   * This method creates a ns3::WirelessPointToPointChannel with the
   * attributes configured by WirelessPointToPointHelper::SetChannelAttribute,
   * then, for each node in the input container, we create a 
   * ns3::WirelessPointToPointNetDevice with the requested attributes, 
   * a queue for this ns3::NetDevice, and associate the resulting 
   * ns3::NetDevice with the ns3::Node and ns3::WirelessPointToPointChannel.
   */
  NetDeviceContainer Install (NodeContainer c, int devicesPerNode);

private:
  /**
   * \brief Enable pcap output the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the 
   *     device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, 
                                   bool promiscuous, bool explicitFilename);

  /**
   * \brief Enable ascii trace output on the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnableAsciiInternal (
    Ptr<OutputStreamWrapper> stream,
    std::string prefix,
    Ptr<NetDevice> nd,
    bool explicitFilename);

  ObjectFactory m_queueFactory;         //!< Queue Factory
  ObjectFactory m_channelFactory;       //!< Channel Factory
  ObjectFactory m_remoteChannelFactory; //!< Remote Channel Factory
  ObjectFactory m_deviceFactory;        //!< Device Factory
  ObjectFactory m_propagationDelay;     //!< Propagation Delay Factory
};

} // namespace ns3

#endif /* WIRELESS_POINT_TO_POINT_HELPER_H */
