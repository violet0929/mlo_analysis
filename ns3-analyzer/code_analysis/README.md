### Overview
* All code to perform the analysis is based on [ns-3.40](https://www.nsnam.org/releases/ns-3-40/).
* ns-3 MAC low & MAC high & PHY model implementation
* WifiNetDevice architecture. For 802.11be Multi-Link Devices (MLDs), there as many instances of WifiPhy, FrameExchangeManager and ChannelAccessManager as the number of links.
  * Reference: https://www.nsnam.org/docs/release/3.40/models/singlehtml/index.html#document-wifi
    
  ![image](https://www.nsnam.org/docs/release/3.40/models/singlehtml/_images/WifiArchitecture.png)

### Appendix
* A. [AC_BE retransmission](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_A.md)
* B. [AC_VI retransmission](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_B.md)
* C. [Block ACK request](https://github.com/violet0929/mlo_analysis/blob/main/ns3-analyzer/code_analysis/Appendix_C.md)
