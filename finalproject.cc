// /*
//  * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
//  *
//  * This program is free software; you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License version 2 as
//  * published by the Free Software Foundation;
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program; if not, write to the Free Software
//  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  *
//  * Authors: Justin P. Rohrer, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>, Siddharth Gangadhar
//  * <siddharth@ittc.ku.edu>
//  *
//  * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
//  * ResiliNets Research Group  https://resilinets.org/
//  * Information and Telecommunication Technology Center (ITTC)
//  * and Department of Electrical Engineering and Computer Science
//  * The University of Kansas Lawrence, KS USA.
//  *
//  * Work supported in part by NSF FIND (Future Internet Design) Program
//  * under grant CNS-0626918 (Postmodern Internet Architecture),
//  * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
//  * US Department of Defense (DoD), and ITTC at The University of Kansas.
//  *
//  * "TCP Westwood(+) Protocol Implementation in ns-3"
//  * Siddharth Gangadhar, Trúc Anh Ngọc Nguyễn , Greeshma Umapathi, and James P.G. Sterbenz,
//  * ICST SIMUTools Workshop on ns-3 (WNS3), Cannes, France, March 2013
//  */

// #include "ns3/applications-module.h"
// #include "ns3/core-module.h"
// #include "ns3/enum.h"
// #include "ns3/error-model.h"
// #include "ns3/event-id.h"
// #include "ns3/flow-monitor-helper.h"
// #include "ns3/internet-module.h"
// #include "ns3/ipv4-global-routing-helper.h"
// #include "ns3/network-module.h"
// #include "ns3/point-to-point-module.h"
// #include "ns3/tcp-header.h"
// #include "ns3/traffic-control-module.h"
// #include "ns3/udp-header.h"

// #include <fstream>
// #include <iostream>
// #include <string>

// using namespace ns3;

// NS_LOG_COMPONENT_DEFINE("TcpVariantsComparison");

// static std::map<uint32_t, bool> firstCwnd;                      //!< First congestion window.
// static std::map<uint32_t, bool> firstSshThr;                    //!< First SlowStart threshold.
// static std::map<uint32_t, bool> firstRtt;                       //!< First RTT.
// static std::map<uint32_t, bool> firstRto;                       //!< First RTO.
// static std::map<uint32_t, Ptr<OutputStreamWrapper>> cWndStream; //!< Congstion window output stream.
// static std::map<uint32_t, Ptr<OutputStreamWrapper>>
//     ssThreshStream; //!< SlowStart threshold output stream.
// static std::map<uint32_t, Ptr<OutputStreamWrapper>> rttStream;      //!< RTT output stream.
// static std::map<uint32_t, Ptr<OutputStreamWrapper>> rtoStream;      //!< RTO output stream.
// static std::map<uint32_t, Ptr<OutputStreamWrapper>> nextTxStream;   //!< Next TX output stream.
// static std::map<uint32_t, Ptr<OutputStreamWrapper>> nextRxStream;   //!< Next RX output stream.
// static std::map<uint32_t, Ptr<OutputStreamWrapper>> inFlightStream; //!< In flight output stream.
// static std::map<uint32_t, uint32_t> cWndValue;                      //!< congestion window value.
// static std::map<uint32_t, uint32_t> ssThreshValue;                  //!< SlowStart threshold value.
// std::vector<double> flowStarts;

// /**
//  * Get the Node Id From Context.
//  *
//  * \param context The context.
//  * \return the node ID.
//  */
// /* static uint32_t
// GetNodeIdFromContext(std::string context)
// {
//     const std::size_t n1 = context.find_first_of('/', 1);
//     const std::size_t n2 = context.find_first_of('/', n1 + 1);
//     return std::stoul(context.substr(n1 + 1, n2 - n1 - 1));
// } */

