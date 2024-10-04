## Appendix. E. Latency

### Objective
* ns-3 <-> Wireshark 간의 동기화 문제 때문에 wireshark를 못씀
* ns-3 환경에서 standalone하게 latency등과 같은 성능 지표를 측정할 수 있는 코드를 생성하기 위한 분석 문서임
* 해당 문서에서는 ns-3.40 기반 PHY 계층에서 송신과 수신에 대한 자세한 내용을 분석함

### Preview
* 진짜 한 6개월 하면서 눈치를 못챈 내가 레전드
* 아래와 같은 Wireshark 기반으로 찍힌 AP <-> STA2 통신 과정 시나리오를 보자

```
⭐ AP 입장, link 1
1. Time: 0.963129s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 8)
1. Time: 0.963129s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 9)
...
1. Time: 0.963129s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 11)
9. Time: 0.963139s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:05 / length: 80 / Info: 802.11 Block Ack
```

```
⭐ STA2 입장, link 1
1. Time: 0.960405s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 8)
1. Time: 0.960405s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 9)
...
1. Time: 0.960405s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 11)
9. Time: 0.961073s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:05 / length: 80 / Info: 802.11 Block Ack
```

* 각 device의 NIC에서 패킷 캡처를 했을 텐데,, 뭔가 이상함
  * 뭐가 이상하냐면,, 기존의 특정 seq #를 가지는 mpdu에 대한 latency 측정 방법은 아래와 같음
  * latency = ((AP가 수신한 시점 - STA이 송신한 시점) / aggregation size)로 계산했음 (즉, (0.963129s - 0.960405s) / 4 = 0.681ms)
  * AP 입장에서, STA2가 보낸 A-mpdu에 대한 BA를 0.963139s에 보냄
  * 근데 STA2 입장에서, AP가 보낸 BA를 받은 시점은 0.961073s 임. (즉, AP가 BA를 보내기도 전에 BA를 받은 거랑 똑같음)
  * 이때까지 link 1과 link 2간의 시간 동기화만 안맞는 줄 알았는데, device 간에도 시간 동기화가 안맞음
* 해결 방안 1: AP pcap 파일만 가지고 측정 시도
  * 문제점 1: 정확한 latency 계산이 안됨
    * 위의 시나리오에서, STA2의 seq #8 ~ 11에 대한 A-mpdu의 latency는 0.961073s - 0.960405s = 0.668ms이지만, AP는 그걸 계산할 수 있는 방법이 없음
  * 문제점 2: uplink 통신 기준으로, 손실이 발생할 때 STA 에서 송신한 원본 A-mpdu에 대한 로그가 없음 (왜냐면, AP 한텐 해당 패킷이 캡처가 안됐을 거니까)
* 해결 방안 2: STA pcap 파일만 가지고 측정 시도
  * 문제점 1: latency를 계산할 수 있어도 AP가 해당 패킷을 수신한 정확한 시점 계산이 안됨 (그림 그리기가 어려움)
  * 문제점 1: STA이 여러 개일 때 시간 동기화가 안맞을 거임 (그림 그리기가 어려움)
  * 두 문제점 모두 손실이 발생할 때의 정확한 시점과 이유가 도출되기 어려움
* 잔머리 굴리다가 그냥 포기

* 그래서 계획은 뭐냐.. ns-3 project에 기존 python 기반으로 되어 있던 analyzer를 올려서 build 할거임
  * ns-3 WifiAnalyzer 만들어서 linking 걸어주기
  * ns-3 time를 포함한 ppdu, psdu, mpdu header 및 payload 정보 analyzer에 넘기기
  * STA 개수 증가 또는 position의 변동과 같은 확장성을 고려한 각 method 별로 network 지표 자동 측정
  * 제안하는 재전송 방식도 구현할 수 있으면 할 예정
 
* 그럼 코드 분석 해야겠지... 송수신간의 코드를 봅시다

### 1. ns-3 WifiAnalyzer linking 걸어주기
* './ns3.40/src/wifi/model/' 디렉토리 위치로 가면 wifi와 관련된 코드 파일 (.cc) 및 헤더 파일 (.h) 있음
* 여기다가 wifi-analyzer.h랑 wifi-analyzer.cc 생성하고 print 테스트 함수 하나 만들어 주자, 코드는 다음과 같음

