## Appendix. A. AC_BE Retransmssion

### Objective
* AC_BE 및 AC_VI의 서로 다른 재전송 과정이 발생하는 원인을 찾기 위해
* 해당 문서에서는 ns-3.40 기반 AC_BE의 재전송 수행 과정을 분석함

### Preview
* No. 13 로그 분석
```
1. 1.051254s STA2 -> AP (AC_BE, A-MPDU ID 42: #234 ~ #272) A-mpdu 송신
2. 1.074739s STA2 -> AP #234 ~ #272 A-mpdu 재전송
3. 1.082250s AP에서 STA2가 재전송한 #234 ~ #272 A-mpdu 수신
```

* 전체 flow를 보기위해 최하위 계층 wifi-phy.cc의 Send()에서 breakpoint를 걸어야함
  * 아래 코드를 통해 wifi-phy.cc에서 ppdu->psdu->mpdu_list->mpdu 접근 가능
```c
auto ptr = ppdu->GetPsdu()->begin();
for(int i = 0; i < (int)ppdu->GetPsdu()->GetNMpdus(); i++){
  NS_LOG_UNCOND(ptr[i]->GetHeader());
}
```

* 여기서 mpdu header의 retry, wlan seq#, AC 확인은 아래 코드로 수행 가능
```c
ptr[i]->GetHeader().IsRetry(); // 1: retry, 0: no retry
ptr[i]->GetHeader().GetSequenceNumber(); // wlan seq #
ptr[i]->GetHeader().GetQosTid(); // 3: AC_BE, 5: AC_VI
```

* 따라서, debug를 위해 아래와 같은 코드를 작성하고 실행
```c
auto ptr = ppdu->GetPsdu()->begin();
for (int i = 0; i < (int)ppdu->GetPsdu()->GetNMpdus(); i++){
  auto mpdu_header = ptr[i]->GetHeader();
  if (mpdu_header.GetSequenceNumber() == 234 && mpdu_header.GetQosTid() == 3){
    if (mpdu_header.IsRetry() == 1)
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": retry");
    else
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": No retry");
  }
}
```

> +1.09477s: No retry  
> +1.11826s: retry
  
* 시간 동기화가 안맞다. wifi-phy.cc에서 전송한 시간과 pcap에서 캡처된 시간이 다른 대신, 간격은 유사하다 (즉, 같은 로그임)
   * 아마도 ns-3에서 pcap 파일을 생성할 때 ns3::Time 값을 write 하는 위치가 다를거라고 예상)
  * ns-3 wifi-phy.cc: 1.11826s - 1.09477s = 23.49ms
  * wireshark: 1.074739s - 1.051254s = 23.485ms
  
* 이제 wifi-phy.cc에 breakpoint 걸고 call stack을 보면

<p align="center">  
  <img src="https://github.com/user-attachments/assets/71dd9b80-1c03-4f7e-a96e-8adb51d30208" width="40%">  
</p>

* 하나씩 순서대로 뜯자. (서브루틴으로 들어가는 code의 line에 'STEP_INTO' 표시)
  
### 1. ns3::ChannelAccessManager::AccessTimeout (중요도 하)
```c
void
ChannelAccessManager::AccessTimeout()
{
  NS_LOG_FUNCTION(this);
  UpdateBackoff();
  DoGrantDcfAccess(); // STEP_INTO
  DoRestartAccessTimeoutIfNeeded();
}
```
* backoff update 말고 뭐 없다 패스

