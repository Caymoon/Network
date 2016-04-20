/*
1.��ǰ����ʵ��Ļ����ϣ��������Ϊһ���ܴ���ָ���ļ����Ƶĵ�Ե��ļ��������
2.��Ʋ�ʵ��һ��֧�ֶ���ͻ��˵��ļ����������
3.�ͻ��˵ȴ����������ļ����ƣ�Ȼ���ļ����ƴ������������
  ��������Ԥ�����úõ��ļ����²��Ҹ��ļ����������ͬ���ļ�����ʼ����ؿͻ��ˣ�
  �ͻ��˽������ļ����ļ���������ļ����Ʊ����ڱ���ĳ��Ŀ¼���ɣ�������߿ͻ����ļ������ڡ�
*/
#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include "header.h"
#pragma comment(lib, "ws2_32.lib")
#define CONNECT_NUM_MAX 10

using namespace std;

/*���ͷ�����֧�ֵ�����*/
void SendCmd(SOCKET sClient)
{
	tMSG_CMD cmd;
	sprintf_s(cmd.commad, "Type filename to get a file, /q to exit.\n>");
	send(sClient, reinterpret_cast<char*>(&cmd), sizeof(cmd), 0);
}

/*�ͻ�����������ļ����������ļ����ȣ����ļ������ڴ���*/
void SendFileLength(SOCKET sClient, tMSG_HEADER *p_msg_header, int id)
{
	tMSG_FILENAME *p_msg_filename = static_cast<tMSG_FILENAME*>(p_msg_header);
	ifstream filein(p_msg_filename->szFileName, ios::in | ios::binary);
	if (!filein)
	{
		//�ļ�������
		cout << "[Client " << id << "]<" << p_msg_filename->szFileName << "> File Not Found!\n";
		tMSG_ERROR ErrMsg(MSG_OPENFILE_ERROR);
		send(sClient, reinterpret_cast<char*>(&ErrMsg), sizeof(ErrMsg), 0);
	}
	else
	{
		//�����ļ�����
		filein.seekg(0, ios::end);
		long length = filein.tellg();
		cout << "[Client " << id << "]<" << p_msg_filename->szFileName << "> File Length = " << length << endl;
		tMSG_FILE_LENGTH FileLength(length);
		strcpy(FileLength.szFileName, p_msg_filename->szFileName);
		send(sClient, reinterpret_cast<char*>(&FileLength), sizeof(FileLength), 0);
	}
	filein.close();
}

/*�����ļ�*/
void SendFile(SOCKET sClient, tMSG_HEADER *p_msg_header, int id)
{
	tMSG_CLIENT_READY *p_msg_client_ready = static_cast<tMSG_CLIENT_READY*>(p_msg_header);
	ifstream fin(p_msg_client_ready->szFileName, ios::in | ios::binary);
	if (!fin) { cout << "[Client " << id << "]<" << p_msg_client_ready->szFileName << "> Error Open File!\n"; }

	/*���ϴν����ĵط���ʼ����*/
	fin.seekg(p_msg_client_ready->lLastPosition, ios::beg);

	tMSG_FILE tSendFile;
	tSendFile.tFile.lStart = p_msg_client_ready->lLastPosition;
	tSendFile.tFile.lFileLength = p_msg_client_ready->lLength;
	strcpy(tSendFile.tFile.szFileName, p_msg_client_ready->szFileName);

	if (tSendFile.tFile.lFileLength - tSendFile.tFile.lStart > MAX_FILE_SIZE)
	{	//Ҫ���͵ĳ��ȴ���һ�δ��͵ĳ���
		tSendFile.tFile.lSize = MAX_FILE_SIZE;
	}
	else
	{	//Ҫ���͵��ļ����ȿ�����һ���ڴ������
		tSendFile.tFile.lSize = tSendFile.tFile.lFileLength - tSendFile.tFile.lStart;
	}

	fin.read(tSendFile.tFile.szBuff, tSendFile.tFile.lSize);
	fin.close();
	//	cout << (int)tSendFile.cMsgId;
//	tMSG_HEADER *p_header = static_cast<tMSG_HEADER*>(&tSendFile);
//	cout << (int)p_header->cMsgId;
	send(sClient, reinterpret_cast<char*>(&tSendFile), sizeof(tSendFile), 0);
	cout << "[Client " << id << "]<" << tSendFile.tFile.szFileName << "> �ѷ���" << tSendFile.tFile.lSize + tSendFile.tFile.lStart << "/" << tSendFile.tFile.lFileLength << endl;
}

void SendSucc(SOCKET socket, tMSG_HEADER *p_msg_header, int id)
{
	tMSG_SEND_FILE_SUCC *succ = static_cast<tMSG_SEND_FILE_SUCC*>(p_msg_header);
	cout << "[Client " << id << "]<" << succ->szFileName << "> Send File Success!\n";
	SendCmd(socket);
}

