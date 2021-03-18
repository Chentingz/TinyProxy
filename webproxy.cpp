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

	sockaddr_in remoteAddr;			// 记录连接的客户端地址、端口
	int remoteAddrLen = sizeof(remoteAddr);

	SOCKET proxySocket;				// 用于与客户端通信的socket
	
//	HANDLE threadHandle;
//	ThreadParam threadParam;



	dirlen = filelen = pathlen = DEFAULT_SIZE;

    time(&timep);  
	ptmNow = localtime(&timep);
	
	memset(path, '\0', pathlen);
	memset(dir, '\0', dirlen);
	memset(filename, '\0', filelen);

	
	//getcwd(dir, dirlen);

	// TODO:路径写死了，需要后期修改
	sprintf(filename,"E:\\Code\\WebProxy\\log\\%d-%d-%d-%d-%d-%d.txt", (1900 + ptmNow->tm_year), (1+ptmNow->tm_mon), ptmNow->tm_mday,ptmNow->tm_hour, ptmNow->tm_min, ptmNow->tm_sec);
	printf("log path: %s\n", filename);


   // strcpy(filename,ctime(&timep));
	
	// 打开日志文件
	
/*	log = fopen(filename, "a+");
	if(log == NULL)
	{
		printf("Couldn't open file.\n");
		return 1;
	}
*/

	/* 初始化winsock */
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	/* 创建套接字用于监听连接 */
	listenSock = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	// 本地地址	
	localAddr.sin_family = PF_INET;
	localAddr.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);
	localAddr.sin_port = htons(LOCAL_LISTEN_PORT);


	/* 绑定套接字 */	
	iResult = Bind(listenSock, (sockaddr*)&localAddr, sizeof(localAddr));


	/* 监听套接字 */	
	iResult = Listen(listenSock, SOMAXCONN);
		
    remoteAddrLen = sizeof(remoteAddr);
	while(true)
	{
		printf("Listening on local port %hu...\n", ntohs(localAddr.sin_port));

		// 接收客户端请求，返回一个socket用于与其通信
		proxySocket = Accept(listenSock, (sockaddr*)&remoteAddr, &remoteAddrLen);

		printf("Accept connectin from %s:%hu\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
	//	fprintf(log, "Accept connectin from %s:%hu\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
		
		ThreadParam threadParam;
		HANDLE		threadHandle;
		// 创建线程
		printf("Creating thread.\n");
		
		// 线程参数
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
		
		// 睡眠1s
		Sleep(1000);
	}

//	fclose(log);
	closesocket(listenSock);
	WSACleanup();

	return 0;
}





// 线程处理函数
DWORD handle_client_request(LPVOID lpParameter)
{
	// 用于与客户端通信的socket
	ThreadParam threadParam = *(ThreadParam*)lpParameter;

	SOCKET proxySocket1 = threadParam.proxySocket1;
	FILE* log = threadParam.log;

	int  iBytes;							 // 记录接收、发送的字节数
	int  iResult;							 // 记录函数的调用结果
	char sendBuf[TENKB_SIZE];			     // 发送缓冲区
	int	 sendBufLen = TENKB_SIZE;			 // 发送缓冲区大小
	char recvBuf[TENKB_SIZE];			     // 接收缓冲区
	int	 recvBufLen = TENKB_SIZE;			 // 接收缓冲区大小

	SOCKET proxySocket2;
	bool	createThreadFlag;
	bool	connectFlag;

	createThreadFlag = connectFlag = true;
	while(1)
	{
		/* 接收客户端的HTTP请求 */
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

		/* 解析HTTP请求 */
		ParseResult pResult;
		pResult.isMethodEqConnect = false;
		parseHTTPRequest(recvBuf, iBytes, &pResult);
		
		
		if(connectFlag)
		{
			connectFlag = false;			// 只连接一次
		 /* 配置服务器地址 */
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

		 // 创建与服务器进行通信的socket
		 proxySocket2 = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
	//	 int RCVTIMEO = 30000;
	//	 setsockopt(proxySocket2, SOL_SOCKET, SO_RCVTIMEO, (char*)&RCVTIMEO, sizeof(RCVTIMEO));

		 /* 与服务器进行连接 */
		 printf("Connecting to server %s...\n", inet_ntoa(serverAddr.sin_addr));
		 iResult = Connect(proxySocket2, (sockaddr*)&serverAddr, sizeof(serverAddr));
		}

		 // 请求方法是CONNECT,则根据https通信进行处理
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
			
		 
		 // 向服务器发送HTTP请求
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
		
	
			/* 创建线程，接收server响应 */
			if(createThreadFlag)
			{
				createThreadFlag = false;		// 该线程只创建一次
				// 创建线程
				printf("Creating thread.\n");
				HANDLE threadHandle;
				ThreadParam threadParam;

				// 线程参数
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
	 // 关闭日志文件
//	 fclose(log);
	 

	 // 关闭socket
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
		
	// 接收来自server的HTTP响应
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

		
			/* 发送新的HTTP响应给client */
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
		/* 发送响应给client */
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

		/* 创建一个线程用于接收server的响应并发送给client */
		ThreadParam threadParam;
		HANDLE		threadHandle;
		
		// 线程参数
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
		
		
		/* 接收client的请求，转发给server */
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