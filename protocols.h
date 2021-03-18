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
    UCHAR		h_lenver;				 // �汾�ź�ͷ���ȣ���ռ4λ��
    UCHAR		tos;					 // �������� 
    USHORT		total_len;			     // ����ܳ��ȣ�������IP���ĳ���
    USHORT		ident;					 // �����ʶ��Ωһ��ʶ���͵�ÿһ�����ݱ�
    USHORT	 	frag_and_flags;			 // ��־
    UCHAR		ttl;					 // ����ʱ�䣬����TTL
    UCHAR		protocol;				 // Э�飬������TCP��UDP��ICMP��
    USHORT      checksum;				 // У���
    ULONG		saddr;					 // ԴIP��ַ
    ULONG		daddr;					 // Ŀ��IP��ַ
} IPHDR, *PIPHDR;




typedef struct TCPHDR    
{
    USHORT    srcport;				// 16λԴ�˿ں�
    USHORT    dstport;				// 16λĿ�Ķ˿ں�
    ULONG	  seq;					// 32λ���к�
    ULONG     ack;					// 32λȷ�Ϻ�
    UCHAR     h_len;				// ��4λ��ʾ����ƫ��
    UCHAR     flags;				// 6λ��־λ
    //FIN - 0x01
    //SYN - 0x02
    //RST - 0x04 
    //PSH - 0x08
    //ACK - 0x10
    //URG - 0x20
    //ACE - 0x40
    //CWR - 0x80

    USHORT    window;               // 16λ���ڴ�С
    USHORT    checksum;				// 16λУ���
    USHORT    urgptr;               // 16λ��������ƫ���� 

} TCPHDR, *PTCPHDR;

void fprintHTTPMsg(FILE* file, char* msg,int msg_len);

void printHTTPMsg(char* msg, int msg_len);

void getIPAddr(char* addr, int addr_len);

int getHTTPHost(char* msg, char* host);

int getTCPPayload(char* segment, char* payload, int segment_len);

int getIPPayload(char* packet, char* payload);

#endif