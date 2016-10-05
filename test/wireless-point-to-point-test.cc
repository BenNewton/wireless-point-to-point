/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of North Carolina at Chapel Hill
 * Author: Ben Newton (adapted from point-to-point-test.cc)
 */

#include "ns3/test.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/simulator.h"
#include "ns3/wireless-point-to-point-net-device.h"
#include "ns3/wireless-point-to-point-channel.h"

using namespace ns3;

/**
 * \brief Test class for WirelessPointToPoint model
 *
 * It tries to send one packet from one NetDevice to another, over a
 * WirelessPointToPointChannel.
 */
class WirelessPointToPointTest : public TestCase
{
public:
  /**
   * \brief Create the test
   */
  WirelessPointToPointTest ();

  /**
   * \brief Run the test
   */
  virtual void DoRun (void);

private:
  /**
   * \brief Send one packet to the device specified
   *
   * \param device NetDevice to send to
   */
  void SendOnePacket (Ptr<WirelessPointToPointNetDevice> device);
};

WirelessPointToPointTest::WirelessPointToPointTest ()
  : TestCase ("WirelessPointToPoint")
{
}

void
WirelessPointToPointTest::SendOnePacket (Ptr<WirelessPointToPointNetDevice> device)
{
  Ptr<Packet> p = Create<Packet> ();
  device->Send (p, device->GetBroadcast (), 0x800);
}


void
WirelessPointToPointTest::DoRun (void)
{
  Ptr<Node> a = CreateObject<Node> ();
  Ptr<Node> b = CreateObject<Node> ();
  Ptr<WirelessPointToPointNetDevice> devA = 
    CreateObject<WirelessPointToPointNetDevice> ();
  Ptr<WirelessPointToPointNetDevice> devB = 
    CreateObject<WirelessPointToPointNetDevice> ();
  Ptr<WirelessPointToPointChannel> channel = 
    CreateObject<WirelessPointToPointChannel> ();

  devA->Attach (channel);
  devA->SetAddress (Mac48Address::Allocate ());
  devA->SetQueue (CreateObject<DropTailQueue> ());
  devB->Attach (channel);
  devB->SetAddress (Mac48Address::Allocate ());
  devB->SetQueue (CreateObject<DropTailQueue> ());

  a->AddDevice (devA);
  b->AddDevice (devB);

  Ptr<NetDeviceQueueInterface> ifaceA = CreateObject<NetDeviceQueueInterface>();
  devA->AggregateObject (ifaceA);
  ifaceA->CreateTxQueues ();
  Ptr<NetDeviceQueueInterface> ifaceB = CreateObject<NetDeviceQueueInterface>();
  devB->AggregateObject (ifaceB);
  ifaceB->CreateTxQueues ();

  Simulator::Schedule (Seconds (1.0), &WirelessPointToPointTest::SendOnePacket, 
                       this, devA);

  Simulator::Run ();

  Simulator::Destroy ();
}

/**
 * \brief TestSuite for WirelessPointToPoint module
 */
class WirelessPointToPointTestSuite : public TestSuite
{
public:
  /**
   * \brief Constructor
   */
  WirelessPointToPointTestSuite ();
};

WirelessPointToPointTestSuite::WirelessPointToPointTestSuite ()
  : TestSuite ("wireless-point-to-point", UNIT)
{
  AddTestCase (new WirelessPointToPointTest, TestCase::QUICK);
}

static WirelessPointToPointTestSuite g_pointToPointTestSuite; //!< The testsuite
