#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/eht-phy.h"
#include "ns3/enum.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/node.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-acknowledgment.h"
#include "ns3/wifi-net-device.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

#include <array>
#include <functional>
#include <numeric>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("simulator_temp2");

int
main(int argc, char* argv[])
{
    NS_LOG_UNCOND("simulator_temp2.cc");

    uint32_t payloadSize = 1400;
    double simulationTime{11};
    std::size_t be_nStations{4};
    int gi = 800;

    NodeContainer be_wifiStaNodes;
    be_wifiStaNodes.Create(be_nStations);
    NodeContainer wifiApNode;
    wifiApNode.Create(1);

    /*
    NS_LOG_UNCOND(wifiApNode.Get(0)->GetId()); -> 0
    NS_LOG_UNCOND(ax_wifiStaNodes.Get(0)->GetId()); -> 1
    NS_LOG_UNCOND(be_wifiStaNodes.Get(0)->GetId()); -> 2
    */
    NetDeviceContainer apDevice;
    NetDeviceContainer be_staDevices;

    Ssid ssid = Ssid("ns3");

    Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel>();
    Ptr<LogDistancePropagationLossModel> lossModel =
        CreateObject<LogDistancePropagationLossModel>();
    spectrumChannel->AddPropagationLossModel(lossModel);

    WifiMacHelper mac;
    WifiHelper wifi;

    wifi.SetStandard(WIFI_STANDARD_80211be);

    /* Multi Link Device */
    SpectrumWifiPhyHelper phy(2);
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(spectrumChannel);

    phy.Set((uint8_t)0, "ChannelSettings", StringValue("{0, 20, BAND_2_4GHZ, 0}"));
    wifi.SetRemoteStationManager((uint8_t)0,
                                 "ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("EhtMcs7"),
                                 "ControlMode",
                                 StringValue("ErpOfdmRate24Mbps"));

    phy.Set((uint8_t)1, "ChannelSettings", StringValue("{0, 20, BAND_5GHZ, 0}")); // 80Mhz
    wifi.SetRemoteStationManager((uint8_t)1,
                                 "ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("EhtMcs7"),
                                 "ControlMode",
                                 StringValue("OfdmRate24Mbps"));

    /*phy.Set((uint8_t)1, "ChannelSettings", StringValue("{0, 80, BAND_6GHZ, 0}")); // 80Mhz
    wifi.SetRemoteStationManager((uint8_t)1,
                                 "ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("EhtMcs7"),
                                 "ControlMode",
                                 StringValue("EhtMcs7"));*/

    mac.SetType("ns3::StaWifiMac",
                "Ssid",
                SsidValue(ssid),
                "ActiveProbing",
                BooleanValue(false));

    be_staDevices = wifi.Install(phy, mac, be_wifiStaNodes);
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevice = wifi.Install(phy, mac, wifiApNode);

    Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HeConfiguration/GuardInterval",
                TimeValue(NanoSeconds(gi)));
    Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HeConfiguration/MpduBufferSize",
                UintegerValue(256));

    /* Mobility Configuration */
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector3D(0.0, 0.0, 0.0));
    // STA 1 - 4
    positionAlloc->Add(Vector3D(10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, 10.0, 0.0));
    positionAlloc->Add(Vector3D(-10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, -10.0, 0.0));
    // STA 5 - 8
    positionAlloc->Add(Vector3D(7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, -7.1, 0.0));
    positionAlloc->Add(Vector3D(7.1, -7.1, 0.0));
    // STA 9 - 12
    positionAlloc->Add(Vector3D(10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, 10.0, 0.0));
    positionAlloc->Add(Vector3D(-10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, -10.0, 0.0));
    // STA 13 - 16
    positionAlloc->Add(Vector3D(7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, -7.1, 0.0));
    positionAlloc->Add(Vector3D(7.1, -7.1, 0.0));
    // STA 17 - 20
    positionAlloc->Add(Vector3D(10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, 10.0, 0.0));
    positionAlloc->Add(Vector3D(-10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, -10.0, 0.0));
    // STA 21 - 24
    positionAlloc->Add(Vector3D(7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, -7.1, 0.0));
    positionAlloc->Add(Vector3D(7.1, -7.1, 0.0));
    // STA 25 - 28
    positionAlloc->Add(Vector3D(10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, 10.0, 0.0));
    positionAlloc->Add(Vector3D(-10.0, 0.0, 0.0));
    positionAlloc->Add(Vector3D(0.0, -10.0, 0.0));
    // STA 29 - 32
    positionAlloc->Add(Vector3D(7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, 7.1, 0.0));
    positionAlloc->Add(Vector3D(-7.1, -7.1, 0.0));
    positionAlloc->Add(Vector3D(7.1, -7.1, 0.0));

    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(wifiApNode);
    mobility.Install(be_wifiStaNodes);
