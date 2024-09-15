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
  * 그래서 여태까지 나온 MAC 계층 클래스 중에서 blockack과 관련된 모든 function 다 찾아보기로 함 
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
* 전송했던 psdu에 대해 기대한 BA가 수신되지 않으면 (즉, timeout이 발생하면) event가 invoke 되는 형식임
* 우리는 wlan seq #174를 가지는 mpdu의 BA 프레임 전송 로그가 필요하므로, 추가 코드를 기반으로 breakpoint 걸어줌
* 이후, VhtFrameExchangeManager::BlockAckTimeout(psdu, txVector) << 여기로 STEP INTO
  
### 2. ns3::HtFrameExchangeManager::BlockAckTimeout (중요도 중, 서브루틴이 많음)
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
  * 2.2. 참고
* 서브루틴 3. Contention Window 재설정
  * 2.3.1. 및 2.3.2. 참고
* 서브루틴 4. TransmissionFailed();
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
* RemoteStationManager 동일 link에 association되어 있는 모든 device를 관리하는 클래스
* m_macTxDataFailed -> ns-3 Trace source, 별도 동작 없음
* DoReportDataFailed: C++ virtual method, 손실된 mpdu를 포함하고 있는 주소와 association 되어 있는 device (즉, STA1)를 인자 값으로 전달, 별도 동작 없음 (나중에 집가서 까먹지 말고 해봐야 됨)

### 2.2. ns3::HtFrameExchangeManager::MissedBlockAck (⭐ 중요도 최상)
```c
void
HtFrameExchangeManager::MissedBlockAck(Ptr<WifiPsdu> psdu,
                                       const WifiTxVector& txVector,
                                       bool& resetCw)
{
    NS_LOG_FUNCTION(this << psdu << txVector << resetCw);
 
    auto recipient = psdu->GetAddr1();
    auto recipientMld = GetWifiRemoteStationManager()->GetMldAddress(recipient).value_or(recipient);
    bool isBar;
    uint8_t tid;
 
    if (psdu->GetNMpdus() == 1 && psdu->GetHeader(0).IsBlockAckReq())
    {
        isBar = true;
        CtrlBAckRequestHeader baReqHdr;
        psdu->GetPayload(0)->PeekHeader(baReqHdr);
        tid = baReqHdr.GetTidInfo();
    }
    else
    {
        isBar = false;
        GetWifiRemoteStationManager()
            ->ReportAmpduTxStatus(recipient, 0, psdu->GetNMpdus(), 0, 0, txVector);
        std::set<uint8_t> tids = psdu->GetTids();
        NS_ABORT_MSG_IF(tids.size() > 1, "Multi-TID A-MPDUs not handled here");
        NS_ASSERT(!tids.empty());
        tid = *tids.begin();
    }
 
    Ptr<QosTxop> edca = m_mac->GetQosTxop(tid);
 
    if (edca->UseExplicitBarAfterMissedBlockAck() || isBar)
    {
        // we have to send a BlockAckReq, if needed
        if (GetBaManager(tid)->NeedBarRetransmission(tid, recipientMld))
        {
            NS_LOG_DEBUG("Missed Block Ack, transmit a BlockAckReq");
            if (isBar)
            {
                psdu->GetHeader(0).SetRetry();
            }
            else
            {
                // missed block ack after data frame with Implicit BAR Ack policy
                auto [reqHdr, hdr] = edca->PrepareBlockAckRequest(recipient, tid);
                GetBaManager(tid)->ScheduleBar(reqHdr, hdr);
            }
            resetCw = false;
        }
        else
        {
            NS_LOG_DEBUG("Missed Block Ack, do not transmit a BlockAckReq");
            // if a BA agreement exists, we can get here if there is no outstanding
            // MPDU whose lifetime has not expired yet.
            GetWifiRemoteStationManager()->ReportFinalDataFailed(*psdu->begin());
            if (isBar)
            {
                DequeuePsdu(psdu);
            }
            if (m_mac->GetBaAgreementEstablishedAsOriginator(recipient, tid))
            {
                // schedule a BlockAckRequest to be sent only if there are data frames queued
                // for this recipient
                GetBaManager(tid)->AddToSendBarIfDataQueuedList(recipientMld, tid);
            }
            resetCw = true;
        }
    }
    else
    {
        // we have to retransmit the data frames, if needed
        if (!GetWifiRemoteStationManager()->NeedRetransmission(*psdu->begin()))
        {
            NS_LOG_DEBUG("Missed Block Ack, do not retransmit the data frames");
            GetWifiRemoteStationManager()->ReportFinalDataFailed(*psdu->begin());
            for (const auto& mpdu : *PeekPointer(psdu))
            {
                NotifyPacketDiscarded(mpdu);
                DequeueMpdu(mpdu);
            }
            resetCw = true;
        }
        else
        {
            NS_LOG_DEBUG("Missed Block Ack, retransmit data frames");
            GetBaManager(tid)->NotifyMissedBlockAck(m_linkId, recipientMld, tid);
            resetCw = false;
        }
    }
}
```
* 전송했던 A-mpdu에 대한 BlockAck이 손실됨에 따라 처리해야하는 로직을 포함
* Supplementary: Implicit BlockAckReq vs Explicit BlockAckReq
  * BlockAckReq: how the acknowledgment process is initiated and handled during data transmission in wireless communication
  * Implicit BlockAckReq: the sender assumes that the receiver will automatically acknowledge the reception of data blocks without needing an explicit request to trigger the acknowledgment process
  * Explicit BlockAckReq: the sender explicitly requests a block acknowledgment from the receiver after transmitting a series of data frames

