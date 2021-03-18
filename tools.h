#include <winsock2.h>

#define LOCAL_IP_ADDRESS "192.168.1.102"
//#define LOCAL_IP_ADDRESS "172.20.10.2"
#define	LOCAL_LISTEN_PORT 8080
#define SERVER_PORT			80
#define	SSL_PORT			443

#define DEFAULT_SIZE	100
#define KB_SIZE			1024
#define TENKB_SIZE		10240
#define MB_SIZE			1048576
 



typedef struct ParseResult
{
	ULONG	serverIPAddr;
	char	new_http_request[TENKB_SIZE];
	int		new_http_request_len;
	bool	isMethodEqConnect;

}ParseResult;




void fprintHTTPMsg(FILE* file, char* msg,int msg_len);

void printHTTPMsg(char* msg, int msglen, char* str);

void parseHTTPRequest(char* http_request, int http_request_len, ParseResult* result);


void getIPAddr(char* addr, int addr_len);

int getHTTPHost(char* msg, char* host);


// 读取HTTP报文，若是请求首部包含connect方法，则构造一个回复
int handleSSLConnect(char* httpmsg, char* response);
