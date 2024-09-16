## Appendix. B. AC_VI Retransmssion

### Objective
* AC_BE 및 AC_VI의 서로 다른 재전송 과정이 발생하는 원인을 찾기 위해
* 해당 문서에서는 ns-3.40 기반 AC_VI의 재전송 수행 과정을 분석함
* 앞서 분석한 내용 (Appendix. A. 참고)과 공통되는 부분은 다루지 않음

### Preview
* No. 11 로그 분석 
```
1. 1.045457s STA1 -> AP (AC_VI, A-MPDU ID 39: #174 ~ #202) A-mpdu 송신
2. 1.084516s STA1 -> AP #174 ~ #201 A-mpdu 재전송
3. 1.090517s AP에서 STA1가 재전송한 #174 ~ #201 A-mpdu 수신
```

* 분석을 위해 ns3::WifiPhy::Send에서 breakpoint 걸어줌
```c
auto ptr = ppdu->GetPsdu()->begin();
for (int i = 0; i < (int)ppdu->GetPsdu()->GetNMpdus(); i++){
  auto mpdu_header = ptr[i]->GetHeader();
  if (mpdu_header.GetSequenceNumber() == 174 && mpdu_header.GetQosTid() == 5){
    if (mpdu_header.IsRetry() == 1)
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": retry");
    else
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": No retry");
  }
}
```
> +1.02239s: No retry  
> +1.08898s: No retry  
> +1.12803s: retry  

* 문제가 있다. 1AP + 2STA의 topology를 구성했으므로, STA의 식별이 필요함
* mpdu header에는 Addr1, Addr2 field가 있는데, 각각 수신 및 송신 device의 MAC address가 포함되어 있음
* 따라서, 코드를 다음과 같이 수정함
```c
auto ptr = ppdu->GetPsdu()->begin();
for (int i = 0; i < (int)ppdu->GetPsdu()->GetNMpdus(); i++){
  auto mpdu_header = ptr[i]->GetHeader();
  if (mpdu_header.GetSequenceNumber() == 174 && mpdu_header.GetQosTid() == 5){
    if (mpdu_header.IsRetry() == 1)
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": " << mpdu_header.GetAddr2() << ", send retry mpdu");
    else
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": " << mpdu_header.GetAddr2() << ", send mpdu");
  }
}
```
> +1.02239s: 00:00:00:00:00:05 send mpdu  
> +1.08898s: 00:00:00:00:00:02 send mpdu  
> +1.12803s: 00:00:00:00:00:02 send retry mpdu  

* 시간 간격 확인해보면 같은 로그 맞음
  * Wireshark 기준: 1.084516s - 1.045457s = 39.059ms
  * ns-3 기준: 1.12803s - 1.08898s = 39.05ms

> Note 1: Device들의 MAC 주소는 (01 ~ 03: STA1, 04 ~ 06: STA2, 07 ~ 09: AP)가 할당되어 있음  
> Note 2: Device들은 MLD이므로, 또 하나 유추할 수 있는 점은 장치의 UMAC, L-MAC link 1, L-MAC link 2의 순서대로 MAC 주소가 할당되어 있음

* 결국, 재전송을 수행할 때 원본 패킷의 전부가 날아가지 않는 이유를 분석해야 하기 때문에, retry에 breakpoint 걸어줌
* Call stack은 다음과 같음 (left: AC_BE retransmission, right: AC_VI retransmission)

<p align="center">  
  <img src="https://github.com/user-attachments/assets/71dd9b80-1c03-4f7e-a96e-8adb51d30208" width="40%">  
  <img src="https://github.com/user-attachments/assets/2bd8535b-5719-4f8f-9df8-1cc978000bc1" width="40%">
</p>

* Appendix.A. AC_BE retransmission과 다른 점만 분석해보면
  * ns3::ChannelAccessManager::AccessTimeout 없음
  * ns3::ChannelAccessManager::DoGrantDcfAccess 없음
  * ns3::EhtFrameExchangeManager::StartTransmission 없음
* ChannelAccessManager의 특정 AC에 해당하는 TXOP의 backoff가 0에 도달하여 채널 접근을 요청하는 상태를 관리하는 기능을 함
  * (⭐ 중요) 즉, #174 ~ #201에 해당하는 A-mpdu 재전송은 AC_VI TXOP를 획득한 후 처음으로 보내는 초기 프레임이 아님
* 이제 중요한 것만 하나씩 뜯자 (서브루틴으로 들어가는 code의 line에 BREAKPOINT 표시)
  
### 1. ns3::QosFrameExchangeManager::StartTransmission (중요도 중)
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

        if (!StartFrameExchange(m_edca, m_edca->GetRemainingTxop(m_linkId), false)) // STEP_INTO
        {
            NS_LOG_DEBUG("Not enough remaining TXOP time");
            return SendCfEndIfNeeded();
        }

        return true;
    }

    // we get here if TXOP limit is null
    m_initialFrame = true;

    if (StartFrameExchange(m_edca, Time::Min(), true))
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
* 다음과 같은 조건문이 존재함
  * 조건 1. TXOP limit의 값이 0보다 클 때 (AC_VI 및 AC_VO에 해당)
    * 조건 1.1 특정 link의 TXOP가 obtain되지 않은 경우 (즉, initial frame 전송에 해당)
    * 조건 1.2 특정 link의 TXOP limit 값이 남아 있는 경우 (즉, 전송되는 frame은 initial frame이 아님)
  * 조건 2. TXOP limit의 값이 null일 때 (AC_BE 및 AC_BK에 해당)
