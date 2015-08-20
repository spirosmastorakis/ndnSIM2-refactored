/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "../tests-common.hpp"

#include <ndn-cxx/face.hpp>

#include "ns3/application.h"

namespace ns3 {
namespace ndn {

BOOST_FIXTURE_TEST_SUITE(NdncxxFace, ScenarioHelperWithCleanupFixture)

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
    // expressing an Interest
    m_face.expressInterest(interest,
                           [] (const Interest& interest, const Data& data) {
                             // do nothing when you receive the data
                           },
                           [] (const Interest& interest) {
                             // do nothing in case of timeout
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
                             std::bind([this] (const Interest& interest) {
                                 // Interest received
                                 auto data = make_shared<Data>(Name(interest.getName()).append("tadaaaaaa"));
                                 m_keyChain.sign(*data);
                                 m_face.put(*data);
                               }, _2),
                             std::bind([]{}));
  }

private:
  ::ndn::Face m_face;
  KeyChain& m_keyChain;
};

BOOST_AUTO_TEST_CASE(FaceCommunication)
{
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("10Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

    // Creating one 3 node topology: //
    //                               //
    //                 +----+        //
    //              +- | B1 |        //
    //             /   +----+        //
    //  +----+    /                  //
    //  |    | --+                   //
    //  | A1 |                       //
    //  |    | --+                   //
    //  +----+    \                  //
    //             \   +----+        //
    //              +- | C1 |        //
    //                 +----+        //

   createTopology({
       {"A1", "B1"},
       {"A1", "C1"}
     });

    addRoutes({
        {"B1", "A1", "/hello/world", 100},
        {"C1", "A1", "/hello/world", 100},
      });

   auto consumer1 = CreateObject<ApiApp<TrivialConsumer>>();
   getNode("B1")->AddApplication(consumer1);
   consumer1->SetStopTime(Seconds(15.0));

   auto consumer2 = CreateObject<ApiApp<TrivialConsumer>>();
   getNode("C1")->AddApplication(consumer2);
   consumer2->SetStopTime(Seconds(15.0));

   auto producer = CreateObject<ApiApp<TrivialProducer>>();
   getNode("A1")->AddApplication(producer);
   producer->SetStartTime(Seconds(0.5));

    Simulator::Stop(Seconds(20));
    Simulator::Run();

    BOOST_CHECK_EQUAL(getFace("A1", "B1")->getFaceStatus().getNInInterests(), 1);
    BOOST_CHECK_EQUAL(getFace("A1", "C1")->getFaceStatus().getNInInterests(), 1);

}

BOOST_AUTO_TEST_SUITE_END()

} // namespace ndn
} // namespace ns3
