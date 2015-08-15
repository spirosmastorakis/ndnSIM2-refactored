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

#ifndef NDN_REAL_APP_HELPER_H
#define NDN_REAL_APP_HELPER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/ptr.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-helpers
 * @brief A helper to make it easier to instantiate a real-word application
 *        using the ndn::Face on a set of nodes
 */
class RealAppHelper {
public:
  /**
   * \brief Create an NdnAppHelper to make it easier to work with Ndn apps
   *
   * \param app Class of the application
   */
  RealAppHelper(const std::string& prefix);

  /**
   * Install a real-world app on each node of the input container.
   *
   * \param c NodeContainer of the set of nodes on which a real-world app
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  void
  Install(NodeContainer c);

  /**
   * Install a real-world app on each node of the input container.
   *
   * \param node The node on which the app will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  void
  Install(Ptr<Node> node);

  /**
   * Install a real-world app on each node of the input container.
   *
   * \param nodeName The node on which the app will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  void
  Install(std::string nodeName);

private:

  std::string m_appName;

};


} // namespace ndn
} // namespace ns3

#endif // NDN_REAL_APP_HELPER_H
