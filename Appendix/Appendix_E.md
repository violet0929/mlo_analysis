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
  * 아무 이상한게 없지 않나요?
  * ㄴㄴ 이상한게 있음 진짜 레전드임 잘 보면 AP 입장에서, STA2가 보낸 A-mpdu에 대한 BA를 0.963139s에 보냄
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
  * analyzer class 만들어서 linking 걸어주기
  * ns-3 time를 포함한 ppdu, psdu, mpdu header 및 payload 정보 analyzer에 넘기기
  * STA 개수 증가 또는 position의 변동과 같은 확장성을 고려한 각 method 별로 network 지표 자동 측정
  * 제안하는 재전송 방식도 구현할 수 있으면 할 예정
 
* 그럼 코드 분석 해야겠지... 송수신간의 코드를 봅시다

### 1. wifi-analyzer linking 걸어주기
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

### 2. ns-3 send 및 receive 동작 코드 분석
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
  * 그리고 receive event가 invoke되면 예외 없이 아래의 호출 순서를 따름
    * ns3::PhyEntity::DoEndReceivePayload
    * ns3::PhyEntity::RxPayloadSucceeded
    * ns3::WifiPhyStateHelper::NotifyRxPsduSucceeded
    * ns3::Callback::operator()
    * ns3::CallbackImpl::operator()
    * ns3::FrameExchangeManager::Receive

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
  * (⭐ 매우 중요) 이러한 현상은 **실제로** A-mpdu가 수신되는 방식이 독특하기 때문임 -> 5. 뭔가 이상한 로그 (A-mpdu에 대한 실제 수신 동작) 참고
  * Receive()가 호출되는 조건은 크게 2가지로 분류됨
  * 조건 1: MPDU가 수신될 때
    * 실제 mpdu 전송에 대한 신호가 수신될 때, 전송된 데이터가 A-mpdu든 단일 mpdu든 상관 x
  * 조건 2: A-MPDU에 대한 수신이 완료 될 때
    * ppdu의 preamble을 통해 전처리 된 Aggregation 정보를 기반으로 A-mpdu 전송이 완료 될 때, 전송된 데이터가 A-mpdu 인 경우
  * 따라서, 수신 정보를 실제 mpdu 전송에 대한 신호가 수신되는 조건에서 ns3-analyzer에 인자 값으로 전달해야됨

### 3. ns3-analyzer implementaion
* 일단 필요한 정보 리스트업 부터 (처음에는 psdu 자체를 넘기려다가, 그냥 wifi module에서 값을 처리하고 넘기기로 함)
  * 현재 시간: Simulator::Now().As(Time::S)
  * 함수가 호출된 device의 MAC 주소: m_self
  * mpdu가 전송된 link 정보: m_linkId
  * mpdu header에 포함된 송신 및 수신 MAC 주소: mpdu_header.GetAddr2() 및 mpdu_header.GetAddr1()
  * payload size (ppdu의 payload 이니까, psdu size = mpdu size): mpdu->GetSize()
  * mpdu의 seq #: mpdu_header->GetSeqeunceNumber()
  * mpdu의 qos tid: mpdu_header->GetQosTid()
  * mpdu의 재전송 여부: mpdu_header->IsRetry()
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

### 4. ns3-analyzer vs Wireshark

### 5. 뭔가 이상한 로그 (A-mpdu에 대한 실제 수신 동작)
* ⭐ 로그가 특이하다

