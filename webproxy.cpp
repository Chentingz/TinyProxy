#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <direct.h>
#include <string.h>

#include "sockwrapper.h"
#include "tools.h"

#pragma comment(lib,"ws2_32.lib")



typedef struct ThreadParam
{
	SOCKET	proxySocket1;
	SOCKET proxySocket2;
	FILE*	log;

}ThreadParam;

DWORD  handle_client_request(LPVOID lpParameter);
int handle_https_communication(SOCKET proxySocket, SOCKET proxySocket2, FILE* log);
int handle_client_request(SOCKET proxySocket1, SOCKET proxySocket2, FILE* log);
DWORD handle_server_response(LPVOID lpParameter);

int main()
{
	FILE* log;
	time_t timep;
	tm* ptmNow;

	char filename[DEFAULT_SIZE];
	char dir[DEFAULT_SIZE];
	char path[DEFAULT_SIZE];
	int	filelen;
	int	dirlen;
	int pathlen;

	WSAData wsaData;
	int iResult;

	sockaddr_in localAddr;
	SOCKET listenSock;

	sockaddr_in remoteAddr;			// ��¼���ӵĿͻ��˵�ַ���˿�
	int remoteAddrLen = sizeof(remoteAddr);

	SOCKET proxySocket;				// ������ͻ���ͨ�ŵ�socket
	
//	HANDLE threadHandle;
//	ThreadParam threadParam;



	dirlen = filelen = pathlen = DEFAULT_SIZE;

    time(&timep);  
	ptmNow = localtime(&timep);
	
	memset(path, '\0', pathlen);
	memset(dir, '\0', dirlen);
	memset(filename, '\0', filelen);

	
	//getcwd(dir, dirlen);

	// TODO:·��д���ˣ���Ҫ�����޸�
	sprintf(filename,"E:\\Code\\WebProxy\\log\\%d-%d-%d-%d-%d-%d.txt", (1900 + ptmNow->tm_year), (1+ptmNow->tm_mon), ptmNow->tm_mday,ptmNow->tm_hour, ptmNow->tm_min, ptmNow->tm_sec);
	printf("log path: %s\n", filename);


   // strcpy(filename,ctime(&timep));
	
	// ����־�ļ�
	
/*	log = fopen(filename, "a+");
	if(log == NULL)
	{
		printf("Couldn't open file.\n");
		return 1;
	}
*/

	/* ��ʼ��winsock */
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	/* �����׽������ڼ������� */
	listenSock = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	// ���ص�ַ	
	localAddr.sin_family = PF_INET;
	localAddr.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);
	localAddr.sin_port = htons(LOCAL_LISTEN_PORT);


	/* ���׽��� */	
	iResult = Bind(listenSock, (sockaddr*)&localAddr, sizeof(localAddr));


	/* �����׽��� */	
	iResult = Listen(listenSock, SOMAXCONN);
		
    remoteAddrLen = sizeof(remoteAddr);
	while(true)
	{
		printf("Listening on local port %hu...\n", ntohs(localAddr.sin_port));

		// ���տͻ������󣬷���һ��socket��������ͨ��
		proxySocket = Accept(listenSock, (sockaddr*)&remoteAddr, &remoteAddrLen);

		printf("Accept connectin from %s:%hu\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
	//	fprintf(log, "Accept connectin from %s:%hu\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
		
		ThreadParam threadParam;
		HANDLE		threadHandle;
		// �����߳�
		printf("Creating thread.\n");
		
		// �̲߳���
		threadParam.proxySocket1 = proxySocket;
		threadParam.proxySocket2 = NULL;
		threadParam.log = log;

		threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_client_request, (void*)&threadParam, 0, 0);
		if(threadHandle == NULL)
		{
			printf("Error at CreateThread()\n");
			return 1;
		}
		CloseHandle(threadHandle); 

		printf("Thread created.\n");
		
		// ˯��1s
		Sleep(1000);
	}

//	fclose(log);
	closesocket(listenSock);
	WSACleanup();

	return 0;
}