* BA Req 프레임의 목적에 대해 생각을 해보면 이해하기 쉬움 (손실의 관점에서 생각하면 어려움)
  * 목적: 송신기에서 전송한 A-mpdu에 포함된 각 mpdu 들에 대해 ack 응답을 받기 위함
  * 예시 1: STA1->AP A-mpdu, STA1->AP BA Req, AP->STA1 BA (여기서 BA Req는 explicit에 해당함)
  * 예시 2: STA1->AP A-mpdu, AP->STA1 BA (여기서 BA Req는 implicit에 해당함, explicitly하게 BA Req를 보내지 않고도 BA를 수신 받음)
  * 예시 1이 예시 2보다 신뢰성이 더 높음 (여기서 신뢰성이 높다는 관점은 수신자 입장에서 송신자의 A-mpdu는 수신 받지 못하고 BA Req를 수신 받았으면, 빠른 대처가 가능함. 왜냐하면 timeout event의 invoke로 인한 BA Req frame이 아니기 때문)
  * 예시 2가 예시 1보다 overhead가 작음 (explicitly하게 보내는 BA Req - BA를 통해 channel을 occupy하기 때문에)
  * 따라서, 일반적으로 implicit 방식을 사용하며, 원본 A-mpdu에 대한 timeout이 발생했을 때는 explicit 방식을 사용
  * 결론적으로, 송신기의 BA Req frame 전송 여부로 Block Ack mechanism을 분류할 수 있음 (그럼 재전송은? 당연히 Explicit Block Ack임)
  
* 이제 코드를 보면, Case 분류하기 전에 두가지 변수에 대해 값을 할당함
  * 변수1 (tid): 전송했던 psdu에 해당하는 tid 값을 가져오고, 해당 값을 기반으로 QosTxop object에 접근 (MAC Queue handling을 위해)
  * 변수2 (isBar): 전송했던 psdu가 이미 BA Req frame인 경우 True 할당, 해당 값을 기반으로 Case 분류를 수행함
* Case Study
  * Case 1. 전송한 psdu가 이미 BA Req frame이며, BA Req frame의 전송이 필요한 경우
    * psdu의 Retry header=1, resetCw=False
  * Case 2. 전송한 psdu가 BA Req frame이 아니고, Explicit BA Request mechanism을 사용하며, BA Req frame의 전송이 필요한 경우
    * BA Req frame 생성 및 schedule, resetCw=False, 2.2.1. ~ 2.2.3. 참고
  * Case 3. 전송했던 psdu가 이미 BA Req frame이지만, BA Req frame의 전송이 필요하지 않은 경우
    * Dequeue, resetCw=True
  * Case 4. Implicit BA Request mechanism을 사용하는 경우
    * 해당 x

