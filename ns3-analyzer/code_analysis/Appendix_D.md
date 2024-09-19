## Appendix. D. MPDU Buffer Size

### Objective
* 해당 문서에서는 ns-3.40 기반 Device의 MPDU Buffer Size와 관련된 분석을 수행함
* 수신기의 Buffer Size의 한계로 인한 aggregation size의 제약
* Buffer에 존재하는 MPDU들의 state 정보 분석
* 앞서 분석한 내용 (Appendix. A, B, C 참고)과 공통되는 부분은 다루지 않음

### Preview
* AP 입장에서, wireshark pcap 로그를 보다보면 아래 그림과 같이 작은 개수 (6개)에 해당하는 A-mpdu가 수신된 것을 확인할 수 있음
  * 일반적으로 VI에 해당하는 TXOP를 획득하면 29개의 mpdu가 aggregation되어 전송됨 (20MHz channel, 1400 byte payload의 경우)

<p align="center">  
  <img src="https://github.com/user-attachments/assets/07ae55a8-bbc6-443d-990c-16b1ddbe42f8" width="40%">  
</p>

* 예쁘게 정리해보면
```
⭐ AP 입장, link 1
1. Time: 1.478139s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 54 / Info: 802.11 Block Ack
2. Time: 1.479107s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1842)
...
8. Time: 1.479107s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1847)
9. Time: 1.479117s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 54 / Info: 802.11 Block Ack
```
* 이러한 현상이 발생하는 이유를 아래와 같은 관점에서 분석
  * TXOP를 획득한 후 전송한 initial frame인지?
  * Appendix A, B와 같이 최대 PPDU 전송 시간 및 TXOP Limit에 제약을 받아 조정된 aggregation size인지?
  * 다른 이유가 있는지?

