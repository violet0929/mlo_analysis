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
* python: 3.12
* pip: 24.2
* pytorch: 2.3.1+cu118
* numpy: 1.26.0
* pandas: 2.2.2
* matplotlib: 3.9.0

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

  * 실험 환경: **topology** (1AP + 2STA), **distance** (10m), **Channel** (2.4GHz 20MHz + 5GHz 20MHz), **traffic flow** (uplink only), **data rate** (Video: 100Mbit/s + Best Effort: 100Mbit/s)기반
    link 1 (2.4GHz, 20MHz)에서 발생한 아래와 같은 재전송 사건에 대한 분석 (1.024796 ~ 1.068316 시간 근처에 발생한 모든 송수신 패킷을 리스트업 해보면...)  
  __* 1.024796 STA2 -> AP (AC_VI, A-MPDU ID 36: #538 ~ #566) 패킷 송신__  
  __* 1.062315 STA2 -> AP #538 패킷 재전송__  
  __* 1.068316 AP STA2가 전송한 #538 패킷 수신__  

  * AP 입장
    * 1.024648 STA1에서 전송된 (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 수신
    * 1.034995 STA1에서 전송된 (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 수신
    * 1.039131 STA1에서 전송된 (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 수신
    * 1.043285 STA1에서 전송된 (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 수신
    * 1.047431 STA1에서 전송된 (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 수신
    * 1.064200 STA1에서 전송된 (AC_BE, A-MPDU ID 39: #39 ~ #77) 패킷 수신
    * 1.068316 STA2에서 전송된 (AC_VI, A-MPDU ID 41: #538 ~ #565) 패킷 수신
    * 1.072622 STA2에서 전송된 (AC_VI, A-MPDU ID 43: #566, #741 ~ #768) 패킷 수신
   
  * STA1 입장
    * 1.018510 STA1->AP (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 송신
    * 1.028858 STA1->AP (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 송신
    * 1.032994 STA1->AP (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 송신
    * 1.037148 STA1->AP (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 송신
    * 1.041293 STA1->AP (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 송신
    * 1.045457 STA1->AP (AC_VI, A-MPDU ID 39: #174 ~ #202) 패킷 송신
    * 1.056690 STA1->AP (AC_BE, A-MPDU ID 41: #39 ~ #77) 패킷 송신
   
  * STA2 입장
    * 1.024796 STA2->AP (AC_VI, A-MPDU ID 36: #538 ~ #566) 패킷 송신
    * 1.051254 STA2->AP (AC_BE, A-MPDU ID 42: #234 ~ #272) 패킷 송신
    * 1.062315 STA2->AP (AC_VI, A-MPDU ID 45: #538 ~ #565) 패킷 송신 (재전송)
    * 1.066485 STA2->AP (AC_VI, A-MPDU ID 47: #566, #741 ~ #768) 패킷 송신 (부분 재전송)
   
  * 시간 순으로 나열해보면
    (1). 1.018510 STA1->AP (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 송신
       + 6.138ms
    (2). 1.024648 STA1에서 전송된 (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 수신 (latency: 6.138ms)
       + 0.148ms
    (3). 1.024796 STA2->AP (AC_VI, A-MPDU ID 36: #538 ~ #566) 패킷 송신
       + 4.062ms
    4. 1.028858 STA1->AP (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 송신
       + 4.136ms
    5. 1.032994 STA1->AP (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 송신
       + 2.001ms
    6. 1.034995 STA1에서 전송된 (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 수신 (latency: 6.137ms)
       + 2.153ms
    7. 1.037148 STA1->AP (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 송신
       + 1.983ms
    8. 1.039131 STA1에서 전송된 (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 수신 (latency: 6.137ms)
       + 2.162ms
    9. 1.041293 STA1->AP (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 송신
       + 1.992ms
    10. 1.043285 STA1에서 전송된 (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 수신 (latency: 6.137ms)
       + 2.172ms
    11. 1.045457 STA1->AP (AC_VI, A-MPDU ID 39: #174 ~ #202) 패킷 송신
       + 1.974ms  
    12. 1.047431 STA1에서 전송된 (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 수신 (latency: 6.138ms)
       + 3.823ms  
    13. 1.051254 STA2->AP (AC_BE, A-MPDU ID 42: #234 ~ #272) 패킷 송신
       + 5.436ms
    14. 1.056690 STA1->AP (AC_BE, A-MPDU ID 41: #39 ~ #77) 패킷 송신
       + 5.625ms  
    15. 1.062315 STA2->AP (AC_VI, A-MPDU ID 45: #538 ~ #565) 패킷 송신 (재전송)
       + 1.885ms  
    16. 1.064200 STA1에서 전송된 (AC_BE, A-MPDU ID 39: #39 ~ #77) 패킷 수신 (latency: 7.51ms)
       + 2.285ms  
    17. 1.066485 STA2->AP (AC_VI, A-MPDU ID 47: #566, #741 ~ #768) 패킷 송신 (부분 재전송)
       + 1.831ms  
    18. 1.068316 STA2에서 전송된 (AC_VI, A-MPDU ID 41: #538 ~ #565) 패킷 수신 (재전송 기준 latency: 6.001ms, 전송 기준: 43.52ms)
       + 4.306ms  
    19. 1.072622 STA2에서 전송된 (AC_VI, A-MPDU ID 43: #566, #741 ~ #768) 패킷 수신
      - #566 재전송 기준 latency: 10.307ms, 전송 기준: 47.826ms)
      - #741 ~ #768 latency: 6.137ms
  * 도표로 나타내면
    ![image](https://github.com/user-attachments/assets/15550ab9-f94c-4301-8209-16c9b07433b8)

  * Insight
    1. STA이 전송할 때 사용한 A-MPDU ID와 AP가 수신받은 A-MPDU ID가 다르다
      - 근거: 14번 사건에서 STA2가 전송한 A-MPDU ID: 41, 16번 사건에서 AP가 수신한 A-MPDU ID: 39
      - 예상컨데, A-MPDU ID는 특정 device에서 aggregation 된 패킷을 구분하기 위해 사용한다.
      - 근거: 각 device 입장에서 보면 A-MPDU의 AC가 다르더라도 ID는 sequential하게 증가함
      **- 정답: https://www.radiotap.org/fields/A-MPDU%20status.html**
    2. 3번 사건에서 STA2가 전송한 패킷이 손실된 이유가 뭘까?
      - 2번 사건과 3번 사건의 시간차는 0.148ms로 매우 낮음.
      - 따라서, AP가 STA1이 전송한 패킷에 대해 BA를 처리하는 시점에서 STA의 BO가 0에 도달하고, 전송하는 부분에서 간섭이 발생함.
      - 근거: STA 별로 Backoff procedure는 독립적으로 동작하기 때문에
      **- 정답: 교수님께 여쭤보기**