### 2. ns3::ChannelAccessManager::DoGrantDcfAccess (⭐ 중요도 상) 
```c
void
ChannelAccessManager::DoGrantDcfAccess()
{
    NS_LOG_FUNCTION(this);
    uint32_t k = 0;
    Time now = Simulator::Now();
    for (auto i = m_txops.begin(); i != m_txops.end(); k++)
    {
        Ptr<Txop> txop = *i;
        if (txop->GetAccessStatus(m_linkId) == Txop::REQUESTED &&
            (!txop->IsQosTxop() || !StaticCast<QosTxop>(txop)->EdcaDisabled(m_linkId)) &&
            GetBackoffEndFor(txop) <= now)
        {
            /**
             * This is the first Txop we find with an expired backoff and which
             * needs access to the medium. i.e., it has data to send.
             */
            NS_LOG_DEBUG("dcf " << k << " needs access. backoff expired. access granted. slots="
                                << txop->GetBackoffSlots(m_linkId));
            i++; // go to the next item in the list.
            k++;
            std::vector<Ptr<Txop>> internalCollisionTxops;
            for (auto j = i; j != m_txops.end(); j++, k++)
            {
                Ptr<Txop> otherTxop = *j;
                if (otherTxop->GetAccessStatus(m_linkId) == Txop::REQUESTED &&
                    GetBackoffEndFor(otherTxop) <= now)
                {
                    NS_LOG_DEBUG(
                        "dcf " << k << " needs access. backoff expired. internal collision. slots="
                               << otherTxop->GetBackoffSlots(m_linkId));
                    /**
                     * all other Txops with a lower priority whose backoff
                     * has expired and which needed access to the medium
                     * must be notified that we did get an internal collision.
                     */
                    internalCollisionTxops.push_back(otherTxop);
                }
            }

            /**
             * Now, we notify all of these changes in one go if the EDCAF winning
             * the contention actually transmitted a frame. It is necessary to
             * perform first the calculations of which Txops are colliding and then
             * only apply the changes because applying the changes through notification
             * could change the global state of the manager, and, thus, could change
             * the result of the calculations.
             */
            NS_ASSERT(m_feManager);
            // If we are operating on an OFDM channel wider than 20 MHz, find the largest
            // idle primary channel and pass its width to the FrameExchangeManager, so that
            // the latter can transmit PPDUs of the appropriate width (see Section 10.23.2.5
            // of IEEE 802.11-2020).
            auto interval = (m_phy->GetPhyBand() == WIFI_PHY_BAND_2_4GHZ)
                                ? GetSifs() + 2 * GetSlot()
                                : m_phy->GetPifs();
            auto width = (m_phy->GetOperatingChannel().IsOfdm() && m_phy->GetChannelWidth() > 20)
                             ? GetLargestIdlePrimaryChannel(interval, now)
                             : m_phy->GetChannelWidth();
            if (m_feManager->StartTransmission(txop, width)) // STEP_INTO
            {
                for (auto& collidingTxop : internalCollisionTxops)
                {
                    m_feManager->NotifyInternalCollision(collidingTxop);
                }
                break;
            }
            else
            {
                // reset the current state to the EDCAF that won the contention
                // but did not transmit anything
                i--;
                k = std::distance(m_txops.begin(), i);
            }
        }
        i++;
    }
}
```
* 먼저, m_txops는 각 AC 별 TXOP를 뜻함. 즉, AC_VO txop, AC_VI txop, AC_BE txop, AC_BK txop 총 4개가 존재
* 전송 시작 전, EDCA internal contention 먼저 필터링
* 만약, 여러 AC에 해당하는 TXOP들이 동시에 backoff 만료 및 매체 접근이 필요할 때 상위 AC에 해당하는 TXOP 먼저 획득 (나머지는 내부 경쟁 패배)
* 반대로 말하면, 하위 AC에 해당하는 TXOP를 얻으려면 상위 AC에 해당하는 TXOP의 backoff가 진행 중이거나 매체 접근 요청 상태가 아니어야함
* STEP_INTO
  * 결론적으로, 내부 경쟁에서 승리한 EDCAF (즉, 특정 AC에 해당하는 txop)에 대한 전송을 시작함.
* Note: 내부 경쟁에서 승리한 TXOP의 전송이 완료된 후 (성공하거나 실패하거나 어쨋든) 나머지 internal contention이 발생 TXOP에 대해 계산 (backoff 재계산 등) 수행
* 구현 이슈인거 같음: 변경 사항을 획득한 txop의 전송 전에 적용하면 전역 변수 값 변경과 같은 문제 야기
  
### 3. ns3::EhtFrameExchangeManager::StartTransmission (중요도 하)
```c
bool
EhtFrameExchangeManager::StartTransmission(Ptr<Txop> edca, uint16_t allowedWidth)
{
    NS_LOG_FUNCTION(this << edca << allowedWidth);

    auto started = HeFrameExchangeManager::StartTransmission(edca, allowedWidth); // STEP_INTO

    if (started && m_staMac && m_staMac->IsEmlsrLink(m_linkId))
    {
        // notify the EMLSR Manager of the UL TXOP start on an EMLSR link
        NS_ASSERT(m_staMac->GetEmlsrManager());
        m_staMac->GetEmlsrManager()->NotifyUlTxopStart(m_linkId);
    }

    return started;
}
```
* EMLSR (Enhanced Multi-link Single-Radio)
* 하나의 RF(i.e., Antenna)가 여러개의 대역을 지원하는 경우
* 예를 들어, 단일 RF에서 1s ~ 10s는 2.4GHz 대역에서 전송을 하고, 11s ~ 20s는 5GHz 대역에서 전송을 하는 것과 같음 (동시성 x)
* 관련 없음. 패스.

### 4. ns3::QosFrameExchangeManager::StartTransmission (중요도 하)
```c
bool
QosFrameExchangeManager::StartTransmission(Ptr<Txop> edca, uint16_t allowedWidth)
{
    NS_LOG_FUNCTION(this << edca << allowedWidth);

    if (m_pifsRecoveryEvent.IsRunning())
    {
        // Another AC (having AIFS=1 or lower, if the user changed the default settings)
        // gained channel access while performing PIFS recovery. Abort PIFS recovery
        CancelPifsRecovery();
    }

    // TODO This will become an assert once no Txop is installed on a QoS station
    if (!edca->IsQosTxop())
    {
        m_edca = nullptr;
        return FrameExchangeManager::StartTransmission(edca, allowedWidth);
    }

    m_allowedWidth = allowedWidth;
    auto qosTxop = StaticCast<QosTxop>(edca);
    return StartTransmission(qosTxop, qosTxop->GetTxopLimit(m_linkId)); // STEP_INTO
}
```
* PCF: AP가 통신 관장, STA 들에게 데이터를 전송할 수 있는 기회 할당 (주로 EDCA, Real-time trafifc에서 사용하는 통신 방식)
* PIFS: DCF 모드가 아닌 PCF 모드에서 주로 사용되며, DCF 보다는 짧고, SIFS 보다는 길다.
* 주로 PCF 통신 모드가 끝난 후 STA 들에게 제어 프레임을 전송할 때 빠른 채널 복구를 위해 사용
* 관련 없음. 패스