* 따라서, aggregation 규칙을 확인하기 위해 아래와 같은 코드를 WifiPhy::send에 breakpoint걸고 call stack을 보면
  * MPDU header 정보: (Originator: STA 1, Qos Tid: 5 (VI), Seq #: 1842)
```c
auto ptr = ppdu->GetPsdu()->begin();
for (int i = 0; i < (int) ppdu->GetPsdu()->GetNMpdus(); i++) {
auto mpdu_header = ptr[i]->GetHeader();
if ((int) ppdu->GetPsdu()->GetNMpdus() == 6 && mpdu_header.GetSequenceNumber() == 1842 &&
    mpdu_header.GetQosTid() == 5) {
    NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": " << mpdu_header.GetAddr2() << ", send A-mpdu (6)");
  }
}
```
> +1.51963s: 00:00:00:00:00:02, send A-mpdu (6)

* Call Stack
<p align="center">  
  <img src="https://github.com/user-attachments/assets/e205ede2-eaef-447c-9851-3fc39b9837ff" width="40%">  
</p>

* 다른 점은 없다, Aggregation 제약 조건만 확인해보자

### 1. ns3::HtFrameExchangeManager::SendDataFrame
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
    if(peekedItem->GetHeader().GetSequenceNumber() == 1842 && peekedItem->GetHeader().GetQosTid() == 5){
        NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": BP");
    }
    /* 추가 */

    Ptr<QosTxop> edca = m_mac->GetQosTxop(peekedItem->GetHeader().GetQosTid());
    WifiTxParameters txParams;
    txParams.m_txVector =
        GetWifiRemoteStationManager()->GetDataTxVector(peekedItem->GetHeader(), m_allowedWidth);
    Ptr<WifiMpdu> mpdu =
        edca->GetNextMpdu(m_linkId, peekedItem, txParams, availableTime, initialFrame);

    if (!mpdu)
    {
        NS_LOG_DEBUG("Not enough time to transmit a frame");
        return false;
    }

    // try A-MPDU aggregation
    std::vector<Ptr<WifiMpdu>> mpduList =
        m_mpduAggregator->GetNextAmpdu(mpdu, txParams, availableTime);
    NS_ASSERT(txParams.m_acknowledgment);

    if (mpduList.size() > 1)
    {
        // A-MPDU aggregation succeeded
        SendPsduWithProtection(Create<WifiPsdu>(std::move(mpduList)), txParams); // STEP INTO
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
* Aggregation rule 및 mpdu list 확인을 위해 추가 코드 삽입
* 해당 시점에서 breakpoint걸고, 아래와 같은 variable 값을 보면
  * availableTime: 4096000ns (즉, 4.086ms)
  * initialFrame: True
* 즉, TXOP를 획득하고 전송하는 첫 번째 frame이라는 뜻임 (여기서 의문점이 들어야됨)
  * VI에 해당하는 MPDU는 최대 PPDU 전송시간에 제약을 받을 수 없음
    * 애초에 TXOP Limit에 제약을 먼저 받음 (payload size가 동일하므로, 제약을 받으려면 BE와 같이 최소 39개는 aggregation 되어야 함)
  * 근데, TXOP limit에 제약을 받으려면 30개의 MPDU가 aggregation 되어야함 (일반적으로 29개에 해당하는 MPDU가 aggregation되어 전송되므로)
  * 그럼 남은 조건 딱 하나 있음 (window size)
* 서브루틴 진입 두가자~ -> 1.1. MpduAggregator::GetNextAmpdu 참고

### 1.1. MpduAggregator::GetNextAmpdu (⭐ 중요도 상)
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
                qosTxop->PeekNextMpdu(m_linkId, tid, origRecipient, nextMpdu->GetOriginal()); // 여기 중요!!
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
                if(peekedMpdu->GetHeader().GetSequenceNumber() == 1847 && peekedMpdu->GetHeader().GetQosTid() == 5){
                    NS_LOG_UNCOND("BP");
                }
                /* 추가 */

                nextMpdu =
                    qosTxop->GetNextMpdu(m_linkId, peekedMpdu, txParams, availableTime, false);
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
* Seq # 1842 ~ 1847까지 aggregation 되므로 추가 코드를 통해 BP 새로 걸어줌
* 기존 (Appendix A, B)에는 nextMpdu가 nullptr가 되어 MAC Queue 순회 loop를 벗어났는데, 조건이 조금 다름
  * nextMpdu <= peekedMpdu (seq #: 1847) 할당하고
  * mpduList.push_back(nextMpdu)로 (seq #: 1847) mpdu insert하는 것 까지는 동일
  * 이후 peekedMpdu를 통해 mpdu (seq #: 1848)을 가져오는 과정에서 nullptr이 할당됨
  * 따라서, nextMpdu가 nullptr이 되고 자연스럽게 MAC Queue 순회 loop 탈출하는 방식임
  * 결론적으로 PeekNextMpdu 동작 과정 분석이 필요함 -> 1.1.1. QosTxop::PeekNextMpdu 참고
* 이후 mpduList return하면서 STEP OUT
 
### 1.1.1 QosTxop::PeekNextMpdu (⭐ 중요도 상, 조건문이 되게 많으므로 어디에 걸리는 잘 봐야함)
```c
Ptr<WifiMpdu>
QosTxop::PeekNextMpdu(uint8_t linkId, uint8_t tid, Mac48Address recipient, Ptr<const WifiMpdu> mpdu)
{
    NS_LOG_FUNCTION(this << +linkId << +tid << recipient << mpdu);

    // lambda to peek the next frame
    auto peek = [this, &linkId, &tid, &recipient, &mpdu]() -> Ptr<WifiMpdu> {
        if (tid == 8 && recipient.IsBroadcast()) // undefined TID and recipient
        {
            return m_queue->PeekFirstAvailable(linkId, mpdu);
        }
        WifiContainerQueueId queueId(WIFI_QOSDATA_QUEUE, WIFI_UNICAST, recipient, tid);
        if (auto mask = m_mac->GetMacQueueScheduler()->GetQueueLinkMask(m_ac, queueId, linkId);
            !mask || mask->none())
        {
            return m_queue->PeekByQueueId(queueId, mpdu);
        }
        return nullptr;
    };

    auto item = peek();
    // remove old packets (must be retransmissions or in flight, otherwise they did
    // not get a sequence number assigned)
    while (item && !item->IsFragment())
    {
        if (item->GetHeader().IsCtl())
        {
            NS_LOG_DEBUG("Skipping control frame: " << *item);
            mpdu = item;
            item = peek();
            continue;
        }

        if (item->HasSeqNoAssigned() && IsQosOldPacket(item))
        {
            NS_LOG_DEBUG("Removing an old packet from EDCA queue: " << *item);
            if (!m_droppedMpduCallback.IsNull())
            {
                m_droppedMpduCallback(WIFI_MAC_DROP_QOS_OLD_PACKET, item);
            }
            mpdu = item;
            item = peek();
            m_queue->Remove(mpdu);
            continue;
        }

        if (auto linkIds = item->GetInFlightLinkIds(); !linkIds.empty()) // MPDU is in-flight
        {
            // if the MPDU is not already in-flight on the link for which we are requesting an
            // MPDU and the number of links on which the MPDU is in-flight is less than the
            // maximum number, then we can transmit this MPDU
            if (linkIds.count(linkId) == 0 && linkIds.size() < m_nMaxInflights)
            {
                break;
            }

            // if no BA agreement, we cannot have multiple MPDUs in-flight
            if (item->GetHeader().IsQosData() &&
                !m_mac->GetBaAgreementEstablishedAsOriginator(item->GetHeader().GetAddr1(),
                                                              item->GetHeader().GetQosTid()))
            {
                NS_LOG_DEBUG("No BA agreement and an MPDU is already in-flight");
                return nullptr;
            }

            NS_LOG_DEBUG("Skipping in flight MPDU: " << *item);
            mpdu = item;
            item = peek();
            continue;
        }

        if (item->GetHeader().HasData() &&
            !m_mac->CanForwardPacketsTo(item->GetHeader().GetAddr1()))
        {
            NS_LOG_DEBUG("Skipping frame that cannot be forwarded: " << *item);
            mpdu = item;
            item = peek();
            continue;
        }
        break;
    }

    if (!item)
    {
        return nullptr;
    }

    WifiMacHeader& hdr = item->GetHeader();

    // peek the next sequence number and check if it is within the transmit window
    // in case of QoS data frame
    uint16_t sequence = item->HasSeqNoAssigned() ? hdr.GetSequenceNumber()
                                                 : m_txMiddle->PeekNextSequenceNumberFor(&hdr);
    if (hdr.IsQosData())
    {
        Mac48Address recipient = hdr.GetAddr1();
        uint8_t tid = hdr.GetQosTid();

        if (m_mac->GetBaAgreementEstablishedAsOriginator(recipient, tid) && // 여기 중요!!
            !IsInWindow(sequence,
                        GetBaStartingSequence(recipient, tid),
                        GetBaBufferSize(recipient, tid)))
        {
            NS_LOG_DEBUG("Packet beyond the end of the current transmit window");
            return nullptr;
        }
    }

    // Assign a sequence number if this is not a fragment nor it already has one assigned
    if (!item->IsFragment() && !item->HasSeqNoAssigned())
    {
        hdr.SetSequenceNumber(sequence);
    }
    NS_LOG_DEBUG("Packet peeked from EDCA queue: " << *item);
    return item;
}
```
* auto item = peek(); <- 코드 상단 부분에 정의되어 있는 lambda 함수를 통해 mpdu를 검색 후 할당받으며 아래와 같은 특징이 존재함
  * 특징 1: 인자 값으로 받은 mpdu (seq #: 1847)과 같은 QueueId를 가지고 있는 Queue에 접근함
  * 특징 2: 검색후 할당받은 item (mpdu)은 seq #가 할당되지 않은 상태임!!
* 향후 할당받은 item이 유효한 mpdu가 될 때까지 while loop를 순회함
  * 여기서, 유효한 mpdu라는 건 아래 조건들을 만족하지 않는 mpdu를 의미함
    * 조건 1: 재전송이 필요한 mpdu인 경우
    * 조건 2: 이미 전송중인 mpdu인 경우
    * 조건 3: seq#가 할당되어 있으며, 오래된 mpdu인 경우
      * 여기서 오래 되었다의 의미 -> 수신기 (recipient)가 기대하는 mpdu의 seq# 보다 작은 경우
    * 조건 4: 모종의 이유로 전송할 수 없는 mpdu인 경우
      * ~~모종의 이유를 찾아보려고 했는데 virtual method라 implementation 위치를 모르겠음~~ (찾음, AP와 STA이 association되어 있지 않은 경우)
* 이후 seq # (1848)를 local variable에 임시 할당
* (⭐ 중요) IsInWindow function을 호출하는데 이때 넘기는 인자 값은 아래와 같음
  * sequence = 이전에 임시 할당한 wlan seq #, 1848
  * GetBaStartingSeqeuence(recipient, tid) = 수신기 (recipient)의 특정 AC에 해당하는 MAC Queue의 시작 seq #, 1784
  * GetBaBufferSize(recipient, tid)) = 수신기 (recipient)의 특정 AC에 해당하는 MPDU Buffer Size, 64
  * return 값이 false이므로, nullptr 반환 -> 1.1.1.1. ns3::WifiUtils::IsInWindow 참고

### 1.1.1.1. ns3::WifiUtils::IsInWindow 
```c
bool
IsInWindow(uint16_t seq, uint16_t winstart, uint16_t winsize)
{
    return ((seq - winstart + 4096) % 4096) < winsize;
}
```
* 좌변 ((seq - winstart + 4096) % 4096) = ((1848 - 1784 + 4096) % 4096 = 64
* 우변 winsize = 64
* 즉, false 반환
* false의 의미 -> 수신기 (recipient)입장에서, 시작 seq # 및 MPDU buffer size를 기반으로 계산된 기대하고 있는 seq #를 초과한 MPDU를 수신 받을 수 없음
  * 반대로 말하면, 송신기 (originator)입장에서, 수신기 (recipient)가 수신 받을 수 없는 MPDU를 송신할 수 없음

### Supplementary: Recipient MPDU buffer state
* 시나리오를 다시 보자
```
⭐ AP 입장, link 1
1. Time: 1.478139s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 54 / Info: 802.11 Block Ack
2. Time: 1.479107s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1842)
...
8. Time: 1.479107s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1847)
9. Time: 1.479117s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 54 / Info: 802.11 Block Ack
```
* 1.479107s 시점에 STA 1이 link 1을 통해 Seq # 1842 ~ 1847에 해당하는 A-mpdu를 전송할 때, 왜 수신기의 시작 seq #는 1784일까?
  * 증명을 위해 이전 시점의 AP 입장에서, link 1 및 link 2에서의 communication 과정을 봐야함 (굉장히 복잡하니까 천천히 차근차근)
* 아래 그림 (좌측)은 AP MLD (AP STA1 + AP STA2)와 Non-AP MLD1 (Non-AP STA 1 + Non-AP STA 2) 및 Non-AP MLD2 (Non-AP STA3 + Non-AP STA4)와의 통신 과정을 나타냄
* 아래 그림 (우측)은 link 1에서 Non-AP STA 1이 seq # 1842 ~ 1847에 대한 A-mpdu을 전송하려는 시점에서의 수신기 (recipient)의 MPDU buffer State를 나타냄
<p align="center">  
  <img src="https://github.com/user-attachments/assets/77b1f270-3b87-4ad3-9641-2e95a891af71" width="80%">  
</p>

* 여기서 알 수 있는 정보들!!
* link 2에서 Non-AP STA2 및 Non-AP STA4가 동시에 TXOP를 획득한 후 A-mpdu를 전송함에 따라 충돌이 발생함 -> 따라서 향후 BA Timeout이 발생한 후 BA Req frame을 보낼 것임
* 잠시 후, Non-AP STA1은 link 1에서 TXOP를 획득한 후 seq # 1813 ~ 1841에 해당하는 A-mpdu를 전송하고 BA를 수신함
* 또한, Non-AP STA1은 link 1에서 TXOP를 한번 더 획득한 후 seq # 1842 ~ 1847에 해당하는 A-mpdu를 전송하고 BA를 수신함
  * (⭐ 중요) 해당 시점에서의 recipient MPDU buffer state를 보면, seq # 1784 ~ 1812는 전송 중인(inflight) 상태임
  * 해당 시점에서의 recipient MPDU buffer state를 보면, seq # 1813 ~ 1841은 ACK를 받은 상태임
  * (⭐ 매우 중요) 해당 시점은, link 2에서 충돌이 발생한 A-mpdu에 대한 BA Timeout이 발생하기 전임 (즉, BA Req frame이 MAC Queue에 enqueue되기 전이라는 뜻)
  * 따라서, MPDU buffer state에 제약을 받아 6개의 MPDU가 aggregation되어 전송됨
* (⭐ 중요) 또한, link 2에서 seq # 1784 ~ 1812에 해당하는 A-mpdu를 재전송할 때 앞서 BA Req <-> BA의 통신으로 인해 seq # 1812를 제외한 부분 재전송이 수행됨
* (⭐ 매우 중요) 만약, link 1에서 획득한 TXOP가 BA Timeout이 발생한 후의 시점이라 할지라도, seq # 1784 ~ 1811에 해당하는 A-mpdu는 link 1을 통해 부분 재전송되지 않음
  * BA Req frame은 반드시 Timeout이 발생한 psdu가 전송되었던 링크와 같은 링크를 통해 전송됨 (자연스럽게 같은 링크에서 부분 재전송 수행)
* (⭐ 매우 중요) 향후 재전송을 수행 하지 못한 seq # 1812에 해당하는 mpdu는 link 2가 아닌 link 1에서 재전송이 수행될 수 있음
* 결론적으로, recipient MPDU buffer state에 제약을 받는 경우를 생각해보면 다음과 같음
  * 근본적으로 Buffer의 크기가 작을 때 (특히, 다중 링크 동작과 같이 높은 처리량을 제공하는 네트워크 환경에서)
  * 충돌이 발생할 때 (재전송으로 인해 MPDU buffer queue의 winStart 부분이 sliding 되지 않기 때문에)
* 그럼 항상 큰 buffer size가 좋은가?에 대한 고민을 해보면, 당연히 아님 (충돌이 발생했을 때 overhead가 너무 큼)
  * trade-off 관계를 따져가며 실험을 해보는거 괜찮을 거 같음

### Summary
* 수신기의 Buffer Size의 한계로 인한 aggregation size의 제약이 발생할 수 있음 (특히, 작은 buffer size와 잦은 충돌이 발생할 때)
* Buffer에 존재하는 MPDU들의 state 정보는 in-flight, need tx, need re-tx, recived ACK 등 다양한 상태가 존재할 수 있음
* Appendix A ~ D를 통해 MAC 계층에서의 기본적인 동작에 대한 이해를 함
  * 남은건 Backoff procedure, MAC Queue scheduling 정도..? 필요하다면 Appendix는 계속 추가할 예정
