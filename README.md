# MLO Analysis
Analysis of IEEE 802.11be multi-link operation and performance using ns-3 simulator

## Keywords
IEEE 802.11be multi-link operation, Enhanced Distributed Channel Access

## Table of Contents
* [Overview](#overview)
* [Package](#package)
* [Task](#task)
* [Supplementary](#supplementary)
* [References](#references)

## Overview
* ns3 기반 asynchronous multi-link operation with EDCA의 동작 및 코드 분석은 [여기](https://github.com/violet0929/mlo_analysis/tree/main/Appendix)를 확인해주세요
  * A. [AC_BE Retransmission](https://github.com/violet0929/mlo_analysis/blob/main/Appendix/Appendix_A.md)
  * B. [AC_VI Retransmission](https://github.com/violet0929/mlo_analysis/blob/main/Appendix/Appendix_B.md)
  * C. [Block ACK Request](https://github.com/violet0929/mlo_analysis/blob/main/Appendix/Appendix_C.md)
  * D. [MPDU Buffer Size](https://github.com/violet0929/mlo_analysis/blob/main/Appendix/Appendix_D.md)
  * E. [Latency](https://github.com/violet0929/mlo_analysis/blob/main/Appendix/Appendix_E.md)

### Introduction
* IEEE 802.11be 표준에서의 multi-link operation 기법은 초고처리량 (i.e., EHT) 및 저지연 (i.e., low latency)에 초점을 맞춤
  * latency portion
* 동시 송수신 (i.e., STR mode)이 가능한 비동기적 다중 링크 동작 환경은 단일 링크 동작 환경에 비해 재전송에 대한 복구 속도가 빠르기 때문에 지연 시간을 줄이는 것이 가능함
  * 논리적인 근거는?? MLD와 connection 되어 있는 여러 링크를 통해 TXOP를 획득하고 전송을 할 수 있기 때문
* 그럼에도 여전히 worst-case latency는 존재하며, 특히 EDCA 채널 접근 기법을 활용했을 때 상위 AC에 해당하는 데이터에 대해 측정된 높은 지연 시간은 지연에 민감한 AR/VR 및 real-time 특성을 갖는 트래픽에 대해 critical issue로 동작할 수 있음
* 따라서, Asynchronous multi-link operation with EDCA 환경에서, 상위 AC에 해당하는 트래픽의 latency를 줄일 수 있도록 하는 traffic allocation 방법론을 제안

### Related Works
* IEEE 802.11be multi-link operation 환경에서 dynamic traffic allocation을 하기 위한 다양한 방법들이 존재함 [Reference](https://arxiv.org/pdf/2202.12614)
  * Single Link Less Congested Interface (SLCI)
  * Multi Link Congestion-aware Load balancing at flow arrivals (MCAA)
  
### Background
* EDCA
  * EDCA parameter 설명 들어가야함
* MLO architecture
  * 기본 구조 설명 들어가야함
* Channel Access in Asynchronous multi-link operation with EDCA
  * 대역 간 간격이 충분하다 설명 들어가야함
  * STR 모드 언급해야함
  * 특징 언급해야함
  * 이에 따라서, 상위 AC 및 하위 AC에 해당하는 트래픽이 서로 다른 링크를 통해 전송될 수 있음

### 비동기적 multi-link operation에서 상위 AC에 해당하는 mpdu의 worst case latency가 측정되는 원인 분석
* 따라서, worst case latency가 측정되는 시나리오 기반 설명
  * (재전송 이슈)
    * BA req 및 BA을 수신 받고 재전송된 mpdu가 한번 더 손실이 발생했을 때
  * (내부 경쟁 이슈)
    * 상위 AC를 가지는 트래픽이 하위 AC의 트래픽에 의해 추가 전송 지연시간이 발생했을 때

### 특정 link를 primary channel로 사용할 수 있도록 하는 방법
* 핵심은 하위 AC에 해당하는 트래픽을 일시적으로 block
* 따라서, 상위 AC에 해당하는 트래픽이 EDCA 내부 경쟁없이 채널을 독점적으로 사용할 수 있음

* (⭐ 중요) 독점 state를 언제 invoke하고 release 할 것인가
  * invoke: 상위 AC에 해당하는 mpdu가 손실 발생하고, 송신된 BA Req에 대한 BA이 수신되었을 때 invoke
    * 논란의 여지가 없음
    * 구현 가능
  * release: 이전에 손실된 mpdu가 성공적으로 재전송 되었으며, 이에 대한 BA이 수신되었을 때 release
    * 재전송에 성공하고 release를 하면, originator 입장에서는 recipient가 성공적으로 수신했는지 알 수 없음
    * 따라서, BA을 수신받고 release 처리를 수행하는게 맞음
    * 구현은 어떻게 할 건가??

* (⭐중요) 부차적인 문제
  * 공정성 문제: 하위 AC 트래픽이 block 되었을 때, 얼마만큼의 네트워크 손실이 발생하는가 (처리량, 지연 등...)
    * primary channel로 인한 이득과 손실에 대한 trade-off 관계를 논리적으로 잘 풀어야 함
      
  * 100% 신뢰성을 보장할 수 없음: 여전히 다른 STA의 전송으로 인한 간섭을 막을 수 없음
    * 작은 control mpdu를 통해 동기화 과정이 가능은 하지만, 실제 상황에서 동작하기 매우 어려움 (모든 device가 해당 패킷을 인지해야하는 문제도 있음)

### Evaluation
* 성능 지표 (retry ratio + 95th latency 및 99th latency 추가, 환경 5GHz 80MHz + 6GHz 80Mhz 추가)
  * 2.4GHz 20MHz + 5GHz 20MHz -> simulation time 10sec
  * 5GHz 80MHz + 6GHz 80MHz -> simulation time 5sec
 
  * retry가 왜 늘었는가?
 
## Package
* Ubuntu: 22.04
* ns-3: 3.40
* python: 3.12
* CMake: 3.29.6
* GCC: 11.4.0
  
## Task
* Task 1. IEEE 802.11be asynchronous multi-link operation with EDCA 환경에서의 retransmission case 구분
  * 공통 실험 환경
    * **Topology** (1AP + 2STA)
    * **Distance** (10m)
    * **Channel** (2.4GHz 20MHz + 5GHz 20MHz)
    * **Traffic flow** (uplink only)
    * **Data rate** (Video: 100Mbit/s + Best Effort: 100Mbit/s)
      
  * Case 1.1. 재전송되는 패킷이 이전과 동일한 채널로 전송되는 경우

    * 
    * 자세한 분석은 Supplementary - Case 1.1. 참고
  * Case 1.2: 재전송되는 패킷이 이전과 다른 채널로 전송되는 경우
  * Case 2.1: 재전송되는 패킷이 내부 EDCA contention 에서 승리하여 즉시 전송되는 경우
  * Case 2.2: 재전송되는 패킷이 내부 EDCA contention 에서 패배하여 지연되는 경우


## Evaluation
* Evaluation 1. latency
  * Avg latency: 전체 mpdu에 대해 지연 시간에 대해 평균에 해당하는 시간
  * 95th latency: 전체 mpdu에 대해 지연 시간에 대해 95번째 백분위 수에 해당하는 시간
  * 99th latency: 전체 mpdu에 대해 지연 시간에 대해 99번째 백분위 수에 해당하는 시간

  * 전체 mpdu 개수: N, 백분위수: P라 가정하면 인덱스 i는 다음과 같음 (i = P / 100 * (N - 1)) 
  * 이후 linear interpolation을 통해 값 도출

  
## Supplementary


## References
* A Comparison of Neural Networks for Wireless Channel Prediction
  * IEEE Wireless Communications, 2024

* Is Multi-Link Operation of 802.11be TCP Friendly? Analysis and Solution
  * IFIP Networking Conference, 2024
 
* IEEE 802.11 be Wi-Fi 7: New challenges and opportunities
  * IEEE Communications Surveys & Tutorials, 2020

* Current status and directions of IEEE 802.11 be, the future Wi-Fi 7
  * IEEE Access 8, 2020
 
* Multi-link operation with Enhanced Synchronous Channel Access in IEEE 802.11be Wireless LANs: Coexistence Issue and Solutions
  * Sensors, 2021
 
* Performance and coexistence evaluation of IEEE 802.11 be multi-link operation
  * IEEE Wireless Communications and Networking Conference, 2023
 
* Multilink operation in IEEE 802.11 be wireless LANs: Backoff overflow problem and solutions
  * Sensors, 2022
 
* Latency Impact for Massive Real-Time Applications on Multi Link Operation
  * IEEE Region 10 Symposium, 2021
 
* A Load Adaptive IEEE 802.11e EDCA Backoff Scheme with Enhanced Service Differentiation
  * IEEE 12th International Conference on Communication Technology, 2010
 
* Analytical Study of the IEEE 802.11p EDCA Mechanism
  * IEEE Intelligent Vehicles Symposium, 2013
 
* Adaptive Backoff Algorithm for EDCA in the IEEE 802.11p protocol
  * International Wireless Communications and Mobile Computing Conference, 2016
 
* IEEE 802.11 ax: Highly efficient WLANs for intelligent information infrastructure
  * IEEE Communications Magazine, 2017
 
* A review of OFDMA and Single-Carrier FDMA
  * European Wireless Conference, 2010
 
* Spatial Reuse in IEEE 802.11ax WLANs
  * Computer Communications, 2021

* Multi-Link Operation in IEEE802. 11be Extremely High Throughput: A Survey
  * IEEE Access, 2024

## Note
* Adaptive TXOP가 말이 안되는 이유
  * 그러니까, 완벽한 논문이라는게 존재할 수 있을까?
  * 좁은 대역 채널에서는 성립할 수 있음 (획득한 단일 TXOP에 대해 단일 A-mpdu 전송이 발생하기 때문)
  * 넓은 대역 채널에서는 성립할 수 없음 (획득한 단일 TXOP에 대해 여러번의 A-mpdu 전송이 발생하기 때문)
  * 즉, 좁은 대역에서는 마지막 seq#에 해당하는 mpdu가 분할되어 재전송되었지만, 넓은 대역에서는 분할되어 재전송되지 않음
  * 따라서, 의미가 없음 -> 그럼 좁은 대역에서만 발생하는 문제라고 하고 논문을 쓸 것인가? -> 이건 좀 아닌듯...
  * 물론, 실제로 multi-link operation이 20MHz 채널 사용할 수 있음 (그러나, 표준 자체가 EHT 및 6GHz 대역을 통한 광대역 통신을 지원하는데 20MHz??)

* 방법론을 틀어야함
  * 좁은 대역이든 넓은 대역이든 다 적용이 가능한 방법론이 필요함
  * IEEE 802.11be 표준이 worst-case latency를 줄이는데에 초점이 맞춰진 기술임
  * 따라서, worst-case latency를 포기하고 싶은 생각은 없음
  * 고민을 많이 해봐도 선제적인 대응이 아니면서, 이미 손실이 발생한 mpdu의 latency를 줄일 수 있는 방법 같은게 있을까? primary link traffic allocation 말고는 없는듯.. 
  * 그래서 그냥 multi-link operation with EDCA에서, traffic allocation을 제안하는게 낫지 않나...
