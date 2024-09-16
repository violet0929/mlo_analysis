## Appendix. D. MPDU Buffer Size

### Objective
* 해당 문서에서는 ns-3.40 기반 Device의 MPDU Buffer Size와 관련된 분석을 수행함
* 송신기의 Buffer Size의 한계로 인한 aggregation size의 제약
* Buffer에 존재하는 MPDU들의 state 정보 분석
* 앞서 분석한 내용 (Appendix. A, B, C 참고)과 공통되는 부분은 다루지 않음

### Preview
* AP 입장에서, wireshark pcap 로그를 보다보면 아래 그림과 같이 작은 개수 (6개)에 해당하는 A-mpdu가 수신된 것을 확인할 수 있음
  * 일반적으로 VI에 해당하는 TXOP를 획득하면 29개의 mpdu가 aggregation되어 전송됨 (20MHz channel, 1400 byte payload의 경우)

<p align="center">  
  <img src="https://github.com/user-attachments/assets/07ae55a8-bbc6-443d-990c-16b1ddbe42f8" width="40%">  
</p>

* 예쁘게 정리해보면
```
1. Time: 1.478139s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 54 / Info: 802.11 Block Ack
2. Time: 1.479107s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1842)
...
8. Time: 1.479107s / Src: 192.168.1.2 / Dst: 192.168.1.1 / length: 1500 / Info: 49153 -> 9 Len = 1400 (Seq #: 1847)
9. Time: 1.479117s / Src: 00:00:00:00:00:08 / Dst: 00:00:00:00:00:02 / length: 54 / Info: 802.11 Block Ack
```
* 이러한 현상이 발생하는 이유를 아래와 같은 관점에서 분석
  * TXOP를 획득한 후 전송한 initial frame인지?
  * Appendix A, B와 같이 최대 PPDU 전송 시간 및 TXOP Limit에 제약을 받아 조정된 aggregation size인지?
  * 다른 이유가 있는지?

* 따라서, aggregation 규칙을 확인하기 위해 아래와 같은 코드를 WifiPhy::send에 breakpoint걸고 call stack을 보면
  * MPDU header 정보: (Originator: STA 1, Qos Tid: 5 (VI), Seq #: 1842)
```c
auto ptr = ppdu->GetPsdu()->begin();
for (int i = 0; i < (int) ppdu->GetPsdu()->GetNMpdus(); i++) {
auto mpdu_header = ptr[i]->GetHeader();
if ((int) ppdu->GetPsdu()->GetNMpdus() == 6 && mpdu_header.GetSequenceNumber() == 1842 &&
    mpdu_header.GetQosTid() == 5) {
    NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": " << mpdu_header.GetAddr2() << ", send A-mpdu (6)");
  }
}
```
>> +1.51963s: 00:00:00:00:00:02, send A-mpdu (6)

* Call Stack
<p align="center">  
  <img src="https://github.com/user-attachments/assets/e205ede2-eaef-447c-9851-3fc39b9837ff" width="40%">  
</p>

* 다른 점은 없다