### 2.2.1. ns3::BlockAckManager::NeedBarRetransmission
```c
bool
BlockAckManager::NeedBarRetransmission(uint8_t tid, const Mac48Address& recipient)
{
    auto it = m_originatorAgreements.find({recipient, tid});
    if (it == m_originatorAgreements.end() || !it->second.first.IsEstablished())
    {
        // If the inactivity timer has expired, QosTxop::SendDelbaFrame has been called and
        // has destroyed the agreement, hence we get here and correctly return false
        return false;
    }
 
    Time now = Simulator::Now();
 
    // A BAR needs to be retransmitted if there is at least a non-expired in flight MPDU
    for (auto mpduIt = it->second.second.begin(); mpduIt != it->second.second.end();)
    {
        // remove MPDU if old or with expired lifetime
        mpduIt = HandleInFlightMpdu(SINGLE_LINK_OP_ID, mpduIt, STAY_INFLIGHT, it, now);
 
        if (mpduIt != it->second.second.begin())
        {
            // the MPDU has not been removed
            return true;
        }
    }
 
    return false;
}
```
* 집가서 it << 이거 structure 봐야함
* 수신기 (originator)의 MAC Queue에 존재하는 mpdu가 inflight 상태 (lifetime이 expire 되지 않은 상태)이면 true 반환
* 근데 BAR transmission이 아니라 retransmission인거 한번 확인해봐야함
  * 원본 A-mpdu 전송에서 implicit BA Req 방식을 사용했으며, 손실에 따른 첫 번째 BA req전송에도 BAR retransmission이 호출됨

### 2.2.2. ns3::QosTxop::PrepareBlockAckRequest
```c
std::pair<CtrlBAckRequestHeader, WifiMacHeader>
QosTxop::PrepareBlockAckRequest(Mac48Address recipient, uint8_t tid) const
{
    NS_LOG_FUNCTION(this << recipient << +tid);
    NS_ASSERT(QosUtilsMapTidToAc(tid) == m_ac);
 
    auto recipientMld = m_mac->GetMldAddress(recipient);
 
    CtrlBAckRequestHeader reqHdr =
        m_baManager->GetBlockAckReqHeader(recipientMld.value_or(recipient), tid);
 
    WifiMacHeader hdr;
    hdr.SetType(WIFI_MAC_CTL_BACKREQ);
    hdr.SetAddr1(recipient);
    hdr.SetAddr2(m_mac->GetLocalAddress(recipient));
    hdr.SetDsNotTo();
    hdr.SetDsNotFrom();
    hdr.SetNoRetry();
    hdr.SetNoMoreFragments();
 
    return {reqHdr, hdr};
}
```
* BAR Req frame 생성 후 C++ std::pair 자료형으로 반환

### 2.2.3. ns3::BlockAckManager::ScheduleBar
```c
void
BlockAckManager::ScheduleBar(const CtrlBAckRequestHeader& reqHdr, const WifiMacHeader& hdr)
{
    NS_LOG_FUNCTION(this << reqHdr << hdr);
 
    uint8_t tid = reqHdr.GetTidInfo();
 
    WifiContainerQueueId queueId(WIFI_CTL_QUEUE, WIFI_UNICAST, hdr.GetAddr1(), std::nullopt);
    auto pkt = Create<Packet>();
    pkt->AddHeader(reqHdr);
    Ptr<WifiMpdu> item = nullptr;
 
    // if a BAR for the given agreement is present, replace it with the new one
    while ((item = m_queue->PeekByQueueId(queueId, item)))
    {
        if (item->GetHeader().IsBlockAckReq() && item->GetHeader().GetAddr1() == hdr.GetAddr1())
        {
            CtrlBAckRequestHeader otherHdr;
            item->GetPacket()->PeekHeader(otherHdr);
            if (otherHdr.GetTidInfo() == tid)
            {
                auto bar = Create<WifiMpdu>(pkt, hdr, item->GetTimestamp());
                // replace item with bar
                m_queue->Replace(item, bar);
                return;
            }
        }
    }
 
    m_queue->Enqueue(Create<WifiMpdu>(pkt, hdr));
}
```
* 생성된 BA req frame을 WifiMacQueue에 Enqueue (이때, 같은 recipient 주소를 갖고 있는 BA Req frame이 이미 존재 할 경우 replace 처리)
* 따라서, 재전송을 수행하기 위한 BA Req frame은 Mac queue에 enqueue되어 향후 해당 TXOP를 획득할 때 자연스럽게 전송되게 됨.
  * 근데, 왜 BA Req 프레임 하나만 날아가는거지? << 이거 생각해 봐야됨
  * BA Req + data <-> BA 가 아닌 BA req <-> BA + data <-> BA인 이유

