/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 * Author: Ben Newton (adapted from ppp-header.cc)
 */

#include <iostream>
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "wppp-header.h"

#include "ns3/address-utils.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WpppHeader");

NS_OBJECT_ENSURE_REGISTERED (WpppHeader);

WpppHeader::WpppHeader ()
{
}

WpppHeader::~WpppHeader ()
{
}

TypeId
WpppHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WpppHeader")
    .SetParent<Header> ()
    .SetGroupName ("PointToPoint")
    .AddConstructor<WpppHeader> ()
  ;
  return tid;
}

TypeId
WpppHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
WpppHeader::Print (std::ostream &os) const
{
  std::string proto;

  switch(m_protocol)
    {
    case 0x0021: /* IPv4 */
      proto = "IP (0x0021)";
      break;
    case 0x0057: /* IPv6 */
      proto = "IPv6 (0x0057)";
      break;
    default:
      NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
    }
  os << "Point-to-Point Protocol: " << proto; 
}

uint32_t
WpppHeader::GetSerializedSize (void) const
{
  return 14; //2+6+6;
}

void
WpppHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (m_protocol);
  WriteTo (start, m_addr1);
  WriteTo (start, m_addr2);
}

uint32_t
WpppHeader::Deserialize (Buffer::Iterator start)
{
  m_protocol = start.ReadNtohU16 ();
  ReadFrom (start, m_addr1);
  ReadFrom (start, m_addr2);
  return GetSerializedSize ();
}

void
WpppHeader::SetProtocol (uint16_t protocol)
{
  m_protocol=protocol;
}

uint16_t
WpppHeader::GetProtocol (void)
{
  return m_protocol;
}

void
WpppHeader::SetAddr1 (Mac48Address address)
{
  m_addr1 = address;
}
void
WpppHeader::SetAddr2 (Mac48Address address)
{
  m_addr2 = address;
}

Mac48Address
WpppHeader::GetAddr1 (void) const
{
  return m_addr1;
}
Mac48Address
WpppHeader::GetAddr2 (void) const
{
  return m_addr2;
}

} // namespace ns3
