## Appendix. C. Block ACK Request

### Objective
* 해당 문서에서는 ns-3.40 기반 Block ACK Request 프레임이 전송되는 과정을 분석함
* Retransmission event가 invoke 되는 원인 분석
* VI AC에 해당하는 TXOP를 획득 후 전송되는 초기 프레임 분석
* 앞서 분석한 내용 (Appendix. A. 및 B. 참고)과 공통되는 부분은 다루지 않음

### Preview
* 먼저, Appendix B. 에서 기술한 대로 VI AC에 해당하는 TXOP를 획득하고 전송한 초기 프레임의 존재로 인해 재전송되는 프레임이 원본 프레임의 전체에 대해 aggregation을 수행하지 못하는 상황 존재
* 초기 프레임의 존재를 확인하기 위해 Wireshark 기반 pcap 파일 로그 분석이 필요함
  * ns-3에서는 분석이 힘듬
    * invoke되는 event가 독립적이므로, 공통적인 요소를 기반으로 debug를 수행해야 되는데 ns::Time이 대표적
    * 그럼에도 불구하고, Wireshark 기반 pcap 캡처 로그와 ns-3 ns::Time 간의 동기화가 안맞아서 정확한 시간을 기반으로 breakpoint를 걸기 어려움

* No. 11 로그 분석 
```
1. 1.045457s STA1 -> AP (AC_VI, A-MPDU ID 39: #174 ~ #202) A-mpdu 송신
2. 1.084516s STA1 -> AP #174 ~ #201 A-mpdu 재전송
3. 1.090517s AP에서 STA1가 재전송한 #174 ~ #201 A-mpdu 수신
```
* STA1 -> AP link1을 통해 전송한 wlan seq #174에 해당하는 A-mpdu 재전송 부분은 아래 그림과 같음

<p align="center">  
  <img src="https://github.com/user-attachments/assets/1fa06074-5298-4574-906b-02da1bb0d7c2" width="40%">  
</p>

* 예쁘게 정리해보면
```
1. Time: 1.084382s / Src: 00:00:00:00:00:02 / Dst: 00:00:00:00:00:08 / length: 56 / Info: 802.11 Block Ack Req
2. Time: 1.084506s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 80 / Info: 802.11 Block Ack
3. Time: 1.084516s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400
...
n. Time: 1.088461s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 80 / Info: 802.11 Block Ack
```

* 정보 1: 1.084516s 시점에 전송한 A-mpdu에 대한 Block Ack을 1.088461s에 수신 받았으며, 해당 시간 간격은 3.945ms 이다.
  * 그럼 TXOP limit을 기반으로 계산한 TXOP의 유효 시간은 두가지 관점에서 해석을 할 수 있음
  * 해석 1. TXOP 유효 시간 (available Time)은 송신 device가 보낸 프레임에 대한 ACK 프레임을 수신할 때까지의 시간
  * 해석 2. TXOP 유효 시간 (available Time)은 송신 device가 보낸 프레임에 대해 수신 device가 해당 프레임을 수신할 때까지의 시간
  * 참고로, Appendix B. 에서 계산한 TXOP의 유효 시간은 3.906ms임
  * 따라서, 해당 관점에서는 해석 2에 조금 더 가까운 것 같음
  * 근데, 확실한 건 아닌게, 엄연히 송신 device가 계산한 유효 시간은 '예상'에 불과함 지연이 발생할 가능성이 충분히 있음
  * 표준문서를 좀 찾아봤는데, 해당 내용에 대해 자세히 기술된 내용은 없었음 (교수님께 한번 여쭤봐야겠음)

* 정보 2: TXOP를 획득한 후 초기 프레임으로 보낸 프레임은 'BA Req' 프레임임
  * Appendix. B. 에서 예상한대로 1500 byte보다 훨씬 작은 크기를 가짐 (56 byte)
  * BA Req 프레임의 전송 시점 (1.084382s)과 재전송된 A-mpdu에 대한 BA를 받은 시점 (1.088461s)과의 시간 간격은 4.079ms임
  * 참고로, VI TXOP limit의 값은 4.096ms임
  * 따라서, 해당 관점에서는 해석 1에 조금 더 가까운 것 같음

* 관점에 따라 해석이 조금 다를 수 있지만, 아무튼 코드 분석은 해야됨 (목적은 잡고 들어가자)
  * 목적 1. 우선, BA Req frame이 VI에 해당하는 TXOP를 획득하고 전송한 초기 프레임이 맞는지 확인해야 됨 (아닐수도 있으니..)
  * 목적 2. BA Req frame이 전송되면 왜 retranmission event가 invoke 되는지 확인해야 됨

