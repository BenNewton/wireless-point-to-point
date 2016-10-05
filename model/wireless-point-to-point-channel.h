/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 *
 * Adapted from point-to-point-channel
 */

#ifndef WIRELESS_POINT_TO_POINT_CHANNEL_H
#define WIRELESS_POINT_TO_POINT_CHANNEL_H

#include <list>
#include "ns3/channel.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

#include "ns3/pointer.h"
#include <map>
#include "ns3/propagation-delay-model.h" 
#include "ns3/node.h"

namespace ns3 {

class WirelessPointToPointNetDevice;
class Packet;

/**
 * \ingroup wireless-point-to-point
 * \brief Wireless Point To Point Channel.
 *
 * TODO add details here
 *
 * \see Attach
 * \see TransmitStart
 */
class WirelessPointToPointChannel : public Channel 
{
public:
  /**
   * \brief Get the TypeId
   *
   * \return The TypeId for this class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Create a WirelessPointToPointChannel
   *
   * By default, you get a channel that has an "infinitely" fast 
   * transmission speed and zero delay.
   */
  WirelessPointToPointChannel ();

  /**
   * \brief Attach a given netdevice to this channel
   * \param device pointer to the netdevice to attach to the channel
   */
  void Attach (Ptr<WirelessPointToPointNetDevice> device);

  /**
   * \brief Transmit a packet over this channel
   * \param p Packet to transmit
   * \param src Source WirelessPointToPointNetDevice
   * \param txTime Transmit time to apply
   * \returns true if successful (currently always true)
   */
  virtual bool TransmitStart (Ptr<Packet> p, Ptr<WirelessPointToPointNetDevice> src, Time txTime);

  /**
   * \brief Get number of devices on this channel
   * \returns number of devices on this channel
   */
  virtual uint32_t GetNDevices (void) const;

  /**
   * \brief Get WirelessPointToPointNetDevice corresponding to index i on this channel
   * \param i Index number of the device requested
   * \returns Ptr to WirelessPointToPointNetDevice requested
   */
  Ptr<WirelessPointToPointNetDevice> GetWirelessPointToPointDevice (uint32_t i) const;

  /**
   * \brief Get NetDevice corresponding to index i on this channel
   * \param i Index number of the device requested
   * \returns Ptr to NetDevice requested
   */
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

  /**
   * \brief Set the propagation delay model
   * \param delay the new propagation delay model.
   */
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);

  void Connect(Ptr<Node> localNode, Ptr<WirelessPointToPointNetDevice> dev, Ptr<Node> remoteNode);
  void Disconnect(Ptr<Node> localNode, Ptr<WirelessPointToPointNetDevice> dev, Ptr<Node> remoteNode);
  bool IsOneWayConnection(long unsigned int nodeId1, long unsigned int nodeId2);
protected:
  /**
   * \brief Get the delay associated with this channel
   * \returns Time delay
   */
  //Time GetDelay (void) const;

  /**
   * \brief Check to make sure the link is initialized
   * \returns true if initialized, asserts otherwise
   */
  bool IsInitialized (void) const;

  /**
   * TracedCallback signature for packet transmission animation events.
   *
   * \param [in] packet The packet being transmitted.
   * \param [in] txDevice the TransmitTing NetDevice.
   * \param [in] rxDevice the Receiving NetDevice.
   * \param [in] duration The amount of time to transmit the packet.
   * \param [in] lastBitTime Last bit receive time (relative to now)
   * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
   * and will be changed to \c Ptr<const NetDevice> in a future release.
   */
  typedef void (* TxRxAnimationCallback)
    (Ptr<const Packet> packet,
     Ptr<NetDevice> txDevice, Ptr<NetDevice> rxDevice,
     Time duration, Time lastBitTime);
    
  //for now assume only two interfaces can be alligned.  May need to do different later to consider interference  todo ??
  Ptr<PropagationDelayModel> m_delayModel;
  std::map<Ptr<WirelessPointToPointNetDevice>, Ptr<WirelessPointToPointNetDevice> > m_alignmentMap; 
                    
private:

  /**
   * The trace source for the packet transmission animation events that the 
   * device can fire.
   * Arguments to the callback are the packet, transmitting
   * net device, receiving net device, transmission time and 
   * packet receipt time.
   *
   * \see class CallBackTraceSource
   * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
   * and will be changed to \c Ptr<const NetDevice> in a future release.
   */
  /*TracedCallback<Ptr<const Packet>,     // Packet being transmitted
                 Ptr<NetDevice>,  // Transmitting NetDevice
                 Ptr<NetDevice>,  // Receiving NetDevice
                 Time,                  // Amount of time to transmit the pkt
                 Time                   // Last bit receive time (relative to now)
                 > m_txrxWirelessPointToPoint;*/

  std::vector<Ptr<WirelessPointToPointNetDevice> > m_deviceList;

  std::map<std::pair<Ptr<Node>, Ptr<WirelessPointToPointNetDevice> >, Ptr<Node> > m_connectionMap; //for tracking  //overcomes potential issues with alignment processing when using distributed alg.  
  std::map<std::pair<Ptr<Node>, Ptr<WirelessPointToPointNetDevice> >, Ptr<Node> > m_oneWayConnectionMap; 

  Time          m_delay;    //!< Propagation delay  
};

} // namespace ns3

#endif /* WIRELESS_POINT_TO_POINT_CHANNEL_H */
