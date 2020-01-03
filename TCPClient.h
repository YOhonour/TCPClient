#pragma once
#include <iostream>
#include "WinSock2.h"
#include "Windows.h"
#include <iphlpapi.h>
#include<map>
#include<queue>
#include<thread>
#include <stdio.h>
//#include"LoginUtil.h"

#pragma comment(lib,"iphlpapi")
#pragma comment(lib,"Ws2_32")

#define usrNameNotKonw 441
#define pwdError 442
#define MAX_CLIENT 1000
#define MAX_BUF_SIZE 65535
#define SUCCESS 200
using namespace std;



using namespace std;

typedef std::queue<string> msgQueue;
typedef string usrName;
int getIPAddrFromHost();
void prtIP(ULONG ulHostIP);
typedef struct TcpThreadParam
{
	SOCKET socket;
	sockaddr_in addr;
}TCPThreadParam;


#define LoginReq 11
#define LoginResp 12
#define ReqLoginedUsrs 12
#define ReqLoginedUsrs 24
#define sendMsg 31
#define EXIT_sig 88
#define NAME_LEN 16
#define PWD_LEN 16
std::map<usrName, TCPThreadParam> loginClient;
//TCP协议层传输的消息
typedef struct MsgHeader
{
	//状态码为200正常，状态码441为用户不存在，442为密码错误
	UCHAR Status_Code = 200;
	/*
	消息类型：
			11 登录请求 消息体(用户名-密码 16+16 Msg_LoginReqBody) 
			12 登录响应 只响应一个头部信息
			23 请求获取当前在线的用户 
			24 返回当前所有用户map序列化
			31 发送的信息
			41 发送文件请求 Client
			42 确认文件发送 server
			43 发送文件
			88 终止连接
	*/
	UCHAR MsgType;
	//消息总长度
	UINT MsgBodyLen;
}MsgHeader;

//发送的信息
typedef struct MsgPoolEntity {
	char SendFrom_name[NAME_LEN];//发送者
	char SendTO_name[NAME_LEN]; //收方用户
	char msgType = 1; //消息类型，1 文本 ，2 文件
	UINT msgLen; //消息长度
	char msg[2048];//消息
}MsgPoolEntity;
//发送的信息
typedef struct MsgPoolEntity_FILE {
	char SendFrom_name[NAME_LEN];//发送者
	char SendTO_name[NAME_LEN]; //收方用户
	char msgType = 2; //消息类型，1 文本 ，2 文件
	UINT msgLen; //消息长度
	char fileName[32] = {'\0'};//文件名
	char FILE[50000];//文件
}MsgPoolEntity_FILE;

typedef struct Msg_LoginReqBody {
	char usrName[16];//登录用户名
	char pwd[16];//密码
}Msg_LoginReqBody;

typedef struct MSG_TO_Usr {
	MsgHeader msg_header;
	MsgPoolEntity msgPoolEntity;
}MSG_TO_Usr;
typedef struct MSG_TO_Login {
	MsgHeader msg_header;
	Msg_LoginReqBody Msg_LoginReqBody;
}MSG_TO_Login;

typedef struct MSG_UsrInfo {
	MsgHeader msg_header;
	char usrInfo[256] = { "\0" };
}MSG_UsrInfo;

typedef struct MSG_File {
	MsgHeader msg_header;
	MsgPoolEntity_FILE msgPoolEntity_FILE;
}MSG_File;

//订阅消息需要发送的信息
typedef struct MSGComfirm {
	MsgHeader msg_header;
	char userName[16] = { "\0" };
}MSGComfirm;

//询问请求发送文件头部
typedef struct FILESendRequest {
	char SendFrom_name[NAME_LEN];//发送者
	char SendTO_name[NAME_LEN]; //收方用户
	char FileName[32];//文件名
}FILESendRequest;
//发送文件的头部，每个实体承载50000字节文件数据
typedef struct FILEBody {
	//int FileID;//文件id ，标识
	ULONG CorrentPacNO; //当前数据包号
	ULONG AllPackages; //总共数据包个数
	ULONG correntSize;//当前包大小
	char BITES[50000];//文件实体部分
}FILEBody;

//发送文件的头部，每个实体承载50000字节文件数据
typedef struct MSG_FILE {
	MsgHeader msg_header;
	FILEBody fileBody;
}MSG_FILE;


//文件发送
typedef struct FILERequest {
	MsgHeader msg_header;
	FILESendRequest requsBody;
}FILERequest;

//为发送文件构建的包
typedef struct FILEBodyForSend {
	MsgHeader msg_header;
	FILEBody fileBody;//文件包
}FILEBodyForSend;
void FileResv(SOCKET server, FILESendRequest fileRequ);

//分割目标字符串存入二维数组中
int split(char dst[][80], char* str, const char* spl)
{
	int n = 0;
	char* result = NULL;
	result = strtok(str, spl);
	while (result != NULL)
	{
		strcpy(dst[n++], result);
		result = strtok(NULL, spl);
	}
	return n;
}

//将标准字符串反序列化为map
void SerializationReverse(char* resBuf, map<string, string>& mapDes) {
	char usrInfo[20][80] = { '\0' };
	split(usrInfo, resBuf, ";");
	for (int i = 0; i < 20; i++) {
		if (usrInfo[i][0] == '\0')
		{
			break;
		}
		char tempS[2][80] = { '\0' };
		split(tempS, usrInfo[i], ":");
		mapDes[tempS[0]] = tempS[1];

	}
}
void printPanal();

