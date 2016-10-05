/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 * Author: Ben Newton (adpated from ppp-header.h)
 */

#ifndef WPPP_HEADER_H
#define WPPP_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup point-to-point
 * \brief Packet header for PPP
 *
 * This class can be used to add a header to PPP packet.  Currently we do not
 * implement any of the state machine in \RFC{1661}, we just encapsulate the
 * inbound packet send it on.  The goal here is not really to implement the
 * point-to-point protocol, but to encapsulate our packets in a known protocol
 * so packet sniffers can parse them.
 *
 * if PPP is transmitted over a serial link, it will typically be framed in
 * some way derivative of IBM SDLC (HDLC) with all that that entails.
 * Thankfully, we don't have to deal with all of that -- we can use our own
 * protocol for getting bits across the serial link which we call an ns3 
 * Packet.  What we do have to worry about is being able to capture PPP frames
 * which are understandable by Wireshark.  All this means is that we need to
 * teach the PcapWriter about the appropriate data link type (DLT_PPP = 9),
 * and we need to add a PPP header to each packet.  Since we are not using
 * framed PPP, this just means prepending the sixteen bit PPP protocol number
 * to the packet.  The ns-3 way to do this is via a class that inherits from
 * class Header.
 */
class WpppHeader : public Header
{
public:

  /**
   * \brief Construct a PPP header.
   */
  WpppHeader ();

  /**
   * \brief Destroy a PPP header.
   */
  virtual ~WpppHeader ();

  /**
   * \brief Get the TypeId
   *
   * \return The TypeId for this class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the TypeId of the instance
   *
   * \return The TypeId for this instance
   */
  virtual TypeId GetInstanceTypeId (void) const;


  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * \brief Set the protocol type carried by this PPP packet
   *
   * The type numbers to be used are defined in \RFC{3818}
   *
   * \param protocol the protocol type being carried
   */
  void SetProtocol (uint16_t protocol);

  /**
   * \brief Get the protocol type carried by this PPP packet
   *
   * The type numbers to be used are defined in \RFC{3818}
   *
   * \return the protocol type being carried
   */
  uint16_t GetProtocol (void);

  /**
   * Fill the Address 1 field with the given address.
   *
   * \param address the address to be used in the Address 1 field
   */
  void SetAddr1 (Mac48Address address);
  /**
   * Fill the Address 2 field with the given address.
   *
   * \param address the address to be used in the Address 2 field
   */
  void SetAddr2 (Mac48Address address);

  /**
   * Return the address in the Address 1 field.
   *
   * \return the address in the Address 1 field
   */
  Mac48Address GetAddr1 (void) const;
  /**
   * Return the address in the Address 2 field.
   *
   * \return the address in the Address 2 field
   */
  Mac48Address GetAddr2 (void) const;

private:

  /**
   * \brief The PPP protocol type of the payload packet
   */
  uint16_t m_protocol;
  Mac48Address m_addr1;
  Mac48Address m_addr2;
};

} // namespace ns3


#endif /* WPPP_HEADER_H */
