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

#include "ndn-real-app-helper.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE("ndn.RealAppHelper");

namespace ns3 {
namespace ndn {

RealAppHelper::RealAppHelper(const std::string& app)
{
  NS_LOG_FUNCTION(this << app);
  m_appName = app;
}

void
RealAppHelper::Install(Ptr<Node> node)
{
  NS_LOG_INFO("Installing application " << m_appName << " on node " << node);
  if (!m_appName.empty()) {
    //std::system("pwd");
    std::system((std::string("./src/ndnSIM/real-apps/") + m_appName).c_str());
    NS_LOG_INFO("Application " << m_appName << " was installed on node " << node);
  }
}

void
RealAppHelper::Install(std::string nodeName)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return Install(node);
}

void
RealAppHelper::Install(NodeContainer c)
{
  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
    if (!m_appName.empty()) {
      NS_LOG_INFO("Installing application " << m_appName << " on node " << *i);
      std::system((std::string("./") + m_appName).c_str());
      NS_LOG_INFO("Application " << m_appName << " was installed on node " << *i);
    }
  }
}

} // namespace ndn
} // namespace ns3