// /**
//  * Congestion window tracer.
//  *
//  * \param context The context.
//  * \param oldval Old value.
//  * \param newval New value.
//  */
// static void
// CwndTracer(uint32_t oldval, uint32_t newval)
// {
//     uint32_t nodeId = 0;//GetNodeIdFromContext(context);
//     //std::cout << "TRACE CALLED\n";
    
//     if (firstCwnd[nodeId])
//     {
//         *cWndStream[nodeId]->GetStream() << "0.0\t" << oldval << std::endl;
//         firstCwnd[nodeId] = false;
//     }
//     *cWndStream[nodeId]->GetStream() << Simulator::Now().GetSeconds() << " " << newval << std::endl;
//     cWndValue[nodeId] = newval;

//     if (!firstSshThr[nodeId])
//     {
//         *ssThreshStream[nodeId]->GetStream()
//             << Simulator::Now().GetSeconds() << "\t" << ssThreshValue[nodeId] << std::endl;
//     }
// }

// /**
//  * Congestion window trace connection.
//  *
//  * \param cwnd_tr_file_name Congestion window trace file name.
//  * \param nodeId Node ID.
//  */
// static void
// TraceCwnd(std::string cwnd_tr_file_name, uint32_t nodeId)
// {
//     AsciiTraceHelper ascii;
//     //std::cout << "TRACE STARTED\n";
//     cWndStream[nodeId] = ascii.CreateFileStream(cwnd_tr_file_name);
//     Config::ConnectWithoutContext("/NodeList/" + std::to_string(nodeId) +
//                         "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
//                     MakeCallback(&CwndTracer));
// }
// static void getRx(ApplicationContainer packetSink, uint32_t index)
// {
//     //Taken from examples/routing/simple-multicast-flooding.cc

//     //THIS SHOULD ALWAYS LOOP TWICE
//     for (auto end = packetSink.End(), iter = packetSink.Begin(); iter != end; ++iter)
//     {
//         auto node = (*iter)->GetNode();
//         auto sink = (*iter)->GetObject<PacketSink>();
//         std::cout << "Flow " << index + 1 << " at node " << Names::FindName(node) << " received " << sink->GetTotalRx()
//                   << " bytes; goodput was "<< static_cast<double>(sink->GetTotalRx()) / (20.0 - flowStarts[index]) << std::endl;
//     }
// }
// int
// main(int argc, char* argv[])
// {    
//     std::string transport_prot = "TcpCubic";
//     double errorRate = 0.00001;
//     std::string dataRate = "1Mbps";
//     std::string delay = "0.01ms";
    
//     bool tracing = true;
//     std::string prefix_file_name = "TcpVariantsComparison";
//     uint16_t nFlows = 2;
//     bool flow_monitor = false;
//     bool pcap = false;

//     CommandLine cmd(__FILE__);
//     cmd.AddValue("transport_prot", "Transport protocol to use: TcpNewReno, TcpCubic",transport_prot);
//     cmd.AddValue("errorRate", "Packet error rate", errorRate);
//     cmd.AddValue("dataRate", "Bottleneck bandwidth", dataRate);
//     cmd.AddValue("delay", "Bottleneck delay", delay);
//     cmd.AddValue("tracing", "Flag to enable/disable tracing", tracing);
//     cmd.AddValue("nFlows", "Number of flows", nFlows);
//     cmd.AddValue("flow_monitor", "Enable flow monitor", flow_monitor);
//     cmd.AddValue("pcap", "Enable or disable PCAP tracing", pcap);
//     cmd.Parse(argc, argv);


//     transport_prot = std::string("ns3::") + transport_prot;

//     SeedManager::SetSeed(123456789);

//     //LogComponentEnable("TcpVariantsComparison", LOG_LEVEL_ALL);

//     TypeId tcpTid;
//     NS_ABORT_MSG_UNLESS(TypeId::LookupByNameFailSafe(transport_prot, &tcpTid),
//                         "TypeId " << transport_prot << " not found");
//     Config::SetDefault("ns3::TcpL4Protocol::SocketType",
//                        TypeIdValue(TypeId::LookupByName(transport_prot)));