### 2.3.1. ns3::Txop::ResetCw (중요도 중)
```c
void
Txop::ResetCw(uint8_t linkId)
{
    NS_LOG_FUNCTION(this);
    auto& link = GetLink(linkId);
    link.cw = GetMinCw(linkId);
    m_cwTrace(link.cw, linkId);
}
```
* 서브루틴 2. MissedBlockAck의 인자 값으로 넘기는 bool 변수 resetCw의 상태에 따라 Contention Window 조정하는 로직을 포함

### 2.3.2. ns3::Txop::UpdateFailedCw (중요도 중)
```c
void
Txop::UpdateFailedCw(uint8_t linkId)
{
    NS_LOG_FUNCTION(this);
    auto& link = GetLink(linkId);
    // see 802.11-2012, section 9.19.2.5
    link.cw = std::min(2 * (link.cw + 1) - 1, GetMaxCw(linkId));
    // if the MU EDCA timer is running, CW cannot be less than MU CW min
    link.cw = std::max(link.cw, GetMinCw(linkId));
    m_cwTrace(link.cw, linkId);
}
```
* 서브루틴 2. MissedBlockAck의 인자 값으로 넘기는 bool 변수 resetCw의 상태에 따라 Contention Window 조정하는 로직을 포함

### 2.4. ns3::QosFrameExchangeManager::TransmissionFailed (중요도 상)
```c
void
QosFrameExchangeManager::TransmissionFailed()
{
    NS_LOG_FUNCTION(this);
 
    // TODO This will be removed once no Txop is installed on a QoS station
    if (!m_edca)
    {
        FrameExchangeManager::TransmissionFailed();
        return;
    }
 
    if (m_initialFrame)
    {
        // The backoff procedure shall be invoked by an EDCAF when the transmission
        // of an MPDU in the initial PPDU of a TXOP fails (Sec. 10.22.2.2 of 802.11-2016)
        NS_LOG_DEBUG("TX of the initial frame of a TXOP failed: terminate TXOP");
        NotifyChannelReleased(m_edca);
        m_edca = nullptr;
    }
    else
    {
        NS_ASSERT_MSG(m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive(),
                      "Cannot transmit more than one frame if TXOP Limit is zero");
 
        // A STA can perform a PIFS recovery or perform a backoff as a response to
        // transmission failure within a TXOP. How it chooses between these two is
        // implementation dependent. (Sec. 10.22.2.2 of 802.11-2016)
        if (m_pifsRecovery)
        {
            // we can continue the TXOP if the carrier sense mechanism indicates that
            // the medium is idle in a PIFS
            NS_LOG_DEBUG("TX of a non-initial frame of a TXOP failed: perform PIFS recovery");
            NS_ASSERT(!m_pifsRecoveryEvent.IsRunning());
            m_pifsRecoveryEvent =
                Simulator::Schedule(m_phy->GetPifs(), &QosFrameExchangeManager::PifsRecovery, this);
        }
        else
        {
            // In order not to terminate (yet) the TXOP, we call the NotifyChannelReleased
            // method of the Txop class, which only generates a new backoff value and
            // requests channel access if needed,
            NS_LOG_DEBUG("TX of a non-initial frame of a TXOP failed: invoke backoff");
            m_edca->Txop::NotifyChannelReleased(m_linkId);
            m_edcaBackingOff = m_edca;
            m_edca = nullptr;
        }
    }
    m_initialFrame = false;
}
```
* A-mpdu 전송 실패에 따른 Channel State 관리하는 로직을 포함
