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
  * pcap <-> json <-> csv 변환 완료
  
