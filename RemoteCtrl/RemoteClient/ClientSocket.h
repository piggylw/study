#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
	{
		sHead = 0xFEFF;
		nLength = 4 + nSize;
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize)
	{
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) //���ݲ�����
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);i += 4;
		if (nLength + i > nSize)//��û����ȫ���յ�
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 2 - 2);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);i += 2;

		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i;//��Ϊ�����в��õ����ݣ�������i����������
			return;
		}
		nSize = 0;
	}
	~CPacket() {};

	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size()
	{
		return nLength + 6;
	}
	const char* Data()
	{
		strOut.resize(Size());
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;pData += 2;
		*(DWORD*)pData = nLength;pData += 4;
		*(WORD*)pData = sCmd;pData += 2;
		memcpy(pData, strData.c_str(), strData.size());pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;//��ͷ���̶�0xFEFF
	DWORD nLength;//�ӿ��������У��ĳ���
	WORD sCmd;//��������
	std::string strData;//����
	WORD sSum;//�ͼ���

	std::string strOut;//������������
};

#pragma pack(pop)

typedef struct MouseEvent {

	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//������ƶ���˫��
	WORD nButton;//������Ҽ����н�
	POINT ptXY;
}MOUSEEV, * PMOUSEEV;

std::string GetErrorInfo(int wsaErrCode)
{
	std::string ret;
	LPVOID lpMsgbuf = NULL;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgbuf,0,NULL
		);
	ret = (char*)lpMsgbuf;
	LocalFree(lpMsgbuf);
	return ret;
}

class CClientSocket
{
public:
	static CClientSocket* getInstance()
	{
		if (m_instance == NULL)//��̬����û��thisָ�룬���ܷ��ʳ�Ա����
		{
			m_instance = new CClientSocket();
		}
		return m_instance;
	}

	bool InitSocket(const std::string& strIPAddress)
	{
		//У��
		if (m_socket == -1)return false;

		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		//serv_adr.sin_addr.S_un.S_addr = inet_pton(strIPAddress.c_str());
		inet_pton(AF_INET,strIPAddress.c_str(),(void*)serv_adr.sin_addr.S_un.S_addr);

		serv_adr.sin_port = htons(9999);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			AfxMessageBox("IP������");
			return false;
		}

		int ret = connect(m_socket,(sockaddr*)&serv_adr,sizeof(serv_adr));
		if (ret == -1)
		{
			AfxMessageBox("����ʧ��");
			TRACE("����ʧ��:%d %s\r\n",WSAGetLastError(),GetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096
	int DealCommand()
	{
		if (m_socket == -1)
		{
			return -1;
		}
		char* buffer = new char[BUFFER_SIZE]; //delete
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_socket, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd == 2) || (m_packet.sCmd == 3) || (m_packet.sCmd == 4))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	bool SendData(const char* pData, int nSzie)
	{
		if (m_socket == -1)
		{
			return false;
		}
		return send(m_socket, pData, nSzie, 0) > 0;
	}

	bool SendData(CPacket& pack)
	{
		if (m_socket == -1)
		{
			return false;
		}
		return send(m_socket, pack.Data(), pack.Size(), 0) > 0;
	}

private:
	SOCKET m_socket;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss)
	{

	}
	CClientSocket(const CClientSocket& ss)
	{
		m_socket = ss.m_socket;
	}
	CClientSocket()
	{
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_socket = socket(PF_INET, SOCK_STREAM, 0);
		//MessageBox(NULL, _T("�ɹ���ʼ���׽��ֻ���"), _T("��ʼ���ɹ���"), MB_OK);
	}
	~CClientSocket()
	{
		closesocket(m_socket);
		WSACleanup();
		//MessageBox(NULL, _T("�ɹ�����"), _T("CServerSocket��������"), MB_OK);
	}
	BOOL InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CClientSocket* m_instance;

	class CHelper {
	public:
		CHelper()
		{
			CClientSocket::getInstance();
		}
		~CHelper()
		{
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};