### 5. ns3::QosFrameExchangeManager::StartTransmission (⭐ 중요도 상)
```c
bool
QosFrameExchangeManager::StartTransmission(Ptr<QosTxop> edca, Time txopDuration)
{
    NS_LOG_FUNCTION(this << edca << txopDuration);

    if (m_pifsRecoveryEvent.IsRunning())
    {
        // Another AC (having AIFS=1 or lower, if the user changed the default settings)
        // gained channel access while performing PIFS recovery. Abort PIFS recovery
        CancelPifsRecovery();
    }

    if (m_txTimer.IsRunning())
    {
        m_txTimer.Cancel();
    }
    m_dcf = edca;
    m_edca = edca;

    // We check if this EDCAF invoked the backoff procedure (without terminating
    // the TXOP) because the transmission of a non-initial frame of a TXOP failed
    bool backingOff = (m_edcaBackingOff == m_edca);

    if (backingOff)
    {
        NS_ASSERT(m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive());
        NS_ASSERT(m_edca->IsTxopStarted(m_linkId));
        NS_ASSERT(!m_pifsRecovery);
        NS_ASSERT(!m_initialFrame);

        // clear the member variable
        m_edcaBackingOff = nullptr;
    }

    if (m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive())
    {
        // TXOP limit is not null. We have to check if this EDCAF is starting a
        // new TXOP. This includes the case when the transmission of a non-initial
        // frame of a TXOP failed and backoff was invoked without terminating the
        // TXOP. In such a case, we assume that a new TXOP is being started if it
        // elapsed more than TXOPlimit since the start of the paused TXOP. Note
        // that GetRemainingTxop returns 0 iff Now - TXOPstart >= TXOPlimit
        if (!m_edca->IsTxopStarted(m_linkId) ||
            (backingOff && m_edca->GetRemainingTxop(m_linkId).IsZero()))
        {
            // starting a new TXOP
            m_edca->NotifyChannelAccessed(m_linkId, txopDuration);

            if (StartFrameExchange(m_edca, txopDuration, true))
            {
                m_initialFrame = true;
                return true;
            }

            // TXOP not even started, return false
            NS_LOG_DEBUG("No frame transmitted");
            NotifyChannelReleased(m_edca);
            m_edca = nullptr;
            return false;
        }

        // We are continuing a TXOP, check if we can transmit another frame
        NS_ASSERT(!m_initialFrame);

        if (!StartFrameExchange(m_edca, m_edca->GetRemainingTxop(m_linkId), false))
        {
            NS_LOG_DEBUG("Not enough remaining TXOP time");
            return SendCfEndIfNeeded();
        }

        return true;
    }

    // we get here if TXOP limit is null
    m_initialFrame = true;

    if (StartFrameExchange(m_edca, Time::Min(), true)) // STEP_INTO
    {
        m_edca->NotifyChannelAccessed(m_linkId, Seconds(0));
        return true;
    }

    NS_LOG_DEBUG("No frame transmitted");
    NotifyChannelReleased(m_edca);
    m_edca = nullptr;
    return false;
}
```    
* 조건 1. 획득한 TXOP의 TXOP limit의 값이 0이상인 경우 (VO 및 VI에 해당)
  * 조건 1.1 남은 TXOP limit 0인 경우 (즉, 획득한 TXOP에 대해 첫 번째 프레임을 전송하는 경우)
    * StartFrameExchange(TXOP, 할당된 시간)
  * 조건 1.2 남은 TXOP limit 값이 0이 아닌 경우 (즉, 획득한 TXOP에 대해 첫 번째 프레임을 전송하는 것이 아닌 경우)
    * StartFrameExchange(TXOP, 남은 시간)
    * 만약 전송 실패한 경우, 남은 TXOP의 시간이 프레임을 전송하기에 충분하지 않은 시간이므로, CF-End 프레임 전송)
* 조건 2. 획득한 TXOP의 TXOP limit의 값이 null인 경우 (STEP_INTO, BE 및 BK에 해당)
  * StartFrameExchange(TXOP, 0)
* 조건 3. 전송하지 않은 경우
  * return false
      