//     // Create gateways, sources, and sinks
//     NodeContainer sources;
//     sources.Create(1);

//     NodeContainer gateways;
//     gateways.Create(6);
    
//     NodeContainer sinks;    //destination 
//     sinks.Create(1);
    
//     Names::Add("source1", sources.Get(0));
//     Names::Add("gateway11", gateways.Get(0));
//     Names::Add("gateway12", gateways.Get(1));
//     Names::Add("gateway21", gateways.Get(2));
//     Names::Add("gateway22", gateways.Get(3));
//     Names::Add("gateway31", gateways.Get(4));
//     Names::Add("gateway32", gateways.Get(5));
//     Names::Add("sink1", sinks.Get(0));

//     // Configure the error model
//     // Here we use RateErrorModel with packet error rate
//     Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
//     RateErrorModel error_model;
//     error_model.SetRandomVariable(uv);
//     error_model.SetUnit(RateErrorModel::ERROR_UNIT_BYTE);
//     error_model.SetRate(errorRate);

//     PointToPointHelper UnReLink;
//     UnReLink.SetDeviceAttribute("DataRate", StringValue(dataRate));
//     UnReLink.SetChannelAttribute("Delay", StringValue(delay));
//     UnReLink.SetDeviceAttribute("ReceiveErrorModel", PointerValue(&error_model));

//     InternetStackHelper stack;
//     stack.InstallAll();

//     Ipv4AddressHelper address;
//     address.SetBase("10.0.0.0", "255.255.255.0");

//     // Configure the sources and sinks net devices
//     // and the channels between the sources/sinks and the gateways
//     PointToPointHelper LocalLink;
//     LocalLink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
//     LocalLink.SetChannelAttribute("Delay", StringValue("0.01ms"));

//     PointToPointHelper RemoteLink;
//     RemoteLink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
//     RemoteLink.SetChannelAttribute("Delay", StringValue("0.01ms"));

//     Ipv4InterfaceContainer sink_interfaces; 
//     Ipv4InterfaceContainer source_interfaces;
//     Ipv4InterfaceContainer gate_interfaces;

//     Ipv4InterfaceContainer interfaces;
//     NetDeviceContainer devices;

//     devices = LocalLink.Install(sources.Get(0), gateways.Get(0));
//     interfaces = address.Assign(devices);
//     source_interfaces.Add(interfaces.Get(0));
//     address.NewNetwork();

//     devices = LocalLink.Install(sources.Get(1), gateways.Get(0));
//     interfaces = address.Assign(devices);
//     source_interfaces.Add(interfaces.Get(0));
//     address.NewNetwork();

//     //std::cout << "Link between src1(" << source_interfaces.GetAddress(0) << ") and g1(" << interfaces.GetAddress(1) << ")\n"; 
//     //std::cout << "Link between src2(" << source_interfaces.GetAddress(1) << ") and g1(" << interfaces.GetAddress(3) << ")\n\n"; 

    
//     NetDeviceContainer gatedevices = UnReLink.Install(gateways);
//     gate_interfaces = address.Assign(gatedevices);
//     address.NewNetwork();
//     //std::cout << "Link between g1(" << gate_interfaces.GetAddress(0) << ") and g2(" << gate_interfaces.GetAddress(1) << ")\n\n"; 

    
//     devices = RemoteLink.Install(gateways.Get(1), sinks.Get(0));
//     interfaces = address.Assign(devices);
//     sink_interfaces.Add(interfaces.Get(1));
//     address.NewNetwork();

//     devices = RemoteLink.Install(gateways.Get(1), sinks.Get(1));
//     interfaces = address.Assign(devices);
//     sink_interfaces.Add(interfaces.Get(1));