```c
// wifi-analyzer.h
#ifndef NS3_WIFI_ANALYZER_H
#define NS3_WIFI_ANALYZER_H

#endif //NS3_WIFI_ANALYZER_H

namespace ns3
{
    class WifiAnalyzer
    {
    public:
        void Print();
    };
}
```

```c
// wifi-analyzer.cc
#include "wifi-analyzer.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE("WifiAnalyzer");

    void
    WifiAnalyzer::Print() {
        NS_LOG_UNCOND("Test");
    }

}
```

* 그다음에 './ns3.40/src/wifi/CMakeLists.txt' 에서 아래 코드 추가해주면 빌드 할 수 있음
  * set(source_files) -> model/wifi-analyzer.cc 추가
  * set(header_files) -> model/wifi-analyzer.h 추가

### 2. ns-3 송신 및 수신 동작 코드 분석
* 다 정리하기에는 시간이 부족함 그래서 일단 결론만 빠르게
  * wifi-phy.cc의 Send()와 StartPreambleReceive()에서 ns:Time을 찍으면 같은 시간이 나옴 (즉, phy entity 정보를 기반으로 duration 계산을 하지 않음)
  * 따라서, 수신 시간을 계산한 뒤의 코드에서 인자 값을 넘겨야 됨
  * 결론적으로, 송신은 ns3::HtFrameExchangeManager::ForwardPsduDown에서 넘기고 수신은 ns3::FrameExchangeManager::Receive에서 넘김

* 송신 과정은 다음과 같음
* mpdu 전송 / Normal Ack 전송 / A-mpdu 전송 / BA 전송 / 기타 중 1개의 과정을 수행하고 아래의 공통 과정을 수행함
* ns3::HtFrameExchangeManager::ForwardPsduDown
```c
void
HtFrameExchangeManager::ForwardPsduDown(Ptr<const WifiPsdu> psdu, WifiTxVector& txVector)
{
    NS_LOG_FUNCTION(this << psdu << txVector);

    NS_LOG_DEBUG("Transmitting a PSDU: " << *psdu << " TXVECTOR: " << txVector);
    FinalizeMacHeader(psdu);
    NotifyTxToEdca(psdu);
    m_allowedWidth = std::min(m_allowedWidth, txVector.GetChannelWidth());

    if (psdu->IsAggregate())
    {
        txVector.SetAggregation(true);
    }

    m_phy->Send(psdu, txVector);
}
```
* ns3::WifiPhy::Send

* 수신 과정은 다음과 같음
  * ns3::WifiPhy::StartPreambleReceive에서 Preamble 정보를 기반으로 특정 시간에 receive event가 invoke되도록 설정해 줌
  * 그리고 qos data에 대한 receive event가 invoke되면 예외 없이 아래의 호출 순서를 따름

<p align="center">  
  <img src="https://github.com/user-attachments/assets/2910627d-332c-440f-aecf-c09c6a04bad3" width="40%">  
</p>

* 중요한 function 
  * ns3::FrameExchangeManager::Receive
  * ns3::MacRxMiddle::Receive
  * ns3::ApWifiMac::Receive
  * ns3::WifiMac::ForwardUp
  * ns3::WifiNetDevice::ForwardUp
      
* ACK는 별도 event로서 따로 scheduling 해줌
  * ns3::FrameExchangeManager::Receive에서
  * 단일 mpdu의 경우, ReceiveMpdu 호출 후 NA schedule
  * A-mpdu의 경우, EndReceiveAmpdu 호출 후 BA schedule

* A-mpdu에 포함된 mpdu가 도착할 경우, A-mpdu가 완성될 때까지 LLC layer로 forwarding을 대기하지 않음 (⭐ 매우 중요)
* 즉, mpdu가 도착하는대로 상위 계층으로 forwarding 해줌