### 6. ns3::HeFrameExchangeManager::StartFrameExchange (중요도 하)
```c
bool
HeFrameExchangeManager::StartFrameExchange(Ptr<QosTxop> edca, Time availableTime, bool initialFrame)
{
    NS_LOG_FUNCTION(this << edca << availableTime << initialFrame);

    MultiUserScheduler::TxFormat txFormat = MultiUserScheduler::SU_TX;
    Ptr<const WifiMpdu> mpdu;

    /*
     * We consult the Multi-user Scheduler (if available) to know the type of transmission to make
     * if:
     * - there is no pending BlockAckReq to transmit
     * - either the AC queue is empty (the scheduler might select an UL MU transmission)
     *   or the next frame in the AC queue is a non-broadcast QoS data frame addressed to
     *   a receiver with which a BA agreement has been already established
     */
    if (m_muScheduler && !GetBar(edca->GetAccessCategory()) &&
        (!(mpdu = edca->PeekNextMpdu(m_linkId)) ||
         (mpdu->GetHeader().IsQosData() && !mpdu->GetHeader().GetAddr1().IsGroup() &&
          m_mac->GetBaAgreementEstablishedAsOriginator(mpdu->GetHeader().GetAddr1(),
                                                       mpdu->GetHeader().GetQosTid()))))
    {
        txFormat = m_muScheduler->NotifyAccessGranted(edca,
                                                      availableTime,
                                                      initialFrame,
                                                      m_allowedWidth,
                                                      m_linkId);
    }

    if (txFormat == MultiUserScheduler::SU_TX)
    {
        return VhtFrameExchangeManager::StartFrameExchange(edca, availableTime, initialFrame); // STEP_INTO
    }

    if (txFormat == MultiUserScheduler::DL_MU_TX)
    {
        if (m_muScheduler->GetDlMuInfo(m_linkId).psduMap.empty())
        {
            NS_LOG_DEBUG(
                "The Multi-user Scheduler returned DL_MU_TX with empty psduMap, do not transmit");
            return false;
        }

        SendPsduMapWithProtection(m_muScheduler->GetDlMuInfo(m_linkId).psduMap,
                                  m_muScheduler->GetDlMuInfo(m_linkId).txParams);
        return true;
    }

    if (txFormat == MultiUserScheduler::UL_MU_TX)
    {
        auto packet = Create<Packet>();
        packet->AddHeader(m_muScheduler->GetUlMuInfo(m_linkId).trigger);
        auto trigger = Create<WifiMpdu>(packet, m_muScheduler->GetUlMuInfo(m_linkId).macHdr);
        SendPsduMapWithProtection(
            WifiPsduMap{
                {SU_STA_ID,
                 GetWifiPsdu(trigger, m_muScheduler->GetUlMuInfo(m_linkId).txParams.m_txVector)}},
            m_muScheduler->GetUlMuInfo(m_linkId).txParams);
        return true;
    }

    return false;
}
```
* 802.11ax에서 지원하는 Multi-User Transmission 관련 내용
* SU_TX 사용함. 관련 없음. 패스.

### 7. ns3::HtFrameExchangeManager::StartFrameExchange (⭐ 중요도 최상)
```c
bool
HtFrameExchangeManager::StartFrameExchange(Ptr<QosTxop> edca, Time availableTime, bool initialFrame)
{
    NS_LOG_FUNCTION(this << edca << availableTime << initialFrame);

    // First, check if there is a BAR to be transmitted
    if (auto mpdu = GetBar(edca->GetAccessCategory());
        mpdu && SendMpduFromBaManager(mpdu, availableTime, initialFrame))
    {
        return true;
    }

    Ptr<WifiMpdu> peekedItem = edca->PeekNextMpdu(m_linkId);

    // Even though channel access is requested when the queue is not empty, at
    // the time channel access is granted the lifetime of the packet might be
    // expired and the queue might be empty.
    if (!peekedItem)
    {
        NS_LOG_DEBUG("No frames available for transmission");
        return false;
    }

    const WifiMacHeader& hdr = peekedItem->GetHeader();
    // setup a Block Ack agreement if needed
    if (hdr.IsQosData() && !hdr.GetAddr1().IsGroup() &&
        NeedSetupBlockAck(hdr.GetAddr1(), hdr.GetQosTid()))
    {
        // if the peeked MPDU has been already transmitted, use its sequence number
        // as the starting sequence number for the BA agreement, otherwise use the
        // next available sequence number
        uint16_t startingSeq =
            (hdr.IsRetry()
                 ? hdr.GetSequenceNumber()
                 : m_txMiddle->GetNextSeqNumberByTidAndAddress(hdr.GetQosTid(), hdr.GetAddr1()));
        return SendAddBaRequest(hdr.GetAddr1(),
                                hdr.GetQosTid(),
                                startingSeq,
                                edca->GetBlockAckInactivityTimeout(),
                                true,
                                availableTime);
    }

    // Use SendDataFrame if we can try aggregation
    if (hdr.IsQosData() && !hdr.GetAddr1().IsGroup() && !peekedItem->IsFragment() &&
        !GetWifiRemoteStationManager()->NeedFragmentation(peekedItem =
                                                              CreateAliasIfNeeded(peekedItem)))
    {
        return SendDataFrame(peekedItem, availableTime, initialFrame); // STEP_INTO
    }

    // Use the QoS FEM to transmit the frame in all the other cases, i.e.:
    // - the frame is not a QoS data frame
    // - the frame is a broadcast QoS data frame
    // - the frame is a fragment
    // - the frame must be fragmented
    return QosFrameExchangeManager::StartFrameExchange(edca, availableTime, initialFrame);
}
```
* 먼저, 전송할 BA Req 프레임이 있는지 확인 (즉, 특정 AC에 해당하는 MAC queue에 BA req 프레임이 있는 경우)
  > Note: BA req 프레임 생성은 MPDU expired event가 호출되어야 하고 이는 현재 수행중인 채널 접근 및 데이터 전송과는 독립적으로 발생하는 이벤트임 (따로 루틴 분석을 위한 debug를 해야함) 