// �̴߳�����
DWORD handle_client_request(LPVOID lpParameter)
{
	// ������ͻ���ͨ�ŵ�socket
	ThreadParam threadParam = *(ThreadParam*)lpParameter;

	SOCKET proxySocket1 = threadParam.proxySocket1;
	FILE* log = threadParam.log;

	int  iBytes;							 // ��¼���ա����͵��ֽ���
	int  iResult;							 // ��¼�����ĵ��ý��
	char sendBuf[TENKB_SIZE];			     // ���ͻ�����
	int	 sendBufLen = TENKB_SIZE;			 // ���ͻ�������С
	char recvBuf[TENKB_SIZE];			     // ���ջ�����
	int	 recvBufLen = TENKB_SIZE;			 // ���ջ�������С

	SOCKET proxySocket2;
	bool	createThreadFlag;
	bool	connectFlag;

	createThreadFlag = connectFlag = true;
	while(1)
	{
		/* ���տͻ��˵�HTTP���� */
		if( (iBytes = Recv(proxySocket1, recvBuf, recvBufLen, 0)) == 0)
		{
		//	break;
			closesocket(proxySocket2);
			return 0;
		}
		else if(iBytes == SOCKET_ERROR)
		{
			return 1;                  
		}

	//	printHTTPMsg(recvBuf, iBytes, "HTTP Request From Client");
	//	fprintf(log," HTTP Request From Client(%d)\n", iBytes);

		/* ����HTTP���� */
		ParseResult pResult;
		pResult.isMethodEqConnect = false;
		parseHTTPRequest(recvBuf, iBytes, &pResult);
		
		
		if(connectFlag)
		{
			connectFlag = false;			// ֻ����һ��
		 /* ���÷�������ַ */
		 sockaddr_in serverAddr;
		 ULONG 		serverIPAddr;
		
		 serverIPAddr = pResult.serverIPAddr;	 
		 serverAddr.sin_family=PF_INET;
		 serverAddr.sin_addr.s_addr = serverIPAddr;
		 if(pResult.isMethodEqConnect == true)
		 {
			 serverAddr.sin_port = htons(SSL_PORT);
		 }
		 else
		 {
			 serverAddr.sin_port = htons(SERVER_PORT);
		 }
		 

		 printf("serverIPAddr: %s\n", inet_ntoa(serverAddr.sin_addr));
	//	 fprintf(log, "serverIPAddr: %s\n", inet_ntoa(serverAddr.sin_addr));

		 // ���������������ͨ�ŵ�socket
		 proxySocket2 = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
	//	 int RCVTIMEO = 30000;
	//	 setsockopt(proxySocket2, SOL_SOCKET, SO_RCVTIMEO, (char*)&RCVTIMEO, sizeof(RCVTIMEO));

		 /* ��������������� */
		 printf("Connecting to server %s...\n", inet_ntoa(serverAddr.sin_addr));
		 iResult = Connect(proxySocket2, (sockaddr*)&serverAddr, sizeof(serverAddr));
		}

		 // ���󷽷���CONNECT,�����httpsͨ�Ž��д���
		 if(pResult.isMethodEqConnect)
		 {
			if( (iResult = handle_https_communication(proxySocket1, proxySocket2, log)) == 0)
			{
	
			 printf("thread exit.\n");
			 return 0;
			}
			else if(iResult != 0)
			{
				printf("thread exit.\n");
				return 1;
			}

		 }
		 else
		 {
			
		 
		 // �����������HTTP����
	//	 fprintf(log, "Ready to send HTTP Request to server.\n");

		memcpy(sendBuf, pResult.new_http_request, pResult.new_http_request_len);
		if( (iBytes = Send(proxySocket2, sendBuf, pResult.new_http_request_len, 0)) == 0)
		{
			closesocket(proxySocket1);
			return 0;
		}
		else if(iBytes == SOCKET_ERROR)
		{
			closesocket(proxySocket1);
			return 1;
		}
	//	printHTTPMsg(sendBuf, iBytes, "HTTP Request To Server");
	//	fprintf(log," HTTP Request To Server(%d)\n", iBytes);
		
	
			/* �����̣߳�����server��Ӧ */
			if(createThreadFlag)
			{
				createThreadFlag = false;		// ���߳�ֻ����һ��
				// �����߳�
				printf("Creating thread.\n");
				HANDLE threadHandle;
				ThreadParam threadParam;

				// �̲߳���
				threadParam.proxySocket1 = proxySocket1;
				threadParam.proxySocket2 = proxySocket2;
				threadParam.log = log;

				threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_server_response, (void*)&threadParam, 0, 0);
				if(threadHandle == NULL)
				{
					printf("Error at CreateThread()\n");
					return 1;
				}
				CloseHandle(threadHandle); 

				printf("Thread created.\n");
			}
	

		 }
	}
	 // �ر���־�ļ�
//	 fclose(log);
	 

	 // �ر�socket
	 closesocket(proxySocket1);
	 closesocket(proxySocket2);
	
	 printf("thread exit.\n");

	 return 0;
}