string filePath = "C:\\MyCode\\C++\\file\\client\\";
string pathDis = "\\";

int msgTecvThread(string Name, u_short ServerMSG_TCPPort) {
	SOCKET c_MSG_TCPSocket;//消息发送的本地Socket
	WSADATA wsadata;
	SOCKADDR_IN TCPServer;//构建本地地址
	int RetValue;
	char ip[64] = "192.168.102.1";
	u_long ServerIP = inet_addr(ip);
	//u_short ServerTCPPort = ServerMSG_TCPPort;
	u_short ServerTCPPort = 8849;
	if ((c_MSG_TCPSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket() for cTCPSocket failed with error %d\n", WSAGetLastError());
		return 3;
	}
	//初始化地址
	TCPServer.sin_family = AF_INET;
	TCPServer.sin_port = htons(ServerTCPPort);
	TCPServer.sin_addr.S_un.S_addr = ServerIP;
	//通过TCP Socket连接server
	if ((RetValue = WSAConnect(c_MSG_TCPSocket, (sockaddr*)&TCPServer, sizeof(TCPServer), NULL, NULL, NULL, NULL)) == SOCKET_ERROR)
	{
		printf("WSAConnect() failed for cTCPSocket with error %d\n", WSAGetLastError());
		printf("Can't connect to server.\n");
		return 5;
	}
	std::cout << "\n消息监听服务连接成功！" << endl;
	char* MSG_Buf = (char*)malloc(65535);
	MSGComfirm cf;
	memcpy((char*)cf.userName, Name.c_str(), 16);
	send(c_MSG_TCPSocket, (char*)&cf, sizeof(cf), 0);
	while (true)
	{
		recv(c_MSG_TCPSocket, MSG_Buf, 65535, 0);
		MSG_TO_Usr* usrMsg = (MSG_TO_Usr*)MSG_Buf;
		MSG_File* f = (MSG_File*)MSG_Buf;
		
		if (usrMsg->msg_header.MsgType == 41)//收到文件传送请求
		{
			FileResv(c_MSG_TCPSocket,*((FILESendRequest*)(MSG_Buf+sizeof(MsgHeader))));
		}
		else if (usrMsg->msgPoolEntity.msgType == 1)
		{
			std::cout << "\n收到来自 " << usrMsg->msgPoolEntity.SendFrom_name
				<< "的消息：" << usrMsg->msgPoolEntity.msg << endl;
		}
		else if (f->msgPoolEntity_FILE.msgType == 2)
		{
			string fn = f->msgPoolEntity_FILE.fileName;
			string path = filePath + Name + pathDis + fn;
			ofstream out(path, ios::app, ios::binary| ios::trunc);
			out.write(f->msgPoolEntity_FILE.FILE, f->msgPoolEntity_FILE.msgLen);
			out.close();
			cout << "\n收到来自 " << f->msgPoolEntity_FILE.SendFrom_name
				<< "的文件：" << f->msgPoolEntity_FILE.fileName << endl;
		}
		ZeroMemory(MSG_Buf, sizeof(65535));
	}
}

//文件发送函数
void SendBigFile(SOCKET des, string SendFrom_name, string SendTO_name) {
	//先发送一个文件发送询问请求
	std::string fpath;
	char fileName[32] = { '\0' };
	cout << "请输入文件全路径" << endl;
	cin >> fpath;
	cout << "请输入文件名" << endl;
	cin >> fileName;
	FILERequest* rq = (FILERequest*)malloc(sizeof(FILERequest));
	rq->msg_header.Status_Code = 200;
	rq->msg_header.MsgType = 41;
	char* reBuf = (char*)malloc(65535);
	ZeroMemory(reBuf, 65535);
	memcpy(rq->requsBody.FileName, fileName, 32);
	memcpy(rq->requsBody.SendFrom_name, SendFrom_name.c_str(), NAME_LEN);
	memcpy(rq->requsBody.SendTO_name, SendTO_name.c_str(), NAME_LEN);
	send(des, (char*)rq, sizeof(FILERequest), 0);//发送文件询问请求
	recv(des, reBuf, 65535, 0);
	//收到后判断是否为确认信息 C:\\MyCode\\21.png
	MsgHeader* hd = (MsgHeader*)reBuf;
	if (hd->MsgType != 42)
	{
		std::cout << "确认失败\n";
		return;
	}
	ifstream in;
	in.open(fpath, ifstream::binary);
	if (!in)
	{
		cerr << "源文件打开失败！" << endl;
		return;
	}
	MsgHeader* hdBuf = (MsgHeader*)malloc(sizeof(MsgHeader));
	while (1)
	{
		FILEBodyForSend Filesend;
		
		in.read(Filesend.fileBody.BITES, 50000 * sizeof(char));
		Filesend.fileBody.AllPackages = in.gcount();
		int cu = in.gcount();
		send(des, (char*)&Filesend, sizeof(FILEBodyForSend), 0);
		//char* hdBuf = (char*)malloc(sizeof(MsgHeader));
		recv(des, (char*)hdBuf, sizeof(MsgHeader), 0);
		
		if (hdBuf->Status_Code != 200)
		{
			cout << "读取失败！" << endl;
			break;
		}
		if (in.gcount() < 50000)
		{

			cout << "文件读取完毕！" << endl;
			break;
		}
	}
	free(reBuf);
	cout << "文件发送完毕！" << endl;
}