* 또한, 큐에 MPDU가 있어 채널 접근 요청을 수행했다 하더라도 채널 접근을 위해 대기하는 시간 동안 MPDU의 시간이 만료되었을 수 있으므로 한번 더 체크
* 결론적으로 크게 3가지의 조건문 존재
* 조건 1. BlockAckAgreement
  * 이 경우 데이터가 전송되는 것이 아닌 ADDBA Request 프레임을 전송하며, 초기 STA 및 AP 간 BlockAck session 설정에 목적이 있음
  * ADDBA Request는 재전송을 수행하기 위한 BA Request와 다른 프레임임
  * ADDBA Request 프레임이 전송되는 조건은??? NeedSetupBlockAck function에서 true를 반환할 때 -> 7.1. ns3::HtFrameExchangeManager::NeedSetupBlockAck 확인
* 조건 2.1. 일반적인 전송 (STEP_INTO)
  * Alias는 별칭, 별명을 뜻하며 해당 함수는 그냥 copy를 생각하면 됨
* 조건 2.2. 특정 조건에 부합한 전송 (해당되지 않음 나중에 심심하면 해야지)
  * 특정조건 1. Frame이 QoS data가 아닌 경우
  * 특정조건 2. Frame이 QoS data이지만, broadcast인 경우
  * 특정조건 3. Frame이 fragmentation된 경우
  * 특정조건 4. Frame이 fragmentation되야 하는 경우
          
<p align="center"><img src="https://github.com/user-attachments/assets/0a65f25e-11c8-45ef-9162-01732f63c435"</p>
  
### 7.1. ns3::HtFrameExchangeManager::NeedSetupBlockAck (ADDBA Request 프레임이 전송되는 조건)  
```c
bool
HtFrameExchangeManager::NeedSetupBlockAck(Mac48Address recipient, uint8_t tid)
{
    Ptr<QosTxop> qosTxop = m_mac->GetQosTxop(tid);
    bool establish;

    if (!GetWifiRemoteStationManager()->GetHtSupported(recipient))
    {
        establish = false;
    }
    else if (auto agreement = qosTxop->GetBaManager()->GetAgreementAsOriginator(recipient, tid);
             agreement && !agreement->get().IsReset())
    {
        establish = false;
    }
    else
    {
        WifiContainerQueueId queueId{WIFI_QOSDATA_QUEUE, WIFI_UNICAST, recipient, tid};
        uint32_t packets = qosTxop->GetWifiMacQueue()->GetNPackets(queueId);
        establish =
            ((qosTxop->GetBlockAckThreshold() > 0 && packets >= qosTxop->GetBlockAckThreshold()) ||
             (m_mpduAggregator->GetMaxAmpduSize(recipient, tid, WIFI_MOD_CLASS_HT) > 0 &&
              packets > 1) ||
             GetWifiRemoteStationManager()->GetVhtSupported());
    }

    NS_LOG_FUNCTION(this << recipient << +tid << establish);
    return establish;
}
```
* return 값 bool 변수 establish가 true를 반환하려면 앞서 false를 반환하는 조건문 2개에 해당되지 않고, 마지막 조건문에서 true를 할당받아야 함
* false를 반환하는 조건문 1: 수신 device (recipient, 여기선 AP)가 HT 표준을 지원하지 않는 경우 -> BlockAck 메커니즘은 802.11n(HT) 표준 이후로 사용됨
* false를 반환하는 조건문 2: 이미 agreement가 존재하고, agreement의 상태가 reset이 아닌 경우 (originator와 recipent간의 blockack session이 유효한 경우)
* 마지막 조건문에서 true가 반환되려면 아래 조건 3개 중 하나를 만족해야 함
  * 조건 1. 현재 할당된 TXOP의 BlockAck threshold가 0보다 크고, MAC queue에 있는 packet 개수가 threshold 보다 크거나 같은 경우
    * BlockAck threshold: MAC에서도 ACK를 지원하는데, 몇 개의 패킷을 보냇을때 Block ACK를 받을 건지 결정하는 값
  * 조건 2. 사전에 설정된 aggregator가 지원하는 최대 A-MPDU 크기가 0보다 크고, MAC queue에 있는 packet 개수가 0보다 큰 경우
  * 조건 3. RemoteStationManager가 VHT 표준을 지원하는 경우
    * ns-3 RemoteStationManager: 동일 link에 association되어 있는 모든 device를 관리하는 클래스
> Note: 각 조건 별 debug 수행했을때, 조건 3에 걸리고 나머지 조건에는 안걸림 (초기 agreement가 establishment 이후 true를 반환하는 경우가 없었음. 즉, false를 반환하는 조건문에 안 걸린 경우가 없음)
 
