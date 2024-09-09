# MLO Analysis
Analysis of IEEE 802.11be multi-link operation and performance using ns-3 simulator

## Keywords
IEEE 802.11be multi-link operation, Enhanced Distributed Channel Access

## Table of Contents
* [Overview](#overview)
* [Goal](#goal)
* [Package](#package)
* [Running](#running)
* [References](#references)

## Overview

## Goal
* 

## Package
* Ubuntu: 22.04
* ns-3: 3.40
* python: 3.12
* CMake: 3.29.6
* GCC: 11.4.0
  
## Running
* Task 1: IEEE 802.11be asynchronous multi-link operation with EDCA 환경에서의 retransmission case 구분
  * Case 1.1: 재전송되는 패킷이 이전과 동일한 채널로 전송되는 경우
    - 실험 환경: **topology** (1AP + 2STA), **distance** (10m), **Channel** (2.4GHz 20MHz + 5GHz 20MHz), **traffic flow** (uplink only), **data rate** (Video: 100Mbit/s + Best Effort: 100Mbit/s)
  * Case 1.2: 재전송되는 패킷이 이전과 다른 채널로 전송되는 경우
  * Case 2.1: 재전송되는 패킷이 내부 EDCA contetnion 에서 승리하여 즉시 전송되는 경우
  * Case 2.2: 재전송되는 패킷이 내부 EDCA contention 에서 패배하여 지연되는 경우

  
## References


## Note
* ns3 <-> analyzer 간 ip 동기화가 안맞음 -> AP가 192.168.1.1 / STA이 192.168.1.2 부터 할당되게 변경
* module/json2csv.py 함수 load_df에서 AP 데이터 기준 json에서 dataframe으로 변환이 안됨 -> uplink 통신이므로 구분을 위해 target 매개변수 값 추가
* module/json2csv.py 구현 완료: pcap <-> json <-> csv 변환 완료

* 실험 환경
  * **Topology** (1AP + 2STA)
  * **Distance** (10m)
  * **Channel** (2.4GHz 20MHz + 5GHz 20MHz)
  * **Traffic flow** (uplink only)
  * **Data rate** (Video: 100Mbit/s + Best Effort: 100Mbit/s)
* link 1 (2.4GHz, 20MHz)에서 발생한 아래와 같은 재전송 사건에 대한 분석
  ```
  1. 1.024796s STA2 -> AP (AC_VI, A-MPDU ID 36: #538 ~ #566) 패킷 송신
  2. 1.062315s STA2 -> AP #538 ~ #565 패킷 재전송
  3. 1.066485s STA2 -> AP #566, #741 ~ #768 패킷 재전송
  4. 1.068316s AP에서 STA2가 재전송한 #538 패킷 수신
  5. 1.072622s AP에서 STA2가 재전송한 #566, #741 ~ #768 패킷 수신
  ```
  
* 1.024796 ~ 1.068316 시간 근처에 발생한 모든 송수신 패킷 리스트업
* AP 입장
  * 1.024648s, STA1에서 전송된 (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 수신
  * 1.034995s, STA1에서 전송된 (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 수신
  * 1.039131s, STA1에서 전송된 (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 수신
  * 1.043285s, STA1에서 전송된 (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 수신
  * 1.047431s, STA1에서 전송된 (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 수신
  * 1.064200s, STA1에서 전송된 (AC_BE, A-MPDU ID 39: #39 ~ #77) 패킷 수신
  * 1.068316s, STA2에서 전송된 (AC_VI, A-MPDU ID 41: #538 ~ #565) 패킷 수신
  * 1.072622s, STA2에서 전송된 (AC_VI, A-MPDU ID 43: #566, #741 ~ #768) 패킷 수신
   
* STA1 입장
  * 1.018510s, STA1->AP (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 송신
  * 1.028858s, STA1->AP (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 송신
  * 1.032994s, STA1->AP (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 송신
  * 1.037148s, STA1->AP (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 송신
  * 1.041293s, STA1->AP (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 송신
  * 1.045457s, STA1->AP (AC_VI, A-MPDU ID 39: #174 ~ #202) 패킷 송신
  * 1.056690s, STA1->AP (AC_BE, A-MPDU ID 41: #39 ~ #77) 패킷 송신
   
* STA2 입장
  * 1.024796s, STA2->AP (AC_VI, A-MPDU ID 36: #538 ~ #566) 패킷 송신
  * 1.051254s, STA2->AP (AC_BE, A-MPDU ID 42: #234 ~ #272) 패킷 송신
  * 1.062315s, STA2->AP (AC_VI, A-MPDU ID 45: #538 ~ #565) 패킷 송신 (재전송)
  * 1.066485s, STA2->AP (AC_VI, A-MPDU ID 47: #566, #741 ~ #768) 패킷 송신 (부분 재전송)
   
* 시간 순으로 나열
  * Need Tx -> A-MPDU가 모종의 원인으로 인해 손실되어 재전송이 필요한 경우
  * Partially Tx/Rx -> 단일 A-MPDU에 재전송과 일반 전송에 해당하는 MPDU가 공존하는 경우
  * Totally Tx/Rx -> 단일 A-MPDU에 포함된 모든 MPDU들이 재전송에 해당하는 경우

| No  | Time      | Description                  | AC  | A-mpdu ID | Wlan Seq #        | Retry        |
| :-: | :-------: | :--------------------------: | :-: | :-------: | :---------------: | :----------: |
| 1   | 1.018510s | `STA1` -> AP transmit A-MPDU | VI  | 34        | #29 ~ #57         | -            |
| 2   | 1.024648s | AP <- `STA1` receive A-MPDU  | VI  | 34        | #29 ~ #57         | -            |
| 3   | 1.024796s | `STA2` -> AP transmit A-MPDU | VI  | 36        | #538 ~ #566       | Need Tx      |
| 4   | 1.028858s | `STA1` -> AP transmit A-MPDU | VI  | 35        | #58 ~ #86         | -            |
| 5   | 1.032994s | `STA1` -> AP transmit A-MPDU | VI  | 36        | #87 ~ #115        | -            |
| 6   | 1.034995s | AP <- `STA1` received A-MPDU | VI  | 35        | #58 ~ #86         | -            |
| 7   | 1.037148s | `STA1` -> AP transmit A-MPDU | VI  | 37        | #116 ~ #144       | -            |
| 8   | 1.039131s | AP <- `STA1` received A-MPDU | VI  | 36        | #87 ~ #115        | -            |
| 9   | 1.041293s | `STA1` -> AP transmit A-MPDU | VI  | 38        | #145 ~ #173       | -            |
| 10  | 1.043285s | AP <- `STA1` received A-MPDU | VI  | 37        | #116 ~ #144       | -            |
| 11  | 1.045457s | `STA1` -> AP transmit A-MPDU | VI  | 39        | #174 ~ #202       | Need Tx      |
| 12  | 1.047431s | AP <- `STA1` received A-MPDU | VI  | 38        | #145 ~ #173       | -            |
| 13  | 1.051254s | `STA2` -> AP transmit A-MPDU | BE  | 42        | #234 ~ #272       | Need Tx      |
| 14  | 1.056690s | `STA1` -> AP transmit A-MPDU | BE  | 41        | #39 ~ #77         | -            |
| 15  | 1.062315s | `STA2` -> AP transmit A-MPDU | VI  | 45        | #538 ~ #565       | Totally Tx   |
| 16  | 1.064200s | AP <- `STA1` received A-MPDU | VI  | 39        | #39 ~ #77         | -            |
| 17  | 1.066485s | `STA2` -> AP transmit A-MPDU | VI  | 47        | #566, #741 ~ #768 | Partially Tx |
| 18  | 1.068316s | AP <- `STA2` received A-MPDU | VI  | 41        | #538 ~ #565       | Totally Rx   |
| 19  | 1.072622s | AP <- `STA2` received A-MPDU | VI  | 43        | #566, #741 ~ #768 | Partially Rx |

* 그림으로 표현
  * 파란색 및 초록색 block에 표기되어 있는 번호는 A-mpdu ID를 나타냄
  * 1 칸당 약 1ms를 의미함
  * 가독성을 위해 시간은 소수점 아래 3번째 자리까지 표현 (이하 반올림)
  * (⭐중요) 아래 그림은, `특정 device의 입장`이 아닌 `각 device의 입장`을 기준으로 나타낸 그림
  
  ![image](https://github.com/user-attachments/assets/15550ab9-f94c-4301-8209-16c9b07433b8)

* 논의사항
  * STA이 전송할 때 사용한 A-mpdu ID와 AP가 수신받은 A-mpdu ID가 다름
    * e.g., 1.057s 시점 (No. 14)에서 STA2가 전송한 A-mpdu ID: 41, 1.064s 시점 (No. 16)에서 AP가 수신한 A-mpdu ID: 39
    * 예상: A-mpdu ID는 특정 device에서 aggregation 된 패킷을 구분하기 위해 사용
    * 근거: 각 device 입장에서 보면 A-mpdu의 AC가 다르더라도 ID는 sequential하게 증가함
    * 정답: https://www.radiotap.org/fields/A-MPDU%20status.html
      
  * 1.024796s 시점(No. 3)에서 STA2가 전송한 패킷이 손실된 이유
    * 예상: 1.025s 시점에서 매우 작은 시간 차이로 간섭이 발생 (i.e., 1.024796s - 1.024648s = 0.148ms)
    * 근거 1: AP가 STA1이 전송한 패킷에 대해 BA를 처리하는 시점에서 STA의 BO가 0에 도달하고, 전송하는 부분에서 간섭이 발생
    * 근거 2: STA 별로 backoff procedure는 독립적으로 동작하기 때문에 발생할 수 있음
    * 정답: 교수님께 여쭤보기

  * (⭐중요) 특정 TXOP를 획득했을 때 송신한 A-mpdu에 포함된 wlan seq #와 수신한 A-mpdu에 대한 BA의 wlan seq #는 다를 수 있음
    * EDCA 표준에 근거하여, VI TXOP Limit: 4.096ms
    * 따라서, 1.032994s 시점 (No. 5)에 획득한 VI TXOP는 ~ 1.03709s 시점까지 유효 (i.e., 1.037148s 시점에 획득한 VI TXOP는 새로운 TXOP임)
    * 즉, 1.032994s 시점 (No. 5)에 획득한 VI TXOP는 1.034995s 시점 (No.6)의 로그까지 유효
    * No. 5 - 송신 A-mpdu wlan seq #: 87 ~ 115
    * No. 6 - 수신 A-mpdu wlan seq #: 58 ~ 86

  * (⭐중요) 1.045457s 시점 (No. 11)에서 전송한 39번 A-mpdu의 BA는 어디있지?
    * 증명을 위해서는, No. 11 사건과 동일한 No. 5, 7, 9 사건의 latency 측정이 필요
    * 여기서 `동일한 사건`의 의미: 동일한 channel, STA, AC, Aggregation size
    * No. 5: 1.039131s - 1.032994s = 6.137ms
    * No. 7: 1.043285s - 1.037148s = 6.137ms
    * No. 9: 1.047431s - 1.041293s = 6.138ms
    * 위 latency에 기반하여, No. 11에서 전송한 패킷에 대해 AP의 추정 수신 시간: 1.045457s + 0.006137s = 1.051594s
    * 한편, 1.051254s 시점 (No. 13)에 STA2의 backoff procedure가 종료되고, AP로 42번 A-mpdu 전송
    * **STA1에서 전송된 패킷에 대한 AP의 추정 수신 시간**과 **STA2가 패킷을 전송한 시간** 간격: 1.051594s - 1.051254s = 0.34ms
    * 따라서, 간섭으로 인해 AP가 STA1이 전송한 39번 A-mpdu에 대한 BA를 송신하지 못함

  * (⭐중요) 위 논의사항과 연계하여, 1.051245s 시점 (No. 13)에서 전송한 STA2의 42번 A-mpdu는 손실되었을까?
    * No. 11 로그 추가 분석 
    ```
    1. 1.045457s STA1 -> AP (AC_VI, A-MPDU ID 39: #174 ~ #202) A-mpdu 송신
    2. 1.084516s STA1 -> AP #174 ~ #201 A-mpdu 재전송
    3. 1.090517s AP에서 STA1가 재전송한 #174 ~ #201 A-mpdu 수신
    ```
    * Not identified issue: #202는 어디갔지...로그 봤는데 아무데도 없음... 예상컨데, 설정된 throughtput의 값이 너무 커서 MAC queue에 이슈가 있는거 같음
    * No. 13 로그 추가 분석 
    ```
    1. 1.051254s STA2 -> AP (AC_BE, A-MPDU ID 42: #234 ~ #272) A-mpdu 송신
    2. 1.074739s STA2 -> AP #234 ~ #272 A-mpdu 재전송
    3. 1.082250s AP에서 STA2가 재전송한 #234 ~ #272 A-mpdu 수신
    ```
    * 따라서, No. 11과 No. 13에 송신한 STA1 및 STA2의 A-mdpu 모두 손실

  * (⭐중요) VI와 BE의 손실된 패킷을 복구하기 위한 재전송 방식이 다르다!?
    * 전송하는 A-mpdu의 AC가 VI인 경우, 원본 A-mpdu가 분할되어 재전송
    * 반면, A-mpdu의 AC가 BE인 경우, 원본 A-mpdu와 동일한 A-mpdu가 재전송
    * AC에 따른 TXOP Limit 값과 연관성이 있음
    * 자세한 분석은 [Appendix A](#appendix-a) 참조
    ```
    1. 1.024796s STA2 -> AP (AC_VI, A-MPDU ID 36: #538 ~ #566) 패킷 송신
    2. 1.062315s STA2 -> AP #538 ~ #565 패킷 재전송
    3. 1.066485s STA2 -> AP #566, #741 ~ #768 패킷 재전송
    4. 1.068316s AP에서 STA2가 재전송한 #538 패킷 수신
    5. 1.072622s AP에서 STA2가 재전송한 #566, #741 ~ #768 패킷 수신
    ```

    ```
    1. 1.045457s STA1 -> AP (AC_VI, A-MPDU ID 39: #174 ~ #202) A-mpdu 송신
    2. 1.084516s STA1 -> AP #174 ~ #201 A-mpdu 재전송
    3. 1.090517s AP에서 STA1가 재전송한 #174 ~ #201 A-mpdu 수신
    ```
        
  * 결론적으로 예쁘게 정리
    ![image](https://github.com/user-attachments/assets/95589418-12a4-460c-ad57-c5842ecccb1b)

## Appendix A
  * WifiNetDevice architecture. For 802.11be Multi-Link Devices (MLDs), there as many instances of WifiPhy, FrameExchangeManager and ChannelAccessManager as the number of links.
  * Reference: https://www.nsnam.org/docs/release/3.40/models/singlehtml/index.html#document-wifi)
    
  ![image](https://www.nsnam.org/docs/release/3.40/models/singlehtml/_images/WifiArchitecture.png)

  * 전체 flow를 보기위해 최하위 계층 wifi-phy.cc의 Send()에서 breakpoint를 걸어야함
  * 아래 코드를 통해 wifi.phy.cc에서 ppdu->psdu->mpdu_list->mpdu 접근 가능
  ```c
  auto ptr = ppdu->GetPsdu()->begin();
  for(int i = 0; i < (int)ppdu->GetPsdu()->GetNMpdus(); i++){
    NS_LOG_UNCOND(ptr[i]->GetHeader());
  }
  ```

  * 여기서 mpdu header의 retry, wlan seq#, AC 확인은 
  ```c
  ptr[i]->GetHeader().IsRetry(); // 1: retry, 0: no retry
  ptr[i]->GetHeader().GetSequenceNumber(); // wlan seq #
  ptr[i]->GetHeader().GetQosTid(); // 3: AC_BE, 5: AC_VI
  ```

  * 따라서, No. 13 로그 분석을 위해 아래와 같은 코드를 작성하고 실행하면
  ```
  1. 1.051254s STA2 -> AP (AC_BE, A-MPDU ID 42: #234 ~ #272) A-mpdu 송신
  2. 1.074739s STA2 -> AP #234 ~ #272 A-mpdu 재전송
  3. 1.082250s AP에서 STA2가 재전송한 #234 ~ #272 A-mpdu 수신
  ```
  
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
  
  * 시간 동기화가 안맞다. wifi-phy.cc에서 전송한 시간과 pcap에서 캡처된 시간이 다른 대신, 간격은 유사하다
    * ns-3 wifi-phy.cc: 1.11826s - 1.09477s = 23.49ms
    * wireshark: 1.074739s - 1.051254s = 23.485ms
  
  * 이제 wifi-phy.cc에 bp 걸고 함수 call stack을 보면
<p align="center"><img src="https://github.com/user-attachments/assets/71dd9b80-1c03-4f7e-a96e-8adb51d30208"</p>

  * 하나씩 순서대로 뜯자.
  * 1. ns3::ChannelAccessManager::AccessTimeout
```c
void
ChannelAccessManager::AccessTimeout()
{
  NS_LOG_FUNCTION(this);
  UpdateBackoff();
  DoGrantDcfAccess();
  DoRestartAccessTimeoutIfNeeded();
}
```
  * backoff update 말고 뭐 없다 패스

  * (⭐중요) 2. ns3::ChannelAccessManager::DoGrantDcfAccess
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
            if (m_feManager->StartTransmission(txop, width))
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
  * 이때, 획득한 TXOP의 전송이 완료된 후 (성공하거나 실패하거나 어쨋든) internal contention txop 계산(backoff 재계산 등) 수행
    * 구현 이슈인거 같음: 변경 사항을 획득한 txop의 전송 전에 적용하면 전역 변수 변경과 같은 문제 야기
  * 내부 경쟁이 끝나면 EDCAF에서 txop에 대한 전송을 시작함.

  * 3. ns3::EhtFrameExchangeManager::StartTransmission
```c
bool
EhtFrameExchangeManager::StartTransmission(Ptr<Txop> edca, uint16_t allowedWidth)
{
    NS_LOG_FUNCTION(this << edca << allowedWidth);

    auto started = HeFrameExchangeManager::StartTransmission(edca, allowedWidth);

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

  * 4. ns3::QosFrameExchangeManager::StartTransmission
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
    return StartTransmission(qosTxop, qosTxop->GetTxopLimit(m_linkId));
}
```
  * PCF: AP가 통신 관장, STA 들에게 데이터를 전송할 수 있는 기회 할당 (주로 EDCA, Real-time trafifc에서 사용하는 통신 방식)
  * PIFS: DCF 모드가 아닌 PCF 모드에서 주로 사용되며, DCF 보다는 짧고, SIFS 보다는 길다.
  * 언제 사용? PCF 통신 모드가 끝난 후 STA 들에게 제어 프레임을 전송할 때 빠른 채널 복구를 위해 사용
  * 관련 없음. 패스

  * (⭐매우 중요 여기가 거의 9할 이라고 해도 무방) 5. ns3::QosFrameExchangeManager::StartTransmission
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
