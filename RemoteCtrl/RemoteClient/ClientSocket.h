#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <list>
#include <map>
#include <mutex>

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize,HANDLE hEvent)
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
		this->hEvent = hEvent;
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		hEvent = pack.hEvent;
	}
	CPacket(const BYTE* pData, size_t& nSize):hEvent(INVALID_HANDLE_VALUE)
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
			hEvent = pack.hEvent;
		}
		return *this;
	}

	int Size()
	{
		return nLength + 6;
	}
	const char* Data(std::string& strOut) const
	{
		strOut.resize(nLength + 6);
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
	HANDLE hEvent;

};

#pragma pack(pop)
typedef struct file_info
{
	file_info()
	{
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}

	BOOL IsInvalid;//�Ƿ���Ч
	BOOL IsDirectory;
	BOOL HasNext;//�Ƿ��к�����0û�У�1��
	char szFileName[256];

}FILEINFO, * PFILEINFO;

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

std::string GetErrInfo(int wsaErrCode);


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

	bool InitSocket();

#define BUFFER_SIZE 4096000
	int DealCommand();

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



	bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosedbool=true);

	CPacket& GetPacket()
	{
		return m_packet;
	}
	void CloseSocket()
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	void UpdateAddress(int nIP, short nPort)
	{
		if ((m_nIP != nIP) || (m_nPort == nPort))
		{
			m_nIP = nIP;
			m_nPort = nPort;
		}
	}
private:
	HANDLE m_hThread;
	std::mutex m_lock;
	bool m_bAutoClose;
	std::list<CPacket> m_listSend;
	std::map<HANDLE, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	int m_nIP;
	short m_nPort;
	std::vector<char> m_buffer;
	SOCKET m_socket;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss)
	{

	}
	CClientSocket(const CClientSocket& ss)
	{
		m_bAutoClose = ss.m_bAutoClose;
		m_socket = ss.m_socket;
		m_nIP = ss.m_nIP;
		m_nPort = ss.m_nPort;
	}
	CClientSocket() :
		m_nIP(INADDR_ANY),
		m_nPort(0),m_socket(INVALID_SOCKET),
		m_bAutoClose(true),
		m_hThread(INVALID_HANDLE_VALUE)
	{
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
		//MessageBox(NULL, _T("�ɹ���ʼ���׽��ֻ���"), _T("��ʼ���ɹ���"), MB_OK);
		
	}
	~CClientSocket()
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		WSACleanup();
		//MessageBox(NULL, _T("�ɹ�����"), _T("CServerSocket��������"), MB_OK);
	}

	static void threadEntry(void* arg);
	void threadFunc();

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

	bool SendData(const char* pData, int nSzie)
	{
		if (m_socket == -1)
		{
			return false;
		}
		return send(m_socket, pData, nSzie, 0) > 0;
	}

	bool SendData(const CPacket& pack);

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