/*
    NS_LOG_UNCOND(wifiApNode.Get(0)->GetObject<MobilityModel>()->GetPosition());
    for (int i = 0; i < int(be_nStations); i++)
    {
        NS_LOG_UNCOND(be_wifiStaNodes.Get(i)->GetObject<MobilityModel>()->GetPosition());
    }
    NS_LOG_UNCOND(wifiApNode.Get(0)->GetDevice(0)->GetAddress());
*/
    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(be_wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");

    Ipv4InterfaceContainer be_staNodeInterfaces;
    be_staNodeInterfaces = address.Assign(be_staDevices);
    Ipv4InterfaceContainer apNodeInterface;
    apNodeInterface = address.Assign(apDevice);

    /* Application Handler */
    // std::vector<uint8_t> tosValues = {0x70, 0x28, 0xb8, 0xc0}; // AC_BE, AC_BK, AC_VI, AC_VO
    /*
    STA 1 -> High: 200Mb/s, Low: 100Mb/s
    STA 2 ~ 4 -> High: 100Mb/s, Low: 100Mb/s
    */
    uint8_t port = 9;
    UdpServerHelper server(port);
    ApplicationContainer serverApp = server.Install(wifiApNode.Get(0));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(simulationTime));

    InetSocketAddress dest(apNodeInterface.GetAddress(0), port);
    /* Application 1 */
    /*for(int i = 0; i < (int)be_nStations; i++)
    {
        uint8_t tosValue1 = 0xc0; // AC_VO
        dest.SetTos(tosValue1);
        ApplicationContainer clientApp1;
        OnOffHelper be_client1("ns3::UdpSocketFactory", dest);
        be_client1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        be_client1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        be_client1.SetAttribute("DataRate", StringValue("60Mb/s"));
        be_client1.SetAttribute("PacketSize", UintegerValue(payloadSize));
        clientApp1 = be_client1.Install(be_wifiStaNodes.Get(i));
        clientApp1.Start(Seconds(1.0));
        clientApp1.Stop(Seconds(simulationTime));
    }*/
    /* Application 2 */
    for(int i = 0; i < (int)be_nStations; i++)
    {
        uint8_t tosValue2 = 0xb8; // AC_VI
        dest.SetTos(tosValue2);
        ApplicationContainer clientApp2;
        OnOffHelper be_client2("ns3::UdpSocketFactory", dest);
        be_client2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        be_client2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        be_client2.SetAttribute("DataRate", StringValue("30Mb/s"));
        be_client2.SetAttribute("PacketSize", UintegerValue(payloadSize));
        clientApp2 = be_client2.Install(be_wifiStaNodes.Get(i));
        clientApp2.Start(Seconds(1.0));
        clientApp2.Stop(Seconds(simulationTime));
    }
    /* Application 3 */
    
    for(int i = 0; i < (int)be_nStations; i++)
    {
        uint8_t tosValue3 = 0x70; // AC_BE
        dest.SetTos(tosValue3);
        ApplicationContainer clientApp3;
        OnOffHelper be_client3("ns3::UdpSocketFactory", dest);
        be_client3.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        be_client3.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        be_client3.SetAttribute("DataRate", StringValue("10Mb/s"));
        be_client3.SetAttribute("PacketSize", UintegerValue(payloadSize));
        clientApp3 = be_client3.Install(be_wifiStaNodes.Get(i));
        clientApp3.Start(Seconds(1.0));
        clientApp3.Stop(Seconds(simulationTime));
    }
    
    /* Application 4 */
    
    /*for(int i = 0; i < (int)be_nStations; i++)
    {
        uint8_t tosValue4 = 0x28; // AC_BK
        dest.SetTos(tosValue4);
        ApplicationContainer clientApp4;
        OnOffHelper be_client4("ns3::UdpSocketFactory", dest);
        be_client4.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        be_client4.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        be_client4.SetAttribute("DataRate", StringValue("0.01Mb/s"));
        be_client4.SetAttribute("PacketSize", UintegerValue(payloadSize));
        clientApp4 = be_client4.Install(be_wifiStaNodes.Get(i));
        clientApp4.Start(Seconds(1.0));
        clientApp4.Stop(Seconds(simulationTime));
    }*/
    

    Simulator::Schedule(Seconds(0), &Ipv4GlobalRoutingHelper::PopulateRoutingTables);

    // Simulator::Stop(Seconds(3.0));
    Simulator::Stop(Seconds(simulationTime));

    /*
    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
    */

    phy.EnablePcapAll("simulator");

    Simulator::Run();

    Simulator::Destroy();
    return 0;
}
