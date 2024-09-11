## Appendix. B. AC_VI retransmssion

### Objective
* AC_BE 및 AC_VI의 서로 다른 재전송 과정이 발생하는 원인을 찾기 위해
* 해당 문서에서는 ns-3.40 기반 AC_VI의 재전송 수행 과정을 분석함
* 앞서 분석한 내용 (Appendix. A. 참고)과 공통되는 부분은 다루지 않음

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
* 이제 중요한 것만 하나씩 뜯자
  
### 1. ns3::QosFrameExchangeManager::StartTransmission
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

        if (!StartFrameExchange(m_edca, m_edca->GetRemainingTxop(m_linkId), false)) // BREAKPOINT
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


