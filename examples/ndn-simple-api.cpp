/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
// ndn-simple-api.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#include <ndn-cxx/face.hpp>

namespace ns3 {
namespace ndn {

template<class APP>
class ApiApp : public Application
{
protected:
  // inherited from Application base class.
  virtual void
  StartApplication()
  {
    m_impl.reset(new APP);
  }

  virtual void
  StopApplication()
  {
    m_impl.reset();
  }

private:
  std::unique_ptr<APP> m_impl;
};


class TrivialConsumer
{
public:
  TrivialConsumer()
  {
    Simulator::Schedule(Seconds(1), &TrivialConsumer::requestData, this);
    Simulator::Schedule(Seconds(10), &TrivialConsumer::requestData, this);
  }

private:
  void
  requestData()
  {
    Interest interest("/hello/world");
    std::cout << Simulator::Now().ToDouble(Time::S) << " Express Interest " << interest << std::endl;
    m_face.expressInterest(interest,
                           [] (const Interest& interest, const Data& data) {
                             std::cout << Simulator::Now().ToDouble(Time::S) << " GOT DATA: \n" << data << std::endl;
                           },
                           [] (const Interest& interest) {
                             std::cout << Simulator::Now().ToDouble(Time::S) << " Interest timed out" << std::endl;
                           });
  }

private:
  ::ndn::Face m_face;
};

class TrivialProducer
{
public:
  TrivialProducer()
    : m_keyChain(StackHelper::getKeyChain())
  {
    m_face.setInterestFilter("/hello",
                             bind([this] (const Interest& interest) {
                                 std::cout << Simulator::Now().ToDouble(Time::S) << " got interest, returning data" << std::endl;
                                 auto data = make_shared<Data>(Name(interest.getName()).append("tadaaaaaa"));
                                 m_keyChain.sign(*data);
                                 m_face.put(*data);
                               }, _2),
                             bind([]{}));
  }

private:
  ::ndn::Face m_face;
  KeyChain& m_keyChain;
};

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.ApiApp:ndn.Producer ./waf --run=ndn-simple-api
 */

int
main(int argc, char *argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(3);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));

  // Install NDN stack on all nodes
  StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Installing applications

  // Consumer
  auto consumer = CreateObject<ApiApp<TrivialConsumer>>();
  nodes.Get(0)->AddApplication(consumer);
  consumer->SetStopTime(Seconds(15.0));

  auto producer = CreateObject<ApiApp<TrivialProducer>>();
  nodes.Get(2)->AddApplication(producer);
  producer->SetStartTime(Seconds(0.5));

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ndn
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::ndn::main(argc, argv);
}