### 8. ns3::HtFrameExchangeManager::SendDataFrame (⭐ 중요도 최상)
```c
bool
HtFrameExchangeManager::SendDataFrame(Ptr<WifiMpdu> peekedItem,
                                      Time availableTime,
                                      bool initialFrame)
{
    NS_ASSERT(peekedItem && peekedItem->GetHeader().IsQosData() &&
              !peekedItem->GetHeader().GetAddr1().IsBroadcast() && !peekedItem->IsFragment());
    NS_LOG_FUNCTION(this << *peekedItem << availableTime << initialFrame);

    /* 추가 */
    if(peekedItem->GetHeader().GetSequenceNumber() == 234 && peekedItem->GetHeader().GetQosTid() == 3){
        NS_LOG_UNCOND("BP");
    }
    /* 추가 */

    Ptr<QosTxop> edca = m_mac->GetQosTxop(peekedItem->GetHeader().GetQosTid());
    WifiTxParameters txParams;
    txParams.m_txVector =
        GetWifiRemoteStationManager()->GetDataTxVector(peekedItem->GetHeader(), m_allowedWidth);
    Ptr<WifiMpdu> mpdu =
        edca->GetNextMpdu(m_linkId, peekedItem, txParams, availableTime, initialFrame); // 여기 중요!!

    if (!mpdu)
    {
        NS_LOG_DEBUG("Not enough time to transmit a frame");
        return false;
    }

    // try A-MPDU aggregation
    std::vector<Ptr<WifiMpdu>> mpduList =
        m_mpduAggregator->GetNextAmpdu(mpdu, txParams, availableTime); // 여기 중요!!
    NS_ASSERT(txParams.m_acknowledgment);

    if (mpduList.size() > 1)
    {
        // A-MPDU aggregation succeeded
        SendPsduWithProtection(Create<WifiPsdu>(std::move(mpduList)), txParams); // STEP_INTO
    }
    else if (txParams.m_acknowledgment->method == WifiAcknowledgment::BAR_BLOCK_ACK)
    {
        // a QoS data frame using the Block Ack policy can be followed by a BlockAckReq
        // frame and a BlockAck frame. Such a sequence is handled by the HT FEM
        SendPsduWithProtection(Create<WifiPsdu>(mpdu, false), txParams);
    }
    else
    {
        // transmission can be handled by the base FEM
        SendMpduWithProtection(mpdu, txParams);
    }

    return true;
}
```
> Note: wifi-phy.cc에 bp 걸고 해당 함수로 왔을 때 mpduList가 출력이 안되는 현상이 있음 (이유는 모름) 그래서 해당 코드 상단에 다시 bp 걸어줌
* m_mpduAggregator->GetNextAmpdu()를 통해, mpduList를 생성함 (이때, 넘기는 인자 mpdu, txParams, availableTime은 아래 내용과 같음)
  * mpdu: edca->GetNextMpdu()에서 반환되는 값
  * txParams: 송신기의 현재 MAC 및 PHY 속성 값들을 포함, txVector는 PHY 계층 정보를 포함하고 있음
  * availableTime: 데이터가 전송 가능한 유효 시간 (BE의 경우 할당되어 있지 않음)
* mpduList의 크기가 1보다 큰 경우, SendPsduWithProtection(psdu) 실행
* 단일 mpdu 전송인 경우, SendMpduWithProtection(mpdu) 실행
* 결론적으로 edca->GetNextMpdu()의 동작, m_mpduAggregator->GetNextAmpdu()의 동작만 분석하면 됨
  * edca->GetNextMpdu() 동작: 8.1. ns3::QosTxop::GetNextMpdu 참고
  * m_mpduAggregator->GetNextAmpdu() 동작: 8.2. ns3::MpduAggregator::GetNextAmpdu 참고

