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


## Insight
이 꽉깨물고 9월안에 무조건 끝낸다

* 24.09.06 (1일차)
  * ns3 <-> analyzer 간 ip 동기화가 안맞음 -> AP가 192.168.1.1 / STA이 192.168.1.2 부터 할당되게 변경
  * module/json2csv.py 함수 load_df에서 AP 데이터 기준 json에서 dataframe으로 변환이 안됨 -> uplink 통신이므로 구분을 위해 target 매개변수 값 추가
  * module/json2csv.py 구현 완료: pcap <-> json <-> csv 변환 완료

  * link 1 (2.4GHz, 20MHz)에서 발생한 아래와 같은 재전송 사건 분석 (1.024796 ~ 1.068316 시간 근처에 발생한 모든 송수신 패킷을 리스트업 해보면...)  
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
    * 1.018510 STA1->AP (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 송신
    * 1.024648 STA1에서 전송된 (AC_VI, A-MPDU ID 34: #29 ~ #57) 패킷 수신 (latency: 6.138ms)
    * 1.024796 STA2->AP (AC_VI, A-MPDU ID 36: #538 ~ #566) 패킷 송신
    * 1.028858 STA1->AP (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 송신 (위 내용과 4.062ms 차이)
    * 1.032994 STA1->AP (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 송신
    * 1.034995 STA1에서 전송된 (AC_VI, A-MPDU ID 35: #58 ~ #86) 패킷 수신 (latency: 6.137ms)
    * 1.037148 STA1->AP (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 송신
    * 1.039131 STA1에서 전송된 (AC_VI, A-MPDU ID 36: #87 ~ #115) 패킷 수신 (latency: 6.137ms)
    * 1.041293 STA1->AP (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 송신
    * 1.043285 STA1에서 전송된 (AC_VI, A-MPDU ID 37: #116 ~ #144) 패킷 수신 (latency: 6.137ms)
    * 1.045457 STA1->AP (AC_VI, A-MPDU ID 39: #174 ~ #202) 패킷 송신
    * 1.047431 STA1에서 전송된 (AC_VI, A-MPDU ID 38: #145 ~ #173) 패킷 수신 (latency: 6.138ms)
    * 1.051254 STA2->AP (AC_BE, A-MPDU ID 42: #234 ~ #272) 패킷 송신
    * 1.056690 STA1->AP (AC_BE, A-MPDU ID 41: #39 ~ #77) 패킷 송신
    * 1.062315 STA2->AP (AC_VI, A-MPDU ID 45: #538 ~ #565) 패킷 송신 (재전송)
    * 1.064200 STA1에서 전송된 (AC_BE, A-MPDU ID 39: #39 ~ #77) 패킷 수신 (latency: 7.51ms)
    * 1.066485 STA2->AP (AC_VI, A-MPDU ID 47: #566, #741 ~ #768) 패킷 송신 (부분 재전송)
    * 1.068316 STA2에서 전송된 (AC_VI, A-MPDU ID 41: #538 ~ #565) 패킷 수신 (재전송 기준 latency: 6.001ms, 전송 기준: 43.52ms)
    * 1.072622 STA2에서 전송된 (AC_VI, A-MPDU ID 43: #566, #741 ~ #768) 패킷 수신
      - #566 재전송 기준 latency: 10.307ms, 전송 기준: 47.826ms)
      - #741 ~ #768 latency: 6.137ms

  * Insight
    - VI가 채널을 획득했을 때 obtain하는 시간은 4.096ms 이다. (EDCA 표준에 근거하여)
    - 졸리다....