DWORD forward_server_response_thread(LPVOID lpParameter)
{
	ThreadParam threadParam = *(ThreadParam*)lpParameter;

	SOCKET proxySocket1 = threadParam.proxySocket1;
	SOCKET proxySocket2 = threadParam.proxySocket2;
	int		iBytes = 0;
	char	recvBuf[TENKB_SIZE];
	char	sendBuf[TENKB_SIZE];
	int		recvBufLen = TENKB_SIZE;

	while(1)
	{
		if( (iBytes = Recv(proxySocket2, recvBuf, recvBufLen, 0)) == 0)
		{
			break;
		}
		else if(iBytes == SOCKET_ERROR)
		{
			return 1;
		}
		printf("Recv From Server\n");
              
		memcpy(sendBuf, recvBuf, iBytes);
		if( (iBytes = Send(proxySocket1, sendBuf, iBytes, 0)) == 0)
		{
			break;
		}
		else if(iBytes == SOCKET_ERROR)
		{
			return 1;
		}
		printf("Send To Client\n");
	}
	return 0;
}


DWORD handle_server_response(LPVOID lpParameter)
{
		ThreadParam threadParam = *(ThreadParam*)lpParameter;
		SOCKET proxySocket1 = threadParam.proxySocket1;
		SOCKET proxySocket2 = threadParam.proxySocket2;
		
	// ��������server��HTTP��Ӧ
		int		counter = 0;
		int		iBytes = 0;
		char	recvBuf[TENKB_SIZE];
		char	sendBuf[TENKB_SIZE];
		int		recvBufLen = TENKB_SIZE;
		while(1)
		{
			if( (iBytes = Recv(proxySocket2, recvBuf, recvBufLen, 0)) == 0)
			{
				closesocket(proxySocket1);
				return 0;
			}
			else if(iBytes == SOCKET_ERROR)
			{
				closesocket(proxySocket1);
				return 1;
			}
		//	printHTTPMsg(recvBuf, iBytes, "HTTP Response From Server");
		//	printf("-----------(%d) HTTP Response From Server (%d Bytes)------------\n", ++counter, iBytes);
		//	fprintf(log," HTTP Response From Server(%d)\n", iBytes);

		
			/* �����µ�HTTP��Ӧ��client */
			memcpy(sendBuf, recvBuf, iBytes);

			if( (iBytes = Send(proxySocket1, sendBuf, iBytes, 0)) == 0)
			{
				closesocket(proxySocket2);
				return 0;
			}
			else if(iBytes == SOCKET_ERROR)
			{
				closesocket(proxySocket2);
				return 1;
			}
		//	printf("-----------(%d) HTTP Response To Client (%d Bytes)------------\n", counter, iBytes);
		//	printHTTPMsg(sendBuf, iBytes, "HTTP Response To Client");
	//		fprintf(log," HTTP Response To Client(%d)\n", iBytes);
		}

		return 0;
}


int handle_https_communication(SOCKET proxySocket1, SOCKET proxySocket2, FILE* log)
{
		/* ������Ӧ��client */
		char response[DEFAULT_SIZE];
		int response_len = 0;
		int iBytes = 0;
			
		strcpy(response, "HTTP/1.1 200 Connection Established\r\n\r\n");
		response_len = strlen(response);
		if( (iBytes = Send(proxySocket1, response, response_len, 0)) == 0)
		{
			closesocket(proxySocket2);
			return 0;
		}
		else if(iBytes == SOCKET_ERROR)
		{
			closesocket(proxySocket2);
			return 1;
		}
	
		printf("Response To Client.\n");

		/* ����һ���߳����ڽ���server����Ӧ�����͸�client */
		ThreadParam threadParam;
		HANDLE		threadHandle;
		
		// �̲߳���
		threadParam.proxySocket1 = proxySocket1;
		threadParam.proxySocket2 = proxySocket2;
		threadParam.log = log;

		threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)forward_server_response_thread, (void*)&threadParam, 0, 0);
		if(threadHandle == NULL)
		{
			printf("Error at CreateThread()\n");
			return 1;
		}
		CloseHandle(threadHandle); 
		
		
		/* ����client������ת����server */
		char recvBuf[TENKB_SIZE];
		char sendBuf[TENKB_SIZE];
		int recvBufLen = TENKB_SIZE;
		
		while(1)
		{
			if( (iBytes = Recv(proxySocket1, recvBuf, recvBufLen, 0)) == 0)
			{
				closesocket(proxySocket2);
				break;
			}
			else if(iBytes == SOCKET_ERROR)
			{
				closesocket(proxySocket2);
				return 1;
			}
			printf("Recv From Client\n");

			memcpy(sendBuf, recvBuf, iBytes);
			
			if( (iBytes = Send(proxySocket2, sendBuf, iBytes, 0)) == 0)
			{
				closesocket(proxySocket1);
				break;
			}
			else if(iBytes == SOCKET_ERROR)
			{
				closesocket(proxySocket1);
				return 1;
			}
			printf("Send To Server\n");
		}
		return 0;
}