* 자 그럼, breakpoint를 걸어야 되는데... 어디에 어떻게 걸 것인가가 관건임 (일반적인 mpdu가 아니기 때문에 header를 기반으로 하기 힘듬)
* 전체 flow를 보기 위해 최하위 계층에 wifi-phy.cc에 걸어보려고 시도 (뭐, psdu size를 100이하 설정하는 등...)
  * 근데, BA Req frame이 전송되는 건 ns3::WifiPhy::Send 함수를 통해서 전송 되는게 아닌거 같음
  * 그래서 그냥 노가다 하기로 했음 (머리가 나쁘면 노가다를 해야지.. 그래도 근거가 있는 노가다..)
  * 여태까지 나온 MAC 계층 클래스 중에서 blockack과 관련된 모든 function 다 찾음
* 코드 분석 시작!
 
### 1. ns3::HeFrameExchangeManager::BlockAckTimeout (중요도 중, 찾느라고 진짜 애먹음)
```c
void
HeFrameExchangeManager::BlockAckTimeout(Ptr<WifiPsdu> psdu, const WifiTxVector& txVector)
{
    NS_LOG_FUNCTION(this << *psdu << txVector);

    /* 추가 */
    auto ptr = psdu->begin();
    for (int i = 0; i < (int) psdu->GetNMpdus(); i++) {
        auto mpdu_header = ptr[i]->GetHeader();
        if (mpdu_header.GetSequenceNumber() == 174 && mpdu_header.GetQosTid() == 5) {
            NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": Wlsn Seq# 174, Qos Tid 5 mpdu's BlockAck Timeout");
        }
    }
    /* 추가 */

    VhtFrameExchangeManager::BlockAckTimeout(psdu, txVector); // STEP INTO

    // If a Block Ack is missed in response to a DL MU PPDU requiring acknowledgment
    // in SU format, we have to set the Retry flag for all transmitted MPDUs that have
    // not been acknowledged nor discarded and clear m_psduMap since the transmission failed.
    for (auto& psdu : m_psduMap)
    {
        for (auto& mpdu : *PeekPointer(psdu.second))
        {
            if (mpdu->IsQueued())
            {
                mpdu->GetHeader().SetRetry();
            }
        }
    }
    m_psduMap.clear();
}
```
* Function 이름 보면 유추할 수 있듯이 전송했던 psdu에 대해 기대한 BA가 안오면 (즉, timeout이 발생하면) event가 invoke 되는 형식임
* 우리는 wlan seq #174를 가지는 mpdu의 BA 프레임 전송 로그가 필요하므로, 추가 코드를 기반으로 breakpoint 걸어줌
* 이후, VhtFrameExchangeManager::BlockAckTimeout(psdu, txVector) << 여기로 STEP INTO
  
### 2. HtFrameExchangeManager::BlockAckTimeout (중요도 중, 서브루틴이 많음)
```c
void
HtFrameExchangeManager::BlockAckTimeout(Ptr<WifiPsdu> psdu, const WifiTxVector& txVector)
{
    NS_LOG_FUNCTION(this << *psdu << txVector);

    GetWifiRemoteStationManager()->ReportDataFailed(*psdu->begin());

    bool resetCw;
    MissedBlockAck(psdu, txVector, resetCw);

    NS_ASSERT(m_edca);

    if (resetCw)
    {
        m_edca->ResetCw(m_linkId);
    }
    else
    {
        m_edca->UpdateFailedCw(m_linkId);
    }

    m_psdu = nullptr;
    TransmissionFailed();
}
```
* 서브루틴 1. GetWifiRemoteStationManager()->ReportDataFailed(*psdu->begin());
  * 2.1. 참고
* 서브루틴 2. MissedBlockAck(psdu, txVector, resetCw);
  * 전송했던 A-mpdu에 대한 BlockAck이 손실됨에 따라 처리해야하는 로직을 포함 (⭐ 제일 중요!!)
  * 2.2. 참고
* 서브루틴 3. Contention Window 재설정
  * 서브루틴 2. MissedBlockAck의 인자 값으로 넘기는 bool 변수 resetCw의 상태에 따라 Contention Window 조정하는 로직을 포함
  * 2.3. 참고
* 서브루틴 4. TransmissionFailed();
  * A-mpdu 전송 실패에 따른 Channel State 관리하는 로직을 포함
  * 2.4. 참고

### 2.1. ns3::WifiRemoteStationManager::ReportDataFailed (중요도 하)
```c
void
WifiRemoteStationManager::ReportDataFailed(Ptr<const WifiMpdu> mpdu)
{
    NS_LOG_FUNCTION(this << *mpdu);
    NS_ASSERT(!mpdu->GetHeader().GetAddr1().IsGroup());
    AcIndex ac =
        QosUtilsMapTidToAc((mpdu->GetHeader().IsQosData()) ? mpdu->GetHeader().GetQosTid() : 0);
    bool longMpdu = (mpdu->GetSize() > m_rtsCtsThreshold);
    if (longMpdu)
    {
        m_slrc[ac]++;
    }
    else
    {
        m_ssrc[ac]++;
    }
    m_macTxDataFailed(mpdu->GetHeader().GetAddr1());
    DoReportDataFailed(Lookup(mpdu->GetHeader().GetAddr1()));
}
```
* 
