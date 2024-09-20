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
* ns3 기반 asynchronous multi-link operation with EDCA의 동작 및 코드 분석은 [여기](https://github.com/violet0929/mlo_analysis/tree/main/ns3-analyzer/code_analysis)를 확인해주세요
  * A. [AC_BE Retransmission](https://github.com/violet0929/mlo_analysis/blob/main/Appendix/Appendix_A.md)
  * B. [AC_VI Retransmission](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_B.md)
  * C. [Block ACK Request](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_C.md)
  * D. [MPDU Buffer Size](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_D.md)
  * E. [Latency](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_E.md)
* 3 tasks are classified and performed sequentially as described in the below
  * Task 1: IEEE 802.11be asynchronous multi-link operation with EDCA 환경에서의 retransmission case 구분
  * Task 2: 특정 재전송 상황에서의 문제점 분석 및 해결 방안 제안
  * Task 3: 3 가지 관점에서의 성능 분석
    * 관점 1: 기존 재전송 방식 vs 제안하는 재전송 방식
    * 관점 2: 특정 Buffer size에 기반한 네트워크 성능
    * 관점 2: 여러 AC이 공존하는 상황에서 STA 수에 기반한 네트워크 성능

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

## Supplementary


## References


  



