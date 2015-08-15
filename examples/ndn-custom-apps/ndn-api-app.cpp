/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
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

#include "ndn-api-app.hpp"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"

NS_LOG_COMPONENT_DEFINE("ndn.ApiApp");

namespace ns3 {
namespace ndn {

// Necessary if you are planning to use ndn::AppHelper
NS_OBJECT_ENSURE_REGISTERED(ApiApp);

TypeId
ApiApp::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::ApiApp")
    .SetParent<Application>()
    .AddConstructor<ApiApp>()

    .AddAttribute("Prefix","Name of the Interest",
                  StringValue("/"),
                  MakeNameAccessor (&ApiApp::m_name),
                  MakeNameChecker())
    .AddAttribute("LifeTime", "LifeTime for interest packet",
                  StringValue("2s"),
                  MakeTimeAccessor(&ApiApp::m_interestLifetime),
                  MakeTimeChecker())
    ;

  return tid;
}

ApiApp::ApiApp()
  : m_face()
{
}

void
ApiApp::RequestData()
{
  NS_LOG_FUNCTION(this);

  Interest interest = Interest();
  interest.setName(m_name);
  time::milliseconds interestLifeTime(m_interestLifetime.GetMilliSeconds());
  interest.setInterestLifetime(interestLifeTime);

  shared_ptr<Exclude> exclude = make_shared<Exclude>();
  exclude->excludeOne(name::Component("unique"));
  interest.setExclude(*exclude);

  m_face->expressInterest(interest,
                          bind(&ApiApp::onData, this,  _1, _2),
                          bind(&ApiApp::onTimeout, this,  _1));

}

void
ApiApp::onData(const Interest origInterest, const Data data)
{
  NS_LOG_FUNCTION(this << origInterest.getName() << data.getName());
  // do nothing else
}

void
ApiApp::onTimeout(const Interest interest)
{
  NS_LOG_FUNCTION(this << "Timeout " << interest.getName());
  // do nothing else
}

void
ApiApp::StartApplication()
{
  NS_LOG_FUNCTION(this);
  m_face = make_shared<::ndn::Face>();
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  node->GetObject<ns3::ndn::L3Protocol>()->addFace(m_face);

  Simulator::Schedule(Seconds(1), &::ns3::ndn::ApiApp::RequestData, this);
  Simulator::Schedule(Seconds(10), &::ns3::ndn::ApiApp::RequestData, this);
}

void
ApiApp::StopApplication()
{
  NS_LOG_FUNCTION(this);
  m_face->close();
}

} // namespace ndn
} // namespace ns3