### 8.1. ns3::QosTxop::GetNextMpdu (동작 분석)
```c
Ptr<WifiMpdu>
QosTxop::GetNextMpdu(uint8_t linkId,
                     Ptr<WifiMpdu> peekedItem,
                     WifiTxParameters& txParams,
                     Time availableTime,
                     bool initialFrame)
{
    NS_ASSERT(peekedItem);
    NS_LOG_FUNCTION(this << +linkId << *peekedItem << &txParams << availableTime << initialFrame);

    Mac48Address recipient = peekedItem->GetHeader().GetAddr1();

    // The TXOP limit can be exceeded by the TXOP holder if it does not transmit more
    // than one Data or Management frame in the TXOP and the frame is not in an A-MPDU
    // consisting of more than one MPDU (Sec. 10.22.2.8 of 802.11-2016)
    Time actualAvailableTime =
        (initialFrame && txParams.GetSize(recipient) == 0 ? Time::Min() : availableTime);

    auto qosFem = StaticCast<QosFrameExchangeManager>(m_mac->GetFrameExchangeManager(linkId));
    if (!qosFem->TryAddMpdu(peekedItem, txParams, actualAvailableTime))
    {
        return nullptr;
    }

    NS_ASSERT(peekedItem->IsQueued());
    Ptr<WifiMpdu> mpdu;

    // If it is a non-broadcast QoS Data frame and it is not a retransmission nor a fragment,
    // attempt A-MSDU aggregation
    if (peekedItem->GetHeader().IsQosData())
    {
        uint8_t tid = peekedItem->GetHeader().GetQosTid();

        // we should not be asked to dequeue an MPDU that is beyond the transmit window.
        // Note that PeekNextMpdu() temporarily assigns the next available sequence number
        // to the peeked frame
        NS_ASSERT(!m_mac->GetBaAgreementEstablishedAsOriginator(recipient, tid) ||
                  IsInWindow(
                      peekedItem->GetHeader().GetSequenceNumber(),
                      GetBaStartingSequence(peekedItem->GetOriginal()->GetHeader().GetAddr1(), tid),
                      GetBaBufferSize(peekedItem->GetOriginal()->GetHeader().GetAddr1(), tid)));

        // try A-MSDU aggregation
        if (m_mac->GetHtSupported() && !recipient.IsBroadcast() &&
            !peekedItem->HasSeqNoAssigned() && !peekedItem->IsFragment())
        {
            auto htFem = StaticCast<HtFrameExchangeManager>(qosFem);
            mpdu = htFem->GetMsduAggregator()->GetNextAmsdu(peekedItem, txParams, availableTime);
        }

        if (mpdu)
        {
            NS_LOG_DEBUG("Prepared an MPDU containing an A-MSDU");
        }
        // else aggregation was not attempted or failed
    }

    if (!mpdu)
    {
        mpdu = peekedItem;
    }

    // Assign a sequence number if this is not a fragment nor a retransmission
    AssignSequenceNumber(mpdu);
    NS_LOG_DEBUG("Got MPDU from EDCA queue: " << *mpdu);

    return mpdu;
}
```
* 길어보이지만 다 필요없다. 왜냐하면 A-msdu aggregation을 수행하지 않음
* 수행하지 않는 이유
  * htFem->GetMsduAggregator()->GetNextAmsdu(peekedItem, txParams, availableTime) 함수 들어가서
  * uint16_t maxAmsduSize = m_mac->GetMaxAmsduSize(ac) 를 통해 MaxAmsduSize 들고오는데, 별도 설정을 하지않았으므로 0 (disable)이 리턴됨
* 결론적으로 mpdu = peekedItem이 됨 즉, copy임

### 8.2. ns3::MpduAggregator::GetNextAmpdu (동작 분석, 여기가 좀 중요함)
```c
std::vector<Ptr<WifiMpdu>>
MpduAggregator::GetNextAmpdu(Ptr<WifiMpdu> mpdu,
                             WifiTxParameters& txParams,
                             Time availableTime) const
{
    NS_LOG_FUNCTION(this << *mpdu << &txParams << availableTime);

    std::vector<Ptr<WifiMpdu>> mpduList;

    Mac48Address recipient = mpdu->GetHeader().GetAddr1();
    NS_ASSERT(mpdu->GetHeader().IsQosData() && !recipient.IsBroadcast());
    uint8_t tid = mpdu->GetHeader().GetQosTid();
    auto origRecipient = mpdu->GetOriginal()->GetHeader().GetAddr1();

    Ptr<QosTxop> qosTxop = m_mac->GetQosTxop(tid);
    NS_ASSERT(qosTxop);

    // Have to make sure that the block ack agreement is established and A-MPDU is enabled
    if (m_mac->GetBaAgreementEstablishedAsOriginator(recipient, tid) &&
        GetMaxAmpduSize(recipient, tid, txParams.m_txVector.GetModulationClass()) > 0)
    {
        /* here is performed MPDU aggregation */
        Ptr<WifiMpdu> nextMpdu = mpdu;

        while (nextMpdu)
        {
            // if we are here, nextMpdu can be aggregated to the A-MPDU.
            NS_LOG_DEBUG("Adding packet with sequence number "
                         << nextMpdu->GetHeader().GetSequenceNumber()
                         << " to A-MPDU, packet size = " << nextMpdu->GetSize()
                         << ", A-MPDU size = " << txParams.GetSize(recipient));

            mpduList.push_back(nextMpdu);

            // If allowed by the BA agreement, get the next MPDU
            auto peekedMpdu =
                qosTxop->PeekNextMpdu(m_linkId, tid, origRecipient, nextMpdu->GetOriginal());
            nextMpdu = nullptr;

            if (peekedMpdu)
            {
                // PeekNextMpdu() does not return an MPDU that is beyond the transmit window
                NS_ASSERT(IsInWindow(peekedMpdu->GetHeader().GetSequenceNumber(),
                                     qosTxop->GetBaStartingSequence(origRecipient, tid),
                                     qosTxop->GetBaBufferSize(origRecipient, tid)));

                peekedMpdu = m_htFem->CreateAliasIfNeeded(peekedMpdu);
                // get the next MPDU to aggregate, provided that the constraints on size
                // and duration limit are met. Note that the returned MPDU differs from
                // the peeked MPDU if A-MSDU aggregation is enabled.
                NS_LOG_DEBUG("Trying to aggregate another MPDU");

                /* 추가 */
                if(peekedMpdu->GetHeader().GetSequenceNumber() == 273 && peekedMpdu->GetHeader().GetQosTid() == 3){
                    NS_LOG_UNCOND("BP");
                }
                /* 추가 */

                nextMpdu =
                    qosTxop->GetNextMpdu(m_linkId, peekedMpdu, txParams, availableTime, false); // 여기 중요!!
            }
        }
        if (mpduList.size() == 1)
        {
            // return an empty vector if it was not possible to aggregate at least two MPDUs
            mpduList.clear();
        }
    }

    return mpduList;
}
```
* 획득한 TXOP의 MAC Queue를 순회 (in-flight 상태가 아닌)하면서 mpdu list를 만드는데 여기서 2가지의 조건을 기준으로 aggregation을 수행함
  * 조건 1. 현재 검색된 mpdu의 seq#가 현재 시점의 수신 device의 수신 윈도우 크기에 포함되어야 함
    * winstart: 송신기 입장에서, 현재 수신기가 기대하고 있는 seq # (즉, 송신기 입장에서는 in-flight 상태일 수도 있음)
    * winsize: 수신기의 최대 buffer 크기
  * 조건 2. nextMpdu가 nullptr이 되기 전까지
