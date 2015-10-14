#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/bridge-module.h"
#include <sstream>
#define enableLB 1

using namespace ns3;
using namespace std;

// CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
static void 
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
}

static void
TraceCwnd ()
{
  // Trace changes to the congestion window
  AsciiTraceHelper ascii;
for(int i = 0; i < 48; i++){
  stringstream filename;
  stringstream socketname;  
  filename << "lb-server-" << i << ".cwnd";
  socketname << "/NodeList/1/$ns3::TcpL4Protocol/SocketList/" << i << "/CongestionWindow";
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (filename.str());
  Config::ConnectWithoutContext ( socketname.str(), MakeBoundCallback (&CwndChange,stream));
}
}

int
main (int argc, char *argv[])
{
//  LogComponentEnable ("DropTailQueue", LOG_LEVEL_LOGIC);
    LogComponentEnable ("Ipv4Nat", LOG_LEVEL_WARN);
//  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
//  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("CsmaNetDevice",LOG_LEVEL_WARN);
//  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

  uint32_t queueSize = 100;
  std::string protocol = "TcpNewReno";
  // Set TCP defaults
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (0));
  //Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (524288));
  //Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (16384));
  //Config::SetDefault("ns3::TcpSocket::SlowStartThreshold", UintegerValue (10000));
  if (protocol == "TcpTahoe")
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
  else
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));

  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  
  NodeContainer ServerRack1;
  ServerRack1.Create (48);
  
  Ptr<Node> Aggregation = CreateObject<Node> ();
  Ptr<Node> TOR = CreateObject<Node> ();

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("4800Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));
  pointToPoint.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  pointToPoint.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(queueSize));
 
  CsmaHelper BetweenSwitch;
  BetweenSwitch.SetChannelAttribute ("DataRate", StringValue ("4800Mbps"));
  BetweenSwitch.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (60000)));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (60000)));

  NetDeviceContainer p2pDevices = pointToPoint.Install (p2pNodes);
  NetDeviceContainer ServerDevices;
  NetDeviceContainer TORDevices;
  NetDeviceContainer AggregationDevices;
  
  NetDeviceContainer link = BetweenSwitch.Install ( NodeContainer(p2pNodes.Get(0), TOR) );
  ServerDevices.Add (link.Get(0));
  TORDevices.Add (link.Get(1));
 // AggregationDevices.Add (link1.Get(1));
  /*
  NetDeviceContainer link2 = BetweenSwitch.Install ( NodeContainer(Aggregation, TOR) );
  AggregationDevices.Add (link2.Get(0));
  TORDevices.Add (link2.Get(1));
  */
  for(int i = 0; i < 48; i++){
  NetDeviceContainer link = csma.Install ( NodeContainer(ServerRack1.Get(i) ,TOR) );
	ServerDevices.Add (link.Get(0));
	TORDevices.Add (link.Get(1));
  }

  BridgeHelper bridge;
  bridge.Install (TOR, TORDevices);
  //bridge.Install (Aggregation, AggregationDevices);

  InternetStackHelper IPstack;
  IPstack.Install (ServerRack1);
  IPstack.Install (p2pNodes);

//
// Desired topology:
//    server1   2     3    4
//192.168.1.2  1.3   1.4  1.5
//       |      |     |    |
//       ==================== <----------- bridge ------------> n4 <---------------> n5
//                                                    192.168.1.1 203.82.48.1     203.82.48.2

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces = address.Assign (ServerDevices);

  address.SetBase ("203.82.48.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces = address.Assign (p2pDevices);

#ifdef enableLB
  Ipv4NatHelper natHelper;
  Ptr<Ipv4Nat> nat = natHelper.Install (p2pNodes.Get (0));

  nat->SetInside (1);
  nat->SetOutside (2);
  //adding NAT rules for all servers
  Ipv4Nat::addingNatRules(nat);
  // Now print them out
  Ptr<OutputStreamWrapper> natStream = Create<OutputStreamWrapper> ("nat.rules", std::ios::out);
  nat->PrintTable (natStream);
 
  BulkSendHelper clientHelper ("ns3::TcpSocketFactory", Address (InetSocketAddress (p2pInterfaces.GetAddress (0), 80)));
#else
  BulkSendHelper clientHelper ("ns3::TcpSocketFactory", Address (InetSocketAddress(csmaInterfaces.GetAddress (1), 80)));
#endif
  clientHelper.SetAttribute ("MaxBytes", UintegerValue (655350));
  // Set the segment size
  clientHelper.SetAttribute ("SendSize", UintegerValue (10000));  
  
  ApplicationContainer clientAppsc;
  for(int i = 0; i < 48; i++){
  clientAppsc.Add (clientHelper.Install (p2pNodes.Get (1)));
  }
  clientAppsc.Start (Seconds (1.0));
  clientAppsc.Stop (Seconds (20));
/*
  ApplicationContainer clientAppsb;
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientAppsb.Start (Seconds (1.0));
  clientAppsb.Stop (Seconds (20));
  
  ApplicationContainer clientApps1;
  for(int i = 0; i < 10; i++){
  clientApps1.Add (clientHelper.Install (p2pNodes.Get (1)));
  clientApps1.Add (clientHelper.Install (p2pNodes.Get (1)));
  }
  clientApps1.Start (Seconds (1.0));
  clientApps1.Stop (Seconds (20));

*/

  PacketSinkHelper sink ("ns3::TcpSocketFactory",
        InetSocketAddress (Ipv4Address::GetAny (), 80));
  
  ApplicationContainer sinkApps = sink.Install (ServerRack1.Get (0));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));
  for(int i = 1; i < 48; i++){
  sinkApps = sink.Install (ServerRack1.Get (i));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (20.0));
  }
 
  UdpEchoServerHelper LBServer (9);
  ApplicationContainer serverAppsLB = LBServer.Install (p2pNodes.Get (0));
  serverAppsLB.Start (Seconds (0.0));
  serverAppsLB.Stop (Seconds (20.0));
 
/*
  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
 
  echoClient.SetFill (clientApps.Get(0), "Hello world!");
*/
// Prepare to run the simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll ("lb-p2p", false);
  csma.EnablePcapAll("lb-lan",false);

  Simulator::Schedule(Seconds(1.00001),&TraceCwnd);

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  Simulator::Destroy ();
}
