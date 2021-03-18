#include <stdio.h>
#include <string.h>
#include "tools.h"
#include <winsock2.h>

// 检查域名中是否含有端口号，有则去掉
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

int readLine(char* line, char* msg)
{
	char *p = msg;
	int counter = 0;

	while(*p != '\n')
	{
		p++;
		counter++;
	}
	counter++;

	memcpy(line, msg, counter);
	return counter;
}

int getHTTPMethod(char* http_request, char* method)
{
	char *p = http_request;
	int counter = 0;

	while( *p != ' ')
	{
		p++;
		counter++;
	}

	memcpy(method, http_request, counter);
	return counter;
}

int getHTTPURL(char* http_request, char* url)
{
	char *p = NULL;
	char *url_start = NULL;
	int counter = 0;

	if( (p = strstr(http_request, "http://")) != NULL)
	{
		p += 7;
		while( *p != '/')
		{
			p++;
		}
		url_start = p;
	}
	else
	{
		p = strstr(http_request, " ");
		p++;
		url_start = p;
	}

	while( *p != ' ')
	{
		p++;
		counter++;
	}

	memcpy(url, url_start, counter);

	return counter;
}

int getHTTPVersion(char* http_request, char* version)
{
	char *p = http_request;
	char *version_start = NULL;
	int counter = 0;

	while( *p != ' ')	p++;
	p++;
	while( *p != ' ')	p++;
	p++;

	version_start = p;
	while( *p != '\r')
	{
		p++;
		counter++;
	}

	memcpy(version, version_start, counter);
	return counter;
}


void parseHTTPRequest(char* http_request, int http_request_len, ParseResult* result)
{
	char new_http_request[TENKB_SIZE];
	int new_http_request_len = 0;
	char *p = NULL;
	char line[KB_SIZE];
	int line_len = 0;
	int http_request_line_header_len = 0;

	// 获取HTTP请求中的Host字段的值
	char host[KB_SIZE];
	int hostlen;

	hostlen = getHTTPHost(http_request, host);
	printf("%s\n", host);
	
	// Host字段是IP地址
	if(atoi(host))
	{
		getIPAddr(host, hostlen);
		result->serverIPAddr = inet_addr(host);
	}
	else{
		 // 获取服务器地址		
		 getIPAddr(host, hostlen);
		 hostent* hostEnt = gethostbyname(host);
		 if(hostEnt == NULL)
		 {
			printf("Error at gethostbyname()\n");
			
		 }
		 result->serverIPAddr = (*(in_addr*)(hostEnt->h_addr_list[0])).s_addr;
	}






	/* 读取method */
	char *method = NULL;
	int method_len = 0;
	if( (method = (char*)malloc(sizeof(char) * KB_SIZE)) != NULL)
	{
		method_len = getHTTPMethod(http_request, method);
	}


	result->isMethodEqConnect = false;
	if(method != NULL && strncmp( method, "CONNECT", 7) == 0) 
	{
		result->isMethodEqConnect = true;
		return;
	}

	memcpy(new_http_request + new_http_request_len, method, method_len);
	new_http_request_len += method_len;

	if(method != NULL)
	{
		free(method);
		method = NULL;
	}
	
	
	memcpy(new_http_request + new_http_request_len, " ", 1);
	new_http_request_len++;

	/* 读取url */
	char *url = NULL;
	int url_len = 0;

	if( (url = (char*)malloc(sizeof(char) * KB_SIZE)) != NULL)
	{
		url_len = getHTTPURL(http_request, url);
	}
	

	memcpy(new_http_request + new_http_request_len, url, url_len);
	new_http_request_len += url_len;

	memcpy(new_http_request + new_http_request_len, " ", 1);
	new_http_request_len++;

	if(url != NULL)
	{
		free(url);
		url = NULL;
	}


	/* 读取版本号 */
	char* version = NULL;
	int version_len = 0;
	
	if( (version = (char*)malloc(sizeof(char) * KB_SIZE)) != NULL)
	{
		version_len = getHTTPVersion(http_request, version);
	}
	
	memcpy(new_http_request + new_http_request_len, version, version_len);
	new_http_request_len += version_len;
	
	if(version != NULL)
	{
		free(version);
		version = NULL;
	}


	memcpy(new_http_request + new_http_request_len, "\r\n", 2);
	new_http_request_len += 2;

	/* 计算请求行的长度 */
	p = http_request;
	while( *p != '\n')
	{
		p++;
		http_request_line_header_len++;
	}
	http_request_line_header_len++;


	/* 读取请求首部 */
	p = strstr(http_request, "\r\n");
	p += 2;


	while( *p != '\r' && *(p+1) != '\n')
	{
		line_len = readLine(line, p);
		p += line_len;
		http_request_line_header_len += line_len;

		/* 读取Proxy-Connection，修改其为Connection*/
		if( strstr(line, "Proxy-Connection: ") != NULL)
		{
			char *value = strstr(line, " ");
			value++;

			char *pTmp = NULL;
			pTmp = value;
			
			int counter = 0;

			while( *pTmp != '\n')
			{
				pTmp++;
				counter++;
			}

			memcpy(new_http_request + new_http_request_len, "Connection: ", strlen("Connection: "));
			new_http_request_len += strlen("Connection: ");

			memcpy(new_http_request + new_http_request_len, value, counter);
			new_http_request_len += counter;

			memcpy(new_http_request + new_http_request_len, "\r\n", 2);
			new_http_request_len += 2;
			
			continue;
		}

		memcpy(new_http_request + new_http_request_len, line, line_len);
		new_http_request_len += line_len;
	}
	memcpy(new_http_request + new_http_request_len, "\r\n", 2);
	new_http_request_len += 2;

	/* p指向实体部分 */
	p += 2;
	http_request_line_header_len += 2;
	
	if(p != NULL && http_request_len - http_request_line_header_len != 0)
	{
		memcpy(new_http_request + new_http_request_len, p, http_request_len - http_request_line_header_len); 
		new_http_request_len += http_request_len - http_request_line_header_len;
	}
	

	/* 将新的HTTP请求写入result */
	memcpy(result->new_http_request, new_http_request, new_http_request_len);
	result->new_http_request_len = new_http_request_len;

//	printHTTPMsg(new_http_request, new_http_request_len, "new_http_request");




	
	
}