```c
void
FrameExchangeManager::Receive(Ptr<const WifiPsdu> psdu,
                              RxSignalInfo rxSignalInfo,
                              WifiTxVector txVector,
                              std::vector<bool> perMpduStatus) {
    NS_LOG_FUNCTION(
            this << psdu << rxSignalInfo << txVector << perMpduStatus.size()
                 << std::all_of(perMpduStatus.begin(), perMpduStatus.end(), [](bool v) { return v; }));

    if (!perMpduStatus.empty()) {
        // for A-MPDUs, we get here only once
        PreProcessFrame(psdu, txVector);
    }

    Mac48Address addr1 = psdu->GetAddr1();

    if (addr1.IsGroup() || addr1 == m_self) {
        // receive broadcast frames or frames addressed to us only
        if (psdu->GetNMpdus() == 1) { // 여기 중요!!
            // if perMpduStatus is not empty (i.e., this MPDU is not included in an A-MPDU)
            // then it must contain a single value which must be true (i.e., the MPDU
            // has been correctly received)
            NS_ASSERT(perMpduStatus.empty() || (perMpduStatus.size() == 1 && perMpduStatus[0]));
            // Ack and CTS do not carry Addr2
            if (!psdu->GetHeader(0).IsAck() && !psdu->GetHeader(0).IsCts()) {
                GetWifiRemoteStationManager()->ReportRxOk(psdu->GetHeader(0).GetAddr2(),
                                                          rxSignalInfo,
                                                          txVector);
            }
            ReceiveMpdu(*(psdu->begin()), rxSignalInfo, txVector, perMpduStatus.empty());
        } else {
            EndReceiveAmpdu(psdu, rxSignalInfo, txVector, perMpduStatus);
        }
    } else if (m_promisc) {
        for (const auto &mpdu: *PeekPointer(psdu)) {
            if (!mpdu->GetHeader().IsCtl()) {
                m_rxMiddle->Receive(mpdu, m_linkId);
            }
        }
    }
    if (!perMpduStatus.empty()) {
        // for A-MPDUs, we get here only once
        PostProcessFrame(psdu, txVector);
    }
}
```
* HtFrameExchangeManager::ForwardPsduDown()와 다르게, FrameExchangeManager::Receive는 A-mpdu를 수신 받을 때 2번 호출됨
  * 단일 mpdu를 수신 받을 때는 한 번만 호출되며, A-mpdu를 수신 받을 때 호출되는 조건은 아래와 같음
  * 첫 번째 호출: 실제 A-mpdu에 포함된 mpdu가 수신될 때
  * 두 번째 호출: A-mpdu에 대한 수신이 완료 될 때 (psdu를 통해 전처리 된 Aggregation 정보를 가지고 있음)
  * (⭐ 매우 중요) 이러한 현상은 **실제로** A-mpdu가 수신되는 방식이 독특하기 때문임 -> 5. A-mpdu에 대한 실제 수신 동작 참고
  * 따라서, 수신 정보를 실제 mpdu 전송에 대한 신호가 수신되는 조건에서 ns3-analyzer에 인자 값으로 전달해야 됨
  

### 3. ns-3 WifiAnalyzer implementaion
* 일단 필요한 정보 리스트업 부터 (처음에는 psdu 자체를 넘기려다가, 그냥 wifi module에서 값을 처리하고 넘기기로 함)
  * 현재 시간: Simulator::Now().As(Time::S), (TimwWithUnit)
  * 함수가 호출된 device의 MAC 주소: m_self, (Mac48Address)
  * mpdu가 전송된 link 정보: m_linkId, (uint8_t)
  * mpdu header에 포함된 송신 및 수신 MAC 주소: mpdu_header.GetAddr2() 및 mpdu_header.GetAddr1(), (Mac48Address)
  * payload size (ppdu의 payload 이니까, psdu size = mpdu size): mpdu->GetSize(), (uint32_t)
  * mpdu의 seq #: mpdu_header->GetSeqeunceNumber(), (uint16_t)
  * mpdu의 qos tid: mpdu_header->GetQosTid(), (uint8_t)
  * mpdu의 재전송 여부: mpdu_header->IsRetry(), (bool)
  * BA의 경우, BA 헤더 정보 
  * 일단 이정도, 필요하면 더 추가할 예정
    
```c
WifiAnalyzer wifiAnalyzer;
auto ptr = psdu->begin();
for (int i = 0; i < (int) psdu->GetNMpdus(); i++) {
  auto mpdu_header = ptr[i]->GetHeader();
  if (ptr[i]->GetSize() > 1400) {
  wifiAnalyzer.Receive(Simulator::Now().As(Time::S), m_self, m_linkId,
                            mpdu_header.GetAddr2(),
                            mpdu_header.GetAddr1(), ptr[i]->GetSize(),
                            mpdu_header.GetSequenceNumber(),
                            mpdu_header.GetQosTid(), mpdu_header.IsRetry());
  }
}
```

### 4. ns-3 WifiAnalyzer vs Wireshark
* ns-3 analyzer trace system에 대한 검증을 wireshark 기반 패킷 캡처된 정보를 기반으로 수행해볼까 함

