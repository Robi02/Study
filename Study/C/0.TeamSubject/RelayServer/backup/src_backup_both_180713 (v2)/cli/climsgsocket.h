#ifndef __CLIMSGSOCKET_H__
#define __CLIMSGSOCKET_H__

#include "stdheader.h"
#include "clirecord.h"

int sendAndRecv(SOCKET *pSocket, SvrRecord *pstSvrRecord, int option);
int connectSocket(SOCKET *pSocket, char *pIP, int port);
int closeSocket(SOCKET *pSocket);
char* makePacketFromMsg(char *pBuf, int szBuf, char *pMsg, int szMsg);
char* makeMsgFromPacket(char *pBuf, int szBuf, char *pPacket, int szPacket);

/* [ Address Families(AF) ]
 * 1. AF_UNSPEC(0) - Unspecified
 * 2. AF_INET(2) - IPv4
 * 3. AF_IPX(6) - IPX/SPX
 * 4. AF_APPLETALK(16) - AppleTalk
 * 5. AF_NETBIOS(17) - NetBIOS(Only support SOCK_DGRAM)
 * 6. AF_INET6(23) - IPv6
 * 7. AF_IRDA(26) - Infrared Data Association(IrDA)
 * 8. AF_BTH(32) - Bluetooth
 */
 
/* [ Socket Type ]
 * 1. SOCK_STREAM(1) - TCP (AF_INET or AF_INET6)
 * 2. SOCK_DGRAM(2) - UDP (AF_INET or AF_INET6 or AF_NETBIOS)
 * 3. SOCK_RAW(3) - Allows an application to manipulate the next upper-layer protocol header
 * 4. SOCK_RDM(4) - A reliable message datagram
 * 5. SOCK_SEQPAKCET(5) - A pseudo-stream packet based on datagrams
 */
  
/* [ Protocol ]
 * 1. (0) - Service provider choose the protocol to use
 * 2. IPPROTO_ICMP(1) - Internet Control Message Protocol(ICMP) (AF_UNSPEC or AF_INET or AF_INET6 + SOCK_RAW)
 * 3. IPPROTO_IGMP(2) - Internet Group Management Protocol(IGMP) (AF_UNSPEC or AF_INET or AF_INET6 + SOCK_RAW)
 * 4. BTHPROTO_RFCOMM(3) - Bluetooth RFC protocol (AF_BTH + SOCK_STREAM)
 * 5. IPPROTO_TCP(6) - TCP (AF_INET or AF_INET6 + SOCK_STREAM)
 * 6. IPPROTO_UDP(17) - UDP (AF_INET or AF_INET6 + SOCK_DGRAM)
 * 7. IPPROTO_ICMPV6(58) - ICMPv6 (AF_UNSPEC or AF_INET or AF_INET6 + SOCK_RAW)
 * 8. IPPROTO_RM(113) == IPPROTO_PGM - PGM protocol for reliable multicast (AF_INET + SOCK_RDM)
 */

#endif