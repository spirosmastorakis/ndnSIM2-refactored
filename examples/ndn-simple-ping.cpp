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
// ndn-simple-ping.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#include "ns3/ndnSIM/ndn-tools/tools/ping/client/ping.hpp"
#include "ns3/ndnSIM/ndn-tools/tools/ping/server/ping-server.hpp"
#include "ns3/ndnSIM/ndn-tools/tools/ping/client/statistics-collector.hpp"
#include "ns3/ndnSIM/ndn-tools/tools/ping/client/tracer.hpp"
#include "ns3/ndnSIM/ndn-tools/tools/ping/server/tracer.hpp"

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

class PingClient
{
public:
  PingClient()
  {
    ::ndn::ping::client::Options options;
    options.prefix = Name("ndn:/edu/arizona");
    options.shouldAllowStaleData = false;
    options.nPings = 2;
    options.interval = time::milliseconds(1000);
    options.timeout = time::milliseconds(4000);
    options.startSeq = 0;
    options.shouldGenerateRandomSeq = false;
    options.shouldPrintTimestamp = false;

    Simulator::Schedule(Seconds(2), &PingClient::createAndStart, this, options);
  }

  /**
   * @brief Create and start the ping client (after RIB manager initialization)
   **/
  void
  createAndStart(::ndn::ping::client::Options& options)
  {
    shared_ptr<::ndn::ping::client::Ping> pingClient =
      make_shared<::ndn::ping::client::Ping>(m_face, options);

    shared_ptr<::ndn::ping::client::StatisticsCollector> statisticsCollector =
        make_shared<::ndn::ping::client::StatisticsCollector>(*pingClient, options);

    shared_ptr<::ndn::ping::client::Tracer> tracer = make_shared<::ndn::ping::client::Tracer>(*pingClient, options);

    pingClient->afterFinish.connect(bind(&PingClient::printStats, this, statisticsCollector));
    pingClient->start();
  }

  /**
   * @brief Print statistics at the end of the ping message exchange
   */
  void
  printStats(shared_ptr<::ndn::ping::client::StatisticsCollector> statisticsCollector)
  {
    statisticsCollector->computeStatistics().printSummary(std::cout);
  }

private:
  ::ndn::Face m_face;
};

class PingServer
{
public:
  PingServer()
  {
    ::ndn::ping::server::Options options;
    options.prefix = Name("ndn:/edu/arizona");
    options.freshnessPeriod = time::milliseconds(1000);
    options.shouldLimitSatisfied = false;
    options.nMaxPings = 2;
    options.shouldPrintTimestamp = false;
    options.payloadSize = 100;

    Simulator::Schedule(Seconds(1), &PingServer::createAndStart, this, options);
  }

  /**
   * @brief Create and start the ping server (after RIB manager initialization)
   **/
  void
  createAndStart(::ndn::ping::server::Options& options)
  {
    shared_ptr<::ndn::ping::server::PingServer> pingServer =
      make_shared<::ndn::ping::server::PingServer>(m_face, options);
    shared_ptr<::ndn::ping::server::Tracer> tracer = make_shared<::ndn::ping::server::Tracer>(*pingServer, options);
    pingServer->afterReceive.connect(bind(&PingServer::pingReceived, this, pingServer, options));
    pingServer->start();
  }

  /**
   * @brief Print statistics for every received ping from the client
   */
  void
  pingReceived(shared_ptr<::ndn::ping::server::PingServer> pingServer, ::ndn::ping::server::Options& options)
  {
    std::cout << "\n--- ping server " << options.prefix << " ---" << std::endl;
    std::cout << "--- ping received! ---" << std::endl;
    std::cout << pingServer->getNPings() << " packets processed" << std::endl;
  }

private:
  ::ndn::Face m_face;
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
  auto consumer = CreateObject<ApiApp<PingClient>>();
  nodes.Get(0)->AddApplication(consumer);
  // consumer->SetStopTime(Seconds(15.0));

  auto producer = CreateObject<ApiApp<PingServer>>();
  nodes.Get(2)->AddApplication(producer);
  // producer->SetStartTime(Seconds(0.5));

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
