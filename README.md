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
  ### 1. ns3::ChannelAccessManager::AccessTimeout (중요도 하)
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
  * 결론적으로, 내부 경쟁에서 승리한 EDCAF (즉, 특정 AC에 해당하는 txop)에 대한 전송을 시작함.
  * Note: 내부 경쟁에서 승리한 TXOP의 전송이 완료된 후 (성공하거나 실패하거나 어쨋든) 나머지 internal contention이 발생 TXOP에 대해 계산 (backoff 재계산 등) 수행
    * 구현 이슈인거 같음: 변경 사항을 획득한 txop의 전송 전에 적용하면 전역 변수 값 변경과 같은 문제 야기
  

  ### 3. ns3::EhtFrameExchangeManager::StartTransmission (중요도 하)
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
    return StartTransmission(qosTxop, qosTxop->GetTxopLimit(m_linkId));
}
```
  * PCF: AP가 통신 관장, STA 들에게 데이터를 전송할 수 있는 기회 할당 (주로 EDCA, Real-time trafifc에서 사용하는 통신 방식)
  * PIFS: DCF 모드가 아닌 PCF 모드에서 주로 사용되며, DCF 보다는 짧고, SIFS 보다는 길다.
  * 언제 사용? PCF 통신 모드가 끝난 후 STA 들에게 제어 프레임을 전송할 때 빠른 채널 복구를 위해 사용
  * 관련 없음. 패스

  ### 5. ns3::QosFrameExchangeManager::StartTransmission (⭐ 중요도 최상 여기가 3할)
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
  * 조건 1. 획득한 TXOP의 TXOP limit의 값이 0이상인 경우 (VO 및 VI에 해당)
    * 조건 1.1 남은 TXOP limit 0인 경우 (즉, 획득한 TXOP에 대해 첫 번째 프레임을 전송하는 경우)
      * StartFrameExchange(TXOP, 할당된 시간)
    * 조건 1.2 남은 TXOP limit 값이 0이 아닌 경우 (즉, 획득한 TXOP에 대해 첫 번째 프레임을 전송하는 것이 아닌 경우)
      * StartFrameExchange(TXOP, 남은 시간)
      * 만약 전송 실패한 경우, 남은 TXOP의 시간이 프레임을 전송하기에 충분하지 않은 시간이므로, CF-End 프레임 전송)
  * 조건 2. 획득한 TXOP의 TXOP limit의 값이 null인 경우 (BE 및 BK에 해당)
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
        return VhtFrameExchangeManager::StartFrameExchange(edca, availableTime, initialFrame);
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

  ### 7. ns3::HtFrameExchangeManager::StartFrameExchange (⭐ 중요도 최상 여기가 3할)
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
        return SendDataFrame(peekedItem, availableTime, initialFrame);
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
  * 조건 2.1. 일반적인 전송
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
  * false를 반환하는 조건문 1: recipient (수신 device)가 HT 표준을 지원하지 않는 경우 -> BlockAck 메커니즘은 802.11n(HT) 표준 이후로 사용됨
  * false를 반환하는 조건문 2: 이미 agreement가 존재하고, agreement의 상태가 reset이 아닌 경우 (originator와 recipent간의 blockack session이 유효한 경우)
  * 마지막 조건문에서 true가 반환되려면 아래 조건 3개 중 하나를 만족해야 함
    * 조건 1. 현재 할당된 TXOP의 BlockAck threshold가 0보다 크고, MAC queue에 있는 packet 개수가 threshold 보다 크거나 같은 경우
      * BlockAck threshold: MAC에서도 ACK를 지원하는데, 몇 개의 패킷을 보냇을때 Block ACK를 받을 건지 결정하는 값
    * 조건 2. 사전에 설정된 aggregator가 지원하는 최대 A-MPDU 크기가 0보다 크고, MAC queue에 있는 packet 개수가 0보다 큰 경우
    * 조건 3. RemoteStationManager가 VHT 표준을 지원하는 경우
      * ns-3 RemoteStationManager: 동일 link에 association되어 있는 모든 device를 관리하는 클래스
  > Note: 각 조건 별 debug 수행했을때, 조건 3에 걸리고 나머지 조건에는 안걸림 (초기 agreement가 establishment 이후 true를 반환하는 경우가 없었음. 즉, false를 반환하는 조건문에 안 걸린 경우가 없음)
 
  ### 8. ns3::HtFrameExchangeManager::SendDataFrame (⭐ 중요도 최상 여기가 3할)
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
        SendPsduWithProtection(Create<WifiPsdu>(std::move(mpduList)), txParams);
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
                nextMpdu =
                    qosTxop->GetNextMpdu(m_linkId, peekedMpdu, txParams, availableTime, false);
                /* 추가 */
                if (nextMpdu){
                    if(nextMpdu->GetHeader().GetSequenceNumber() == 272 && nextMpdu->GetHeader().GetQosTid() == 3){
                        NS_LOG_UNCOND("BP");
                    }
                }
                /* 추가 */
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
* 획득한 TXOP의 MAC Queue를 순회하면서 mpdu list를 만드는데 여기서 2가지의 조건을 기준으로 aggregation을 수행함
  * 조건 1. 현재 검색된 mpdu의 seq#가 현재 시점의 수신 device의 수신 윈도우 크기에 포함되어야 함
  ```c
  bool
  IsInWindow(uint16_t seq, uint16_t winstart, uint16_t winsize)
  {
    return ((seq - winstart + 4096) % 4096) < winsize;
  }
  ```
  * 조건 2. nextMpdu가 nullptr이 되기 전까지
* 근데, 조건 1에는 안걸림 (debug 해보면, 현재 시점의 수신 device의 수신 윈도우는 117 ~ 256이므로, 이렇게 따지면 140개가 aggregation 되야함)
* 조건 2에 걸림 그럼 왜 왜 #234 ~ #272까지 aggregation 되는지 확인해야함 (즉, #273은 왜 안되는지)
* 따라서, 추가적인 코드를 통해 BP 걸어주고 서브루틴 진입
* 서브루틴이 정말 많음
  * ns3::MpduAggregator::GetNextAmpdu
  * ns3::QosTxop::GetNextMpdu
  * ns3::QosFrameExchangeManager::TryAddMpdu
  * ns3::HtFrameExchangeManager::IsWithinLimitsIfAddMpdu
  * ns3::QosFrameExchangeManager::IsWithinSizeAndTimeLimits <- 여기서 답 찾을 수 있음 8.2.1. ns3::QosFrameExchangeManager::IsWithinSizeAndTimeLimits 참고
 
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
        (maxPpduDuration.IsStrictlyPositive() && txTime > maxPpduDuration))
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