* 전송 로그 기준 - ns-3 WifiAnalyzer
```
⭐ STA 1 및 STA2의 전송 로그
1. Time: +1.10928s	 STA 1 send MPDU, link Id: 1	 Seq #: 0	 QosTid: 5	 Retry: 0
2. Time: +1.10928s	 STA 1 send MPDU, link Id: 1	 Seq #: 1	 QosTid: 5	 Retry: 0
3. Time: +1.10928s	 STA 1 send MPDU, link Id: 1	 Seq #: 2	 QosTid: 5	 Retry: 0
4. Time: +1.10928s	 STA 1 send MPDU, link Id: 1	 Seq #: 3	 QosTid: 5	 Retry: 0
5. Time: +1.10928s	 STA 1 send MPDU, link Id: 1	 Seq #: 4	 QosTid: 5	 Retry: 0
6. Time: +1.10928s	 STA 1 send MPDU, link Id: 1	 Seq #: 5	 QosTid: 5	 Retry: 0
7. Time: +1.10928s	 STA 1 send MPDU, link Id: 1	 Seq #: 6	 QosTid: 5	 Retry: 0
8. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 465	 QosTid: 3	 Retry: 0
9. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 466	 QosTid: 3	 Retry: 0
10. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 467	 QosTid: 3	 Retry: 0
11. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 468	 QosTid: 3	 Retry: 0
12. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 469	 QosTid: 3	 Retry: 0
13. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 470	 QosTid: 3	 Retry: 0
14. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 471	 QosTid: 3	 Retry: 0
15. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 472	 QosTid: 3	 Retry: 0
16. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 473	 QosTid: 3	 Retry: 0
17. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 474	 QosTid: 3	 Retry: 0
18. Time: +1.10928s	 STA 2 send MPDU, link Id: 1	 Seq #: 475	 QosTid: 3	 Retry: 0
```
* 1.10928s 시점에 STA1 -> AP seq # 0 ~ 6에 해당하는 A-mpdu 전송
* 1.10928s 시점에 STA2 -> AP seq # 465 ~ 475에 해당하는 A-mpdu 전송
* 확인해야 점
  * 같은 크기에 해당하는 aggregation이 수행되었는지 (STA 1: 7, STA 2: 11)
  * 동시에 전송했기 때문에 손실이 발생했을 거고, 재전송 되었는지

* 전송 로그 기준 - Wireshark
```
⭐ STA1 입장, link 2
1. Time: 1.060329s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 0)
2. Time: 1.060329s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1)
...
7. Time: 1.060329s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 6)
8. Time: 1.062054s / Src: 00:00:00:00:00:06 / Dst: 00:00:00:00:00:09 / length: 56 / Info: 802.11 Block Ack Req
```
* 같은 크기에 해당하는 aggregation이 수행된 걸 확인할 수 있음
* 마지막에 찍힌 BA Req frame은 STA2에서 전송한 frame임
* 그럼 STA2 로그도 한번 보자

```
⭐ STA2 입장, link 2
1. Time: 1.060329s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 465)
2. Time: 1.060329s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 466)
...
3. Time: 1.060329s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 475)
n. Time: 1.061992s / Src: 00:00:00:00:00:06 / Dst: 00:00:00:00:00:09 / length: 80 / Info: 802.11 Block Ack Req
```
* 전송 시간이 동일하다? (즉, 시간 동기화가 맞다, 안맞을 줄 알았는데..)
  * STA 간의 동기화는 맞는거 같지만, AP와의 동기화는 안 맞음
  * 아무튼 실제 정확한 전송 및 수신 시간이 필요하니까 wireshark를 기반으로 지표를 측정하지는 않을 예정