//     // sink_interfaces.Add(interfaces.Get(3));
//     // std::cout << "Link between g2(" << interfaces2.GetAddress(0) << ") and sink1(" << sink_interfaces.GetAddress(0) << ")\n"; 
//     // std::cout << "Link between g2(" << interfaces2.GetAddress(2) << ") and sink2(" << sink_interfaces.GetAddress(1) << ")\n\n";

//     std::vector<ApplicationContainer> apps;
//     //Installs 2 flows on each source (one to each dest) - repeats nFlows/2 times
    
//     //if i < nFlows/2, install on source 1 (>= means source 2)
//     //if i is odd, install on sink 1 (even means sink 2)
//     for (uint32_t i = 0; i < 2*nFlows; ++i)
//     {
//         uint16_t port = 50000 + i;
//         uint32_t sinkNum = i % 2 == 0 ? 0 : 1; //alternates even-odd
//         uint32_t sourceNum = i < nFlows ? 0 : 1; //does first source first, then second one
//         Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
//         PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
//         sinkHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));

//         //Install sink app on proper sink
//         ApplicationContainer sinkApp = sinkHelper.Install(sinks.Get(sinkNum));
//         sinkApp.Start(Seconds(0.0));
//         sinkApp.Stop(Seconds(20.0));
//         apps.push_back(sinkApp);

//         AddressValue remoteAddress(InetSocketAddress(sink_interfaces.GetAddress(sinkNum), port));

//         BulkSendHelper ftp("ns3::TcpSocketFactory", Address());
//         ftp.SetAttribute("Remote", remoteAddress);
//         ftp.SetAttribute("MaxBytes", UintegerValue(0));
//         ApplicationContainer sourceApp = ftp.Install(sources.Get(sourceNum));
//         //std::cout << "Installed BulkSendHelper on source " << sourceNum + 1 << " going to " << sink_interfaces.GetAddress(sinkNum) << ":" << port;
//         double start = uv->GetValue(1.0,1.5);
//         flowStarts.emplace_back(start);
//         //std::cout << ", starting at " << start << std::endl;
//         sourceApp.Start(Seconds(start));
//         sourceApp.Stop(Seconds(20.0));
//         apps.push_back(sourceApp);
//         Simulator::Schedule(Seconds(19.9999), &getRx, sinkApp, i);
//     }
    

//     // Set up tracing if enabled
//     if (tracing)
//     {
//         std::ofstream ascii;
//         Ptr<OutputStreamWrapper> ascii_wrap;
//         ascii.open(prefix_file_name + "-ascii");
//         ascii_wrap = new OutputStreamWrapper(prefix_file_name + "-ascii", std::ios::out);
//         stack.EnableAsciiIpv4All(ascii_wrap);

//         //for (uint16_t index = 0; index < sources.GetN(); index++)
//         {
//             std::string flowString;
//             if (nFlows > 1)
//             {
//                 flowString = "-flow" + std::to_string(0);
//             }

//             firstCwnd[sources.Get(0)->GetId()] = true;
//             firstSshThr[sources.Get(0)->GetId()] = true;
//             firstRtt[sources.Get(0)->GetId()] = true;
//             firstRto[sources.Get(0)->GetId()] = true;

//             Simulator::Schedule(Seconds(flowStarts[0] + 0.001),
//                                 &TraceCwnd,
//                                 prefix_file_name + flowString + "-cwnd.data",
//                                 sources.Get(0)->GetId());
            
//         }
//     }

//     if (pcap)
//     {
//         UnReLink.EnablePcapAll(prefix_file_name, true);
//         LocalLink.EnablePcapAll(prefix_file_name, true);
//         RemoteLink.EnablePcapAll(prefix_file_name, true);
//     }

//     NS_LOG_INFO("Initialize Global Routing.");
//     Ipv4GlobalRoutingHelper::PopulateRoutingTables();