* STEP_INTO
  * 조건 1.2.에 해당함
  * Available Time인 m_edca->GetRemainingTxop(m_linkId)의 값을 보면 3962400ns임 (즉, 3.962ms)가 남아 있음
  * VI TXOP Limit = 4.096ms임을 감안했을 때 아직 획득한 TXOP에서 frame을 전송하기에는 충분한 시간임 (반대로 말하면, 전송한 initial frame의 크기가 작음)

### 2. ns3::HtFrameExchangeManager::SendDataFrame (⭐ 중요도 상)
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
    if(peekedItem->GetHeader().GetSequenceNumber() == 174 && peekedItem->GetHeader().GetQosTid() == 5 && peekedItem->GetHeader().IsRetry()){
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
* Aggregation 되는 부분까지 와서 추가 코드 기반으로 다시 breakpoint 걸어줌
* STEP_INTO
  * mpdu list를 만드는 것 까지는 BE와 동일하지만, aggregation 되는 크기가 다르기 때문에, 서브루틴의 동작 과정에서 차이가 있을 거임
  * 따라서, GetNextAmpdu() 함수 동작 과정에 대한 분석이 필요함
 
### 2.1. ns3::MpduAggregator::GetNextAmpdu (⭐ 중요도 상)
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
                if (peekedMpdu->GetHeader().GetSequenceNumber() == 202 &&
                        peekedMpdu->GetHeader().GetQosTid() == 5 && peekedMpdu->GetHeader().IsRetry()) {
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
* #202에 해당하는 mpdu가 aggregation되지 않는 이유를 분석해야 하므로 추가 코드를 기반으로 BP 걸어줌
* nextMpdu의 값이 null이 되는 이유를 찾으려면 GetNextMpdu() 분석 필요
* 넘기는 인자 값
  * m_linkId: 0, 첫 번째 채널을 뜻함
  * peekedMpdu: #202에 해당하는 mpdu를 뜻함
  * txParams: MAC 및 PHY 속성 정보
  * availableTime: 3962400, 남은 TXOP 시간 약 3.962ms
  * initialFrame: false
* 서브루틴 목록 (BE와 동일함)
  * ns3::QosTxop::GetNextMpdu
  * ns3::QosFrameExchangeManager::TryAddMpdu
  * ns3::HtFrameExchangeManager::IsWithinLimitsIfAddMpdu
  * ns3::QosFrameExchangeManager::IsWithinSizeAndTimeLimits <- 답 찾을 수 있음 2.1.1. 참고
 
### 2.1.1. ns3::QosFrameExchangeManager::IsWithinSizeAndTimeLimits (⭐ 중요도 상)
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
* BE retransmission과 과정은 똑같은데, 다른 점 딱 하나 있음 (걸리는 조건이 다름)
* txTime: #202 프레임이 mpdulist에 포함되었을 때, 전송 시간 (예정)
* maxPpduDuration: txParams의 속성 정보를 기반으로 최대 지원 가능한 전송 시간 (기준 1)
* ppduDurationLimit: 앞서 TXOP를 obtain하고 initial frame을 전송하고 남은 시간 availableTime (기준 2)
* Debug 결과 txTime: 4025200 (4.025ms), maxPpduDuration: 5484000 (5.484ms), ppduDurationLimit: 3906400 (3.906ms)
* BE는 maxPpduDuration (기준 1)에 걸린 반면, VI는 ppduDurationLimit (기준 2)에 걸림
* 해석하면, 획득한 TXOP의 유효 시간이 3.906ms인데, 해당 시간을 초과하는 전송 시간을 가진 A-mpdu를 보낼 수 없다는 의미
* 참고로 mpdu 1개는 1500byte로 설정함
  * 3.906ms에서 1500byte mpdu 1개가 추가되면 예상 전송 시간은 4.025ms가 됨
  * 다르게 해석하면, TXOP를 획득하고 전송한 initial frame의 크기는 1500byte보다 무조건 작음
  * 왜냐하면, initial frame으로 1500byte 크기의 프레임을 보냈으면 availableTime이 3.906ms보다 무조건 작았을 수 밖에 없음
 
### Summary
* VI retransmission 과정에서 손실된 원본 프레임의 전체가 아닌 부분적으로 재전송이 수행되는 이유는 TXOP를 획득하고 초기 프레임으로 '무엇'을 전송했기 때문임
* TXOP limit의 시간에서 초기 프레임의 전송 시간 만큼 차감이 되었으며, 해당 시간을 기준으로 aggregation을 수행하였기 때문에 마지막 mpdu가 aggregation되지 못함
* BE retransmission과 함수 호출 과정은 거의 유사하나 걸리는 조건이 달랐음
* 그럼 도대체 initial frame으로 날아간 frame은 무엇일까? Appendix C. Block Ack Request참고
