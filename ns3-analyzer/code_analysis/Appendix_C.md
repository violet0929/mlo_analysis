## Appendix. C. Block ACK Request

### Objective
* 해당 문서에서는 ns-3.40 기반 Block ACK Request 프레임이 전송되는 과정을 분석함
* Retransmission event가 invoke 되는 원인 분석
* VI AC에 해당하는 TXOP를 획득 후 전송되는 초기 프레임 분석
* 앞서 분석한 내용 (Appendix. A. 및 B. 참고)과 공통되는 부분은 다루지 않음

* 먼저, Appendix B. 에서 기술한 대로 VI AC에 해당하는 TXOP를 획득하고 전송한 초기 프레임의 존재로 인해 재전송되는 프레임이 원본 프레임의 전체에 대해 aggregation을 수행하지 못하는 상황 존재
* 초기 프레임의 존재를 확인하기 위해 Wireshark 기반 pcap 파일 로그 분석이 필요함
  * ns-3에서는 분석이 힘듬
    * invoke되는 event가 독립적이므로, 공통적인 요소를 기반으로 debug를 수행해야 되는데 ns::Time이 대표적
    * 그럼에도 불구하고, Wireshark 기반 pcap 캡처 로그와 ns-3 ns::Time 간의 동기화가 안맞아서 정확한 시간을 기반으로 breakpoint를 걸기 어려움

* No. 11 로그 분석 
```
1. 1.045457s STA1 -> AP (AC_VI, A-MPDU ID 39: #174 ~ #202) A-mpdu 송신
2. 1.084516s STA1 -> AP #174 ~ #201 A-mpdu 재전송
3. 1.090517s AP에서 STA1가 재전송한 #174 ~ #201 A-mpdu 수신
```
* STA1 -> AP link1을 통해 전송한 wlan seq #174에 해당하는 A-mpdu 재전송 부분은 아래 그림과 같음

<p align="center">  
  <img src="https://github.com/user-attachments/assets/1fa06074-5298-4574-906b-02da1bb0d7c2" width="40%">  
</p>
