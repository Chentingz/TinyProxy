#ifndef _PROTOCOLS_H
#define _PROTOCOLS_H

#define	TCP_FIN	0x01
#define	TCP_SYN	0x02
#define	TCP_RST	0x04
#define	TCP_PSH	0x08
#define	TCP_ACK	0x10
#define	TCP_URG	0x20
#define	TCP_ACE	0x40
#define	TCP_CWR	0x80

#include <winsock2.h>

typedef struct IPHDR        
{
    UCHAR		h_lenver;				 // 版本号和头长度（各占4位）
    UCHAR		tos;					 // 服务类型 
    USHORT		total_len;			     // 封包总长度，即整个IP报的长度
    USHORT		ident;					 // 封包标识，惟一标识发送的每一个数据报
    USHORT	 	frag_and_flags;			 // 标志
    UCHAR		ttl;					 // 生存时间，就是TTL
    UCHAR		protocol;				 // 协议，可能是TCP、UDP、ICMP等
    USHORT      checksum;				 // 校验和
    ULONG		saddr;					 // 源IP地址
    ULONG		daddr;					 // 目标IP地址
} IPHDR, *PIPHDR;




typedef struct TCPHDR    
{
    USHORT    srcport;				// 16位源端口号
    USHORT    dstport;				// 16位目的端口号
    ULONG	  seq;					// 32位序列号
    ULONG     ack;					// 32位确认号
    UCHAR     h_len;				// 高4位表示数据偏移
    UCHAR     flags;				// 6位标志位
    //FIN - 0x01
    //SYN - 0x02
    //RST - 0x04 
    //PSH - 0x08
    //ACK - 0x10
    //URG - 0x20
    //ACE - 0x40
    //CWR - 0x80

    USHORT    window;               // 16位窗口大小
    USHORT    checksum;				// 16位校验和
    USHORT    urgptr;               // 16位紧急数据偏移量 

} TCPHDR, *PTCPHDR;

void fprintHTTPMsg(FILE* file, char* msg,int msg_len);

void printHTTPMsg(char* msg, int msg_len);

void getIPAddr(char* addr, int addr_len);

int getHTTPHost(char* msg, char* host);

int getTCPPayload(char* segment, char* payload, int segment_len);

int getIPPayload(char* packet, char* payload);

#endif