* 수신 로그 기준 - ns-3 WifiAnalyzer
```
⭐ AP 입장, link1 및 link 2
Time: +1.00367s	 AP receive mpdu from STA 2, link Id: 1	 Seq #: 0	 QosTid: 5	 Retry: 0
Time: +1.0038s	 AP receive mpdu from STA 2, link Id: 1	 Seq #: 1	 QosTid: 5	 Retry: 0
Time: +1.00394s	 AP receive mpdu from STA 2, link Id: 1	 Seq #: 2	 QosTid: 5	 Retry: 0
Time: +1.00408s	 AP receive mpdu from STA 2, link Id: 1	 Seq #: 3	 QosTid: 5	 Retry: 0
Time: +1.00411s	 AP receive mpdu from STA 2, link Id: 0	 Seq #: 8	 QosTid: 5	 Retry: 0
Time: +1.00422s	 AP receive mpdu from STA 2, link Id: 1	 Seq #: 4	 QosTid: 5	 Retry: 0
Time: +1.00425s	 AP receive mpdu from STA 2, link Id: 0	 Seq #: 9	 QosTid: 5	 Retry: 0
Time: +1.00435s	 AP receive mpdu from STA 2, link Id: 1	 Seq #: 5	 QosTid: 5	 Retry: 0
Time: +1.00438s	 AP receive mpdu from STA 2, link Id: 0	 Seq #: 10	 QosTid: 5	 Retry: 0
Time: +1.00449s	 AP receive mpdu from STA 2, link Id: 1	 Seq #: 6	 QosTid: 5	 Retry: 0
Time: +1.00454s	 AP receive mpdu from STA 2, link Id: 0	 Seq #: 11	 QosTid: 5	 Retry: 0
```

* 수신 로그 기준 - Wireshark
```
⭐ AP 입장, link1
1. Time: 0.963129s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 8)
2. Time: 0.963129s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 9)
3. Time: 0.963129s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 10)
4. Time: 0.963129s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 11)
```

```
⭐ AP 입장, link2
1. Time: 0.956042s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 0)
2. Time: 0.956042s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1)
...
n. Time: 0.956042s / Src: 192.168.1.3 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 7)
```

* (⭐ 중요) Wireshark는 같은 시점에 A-mpdu를 수신받았는데, ns-3 WifiAnalyzer의 수신 시간은 뭔가 이상하다 -> 5. A-mpdu에 대한 실제 수신 동작 참고
* 아무튼 STA2로 부터 전송된 seq # 0 ~ 7은 link2를 통해 수신받고, seq # 8 ~ 11은 link1을 통해 수신 받음
* 결론적으로, ns-3 WifiAnalyzer <-> Wireshark 간의 송수신 흐름은 같음

### 5. A-mpdu에 대한 실제 수신 동작 (⭐ 매우매우 중요)
* A-mpdu의 실제 수신 과정
  * Each subframe (MPDU) is processed and received sequentially.
  * A-MPDU is a frame composed of multiple MPDUs (MAC Protocol Data Units), and is structured in a format that can distinguish the start and end of the frame, so that the receiving device can identify each MPDU separately.
  * When a signal is received at the PHY layer, A-MPDUs are received sequentially in MPDU units. Each MPDU is processed as an independent frame.
  * This process also has the same meaning as sequential signal reception physically.
* 정리하면, 수신기는 송신기가 A-mpdu를 송신하는지 mpdu를 송신하는지 알 수가 없음 (단순히, 물리적 신호를 받고 처리한다는 관점에서 생각하면 이해가 쉬움)
* 그럼에도 불구하고, 송신은 동시에 발생하는 이유
  * A-mpdu 송신도 실제로는 순차적으로 MPDU를 송신함
  * A-mpdu 송신과 수신의 가장 큰 차이점은 **하나의 연속된 전송**으로 이루어 진다는 점 (그래서 시간이 동일함)
  * 위의 ns-3 Analyzer 수신로그를 보면 시간에 따라 서로 다른 link에서 seq #가 섞여서 수신받는 것을 확인할 수 있음 (즉, 수신 과정은 하나의 연속된 수신을 보장할 수 없음)
  * 그러나 A-mpdu에 대한 송신은 특정 link에서 발생하기 때문에, seq #가 순차적이지 않을 수가 없음 (MAC Queue의 seq # 할당이 비순차적으로 이루어지지 않는 이상)
* 정리해보면
  * 송신 장치는 PHY 계층에서 MPDU를 aggregation하여 한 번의 연속된 전송으로 처리하지만, MPDU는 순차적으로 신호화되어 전송
  * 수신 장치가 이 신호를 순차적으로 받아 MPDU 단위로 분리하여 처리하게 됨

### Summary
* A-mpdu에 포함된 mpdu가 도착할 경우, A-mpdu가 완성될 때까지 LLC layer로 forwarding을 대기하지 않고 바로 올려보냄
* A-mpdu의 실제 수신 과정은 각각의 mpdu가 순차적으로 처리되며, A-mpdu가 동시에 처리되는 것이 아님
* 따라서, multi-link operation 환경에서 wlan seq #의 out-of-order 현상이 발생할 가능성이 높음 (Appendix F. 참고)


