/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{

  uint32_t nCsma = 3;

    //  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);


  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer p2pDevices = pointToPoint.Install (p2pNodes); 
  NetDeviceContainer csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);

OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address (InetSocketAddress (csmaInterfaces.GetAddress (2), 80)));
  clientHelper.SetAttribute("OnTime", RandomVariableValue (ConstantVariable (1)));
  clientHelper.SetAttribute("OffTime", RandomVariableValue (ConstantVariable (0)));
  
  ApplicationContainer clientAppsc;
  clientAppsc.Add (clientHelper.Install (p2pNodes.Get (0)));
  clientAppsc.Add (clientHelper.Install (p2pNodes.Get (0)));
  clientAppsc.Add (clientHelper.Install (p2pNodes.Get (0)));
  clientAppsc.Add (clientHelper.Install (p2pNodes.Get (0)));
  clientAppsc.Start (Seconds (1.0));
  clientAppsc.Stop (Seconds (1.5));

PacketSinkHelper sink ("ns3::TcpSocketFactory",
        InetSocketAddress (Ipv4Address::GetAny (), 80));
  
  ApplicationContainer sinkApps = sink.Install (csmaNodes.Get (3));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));
sinkApps = sink.Install (csmaNodes.Get (1));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));
sinkApps = sink.Install (csmaNodes.Get (2));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));

 UdpEchoServerHelper LBServer (9);
  ApplicationContainer serverAppsLB = LBServer.Install (csmaNodes.Get (0));
  serverAppsLB.Start (Seconds (0.0));
  serverAppsLB.Stop (Seconds (10.0));
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll ("second");
  csma.EnablePcap ("second", csmaDevices.Get (1), true);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
