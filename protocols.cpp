#include <stdio.h>
#include <string.h>
#include "protocols.h"
#include <winsock2.h>

// ����������Ƿ��ж˿ںţ�����ȥ��
void checkAddr(char* addr, int addr_len)
{
	char* pCurrent;
	int count;
	
	pCurrent = addr;
	count = 0;
	while(*pCurrent != ':')	
	{
		count++;
		*pCurrent++;
	}
	
	count = addr_len - count;
	memset(pCurrent, '\0', addr_len - count + 1);
}

void fprintHTTPMsg(FILE* file, char* msg,int msg_len)
{
	int count = 0;
	char* p = msg;
	while(true)
	{
		if(count >= msg_len)
			break;
		if( (*p >= 32 && *p <=126) || *p == 10 || *p == 13)
			fprintf(file, "%c", *p);
		else
			fprintf(file, ".");
		count++;
		p++;
	}
	fprintf(file, "\n");
}



void printBuffer(char* buf,int buflen)
{
	int count = 0;
	char* p = buf;

	printf("\n--------------%d Bytes----------------\n", buflen);

	while(true)
	{
		if(count >= buflen)
			break;
		if( (*p >= 32 && *p <=126) || *p == 10 || *p == 13)
			printf("%c", *p);
		else
			printf(".");
		count++;
		p++;
	}
	printf("\n------------------------------------------------\n");
}

// ��IP:Port�л�ȡIP��ַ
void getIPAddr(char* addr, int addr_len)
{
	char* pCurrent;
	int count;
	
	pCurrent = addr;
	count = 0;
	while(*pCurrent != ':')	
	{
		count++;
		*pCurrent++;
	}
	
	count = addr_len - count;
	memset(pCurrent, '\0', addr_len - count + 1);
}

int getHTTPHost(char* msg, char* host)
{
	char* pCurrent;
	char* pHostStart;
	char* pHostEnd;
	int	  host_len;
	bool findFlag;

	findFlag = false;
	pCurrent = msg;
	while(true)
	{
		// ����һ��
		while(*pCurrent != '\n')
		{
			// �����ֶ���ΪHost��Ӧ��ֵ
			if(strncmp("Host: ", pCurrent, 6) == 0)
			{
				pCurrent += 6;
				pHostStart = pCurrent;

				while(*pCurrent != '\r')
					pCurrent++;
				pHostEnd = pCurrent;
				
				findFlag = true;
				break;
			}
			pCurrent++;
		}
		pCurrent++;	

		if(findFlag == true)
			break;
	}
	host_len = pHostEnd - pHostStart;
	memcpy(host, pHostStart, host_len);
	host[host_len] = '\0';

	return host_len;
}

// ����payload����
int getTCPPayload(char* segment, char* payload, int segment_len)
{
	PTCPHDR pTCPHDR;
	int tcp_header_len;
	int tcp_total_len;
	int payload_len;

	pTCPHDR = (PTCPHDR)segment;
	
	tcp_header_len = (pTCPHDR->h_len >> 4);
	tcp_total_len = segment_len;
	payload_len = tcp_total_len - tcp_header_len;

	memcpy(payload, segment + tcp_header_len, payload_len); 

	return payload_len;
}

int getIPPayload(char* packet, char* payload)
{
	PIPHDR pIPHDR;
	
	int ip_header_len;
	int ip_total_len;
	int payload_len;
	
	pIPHDR = (PIPHDR)packet;
	ip_header_len = pIPHDR->h_lenver & 0x0f;
	ip_total_len = ntohs(pIPHDR->total_len);
	payload_len = ip_total_len - ip_header_len;

	memcpy(payload, packet + ip_header_len, payload_len);
	
	return payload_len;
}
