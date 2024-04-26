/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
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
 * Authors: Justin P. Rohrer, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>, Siddharth Gangadhar
 * <siddharth@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 *
 * "TCP Westwood(+) Protocol Implementation in ns-3"
 * Siddharth Gangadhar, Trúc Anh Ngọc Nguyễn , Greeshma Umapathi, and James P.G. Sterbenz,
 * ICST SIMUTools Workshop on ns-3 (WNS3), Cannes, France, March 2013
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/enum.h"
#include "ns3/error-model.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/tcp-header.h"
#include "ns3/traffic-control-module.h"
#include "ns3/udp-header.h"

#include <fstream>
#include <iostream>
#include <string>
#include <ns3/quic-helper.h>
#include <ns3/quic-socket-factory.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("QuicVariantsComparison");

static std::map<uint32_t, bool> firstCwnd;                      //!< First congestion window.
static std::map<uint32_t, bool> firstSshThr;                    //!< First SlowStart threshold.
static std::map<uint32_t, bool> firstRtt;                       //!< First RTT.
static std::map<uint32_t, bool> firstRto;                       //!< First RTO.
static std::map<uint32_t, Ptr<OutputStreamWrapper>> cWndStream; //!< Congstion window output stream.
static std::map<uint32_t, Ptr<OutputStreamWrapper>>
    ssThreshStream; //!< SlowStart threshold output stream.
static std::map<uint32_t, Ptr<OutputStreamWrapper>> rttStream;      //!< RTT output stream.
static std::map<uint32_t, Ptr<OutputStreamWrapper>> rtoStream;      //!< RTO output stream.
static std::map<uint32_t, Ptr<OutputStreamWrapper>> nextTxStream;   //!< Next TX output stream.
static std::map<uint32_t, Ptr<OutputStreamWrapper>> nextRxStream;   //!< Next RX output stream.
static std::map<uint32_t, Ptr<OutputStreamWrapper>> inFlightStream; //!< In flight output stream.
static std::map<uint32_t, uint32_t> cWndValue;                      //!< congestion window value.
static std::map<uint32_t, uint32_t> ssThreshValue;                  //!< SlowStart threshold value.
std::vector<double> flowStarts;

enum LINK_TYPE {SLOW, FAST, UNREL};

/**
 * Get the Node Id From Context.
 *
 * \param context The context.
 * \return the node ID.
 */
/* static uint32_t
GetNodeIdFromContext(std::string context)
{
    const std::size_t n1 = context.find_first_of('/', 1);
    const std::size_t n2 = context.find_first_of('/', n1 + 1);
    return std::stoul(context.substr(n1 + 1, n2 - n1 - 1));
} */

/**
 * Congestion window tracer.
 *
 * \param context The context.
 * \param oldval Old value.
 * \param newval New value.
 */
static void
CwndTracer(uint32_t oldval, uint32_t newval)
{
    uint32_t nodeId = 0;//GetNodeIdFromContext(context);
    //std::cout << "TRACE CALLED\n";
    
    if (firstCwnd[nodeId])
    {
        *cWndStream[nodeId]->GetStream() << "0.0\t" << oldval << std::endl;
        firstCwnd[nodeId] = false;
    }
    *cWndStream[nodeId]->GetStream() << Simulator::Now().GetSeconds() << " " << newval << std::endl;
    cWndValue[nodeId] = newval;

    if (!firstSshThr[nodeId])
    {
        *ssThreshStream[nodeId]->GetStream()
            << Simulator::Now().GetSeconds() << "\t" << ssThreshValue[nodeId] << std::endl;
    }
}

/**
 * Congestion window trace connection.
 *
 * \param cwnd_tr_file_name Congestion window trace file name.
 * \param nodeId Node ID.
 */
