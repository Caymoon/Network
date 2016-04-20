#pragma once
#ifndef SEND_FILE_HEADER__
#define SEND_FILE_HEADER__

#define MAX_PACKET_SIZE				10240	// ���ݰ�����󳤶�, ��λ�� sizeof(char)
#define	FILENAME_LENGTH				256		//�ļ�������
#define MAX_FILE_SIZE				((MAX_PACKET_SIZE)-3*sizeof(long)-FILENAME_LENGTH-sizeof(char))
#define CMD_LENGTH					256		//������������󳤶�
// ������Ϣ�ĺ궨��  
#define INVALID_MSG					-1  // ��Ч����Ϣ��ʶ
#define MSG_CMD						1	// �ͻ��˸�����ʱ������������֧�ֵ�����
#define MSG_FILENAME				2   // �ļ�������  
#define MSG_FILELENGTH				3   // �����ļ��ĳ���  
#define MSG_CLIENT_READY			4   // �ͻ���׼�������ļ�  
#define MSG_FILE					5   // �����ļ�  
#define MSG_SEND_FILE_SUCCESS		6   // �����ļ��ɹ�  
#define MSG_OPENFILE_ERROR			7   // ���ļ�ʧ��, �������ļ�·�������Ҳ����ļ���ԭ��  
#define MSG_FILE_ALREADYEXIT_ERROR	8   // Ҫ������ļ��Ѿ������� 
#define MSG_BYE						9	//�Ͽ�
#define MSG_HELLO					10	//�ͻ��˷���Hello

#pragma pack(1)
/*��Ϣͷ*/
struct tMSG_HEADER
{
	/*��Ϣ��ʶ*/
	char cMsgId;
	tMSG_HEADER(char id = INVALID_MSG) :cMsgId(id) {};
};

struct tMSG_HELLO :tMSG_HEADER
{
	tMSG_HELLO() :tMSG_HEADER(MSG_HELLO) {}
};
/*���������͵�����*/
struct tMSG_CMD :tMSG_HEADER
{
	char commad[CMD_LENGTH];
	tMSG_CMD() :tMSG_HEADER(MSG_CMD) {}
};

/*���͵����ļ���*/
struct tMSG_FILENAME :tMSG_HEADER
{
	/*�ļ���*/
	char szFileName[FILENAME_LENGTH];
	tMSG_FILENAME() :tMSG_HEADER(MSG_FILENAME) {}
};

/*���͵����ļ�����*/
struct tMSG_FILE_LENGTH :tMSG_HEADER
{
	/*�ļ�����*/
	long lLength;
	char szFileName[FILENAME_LENGTH];
	tMSG_FILE_LENGTH(long l) :tMSG_HEADER(MSG_FILELENGTH), lLength(l) {}
};

/*�ͻ���׼������*/
struct tMSG_CLIENT_READY :tMSG_HEADER
{
	long lLastPosition, lLength;
	char szFileName[FILENAME_LENGTH];
	tMSG_CLIENT_READY(long l, long len) :tMSG_HEADER(MSG_CLIENT_READY), lLastPosition(l), lLength(len) {}
};

/*���������͵����ļ�*/
struct tMSG_FILE :tMSG_HEADER
{
	union // ���� union ��֤�����ݰ��Ĵ�С������ MAX_PACKET_SIZE * sizeof(char)
	{
		char szBuff[MAX_PACKET_SIZE - sizeof(char)];///<strong>���ṹ�廹��һ��char���͵�MsgId������</strong>
		struct
		{
			long lStart;
			long lSize;
			long lFileLength;
			char szFileName[FILENAME_LENGTH];
			char szBuff[MAX_FILE_SIZE];
		}tFile;
	};
	tMSG_FILE() : tMSG_HEADER(MSG_FILE) {}
};

/*�����ļ��ɹ�*/
struct tMSG_SEND_FILE_SUCC :tMSG_HEADER
{
	char szFileName[FILENAME_LENGTH];
	tMSG_SEND_FILE_SUCC() :tMSG_HEADER(MSG_SEND_FILE_SUCCESS) {}
};

/*������Ϣ*/
struct tMSG_ERROR :tMSG_HEADER
{
	tMSG_ERROR(char ErrType) :tMSG_HEADER(ErrType) {}
};

/*�Ͽ�*/
struct tMSG_BYE :tMSG_HEADER
{
	tMSG_BYE() :tMSG_HEADER(MSG_BYE) {}
};
#pragma pack()
#endif