```c
bool
IsInWindow(uint16_t seq, uint16_t winstart, uint16_t winsize)
{
  return ((seq - winstart + 4096) % 4096) < winsize;
}
```
  
* 근데, 조건 1에는 안걸림 (debug 해보면, 현재 시점의 수신 device의 winstart = 117, winsize = 256이므로 이렇게 따지면 #234 ~ #372까지 aggregation 되야함)
* 조건 2에 걸림 그럼 왜 #234 ~ #272까지 aggregation 되는지 확인해야함 (즉, #273은 왜 안되는지)
* 따라서, 추가적인 코드를 통해 BP 걸어주고 서브루틴 진입
* 서브루틴이 정말 많음
  * ns3::MpduAggregator::GetNextAmpdu
  * ns3::QosTxop::GetNextMpdu
  * ns3::QosFrameExchangeManager::TryAddMpdu
  * ns3::HtFrameExchangeManager::IsWithinLimitsIfAddMpdu
  * ns3::QosFrameExchangeManager::IsWithinSizeAndTimeLimits <- 여기서 답 찾을 수 있음 (8.2.1. 참고)
 
### 8.2.1 ns3::QosFrameExchangeManager::IsWithinSizeAndTimeLimits (동작 분석)
```c
bool
QosFrameExchangeManager::IsWithinSizeAndTimeLimits(uint32_t ppduPayloadSize,
                                                   Mac48Address receiver,
                                                   const WifiTxParameters& txParams,
                                                   Time ppduDurationLimit) const
{
    NS_LOG_FUNCTION(this << ppduPayloadSize << receiver << &txParams << ppduDurationLimit);

    if (ppduDurationLimit != Time::Min() && ppduDurationLimit.IsNegative())
    {
        NS_LOG_DEBUG("ppduDurationLimit is null or negative, time limit is trivially exceeded");
        return false;
    }

    if (ppduPayloadSize > WifiPhy::GetMaxPsduSize(txParams.m_txVector.GetModulationClass()))
    {
        NS_LOG_DEBUG("the frame exceeds the max PSDU size");
        return false;
    }

    // Get the maximum PPDU Duration based on the preamble type
    Time maxPpduDuration = GetPpduMaxTime(txParams.m_txVector.GetPreambleType());

    Time txTime = GetTxDuration(ppduPayloadSize, receiver, txParams);
    NS_LOG_DEBUG("PPDU duration: " << txTime.As(Time::MS));

    if ((ppduDurationLimit.IsStrictlyPositive() && txTime > ppduDurationLimit) ||  
        (maxPpduDuration.IsStrictlyPositive() && txTime > maxPpduDuration)) // 여기 중요!!
    {
        NS_LOG_DEBUG(
            "the frame does not meet the constraint on max PPDU duration or PPDU duration limit");
        return false;
    }

    return true;
}
```
* 첫 번째 인자 값 ppduPayloadSize는 현재 mpduList에 포함된 전체 크기를 뜻함
* 세 번째 인자 값 txParams는 송신기의 현재 MAC 및 PHY 속성 값들을 포함하고 있음 (asmduSize, ampduSize, seqNumbers, txVector 등등...)
* 이걸 기반으로 maxPpduDuration (기준)과 txTime (예상)을 계산함
* 이때, 예상 전송 시간 txTime 값이 기준이 되는 maxPpduDuration 값 보다 더 크기 때문에 false를 리턴함
* 해석하면, 사전에 설정된 PHY 파라미터를 기준으로 Preamble (헤더)정보를 생성하는데, 해당 기준을 초과하여 데이터를 전송할 수가 없다는 뜻임
  
### Summary  
  * 이후 실행되는 서브루틴 ~ wifi-phy 까지는 psdu 및 ppdu 생성, preamble 추가 등과 같은 작업 수행 (중요도가 낮음)
  * BE retransmission 과정은 txParams(송신 device의 MAC 및 PHY 속성 값)을 기반으로 계산된 최대 PPDU 전송 시간에 제약을 받음
  * 첫 번째 의문점: 원본 프레임의 전체가 재전송되는 BE와 다르게, 부분적으로 재전송되는 VI retransmission의 과정 -> Appendix B. VI retansmission 참고
  * 두 번째 의문점: Retransmission event가 invoke 되는 원인 -> Appendix C. Block ACK request 참고