static void
TraceCwnd(std::string cwnd_tr_file_name, uint32_t nodeId)
{
    AsciiTraceHelper ascii;
    //std::cout << "TRACE STARTED\n";
    cWndStream[nodeId] = ascii.CreateFileStream(cwnd_tr_file_name);
    Config::ConnectWithoutContext("/NodeList/" + std::to_string(nodeId) +
                        "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/CongestionWindow",
                    MakeCallback(&CwndTracer));
}
static void getRx(ApplicationContainer packetSink, uint32_t index, uint32_t sourceNum, uint32_t sinkNum)
{
    //Taken from examples/routing/simple-multicast-flooding.cc

    for (auto end = packetSink.End(), iter = packetSink.Begin(); iter != end; ++iter)
    {
        auto node = (*iter)->GetNode();
        auto sink = (*iter)->GetObject<QuicServer>();
        std::cout << "Flow " << index + 1 << " at node " << Names::FindName(node) << " received " << sink->GetTotalRx()
                  << " bytes from node source" << sourceNum + 1 << "; goodput was "<< static_cast<double>(sink->GetTotalRx()) / (20.0 - flowStarts[index]) << std::endl;
    }
}

static void Recompute(double interval){
    Ipv4GlobalRoutingHelper::RecomputeRoutingTables();
    Simulator::Schedule(Seconds(interval), Recompute, interval);
}

