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