//     Simulator::Stop(Seconds(20.0));
//     Simulator::Run(); 
//     Simulator::Destroy();
//     //std::cout << "cwnd changes have been written to TcpVariantsComparison-flow0-cwnd.data!\n";
    
    
//     return 0;
// }
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering, University of Padova
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
 * Authors of the original TCP example:
 * Justin P. Rohrer, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>, Siddharth Gangadhar <siddharth@ittc.ku.edu>
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 *
 * Adapted to QUIC by:
 *          Alvise De Biasio <alvise.debiasio@gmail.com>
 *          Federico Chiariotti <chiariotti.federico@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Davide Marcato <davidemarcato@outlook.com>
 *
 */

#include <iostream>
#include <fstream>
#include <string>

#include <ns3/config-store.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/quic-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("QuicVariantsComparison");

// connect to a number of traces
static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
}

static void
Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, const QuicHeader& q, Ptr<const QuicSocketBase> qsb)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() << std::endl;
}

static void
Traces(uint32_t serverId, std::string pathVersion, std::string finalPart)
{
  AsciiTraceHelper asciiTraceHelper;

  std::ostringstream pathCW;
  pathCW << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/CongestionWindow";
  NS_LOG_INFO("Matches cw " << Config::LookupMatches(pathCW.str().c_str()).GetN());

  std::ostringstream fileCW;
  fileCW << pathVersion << "QUIC-cwnd-change"  << serverId << "" << finalPart;

  std::ostringstream pathRTT;
  pathRTT << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/RTT";

  std::ostringstream fileRTT;
  fileRTT << pathVersion << "QUIC-rtt"  << serverId << "" << finalPart;

  std::ostringstream pathRCWnd;
  pathRCWnd<< "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/RWND";

  std::ostringstream fileRCWnd;
  fileRCWnd<<pathVersion << "QUIC-rwnd-change"  << serverId << "" << finalPart;

  std::ostringstream fileName;
  fileName << pathVersion << "QUIC-rx-data" << serverId << "" << finalPart;
  std::ostringstream pathRx;
  pathRx << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/*/QuicSocketBase/Rx";
  NS_LOG_INFO("Matches rx " << Config::LookupMatches(pathRx.str().c_str()).GetN());

  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName.str ().c_str ());
  Config::ConnectWithoutContext (pathRx.str ().c_str (), MakeBoundCallback (&Rx, stream));

  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream (fileCW.str ().c_str ());
  Config::ConnectWithoutContext (pathCW.str ().c_str (), MakeBoundCallback(&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream (fileRTT.str ().c_str ());
  Config::ConnectWithoutContext (pathRTT.str ().c_str (), MakeBoundCallback(&RttChange, stream2));

  Ptr<OutputStreamWrapper> stream4 = asciiTraceHelper.CreateFileStream (fileRCWnd.str ().c_str ());
  Config::ConnectWithoutContextFailSafe (pathRCWnd.str ().c_str (), MakeBoundCallback(&CwndChange, stream4));
}

int main (int argc, char *argv[])
{
  std::string transport_prot = "TcpVegas";
  double error_p = 0.0;
  std::string bandwidth = "2Mbps";
  std::string delay = "0.01ms";
  std::string access_bandwidth = "12Mbps";
  std::string access_delay = "25ms";
  bool tracing = false;
  std::string prefix_file_name = "QuicVariantsComparison";
  double data_mbytes = 0;
  uint32_t mtu_bytes = 1400;
  uint16_t num_flows = 1;
  float duration = 100;
  uint32_t run = 0;
  bool flow_monitor = false;
  bool pcap = false;
  std::string queue_disc_type = "ns3::PfifoFastQueueDisc";

  // LogComponentEnable ("Config", LOG_LEVEL_ALL);
  CommandLine cmd;
  cmd.AddValue ("transport_prot", "Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpLedbat ", transport_prot);
  cmd.AddValue ("error_p", "Packet error rate", error_p);
  cmd.AddValue ("bandwidth", "Bottleneck bandwidth", bandwidth);
  cmd.AddValue ("delay", "Bottleneck delay", delay);
  cmd.AddValue ("access_bandwidth", "Access link bandwidth", access_bandwidth);
  cmd.AddValue ("access_delay", "Access link delay", access_delay);
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.AddValue ("prefix_name", "Prefix of output trace file", prefix_file_name);
  cmd.AddValue ("data", "Number of Megabytes of data to transmit", data_mbytes);
  cmd.AddValue ("mtu", "Size of IP packets to send in bytes", mtu_bytes);
  cmd.AddValue ("num_flows", "Number of flows", num_flows);
  cmd.AddValue ("duration", "Time to allow flows to run in seconds", duration);
  cmd.AddValue ("run", "Run index (for setting repeatable seeds)", run);
  cmd.AddValue ("flow_monitor", "Enable flow monitor", flow_monitor);
  cmd.AddValue ("pcap_tracing", "Enable or disable PCAP tracing", pcap);
  cmd.AddValue ("queue_disc_type", "Queue disc type for gateway (e.g. ns3::CoDelQueueDisc)", queue_disc_type);
  cmd.Parse (argc, argv);

  transport_prot = std::string ("ns3::") + transport_prot;

  SeedManager::SetSeed (1);
  SeedManager::SetRun (run);

  // User may find it convenient to enable logging
  Time::SetResolution (Time::NS);
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  // LogComponentEnable("QuicVariantsComparison", LOG_LEVEL_ALL);
  // LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
  // LogComponentEnable("PfifoFastQueueDisc", LOG_LEVEL_ALL);
  // LogComponentEnable ("QuicSocketBase", LOG_LEVEL_ALL);
  LogComponentEnable("TcpVegas", LOG_LEVEL_ALL);
  // LogComponentEnable("QuicL5Protocol", LOG_LEVEL_ALL);

  // Set the simulation start and stop time
  float start_time = 0.1;
  float stop_time = start_time + duration;

  // 4 MB of buffer
  Config::SetDefault ("ns3::QuicSocketBase::SocketRcvBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::QuicSocketBase::SocketSndBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::QuicStreamBase::StreamSndBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::QuicStreamBase::StreamRcvBufSize", UintegerValue (1 << 21));

  TypeId tcpTid;
  NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (transport_prot, &tcpTid), "TypeId " << transport_prot << " not found");
  Config::SetDefault ("ns3::QuicL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));


  // Create gateways, sources, and sinks
  NodeContainer gateways;
  gateways.Create (2);
  NodeContainer sources;
  sources.Create (num_flows);
  NodeContainer sinks;
  sinks.Create (num_flows);

  // Configure the error model
  // Here we use RateErrorModel with packet error rate
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  uv->SetStream (50);
  RateErrorModel error_model;
  error_model.SetRandomVariable (uv);
  error_model.SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
  error_model.SetRate (error_p);

  PointToPointHelper BottleneckLink;
  BottleneckLink.SetDeviceAttribute ("DataRate", StringValue (bandwidth));
  BottleneckLink.SetChannelAttribute ("Delay", StringValue (delay));
  BottleneckLink.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (&error_model));

  PointToPointHelper AccessLink;
  AccessLink.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
  AccessLink.SetChannelAttribute ("Delay", StringValue (access_delay));

  QuicHelper stack;
  stack.InstallQuic (sources);
  stack.InstallQuic (sinks);
  stack.InstallQuic (gateways);

  TrafficControlHelper tchPfifo;
  tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");

  TrafficControlHelper tchCoDel;
  tchCoDel.SetRootQueueDisc ("ns3::CoDelQueueDisc");

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");

  // Configure the sources and sinks net devices
  // and the channels between the sources/sinks and the gateways
  PointToPointHelper LocalLink;
  LocalLink.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
  LocalLink.SetChannelAttribute ("Delay", StringValue (access_delay));

  Ipv4InterfaceContainer sink_interfaces;

  DataRate access_b (access_bandwidth);
  DataRate bottle_b (bandwidth);
  Time access_d (access_delay);
  Time bottle_d (delay);

  uint32_t size = (std::min (access_b, bottle_b).GetBitRate () / 8) *
    ((access_d + bottle_d) * 2).GetSeconds ();

  Config::SetDefault ("ns3::PfifoFastQueueDisc::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, size / mtu_bytes)));
  Config::SetDefault ("ns3::CoDelQueueDisc::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, size)));

  for (int i = 0; i < num_flows; i++)
    {
      NetDeviceContainer devices;
      devices = AccessLink.Install (sources.Get (i), gateways.Get (0));
      tchPfifo.Install (devices);
      address.NewNetwork ();
      Ipv4InterfaceContainer interfaces = address.Assign (devices);

      devices = LocalLink.Install (gateways.Get (1), sinks.Get (i));
      if (queue_disc_type.compare ("ns3::PfifoFastQueueDisc") == 0)
        {
          tchPfifo.Install (devices);
        }
      else if (queue_disc_type.compare ("ns3::CoDelQueueDisc") == 0)
        {
          tchCoDel.Install (devices);
        }
      else
        {
          NS_FATAL_ERROR ("Queue not recognized. Allowed values are ns3::CoDelQueueDisc or ns3::PfifoFastQueueDisc");
        }
      address.NewNetwork ();
      interfaces = address.Assign (devices);
      sink_interfaces.Add (interfaces.Get (1));

      devices = BottleneckLink.Install (gateways.Get (0), gateways.Get (1));
      if (queue_disc_type.compare ("ns3::PfifoFastQueueDisc") == 0)
        {
          tchPfifo.Install (devices);
        }
      else if (queue_disc_type.compare ("ns3::CoDelQueueDisc") == 0)
        {
          tchCoDel.Install (devices);
        }
      else
        {
          NS_FATAL_ERROR ("Queue not recognized. Allowed values are ns3::CoDelQueueDisc or ns3::PfifoFastQueueDisc");
        }
      address.NewNetwork ();
      interfaces = address.Assign (devices);
    }

  NS_LOG_INFO ("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t port = 50000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  double interPacketInterval = 1000;

  // QUIC client and server
  for (uint16_t i = 0; i < sources.GetN (); i++)
    {
      QuicServerHelper dlPacketSinkHelper (port);
      AddressValue remoteAddress (InetSocketAddress (sink_interfaces.GetAddress (i, 0), port));
      serverApps.Add (dlPacketSinkHelper.Install (sinks.Get (i)));
      QuicClientHelper dlClient (sink_interfaces.GetAddress (i, 0), port);
      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
      dlClient.SetAttribute ("PacketSize", UintegerValue(1400));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(10000000));
      clientApps.Add (dlClient.Install (sources.Get (i)));
    }
    

  serverApps.Start (Seconds (0.99));
  clientApps.Stop (Seconds (20.0));
  clientApps.Start (Seconds (2.0));

  for (uint16_t i = 0; i < num_flows; i++)
    {
      auto n2 = sinks.Get (i);
      auto n1 = sources.Get (i);
      Time t = Seconds(2.100001);
      Simulator::Schedule (t, &Traces, n2->GetId(),
            "./server", ".txt");
      Simulator::Schedule (t, &Traces, n1->GetId(),
            "./client", ".txt");
    }

  if (pcap)
    {
      BottleneckLink.EnablePcapAll (prefix_file_name, true);
      LocalLink.EnablePcapAll (prefix_file_name, true);
      AccessLink.EnablePcapAll (prefix_file_name, true);
    }

  // Flow monitor
  FlowMonitorHelper flowHelper;
  if (flow_monitor)
    {
      flowHelper.InstallAll ();
    }

  Simulator::Stop (Seconds (stop_time));
  Simulator::Run ();

  if (flow_monitor)
    {
      flowHelper.SerializeToXmlFile (prefix_file_name + ".flowmonitor", true, true);
    }

  Simulator::Destroy ();
  return 0;
}
