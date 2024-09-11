## Appendix. A. AC_BE retransmssion

### Objective
* AC_BE 및 AC_VI의 서로 다른 재전송 과정이 발생하는 원인을 찾기 위해
* 해당 문서에서는 ns-3.40 기반 AC_VI의 재전송 수행 과정을 분석함
* 앞서 분석한 내용 (Appendix. A. 참고)과 공통되는 부분은 다루지 않음

* No. 11 로그 분석 
```
1. 1.045457s STA1 -> AP (AC_VI, A-MPDU ID 39: #174 ~ #202) A-mpdu 송신
2. 1.084516s STA1 -> AP #174 ~ #201 A-mpdu 재전송
3. 1.090517s AP에서 STA1가 재전송한 #174 ~ #201 A-mpdu 수신
```

* 분석을 위해 ns3::WifiPhy::Send에서 breakpoint 걸어줌
```c
auto ptr = ppdu->GetPsdu()->begin();
for (int i = 0; i < (int)ppdu->GetPsdu()->GetNMpdus(); i++){
  auto mpdu_header = ptr[i]->GetHeader();
  if (mpdu_header.GetSequenceNumber() == 174 && mpdu_header.GetQosTid() == 5){
    if (mpdu_header.IsRetry() == 1)
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": retry");
    else
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": No retry");
  }
}
```
>> +1.02239s: No retry
>> +1.08898s: No retry
>> +1.12803s: retry

* 문제가 있다. 1AP + 2STA의 topology를 구성했으므로, STA의 식별이 필요함
* mpdu header에는 Addr1, Addr2 field가 있는데, 각각 수신 및 송신 device의 MAC address가 포함되어 있음
* 따라서, 코드를 다음과 같이 수정함
```c
auto ptr = ppdu->GetPsdu()->begin();
for (int i = 0; i < (int)ppdu->GetPsdu()->GetNMpdus(); i++){
  auto mpdu_header = ptr[i]->GetHeader();
  if (mpdu_header.GetSequenceNumber() == 174 && mpdu_header.GetQosTid() == 5){
    if (mpdu_header.IsRetry() == 1)
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": " << mpdu_header.GetAddr2() << ", send retry mpdu");
    else
      NS_LOG_UNCOND(Simulator::Now().As(Time::S) << ": " << mpdu_header.GetAddr2() << ", send mpdu");
  }
}
```
>> +1.02239s: 00:00:00:00:00:05 send mpdu
>> +1.08898s: 00:00:00:00:00:02 send mpdu
>> +1.12803s: 00:00:00:00:00:02 send retry mpdu

* 시간 간격 확인해보면 같은 로그 맞음
  * Wireshark 기준: 1.084516s - 1.045457s = 39.059ms
  * ns-3 기준: 1.12803s - 1.08898s = 39.05ms
>> Note 1: Device들의 MAC 주소는 (01 ~ 03: STA1, 04 ~ 06: STA2, 07 ~ 09: AP)가 할당되어 있음
>> Note 2: Device들은 MLD이므로, 또 하나 유추할 수 있는 점은 장치의 UMAC, L-MAC link 1, L-MAC link 2의 순서대로 MAC 주소가 할당되어 있음

### 1. 


