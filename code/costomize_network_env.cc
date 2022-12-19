#include "tutorial-app.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

// #include "tcp-tahoe.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FifthScriptExample");

// ===========================================================================
//
//         node 3                 node 4
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |  5 Mbps, 2 ms       |
//           +---------------------+ 5 Mbps, 2 ms
//                      |
//                      |
//                    node 2                
//              +----------------+    
//              |    ns-3 TCP    |    
//              +----------------+    
//              |    10.1.1.1    |    
//              +----------------+    
//              | point-to-point |    
//              +----------------+    
//                      |
//                      |   3 Mbps, 2 ms
//                    node 1               
//              +----------------+    
//              |    ns-3 TCP    |    
//              +----------------+    
//              |    10.1.1.1    |    
//              +----------------+    
//              | point-to-point |    
//              +----------------+    
//                      |5 Mbps, 2 ms
//                      |
//                    node 0               
//              +----------------+    
//              |    ns-3 TCP    |    
//              +----------------+    
//              |    10.1.1.1    |     
//              +----------------+    
//              | point-to-point |    
//              +----------------+    
//
//
//
//
//
//
//                
//
// ===========================================================================
//

/**
 * Congestion window change callback
 *
 * \param oldCwnd Old congestion window.
 * \param newCwnd New congestion window.
 */
static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd); // "\t" << oldCwnd 
}

/**
 * Rx drop callback
 *
 * \param p The dropped packet.
 */
static void
RxDrop(Ptr<const Packet> p)
{
    NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
}

