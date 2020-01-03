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
//TCPЭ��㴫�����Ϣ
typedef struct MsgHeader
{
	//״̬��Ϊ200������״̬��441Ϊ�û������ڣ�442Ϊ�������
	UCHAR Status_Code = 200;
	/*
	��Ϣ���ͣ�
			11 ��¼���� ��Ϣ��(�û���-���� 16+16 Msg_LoginReqBody) 
			12 ��¼��Ӧ ֻ��Ӧһ��ͷ����Ϣ
			23 �����ȡ��ǰ���ߵ��û� 
			24 ���ص�ǰ�����û�map���л�
			31 ���͵���Ϣ
			41 �����ļ����� Client
			42 ȷ���ļ����� server
			43 �����ļ�
			88 ��ֹ����
	*/
	UCHAR MsgType;
	//��Ϣ�ܳ���
	UINT MsgBodyLen;
}MsgHeader;

//���͵���Ϣ
typedef struct MsgPoolEntity {
	char SendFrom_name[NAME_LEN];//������
	char SendTO_name[NAME_LEN]; //�շ��û�
	char msgType = 1; //��Ϣ���ͣ�1 �ı� ��2 �ļ�
	UINT msgLen; //��Ϣ����
	char msg[2048];//��Ϣ
}MsgPoolEntity;
//���͵���Ϣ
typedef struct MsgPoolEntity_FILE {
	char SendFrom_name[NAME_LEN];//������
	char SendTO_name[NAME_LEN]; //�շ��û�
	char msgType = 2; //��Ϣ���ͣ�1 �ı� ��2 �ļ�
	UINT msgLen; //��Ϣ����
	char fileName[32] = {'\0'};//�ļ���
	char FILE[50000];//�ļ�
}MsgPoolEntity_FILE;

typedef struct Msg_LoginReqBody {
	char usrName[16];//��¼�û���
	char pwd[16];//����
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

//������Ϣ��Ҫ���͵���Ϣ
typedef struct MSGComfirm {
	MsgHeader msg_header;
	char userName[16] = { "\0" };
}MSGComfirm;

//ѯ���������ļ�ͷ��
typedef struct FILESendRequest {
	char SendFrom_name[NAME_LEN];//������
	char SendTO_name[NAME_LEN]; //�շ��û�
	char FileName[32];//�ļ���
}FILESendRequest;
//�����ļ���ͷ����ÿ��ʵ�����50000�ֽ��ļ�����
typedef struct FILEBody {
	//int FileID;//�ļ�id ����ʶ
	ULONG CorrentPacNO; //��ǰ���ݰ���
	ULONG AllPackages; //�ܹ����ݰ�����
	ULONG correntSize;//��ǰ����С
	char BITES[50000];//�ļ�ʵ�岿��
}FILEBody;

//�����ļ���ͷ����ÿ��ʵ�����50000�ֽ��ļ�����
typedef struct MSG_FILE {
	MsgHeader msg_header;
	FILEBody fileBody;
}MSG_FILE;


//�ļ�����
typedef struct FILERequest {
	MsgHeader msg_header;
	FILESendRequest requsBody;
}FILERequest;

//Ϊ�����ļ������İ�
typedef struct FILEBodyForSend {
	MsgHeader msg_header;
	FILEBody fileBody;//�ļ���
}FILEBodyForSend;
void FileResv(SOCKET server, FILESendRequest fileRequ);

//�ָ�Ŀ���ַ��������ά������
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

//����׼�ַ��������л�Ϊmap
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
	SOCKET c_MSG_TCPSocket;//��Ϣ���͵ı���Socket
	WSADATA wsadata;
	SOCKADDR_IN TCPServer;//�������ص�ַ
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
	//��ʼ����ַ
	TCPServer.sin_family = AF_INET;
	TCPServer.sin_port = htons(ServerTCPPort);
	TCPServer.sin_addr.S_un.S_addr = ServerIP;
	//ͨ��TCP Socket����server
	if ((RetValue = WSAConnect(c_MSG_TCPSocket, (sockaddr*)&TCPServer, sizeof(TCPServer), NULL, NULL, NULL, NULL)) == SOCKET_ERROR)
	{
		printf("WSAConnect() failed for cTCPSocket with error %d\n", WSAGetLastError());
		printf("Can't connect to server.\n");
		return 5;
	}
	std::cout << "\n��Ϣ�����������ӳɹ���" << endl;
	char* MSG_Buf = (char*)malloc(65535);
	MSGComfirm cf;
	memcpy((char*)cf.userName, Name.c_str(), 16);
	send(c_MSG_TCPSocket, (char*)&cf, sizeof(cf), 0);
	while (true)
	{
		recv(c_MSG_TCPSocket, MSG_Buf, 65535, 0);
		MSG_TO_Usr* usrMsg = (MSG_TO_Usr*)MSG_Buf;
		MSG_File* f = (MSG_File*)MSG_Buf;
		
		if (usrMsg->msg_header.MsgType == 41)//�յ��ļ���������
		{
			FileResv(c_MSG_TCPSocket,*((FILESendRequest*)(MSG_Buf+sizeof(MsgHeader))));
		}
		else if (usrMsg->msgPoolEntity.msgType == 1)
		{
			std::cout << "\n�յ����� " << usrMsg->msgPoolEntity.SendFrom_name
				<< "����Ϣ��" << usrMsg->msgPoolEntity.msg << endl;
		}
		else if (f->msgPoolEntity_FILE.msgType == 2)
		{
			string fn = f->msgPoolEntity_FILE.fileName;
			string path = filePath + Name + pathDis + fn;
			ofstream out(path, ios::app, ios::binary| ios::trunc);
			out.write(f->msgPoolEntity_FILE.FILE, f->msgPoolEntity_FILE.msgLen);
			out.close();
			cout << "\n�յ����� " << f->msgPoolEntity_FILE.SendFrom_name
				<< "���ļ���" << f->msgPoolEntity_FILE.fileName << endl;
		}
		ZeroMemory(MSG_Buf, sizeof(65535));
	}
}

