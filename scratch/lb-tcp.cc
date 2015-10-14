/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Sindhuja Venkatesh
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
 * Authors: Sindhuja Venkatesh <intutivestriker88@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NetfilterExample");
int
main (int argc, char *argv[])
{
//  LogComponentEnable ("DropTailQueue", LOG_LEVEL_LOGIC);
  LogComponentEnable ("Ipv4Nat", LOG_LEVEL_WARN);
//  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
//  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
//  LogComponentEnable ("Ipv4Nat", LOG_LEVEL_INFO);



  NS_LOG_INFO ("Create csmaNodes.");


  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  
  NodeContainer csmaNodes;
  csmaNodes.Add ( p2pNodes.Get(1));
  csmaNodes.Create (4);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
 
  NetDeviceContainer p2pDevices = pointToPoint.Install (p2pNodes); 
  NetDeviceContainer csmaDevices = csma.Install (csmaNodes);


  InternetStackHelper IPstack;
  IPstack.Install (p2pNodes.Get (0));
  IPstack.Install (csmaNodes);



//        private address    NAT      public address
// n0 <--------------------> n1 <-----------------------> n2
// 192.168.1.1   192.168.1.2    203.82.48.1  203.82.48.2
//
// Desired topology:
//       n0    n1   n2   n3
//192.168.1.1 1.2   1.3  1.4
//       |     |    |    |
//       ================= <-------------> n4 <---------------> n5
//                              192.168.1.5  203.82.48.1   203.82.48.2

 
 Ipv4AddressHelper address;

  address.SetBase ("203.82.48.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);

  

  Ipv4NatHelper natHelper;
  // The zeroth element of the p2pNodes node container is the NAT node
  Ptr<Ipv4Nat> nat = natHelper.Install (p2pNodes.Get (0));
  // Configure which of its Ipv4Interfaces are inside and outside interfaces
  // The zeroth Ipv4Interface is reserved for the loopback interface
  // Hence, the interface facing n0 is numbered "1" and the interface
  // facing n2 is numbered "2" (since it was assigned in the p2pNodes step above)
  nat->SetInside (1);
  nat->SetOutside (2);

  // Add a rule here to map outbound connections from n0, port 49153, UDP
 /* Ipv4StaticNatRule rule5 (Ipv4Address ("192.168.1.5"), 80, Ipv4Address ("203.82.48.1"), 80, IPPROTO_TCP);
  nat->AddStaticRule (rule5);
  Ipv4StaticNatRule rule4 (Ipv4Address ("192.168.1.4"), 80, Ipv4Address ("203.82.48.1"), 80, IPPROTO_TCP);
  nat->AddStaticRule (rule4);
  Ipv4StaticNatRule rule3 (Ipv4Address ("192.168.1.3"), 80, Ipv4Address ("203.82.48.1"), 80, IPPROTO_TCP);
  nat->AddStaticRule (rule3);
  Ipv4StaticNatRule rule2 (Ipv4Address ("192.168.1.2"), 80, Ipv4Address ("203.82.48.1"), 80, IPPROTO_TCP);
  nat->AddStaticRule (rule2);
 */Ipv4Nat::addingNatRules(nat);
  // Now print them out
  Ptr<OutputStreamWrapper> natStream = Create<OutputStreamWrapper> ("nat.rules", std::ios::out);
  nat->PrintTable (natStream);

OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address (InetSocketAddress (p2pInterfaces.GetAddress (0), 80)));
  clientHelper.SetAttribute("OnTime", RandomVariableValue (ConstantVariable (1)));
  clientHelper.SetAttribute("OffTime", RandomVariableValue (ConstantVariable (0)));

  ApplicationContainer clientApps;
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (2.5));
/*
  ApplicationContainer clientAppsc;
  clientAppsc.Add (clientHelper.Install (csmaNodes.Get (0)));
 clientAppsc.Add (clientHelper.Install (csmaNodes.Get (0)));
 clientAppsc.Add (clientHelper.Install (csmaNodes.Get (0)));
 clientAppsc.Add (clientHelper.Install (csmaNodes.Get (0)));

  clientAppsc.Start (Seconds (1.0));
  clientAppsc.Stop (Seconds (1.5));
*/


// Create a PacketSinkApplication and install it on node 1
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
        InetSocketAddress (Ipv4Address::GetAny (), 80));
  ApplicationContainer sinkApps = sink.Install (csmaNodes.Get (1));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));
  sinkApps = sink.Install (csmaNodes.Get (2));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));
  sinkApps = sink.Install (csmaNodes.Get (3));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));
  sinkApps = sink.Install (csmaNodes.Get (4));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));

 
  UdpEchoServerHelper LBServer (9);
  ApplicationContainer serverAppsLB = LBServer.Install (p2pNodes.Get (0));
  serverAppsLB.Start (Seconds (0.0));
  serverAppsLB.Stop (Seconds (10.0));
 
/*
  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
 
  echoClient.SetFill (clientApps.Get(0), "Hello world!");
*/
// Prepare to run the simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll ("ipv4-nat", false);
  csma.EnablePcapAll("ipv4-nat-lan",false);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
