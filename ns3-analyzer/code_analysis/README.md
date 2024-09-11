### Overview
* 해당 directory에 포함된 모든 코드는 ns-3.40를 기반으로 하며, license 정책은 ns-3를 따릅니다.
* ns-3에서 MAC 및 PHY의 동작의 overview는 아래 내용과 같습니다.
* WifiNetDevice architecture. For 802.11be Multi-Link Devices (MLDs), there as many instances of WifiPhy, FrameExchangeManager and ChannelAccessManager as the number of links.
  * Reference: https://www.nsnam.org/docs/release/3.40/models/singlehtml/index.html#document-wifi)
    
  ![image](https://www.nsnam.org/docs/release/3.40/models/singlehtml/_images/WifiArchitecture.png)

### Appendix
* [A. AC_BE retransmission](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_A.md)
* [B. AC_VI retransmission](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_B.md)