//�ļ����ͺ���
void SendBigFile(SOCKET des, string SendFrom_name, string SendTO_name) {
	//�ȷ���һ���ļ�����ѯ������
	std::string fpath;
	char fileName[32] = { '\0' };
	cout << "�������ļ�ȫ·��" << endl;
	cin >> fpath;
	cout << "�������ļ���" << endl;
	cin >> fileName;
	FILERequest* rq = (FILERequest*)malloc(sizeof(FILERequest));
	rq->msg_header.Status_Code = 200;
	rq->msg_header.MsgType = 41;
	char* reBuf = (char*)malloc(65535);
	ZeroMemory(reBuf, 65535);
	memcpy(rq->requsBody.FileName, fileName, 32);
	memcpy(rq->requsBody.SendFrom_name, SendFrom_name.c_str(), NAME_LEN);
	memcpy(rq->requsBody.SendTO_name, SendTO_name.c_str(), NAME_LEN);
	send(des, (char*)rq, sizeof(FILERequest), 0);//�����ļ�ѯ������
	recv(des, reBuf, 65535, 0);
	//�յ����ж��Ƿ�Ϊȷ����Ϣ C:\\MyCode\\21.png
	MsgHeader* hd = (MsgHeader*)reBuf;
	if (hd->MsgType != 42)
	{
		std::cout << "ȷ��ʧ��\n";
		return;
	}
	ifstream in;
	in.open(fpath, ifstream::binary);
	if (!in)
	{
		cerr << "Դ�ļ���ʧ�ܣ�" << endl;
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
			cout << "��ȡʧ�ܣ�" << endl;
			break;
		}
		if (in.gcount() < 50000)
		{

			cout << "�ļ���ȡ��ϣ�" << endl;
			break;
		}
	}
	free(reBuf);
	cout << "�ļ�������ϣ�" << endl;
}