void printHTTPMsg(char* msg, int msglen, char* str)
{
	int count = 0;
	char* pMsg = NULL;
	if( (pMsg = (char*)malloc(msglen)) != NULL)
	{
		memcpy(pMsg, msg, msglen);
	}
	
	char* p = pMsg;

	printf("\n--------------%s (%d Bytes)----------------\n", str, msglen);

	while(count < msglen)
	{
		if( 0<= *p && *p <= 255)
		{
			if( (32 <= *p  && *p <=126) || *p == 10 || *p == 13)
				printf("%c", *p);
			else
				printf("%c", '.');
		}
		
		count++;
		p++;
	}
	printf("\n------------------------------------------------\n");
	if(pMsg != NULL)
	{
		free(pMsg);
		pMsg = NULL;
	}
	
}

// 从IP:Port中获取IP地址
void getIPAddr(char* addr, int addr_len)
{
	char* pCurrent;
	int count;
	bool findFlag = true;
	
	pCurrent = addr;
	count = 0;
	while(*pCurrent != ':')	
	{
		count++;
		*pCurrent++;
		
		if(count > addr_len)
		{
			findFlag = false;
			break;
		}
			
	}
	
	if(findFlag)
	{
		//count = addr_len - count;
		memset(pCurrent, '\0', addr_len - count + 1);
	}
	
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
		// 跳过一行
		while(*pCurrent != '\n')
		{
			// 查找字段名为Host对应的值
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

// 返回response的长度，检查http报文中是否含有connect方法，若有则构造一个回复报文
int handleSSLConnect(char* httpmsg, char* response)
{
	char *p;

	p = strstr(httpmsg, "CONNECT");
	if(p != NULL)
	{
		strcpy(response, "HTTP/1.1 200 Connection Established\r\n\r\n");
		return strlen(response);
	}

	return 0;
}