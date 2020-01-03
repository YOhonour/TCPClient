#include <iostream>
#include<fstream>
#include "LoginUtil_C.h"
#include "TCPClient.h"
#include "WinSock2.h"
#pragma comment (lib,"Ws2_32.lib")
char MyName[16];
#define MAX_BUF_SIZE 65535
using namespace std;
char ClientBuf[MAX_BUF_SIZE];

//储存server端发送来的所有当前在线的Usr
map<string, string> mapAllUsr;

void runLogin(SOCKET serverSock) {
	while (true)
	{
		char usrName_Pwd[32],recvBuf[64];
		getLoginInfo(usrName_Pwd);//获取密码
		MSG_TO_Login msg;
		msg.Msg_LoginReqBody = *(Msg_LoginReqBody*)usrName_Pwd;//填充密码字段
		msg.msg_header.MsgType = 11;
		msg.msg_header.MsgBodyLen = sizeof(Msg_LoginReqBody);
		MsgHeader hd;
		send(serverSock, (char*)&msg, sizeof(msg), 0);
		recv(serverSock, (char *)&hd, sizeof(hd), 0);
		if (hd.Status_Code == 200)
		{
			memcpy(MyName, usrName_Pwd, 16);
			return;
		}
		else if (hd.Status_Code == 441)
		{
			std::cout << "用户名不存在，请重新登录\n";
		}
		else if (hd.Status_Code == 442)
		{
			std::cout << "密码错误，请重新登录\n";
		}

	}
}
//向服务器发送请求所有用户，响应获得返回后将反序列化为map储存
void getAllUsr(SOCKET serverSock) {
	
	while (true)
	{
		char usrBuf[MAX_BUF_SIZE] = { '\0' };
		MsgHeader hd;
		hd.MsgType = 23;
		hd.MsgBodyLen = sizeof(hd);
		send(serverSock, (char*)&hd, sizeof(hd), 0);
		char allUsrBuf[1024] = {'\0'};
		int bodyLen = 0;
		recv(serverSock, allUsrBuf, sizeof(allUsrBuf), 0);
		MSG_UsrInfo* resvHd = (MSG_UsrInfo*)allUsrBuf;
		if (resvHd->msg_header.MsgType == 24)
		{
			std::cout << "收到用户列表" << endl;
			SerializationReverse(resvHd->usrInfo, mapAllUsr);
			std::cout << "联系人列表初始化成功" << endl;
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	unsigned short ServerUDPPort;

	SOCKET cTCPSocket, cUDPSocket;
	WSADATA wsadata;
	SOCKADDR_IN TCPServer;
	unsigned long BytesReceived, BytesSent;
	int RetValue;
	//char ip[64] = { '\0' };
	char ip[64] = "192.168.102.1";
	char port[2] = { '\0' };

	std::cout << "请输入服务器IP：";
	//cin >> ip;
	std::cout << "请输入端口号：";
	//cin >> port;

	u_long ServerIP = inet_addr(ip);
	//u_short ServerTCPPort = (u_short)atoi(port);
	u_short ServerTCPPort = 8848;
	std::cout << "服务器IP：" << ip << endl;
	std::cout << "服务器port：" << ServerTCPPort << endl;

	//初始化winsock库
	if ((RetValue = WSAStartup(MAKEWORD(2, 2), &wsadata)) != 0)
	{
		printf("WSAStartup() failed with error %d\n", RetValue);
		return 2;
	}
	//创建TCP Socket
	if ((cTCPSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket() for cTCPSocket failed with error %d\n", WSAGetLastError());
		return 3;
	}

	//初始化地址
	TCPServer.sin_family = AF_INET;
	TCPServer.sin_port = htons(ServerTCPPort);
	TCPServer.sin_addr.S_un.S_addr = ServerIP;
	cout << "Conecting Server" << endl;
	//通过TCP Socket连接server
	RetValue = WSAConnect(cTCPSocket, (sockaddr*)&TCPServer, sizeof(TCPServer), NULL, NULL, NULL, NULL);
	if ((RetValue) == SOCKET_ERROR)
	{
		printf("WSAConnect() failed for cTCPSocket with error %d\n", WSAGetLastError());
		printf("Can't connect to server.\n");
		return 5;
	}

	//与server建立连接后读取Server发送过来的登录提示
	BytesReceived = recv(cTCPSocket, ClientBuf, sizeof(ClientBuf), 0);
	std::cout << ClientBuf << endl;
	runLogin(cTCPSocket);//执行登录，登陆成功后才会返回
	memset(ClientBuf, '\0', sizeof(ClientBuf));//重置信息缓冲区
	//接收服务端发送的用户串，并完成反序列化为map
	getAllUsr(cTCPSocket);
	std::cout << "获取所有用户" << endl;
	cout << "#################" << endl;
	for (auto& kv : mapAllUsr) {
		std::cout <<"#   "<< kv.first << endl;
	}
	cout << "#################\n" << endl;
	printPanal();
	cout << MyName << " ，你好" << endl;

	int cmd;
	
	
	
	//开启消息接收线程
	//cout << "请输入消息监听绑定端口：";
	u_short ServerMSG_TCPPort = 8849;
	///cin >> ServerMSG_TCPPort;
	std::thread MSGThread{ msgTecvThread,MyName ,ServerMSG_TCPPort };
	MSGThread.detach();
	while (true)
	{
		//system("cls");
		/*std::cout << "请输入聊天用户名或输入88结束程序:";
		string userName;
		cin >> userName;
		if (userName == "88")  C:\\MyCode\\C++\\file\\server\\1.txt
		{
			break;
		}*/
		
		std::cout << "请输入命令：";
		std::cin >> cmd;
		if (cmd == 1)
		{
			MSG_TO_Usr msg;
			msg.msg_header.MsgType = 31;
			msg.msgPoolEntity.msgType = 1;
			memcpy(msg.msgPoolEntity.SendFrom_name, MyName, 16);
			std::string desUsr,msgText;
			std::cout << "输入接收者：";
			std::cin >> msg.msgPoolEntity.SendTO_name;
			std::cout << "输入消息：\n";
			std::cin >> msg.msgPoolEntity.msg;
			msg.msgPoolEntity.msgLen = strlen(msg.msgPoolEntity.msg);
			send(cTCPSocket, (char*)&msg, sizeof(MSG_TO_Usr), 0);
		}
		else if (cmd == 2)//发送文件
		{
			MSG_File msgFile;
			ifstream in;
			string filePath;
			char ToUsr[16];
			
			char* fileBuf = (char*)malloc(50000);
			std::cout << "输入接收者：";
			cin >> msgFile.msgPoolEntity_FILE.SendTO_name;
			std::cout << "请输入文件全路径：" << endl;
			cin >> filePath;
			std::cout << "请输入文件名：" << endl;
			cin >> msgFile.msgPoolEntity_FILE.fileName;
			
			in.open(filePath, ifstream::binary);
			if (!in)
			{
				cerr << "源文件打开失败！" << endl;
				break;
			}
			while (1)
			{
				in.read(fileBuf, 50000 * sizeof(char));
				
				if (in.gcount() < 50000)
				{
					
					cout << "文件读取完毕！" << endl;
					break;
				}
			}
			msgFile.msgPoolEntity_FILE.msgLen = in.gcount();
			msgFile.msg_header.MsgType = 31;
			memcpy(msgFile.msgPoolEntity_FILE.SendFrom_name, MyName, 16);
			memcpy(msgFile.msgPoolEntity_FILE.FILE, fileBuf, 50000);
			send(cTCPSocket, (char*)&msgFile, sizeof(msgFile), 0);
			std::cout << "文件发送完毕" << endl;

		}
		else if (cmd == 3) //发送大文件
		{
			/*
			发送流程：
					1. client先发送一个文件发送询问请求 type=42
					2. server发送确认消息（此时服务器的Socket会被文件业务占用）
					3. client发送第一个文件块，FILEBody
						
			*/
			string SendTO_name;
			std::cout << "输入接收者：";
			cin >> SendTO_name;
			SendBigFile(cTCPSocket, MyName, SendTO_name);
		}
		else if (cmd == 4)
		{
			break;
		}
		//接收数据
	}
	MsgHeader hd;
	hd.MsgType = 88;
	send(cTCPSocket, (char*)&hd, sizeof(hd), 0);
	std::cout << "关闭连接" << endl;
}
void printPanal() {
	std::cout << "#####################################################################" << endl;
	std::cout << "#     1: 发送消息                                                  #" << endl;
	std::cout << "#     2: 发送小文件(小于50k）                                       #" << endl;
	std::cout << "#     3: 发送大文件                                                #" << endl;
	std::cout << "#     4: 退出                                                    #" << endl;
	std::cout << "##################################################################" << endl;
}
void clientRecvMsg(SOCKET cTCPSocket){
	char* msgBuf = (char*)malloc(MAX_BUF_SIZE*sizeof(char));
	while (true)
	{
		recv(cTCPSocket, msgBuf, sizeof(msgBuf), 0);

	}
}
//大文件文件接收方法  C:\\MyCode\\2.pdf
void FileResv(SOCKET server, FILESendRequest fileRequ) {
	string fileName = fileRequ.FileName;//文件名
	MsgHeader hd;
	hd.MsgType = 42;
	char* fileResvBuf = (char*)malloc(MAX_BUF_SIZE * sizeof(char));
	send(server, (char*)&hd, sizeof(MsgHeader), 0);//发送确认消息
	ZeroMemory(fileResvBuf, MAX_BUF_SIZE * sizeof(char));
	
	ofstream file_out;//文件输出流
	string fileRealPath = filePath + MyName + "\\" + fileName;//接收文件保存路径
	file_out.open(fileRealPath, ifstream::binary);
	
	//file_out.write(pFILEBody->BITES, pFILEBody->AllPackages);//写入
	recv(server, fileResvBuf, MAX_BUF_SIZE, 0);//接收文件包
	
	FILEBody* pFILEBody = (FILEBody*)(fileResvBuf + sizeof(MsgHeader));
	ULONG allPacks = pFILEBody->AllPackages;
	file_out.write(pFILEBody->BITES, pFILEBody->AllPackages);//写入
	send(server, (char*)&hd, sizeof(MsgHeader), 0);//发送确认消息
	while (allPacks >= 50000)
	{
		recv(server, fileResvBuf, MAX_BUF_SIZE, 0);//接收文件包
		send(server, (char*)&hd, sizeof(MsgHeader), 0);//发送确认消息
		//pFILEBody = (FILEBody*)(fileResvBuf + sizeof(MsgHeader));
		file_out.write(pFILEBody->BITES, pFILEBody->AllPackages);
		
		allPacks = pFILEBody->AllPackages;
	}
	send(server, (char*)&hd, sizeof(MsgHeader), 0);//发送确认消息
	cout << endl << "收到来自 " << fileRequ.SendFrom_name << " 的文件，保存为 " << fileRealPath << endl;
	file_out.close();
	free(fileResvBuf);
}