/*������Ϣ����Ҫ�Ͽ�����ʱ����false*/
bool ProcessMsg(SOCKET sClient, int id)
{
	char recvBuf[MAX_PACKET_SIZE];
	//������Ϣ
	int nRecv = recv(sClient, recvBuf, sizeof(recvBuf) + 1, 0);
	if (nRecv > 0)
	{
		recvBuf[nRecv] = '\0';
	}
	//����
	tMSG_HEADER *p_msg_header = reinterpret_cast<tMSG_HEADER*>(recvBuf);
	switch (p_msg_header->cMsgId)
	{
	case MSG_HELLO:					SendCmd(sClient);							break;
	case MSG_FILENAME:				SendFileLength(sClient, p_msg_header, id);	break;
	case MSG_CLIENT_READY:			SendFile(sClient, p_msg_header, id);		break;
	case MSG_SEND_FILE_SUCCESS:		SendSucc(sClient, p_msg_header, id);		break;
	case MSG_FILE_ALREADYEXIT_ERROR:break;//���ǿͻ����ļ�����������ʱ���账��
	case MSG_BYE:return false;
	}
	return true;
}

//http://blog.csdn.net/luoweifu/article/details/46835437
#define NAME_LINE   40
//�����̺߳�����������Ľṹ��
typedef struct __THREAD_DATA
{
	int id;
	SOCKET connSocket;
	SOCKADDR_IN clientAddr;

	__THREAD_DATA(SOCKET socket, SOCKADDR_IN client, int _id = 0)
	{
		connSocket = socket;
		clientAddr = client;
		id = _id;
	}
}THREAD_DATA;

//�̺߳���
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	THREAD_DATA* pThreadData = static_cast<THREAD_DATA*>(lpParameter);
	SOCKET connSocket = pThreadData->connSocket;
	SOCKADDR_IN clientAddr = pThreadData->clientAddr;
	int id = pThreadData->id;

	cout << "[Client " << id << "] Connected.������.\n";

	SendCmd(connSocket);

	while (true)
	{
		if (!ProcessMsg(connSocket, id))break;
	}
	cout << "[Client " << id << "] Disconnected.�Ͽ�����.\n";
	tMSG_BYE p_bye;
	send(connSocket, reinterpret_cast<char*>(&p_bye), sizeof(p_bye), 0);
	closesocket(connSocket);
	return 0L;
}

int main(int argc, char** argv)
{
	int port = 1234;
	if (argc > 1)
	{
		port = atoi(argv[1]);
		if (port < 1024 || port>65534)
		{
			printf("Port error. Use %s port\n", argv[0]);
		}
	}
	//�����׽��ֿ�
	WSADATA wsaData;
	int iRet = 0;
	iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet != 0)
	{
		cout << "WSAStartup(MAKEWORD(2, 2), &wsaData) execute failed!" << endl;;
		return -1;
	}
	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
	{
		WSACleanup();
		cout << "WSADATA version is not correct!" << endl;
		return -1;
	}

	//�����׽���
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "serverSocket = socket(AF_INET, SOCK_STREAM, 0) execute failed!" << endl;
		return -1;
	}

	//��ʼ����������ַ�����
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	//��
	iRet = bind(serverSocket, reinterpret_cast<SOCKADDR*>(&addrSrv), sizeof(SOCKADDR));
	if (iRet == SOCKET_ERROR)
	{
		cout << "bind(serverSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) execute failed!" << endl;
		return -1;
	}


	//����
	iRet = listen(serverSocket, CONNECT_NUM_MAX);
	if (iRet == SOCKET_ERROR)
	{
		cout << "listen(serverSocket," << CONNECT_NUM_MAX << ") execute failed!" << endl;
		return -1;
	}

	//�ȴ�����_����_����
	SOCKADDR_IN clientAddr;
	int len = sizeof(SOCKADDR);
	int count = 0;
	cout << "Server is started\n";
	while (1)
	{
		SOCKET connSocket = accept(serverSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &len);
		if (connSocket == INVALID_SOCKET)
		{
			cout << "accept(serverSocket, (SOCKADDR*)&clientAddr, &len) execute failed!" << endl;
			return -1;
		}

		THREAD_DATA threadData1(connSocket, clientAddr, count++);
		HANDLE thread = CreateThread(nullptr, 0, ThreadProc, &threadData1, 0, nullptr);
		CloseHandle(thread);
		//		//C++ �� map ������˵����ʹ�ü���
		//		//http://www.cnblogs.com/anywei/archive/2011/10/27/2226830.html
		//		g_Map->insert[threadData1.id] = thread;
	}
	return 0;
}