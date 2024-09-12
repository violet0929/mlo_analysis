## Appendix. C. Block ACK Request

### Objective
* 해당 문서에서는 ns-3.40 기반 Block ACK Request 프레임이 전송되는 과정을 분석함
* Retransmission event가 invoke 되는 원인 분석
* VI AC에 해당하는 TXOP를 획득 후 전송되는 초기 프레임 분석
* 앞서 분석한 내용 (Appendix. A. 및 B. 참고)과 공통되는 부분은 다루지 않음

### Preview
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

* 예쁘게 정리해보면
```
1. Time: 1.084382 / Src: 00:00:00:00:00:02 / Dst: 00:00:00:00:00:08 / length: 56 / Info: 802.11 Block Ack Req
2. Time: 1.084506 / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 80 / Info: 802.11 Block Ack
3. Time: 1.084516 / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400
...
n. Time: 1.088461 / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 80 / Info: 802.11 Block Ack
```

* 정보 1: 1.084516 시점에 전송한 A-mpdu에 대한 Block Ack을 1.088461에 수신 받았으며, 해당 시간 간격은 3.945ms 이다.
  * 그럼 TXOP limit을 기반으로 계산한 TXOP의 유효 시간은 두가지 관점에서 해석을 할 수 있음
  * 해석 1. TXOP 유효 시간 (available Time)은 송신 device가 보낸 프레임에 대한 ACK 프레임을 수신할 때까지의 시간
  * 해석 2. TXOP 유효 시간 (available Time)은 송신 device가 보낸 프레임에 대해 수신 device가 해당 프레임을 수신할 때까지의 시간
  * 참고로, Appendix B. 에서 계산한 TXOP의 유효 시간은 3.906ms임
  * 따라서, 해당 관점에서는 해석 2에 조금 더 가까운 것 같음
  * 근데, 확실한 건 아닌게, 엄연히 송신 device에 계산한 유효 시간은 '예상'에 불과함 지연이 발생할 가능성이 충분히 있음
  * 표준문서를 좀 찾아봤는데, 해당 내용에 대해 자세히 기술된 내용은 없었음 (교수님께 한번 여쭤봐야겠음)

* 정보 2: TXOP를 획득한 후 초기 프레임으로 보낸 프레임은 'BA Req' 프레임임
  * Appendix. B. 에서 예상한대로 1500 byte보다 훨씬 작은 크기를 가짐 (56 byte)
  * BA Req 프레임의 전송 시점 (1.084382)과 마지막으로 BA를 받은 시점 (1.088461)과의 시간 간격은 4.079ms임
  * 참고로, VI TXOP limit의 값은 4.096ms임
  * 따라서, 해당 관점에서는 해석 1에 조금 더 가까운 것 같음

* 관점에 따라 해석이 조금 다를 수 있으니 아무튼 코드 분석은 해야됨 (목적은 잡고 들어가자)
  * 목적 1. 우선, BA Req frame이 VI에 해당하는 TXOP를 획득하고 전송한 초기 프레임이 맞는지 확인해야 됨 (아닐수도 있으니..)
  * 목적 2. BA Req frame이 전송되면 왜 retranmission event가 invoke 되는지 확인해야 됨

* 자 그럼, breakpoint를 걸어야 되는데... 어디에 어떻게 걸 것인가가 관건임 (일반적인 mpdu가 아니기 때문에 header를 기반으로 하기 힘듬)


