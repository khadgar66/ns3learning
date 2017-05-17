#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/aodv-helper.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>


using namespace ns3;

int packetsSent = 0;
int packetsReceived = 0;

void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv ()))
    {
	  packetsReceived++;
      std::cout<<"Received packet - "<<packetsReceived<<" and Size is "
    		  <<packet->GetSize ()<<" Bytes."<<std::endl;
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      packetsSent++;
      std::cout<<"Packet sent - "<<packetsSent<<std::endl;

      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

int main(int argc, char **argv)
{
  uint32_t size=6;
//  double step=100;
  double totalTime=100;

  int packetSize = 1024;
  int totalPackets = totalTime-1;
  double interval = 1.0;


  Time interPacketInterval = Seconds (interval);

  NodeContainer nodes;
  nodes.Create (size);
  Ns2MobilityHelper ns2 = Ns2MobilityHelper("scratch/map.map.tcl");
  ns2.Install();



  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);


  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");

  YansWifiChannelHelper wifiChannel ;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
                                  "SystemLoss", DoubleValue(1),
                                "HeightAboveZ", DoubleValue(1.5));

  wifiPhy.SetChannel (wifiChannel.Create ());


  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("OfdmRate6Mbps"),
                                "ControlMode", StringValue ("OfdmRate6Mbps"));

  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);


  //NetDeviceContainer devices, mal_devices;

  //AODV enable
  AodvHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (nodes);


  Ipv4AddressHelper address;
  NS_LOG_INFO("Set IP Address ");
  address.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont = address.Assign(devices);

/*
  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer mal_ifcont = address.Assign(mal_devices);

/*
  //UDP connection N0 -> N3
  uint16_t sinkPort = 6;
  Address sinkAddress (InetSocketAddress (ifcont.GetAddress (3), sinkPort));

  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory",
                                    InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (3)); //n3 as sink
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (100.));

  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (0),
                            dpSocketFactory::GetTypeId ()); //source at n0

  // Create UDP application at n0
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3UdpSocket, sinkAddress, 1040, 5, DataRate ("250Kbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (40.));
  app->SetStopTime (Seconds (100.));
*/
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (size-1), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8080);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (nodes.Get (0), tid);
  InetSocketAddress remote = InetSocketAddress (ifcont.GetAddress (size-1,0), 8080);
  source->Connect (remote);

  Simulator::Schedule (Seconds (1), &GenerateTraffic, source, packetSize, totalPackets, interPacketInterval);

/*
  // Set Mobility for all nodes
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
  positionAlloc ->Add(Vector(100, 0, 0)); // node0
  positionAlloc ->Add(Vector(200, 0, 0)); // node1
  positionAlloc ->Add(Vector(450, 0, 0)); // node2
  positionAlloc ->Add(Vector(550, 0, 0)); // node3
  positionAlloc ->Add(Vector(200, 10, 0)); // node4
  positionAlloc ->Add(Vector(450, 10, 0)); // node5

  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);


  AnimationInterface anim ("aodvattack.xml"); // Mandatory
  AnimationInterface::SetConstantPosition (nodes.Get (0), 0, 300);
  AnimationInterface::SetConstantPosition (nodes.Get (1), 200, 150);
  AnimationInterface::SetConstantPosition (nodes.Get (2), 400, 150);
  AnimationInterface::SetConstantPosition (nodes.Get (3), 600, 300);
  AnimationInterface::SetConstantPosition (nodes.Get (4), 200, 450);
  AnimationInterface::SetConstantPosition (nodes.Get (5), 400, 450);

  anim.EnablePacketMetadata(true);
*/


  // Calculate Throughput
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  //Run simulation
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();

  monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
  {
  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
  std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
  std::cout << " Tx Bytes: " << i->second.txBytes << "\n";
  std::cout << " Rx Bytes: " << i->second.rxBytes << "\n";
  std::cout << " Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i-
  >second.timeFirstTxPacket.GetSeconds())/1024 << " Kbps\n";
  }
  monitor->SerializeToXmlFile("scratch/newtrial.flowmon", true, true);
  Simulator::Destroy ();

}
