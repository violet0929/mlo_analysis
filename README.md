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
* IEEE 802.11be multi-link operation -> EHT 및 low latency 지원 (latency portion에 대한 reference 존재)
* 그럼에도 불구하고, EDCA를 기반으로 한 MLO 환경에서도 여전히 worst-case latency가 존재함
* 따라서, EDCA를 기반으로 한 MLO 환경 기반 worst case latency를 줄일 수 있는 방안 제안

### Background
* EDCA
* MLO architecture
* EDCA + Asynchronous MLO 환경에서의 channel access

### Understanding the process by which worst-case latency is derived in asynchronous multi-link operation with EDCA
* 따라서, worst case latency가 측정되는 시나리오 기반 설명
  * (재전송 이슈)
    * 같은 link를 통해 재전송되는 경우
    * 다른 link를 통해 재전송되는 경우
  * (내부 경쟁 이슈)
    * 상위 AC를 가지는 트래픽이 하위 AC의 트래픽에 의해 지연되는 경우

### Adaptive TXOP
* 결론적으로 해결할 수 있는 방안인 Adaptive TXOP 제안
* Compensation (optional)
* flow chart 추가

### Evaluation
* 성능 지표 (retry ratio + 95th latency 및 99th latency 추가, 환경 5GHz 80MHz + 6GHz 80Mhz 추가)
  * 2.4GHz 20MHz + 5GHz 80MHz -> simulation time 10sec <- (예외 처리가 심각함)
  * 5GHz 80MHz + 6GHz 80MHz -> simulation time 5sec
 
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