static void BreakLink(Ptr<Ipv4> interface1, Ptr<Ipv4> interface2){
    
    for (uint32_t i = 0; i < interface1->GetNInterfaces();++i){
        interface1->SetDown(i);
    }
    for (uint32_t i = 0; i < interface2->GetNInterfaces();++i){
        interface2->SetDown(i);
    }
}
int
main(int argc, char* argv[])
{    
    //LogComponentEnable("QuicClient", LOG_LEVEL_INFO);
    std::string transport_prot = "TcpCubic";
    double errorRate = 0.00001;
    std::string dataRate = "10Mbps";
    std::string delay = "10ms";
    uint32_t rngseed = 123456789;
    std::string link = "unrel";
    LINK_TYPE link_type;
    
    bool tracing = true;
    std::string prefix_file_name = "QuicLinkBreak";
    uint16_t nFlows = 3;
    bool pcap = true;

    CommandLine cmd(__FILE__);
    cmd.AddValue("transport_prot", "Transport protocol to use: TcpNewReno, TcpCubic",transport_prot);
    cmd.AddValue("errorRate", "Packet error rate", errorRate);
    cmd.AddValue("dataRate", "Bottleneck bandwidth", dataRate);
    cmd.AddValue("delay", "Bottleneck delay", delay);
    cmd.AddValue("tracing", "Flag to enable/disable tracing", tracing);
    cmd.AddValue("nFlows", "Number of flows", nFlows);

    cmd.AddValue("pcap", "Enable or disable PCAP tracing", pcap);
    cmd.AddValue("seed", "RNG seed", rngseed);
    cmd.AddValue("link", "Link Type (slow, fast, unrel)", link);
    cmd.Parse(argc, argv);

    if (!link.compare("slow")){
        link_type = SLOW;
    } else if (!link.compare("fast")) {
        link_type = FAST;
    } else {
        link_type = UNREL;
    }
    transport_prot = std::string("ns3::") + transport_prot;

    SeedManager::SetSeed(rngseed);

    TypeId tcpTid;
    NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (transport_prot, &tcpTid), "TypeId " << transport_prot << " not found");
    Config::SetDefault ("ns3::QuicL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));
    Config::SetDefault("ns3::QuicClient::NumStreams", UintegerValue(nFlows - 1));


    //LogComponentEnable("TcpVariantsComparison", LOG_LEVEL_ALL);

    // Create gateways, sources, and sinks
    NodeContainer sources;
    sources.Create(2);

    NodeContainer gateways;
    gateways.Create(2*nFlows);
    
    NodeContainer sinks;    //destination 
    sinks.Create(2);
    
    Names::Add("source1", sources.Get(0));
    Names::Add("source2", sources.Get(1));
    for (uint32_t i = 0; i < gateways.GetN(); ++i) {
         Names::Add("gateway" + std::to_string(i+1), gateways.Get(i));
    }
    Names::Add("sink1", sinks.Get(0));
    Names::Add("sink2", sinks.Get(1));

    // Configure the error model
    // Here we use RateErrorModel with packet error rate
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    RateErrorModel error_model;
    error_model.SetRandomVariable(uv);
    error_model.SetUnit(RateErrorModel::ERROR_UNIT_BYTE);
    error_model.SetRate(errorRate);

    QuicHelper stack;
    stack.InstallQuic(sources);
    stack.InstallQuic(sinks);
    stack.InstallQuic(gateways);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");

    // Configure the sources and sinks net devices
    // and the channels between the sources/sinks and the gateways
    PointToPointHelper FastLink;
    FastLink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    FastLink.SetChannelAttribute("Delay", StringValue("0.01ms"));

    PointToPointHelper SlowLink;
    SlowLink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    SlowLink.SetChannelAttribute("Delay", StringValue("100ms"));

    PointToPointHelper UnReLink;
    UnReLink.SetDeviceAttribute("DataRate", StringValue(dataRate));
    UnReLink.SetChannelAttribute("Delay", StringValue(delay));
    UnReLink.SetDeviceAttribute("ReceiveErrorModel", PointerValue(&error_model));

    Ipv4InterfaceContainer sink_interfaces; 
    Ipv4InterfaceContainer source_interfaces;
    Ipv4InterfaceContainer gate_interfaces;

    Ipv4InterfaceContainer interfaces;
    NetDeviceContainer devices;

    for (uint32_t i = 0; i < nFlows; ++i){
        switch (link_type) {
            case FAST:
                devices = FastLink.Install(sources.Get(0), gateways.Get(2*i));
                interfaces = address.Assign(devices);
                source_interfaces.Add(interfaces.Get(0));
                address.NewNetwork();
                devices = FastLink.Install(gateways.Get(2*i), gateways.Get(2*i+1));
                interfaces = address.Assign(devices);
                gate_interfaces.Add(interfaces);
                address.NewNetwork();
                devices = FastLink.Install(gateways.Get(2*i + 1), sinks.Get(0));
                interfaces = address.Assign(devices);
                sink_interfaces.Add(interfaces.Get(1));
                address.NewNetwork();
                break;
            case SLOW:
                devices = SlowLink.Install(sources.Get(0), gateways.Get(2*i));
                interfaces = address.Assign(devices);
                source_interfaces.Add(interfaces.Get(0));
                address.NewNetwork();
                devices = SlowLink.Install(gateways.Get(2*i), gateways.Get(2*i+1));
                interfaces = address.Assign(devices);
                gate_interfaces.Add(interfaces);
                address.NewNetwork();
                devices = SlowLink.Install(gateways.Get(2*i + 1), sinks.Get(0));
                interfaces = address.Assign(devices);
                sink_interfaces.Add(interfaces.Get(1));
                address.NewNetwork();
                break;
            default:
                devices = UnReLink.Install(sources.Get(0), gateways.Get(2*i));
                interfaces = address.Assign(devices);
                source_interfaces.Add(interfaces.Get(0));
                address.NewNetwork();
                devices = UnReLink.Install(gateways.Get(2*i), gateways.Get(2*i+1));
                interfaces = address.Assign(devices);
                gate_interfaces.Add(interfaces);
                address.NewNetwork();
                devices = UnReLink.Install(gateways.Get(2*i + 1), sinks.Get(0));
                interfaces = address.Assign(devices);
                sink_interfaces.Add(interfaces.Get(1));
                address.NewNetwork();
                break;                
        }
    }
    devices = FastLink.Install(sinks.Get(0), sinks.Get(1));
    interfaces = address.Assign(devices);
    sink_interfaces.Add(interfaces);
    address.NewNetwork();

    devices = FastLink.Install(sources.Get(1), gateways.Get(0));
    interfaces = address.Assign(devices);

    std::vector<ApplicationContainer> apps;

    uint16_t port = 50000;
    
    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    QuicServerHelper sinkHelper(port);
    PacketSinkHelper bulkSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

    //Install sink app on proper sink
    ApplicationContainer sinkApp = sinkHelper.Install(sinks.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(20.0));
    apps.push_back(sinkApp);

    // for (uint32_t i = 0; i < 3; ++i) {
    //     bulkSinkHelper.SetAttribute("Local", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), port + i)));
    //     ApplicationContainer secondSinkApp = bulkSinkHelper.Install(sinks.Get(1));
    //     secondSinkApp.Start(Seconds(0.0));
    //     secondSinkApp.Stop(Seconds(20.0));
    //     apps.push_back(sinkApp);

    //     AddressValue bulkSinkAddress(InetSocketAddress(sink_interfaces.GetAddress(nFlows+1), port + i));
    //     BulkSendHelper bulkSend("ns3::TcpSocketFactory", Address());
    //     bulkSend.SetAttribute("Remote", bulkSinkAddress);
    //     bulkSend.SetAttribute("MaxBytes", UintegerValue(0));
    //     ApplicationContainer bulkSendApp = bulkSend.Install(sources.Get(1));
    //     bulkSendApp.Start(Seconds(5.0));
    //     bulkSendApp.Stop(Seconds(15.0)); 
    //     apps.push_back(bulkSendApp);
    // }

    AddressValue remoteAddress(InetSocketAddress(sink_interfaces.GetAddress(nFlows), port));

    QuicClientHelper ftp(sink_interfaces.GetAddress(nFlows), port);
    ftp.SetAttribute("MaxPackets", UintegerValue(0));
    ftp.SetAttribute("Interval", TimeValue(Seconds(0.005)));
    ApplicationContainer sourceApp = ftp.Install(sources.Get(0));
    //std::cout << "Installed BulkSendHelper on source " << sourceNum + 1 << " going to " << sink_interfaces.GetAddress(sinkNum) << ":" << port;
    double start = uv->GetValue(1.0,1.5);
    std::cout << "Starting application at " << start << "sec\n";
    flowStarts.emplace_back(start);
    //std::cout << ", starting at " << start << std::endl;
    sourceApp.Start(Seconds(start));
    sourceApp.Stop(Seconds(20.0));
    apps.push_back(sourceApp);
    Simulator::Schedule(Seconds(19.9999), &getRx, sinkApp, 0, 0, 1);
    
    

    // Set up tracing if enabled
    if (tracing)
    {
        std::ofstream ascii;
        Ptr<OutputStreamWrapper> ascii_wrap;
        ascii.open(prefix_file_name + "-ascii");
        ascii_wrap = new OutputStreamWrapper(prefix_file_name + "-ascii", std::ios::out);
        stack.EnableAsciiIpv4All(ascii_wrap);

        //for (uint16_t index = 0; index < sources.GetN(); index++)
        {
            std::string flowString;
            if (nFlows > 1)
            {
                flowString = "-flow" + std::to_string(0);
            }

            firstCwnd[sources.Get(0)->GetId()] = true;
            firstSshThr[sources.Get(0)->GetId()] = true;
            firstRtt[sources.Get(0)->GetId()] = true;
            firstRto[sources.Get(0)->GetId()] = true;

            Simulator::Schedule(Seconds(flowStarts[0] + 0.001),
                                &TraceCwnd,
                                prefix_file_name + flowString + "-cwnd.data",
                                sources.Get(0)->GetId());
            
        }
    }

    if (pcap)
    {
        UnReLink.EnablePcapAll(prefix_file_name, true);
        FastLink.EnablePcapAll(prefix_file_name, true);
        SlowLink.EnablePcapAll(prefix_file_name, true);
    }

    NS_LOG_INFO("Initialize Global Routing.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    Simulator::Schedule(Seconds(0.01), &Recompute, 0.01);
    Simulator::Schedule(Seconds(5.0), &BreakLink, gate_interfaces.Get(0).first, gate_interfaces.Get(1).first);
    Simulator::Stop(Seconds(20.0));
    Simulator::Run(); 
    Simulator::Destroy();
    //std::cout << "cwnd changes have been written to TcpVariantsComparison-flow0-cwnd.data!\n";
    
    
    return 0;
}