int
main(int argc, char* argv[])
{
    std::string latency = "2ms";
    std::string bandwidth = "5Mbps";
    std::string botneck_bandwidth = "3Mbps";

    CommandLine cmd(__FILE__);
    cmd.AddValue("latency", "P2P link Latency in miliseconds", latency);
    cmd.AddValue("bandwidth", "P2P data rate in bps", bandwidth);
    cmd.AddValue("botneck_bandwidth", "P2P data rate in bps", botneck_bandwidth);
    cmd.Parse(argc, argv);

    // In the following three lines, TCP NewReno is used as the congestion
    // control algorithm, the initial congestion window of a TCP connection is
    // set to 1 packet, and the classic fast recovery algorithm is used. Note
    // that this configuration is used only to demonstrate how TCP parameters
    // can be configured in ns-3. Otherwise, it is recommended to use the default
    // settings of TCP in ns-3.
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                       TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));

    // **************************************create the nodes************************************************
    // NodeContainer nodes;
    // nodes.Create(2);
    NodeContainer point;
    point.Create(5);
    NodeContainer n0n1 = NodeContainer(point.Get(0), point.Get(1));
    NodeContainer n1n2 = NodeContainer(point.Get(1), point.Get(2));
    NodeContainer n3n2 = NodeContainer(point.Get(3), point.Get(2));
    NodeContainer n4n2 = NodeContainer(point.Get(4), point.Get(2));

    // PointToPointHelper pointToPoint;
    // pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    // pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    
    
    //*********************************install internet stack on the nodes***********************************
    InternetStackHelper internet;
    internet.Install(point);

    //*********************************channels creation without ip addressing***************************************
    PointToPointHelper p2p, p2p_for1_2;
    p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
    p2p.SetChannelAttribute("Delay", StringValue(latency));

    NetDeviceContainer d0d1 = p2p.Install(n0n1);
    NetDeviceContainer d3d2 = p2p.Install(n3n2);
    NetDeviceContainer d4d2 = p2p.Install(n4n2);

    p2p_for1_2.SetDeviceAttribute("DataRate", StringValue(botneck_bandwidth));
    p2p_for1_2.SetChannelAttribute("Delay", StringValue(latency));

    NetDeviceContainer d1d2 = p2p_for1_2.Install(n1n2);


    // NetDeviceContainer devices;
    // devices = pointToPoint.Install(point);


    //*********************************assign ip addresses to the devices*******************************************
    Ipv4AddressHelper ipv4;

    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i0i1 = ipv4.Assign(d0d1);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i3i2 = ipv4.Assign(d3d2);

    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer i4i2 = ipv4.Assign(d4d2);



    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));// 0.00001
    // em->SetAttribute("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
    d4d2.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    d3d2.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    // Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    // em->SetAttribute("ErrorRate", DoubleValue(0.00001));// 0.00001
    // // em->SetAttribute("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
    // devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    // InternetStackHelper stack;
    // stack.Install(nodes);

    // Ipv4AddressHelper address;
    // address.SetBase("10.1.1.0", "255.255.255.252");
    // Ipv4InterfaceContainer interfaces = address.Assign(devices);


    //*********************************install applications n0 to n1*******************************************************

    // uint16_t sinkPort = 8080;
    // Address sinkAddress(InetSocketAddress(i3i2.GetAddress(1), sinkPort));
    // PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
    //                                   InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    // ApplicationContainer sinkApps = packetSinkHelper.Install(point.Get(2));
    // sinkApps.Start(Seconds(0.));
    // sinkApps.Stop(Seconds(20.));

    // Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(point.Get(3), TcpSocketFactory::GetTypeId());
    // ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

    // Ptr<TutorialApp> app = CreateObject<TutorialApp>();
    // app->Setup(ns3TcpSocket, sinkAddress, 1040, 1000, DataRate("1Mbps"));
    // point.Get(3)->AddApplication(app);
    // app->SetStartTime(Seconds(1.));
    // app->SetStopTime(Seconds(20.));

    


    //*********************************install applications n0 to n4*******************************************************

    uint16_t sinkPort1 = 8080;
    Address sinkAddress1(InetSocketAddress(i4i2.GetAddress(0), sinkPort1));
    PacketSinkHelper packetSinkHelper1("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), sinkPort1));
    ApplicationContainer sinkApps1 = packetSinkHelper1.Install(point.Get(4));
    sinkApps1.Start(Seconds(2.));

    Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket(point.Get(2), TcpSocketFactory::GetTypeId());
    // uint16_t sinkPort = 8080;
    // Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    // PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
    //                                   InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    // ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
    // sinkApps.Start(Seconds(0.));
    // sinkApps.Stop(Seconds(20.));

    // Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());

    //**************************************cwn************************************************
    ns3TcpSocket1->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

    // ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

    //**************************************Tcp app************************************************
    Ptr<TutorialApp> app1 = CreateObject<TutorialApp>();
    app1->Setup(ns3TcpSocket1, sinkAddress1, 1040, 1000, DataRate("1Mbps"));
    point.Get(0)->AddApplication(app1);
    app1->SetStartTime(Seconds(2.));
    // app1->SetStopTime(Seconds(20.));

    // //*********************************install applications n3 to n0*******************************************************

    uint16_t sinkPort2 = 8081;
    Address sinkAddress2(InetSocketAddress(i0i1.GetAddress(0), sinkPort2));
    PacketSinkHelper packetSinkHelper2("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), sinkPort2));
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install(point.Get(0));
    sinkApps2.Start(Seconds(5.));

    Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket(point.Get(1), TcpSocketFactory::GetTypeId());
    // // NS_LOG_UNCOND("hello");
    // //**************************************cwn************************************************
    // ns3TcpSocket2->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

    // //**************************************Tcp app************************************************
    Ptr<TutorialApp> app2 = CreateObject<TutorialApp>();
    app2->Setup(ns3TcpSocket2, sinkAddress2, 1040, 10000, DataRate("1Mbps"));
    point.Get(3)->AddApplication(app2);
    app2->SetStartTime(Seconds(5.));
    app2->SetStopTime(Seconds(20.));
    
    
    // Ptr<TutorialApp> app = CreateObject<TutorialApp>();
    // app->Setup(ns3TcpSocket, sinkAddress, 1040, 1000, DataRate("1Mbps"));
    // nodes.Get(0)->AddApplication(app);
    // app->SetStartTime(Seconds(1.));
    // app->SetStopTime(Seconds(20.));

    d4d2.Get(0)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));

    d3d2.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));

    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();


    return 0;
}
