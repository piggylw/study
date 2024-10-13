#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include "Packet.h"


typedef void(*SOCKET_CALLBACK)(void* ,int ,std::list<CPacket>&,CPacket&);

class CServerSocket
{
public:
	static CServerSocket* getInstance()
	{
		if (m_instance == NULL)//静态函数没有this指针，不能访问成员变量
		{
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket(short port)
	{
		
		//校验
		if (m_socket == -1)return false;

		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.S_un.S_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);
		//绑定
		if (bind(m_socket, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		{
			return false;
		}
		//监听
		if (listen(m_socket, 1) == -1)
		{
			return false;
		}

		return true;
	}

	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9999)
	{
		bool ret = InitSocket(port);
		if (ret == false)return -1;
		std::list<CPacket> lstPackets;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true)
		{
			if (AcceptClient()== false)
			{
				if (count >= 3)
				{
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			if (ret > 0)
			{
				m_callback(m_arg, ret,lstPackets,m_packet);
				while(lstPackets.size()>0)
				{
					SendData(lstPackets.front());
					lstPackets.pop_front();
				}
			}
			CloseClient();
		}

		return 0;
	}

	bool AcceptClient()
	{
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		TRACE("----------------ready accept------------\r\n");
		m_client = accept(m_socket, (sockaddr*)&client_adr, &cli_sz);
		TRACE("----------------m_client=%d\r\n", m_client);
		if (m_client == -1)return false;
		return true;
	}
#define BUFFER_SIZE 4096
	int DealCommand()
	{
		if (m_client==-1)
		{
			return -1;
		}
		char* buffer = new char[BUFFER_SIZE]; //delete内存泄漏
		if (buffer == NULL)
		{
			TRACE("内存不足\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer+index, BUFFER_SIZE -index, 0);
			if (len<=0)
			{
				delete[] buffer;
				return -1;
			}
			TRACE("recv %d\r\n", len);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer,len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[] buffer;
				return m_packet.sCmd;
				//return 0;
			}
		}
		delete[] buffer;
		return -1;
	}

	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd == 2)||(m_packet.sCmd == 3) || (m_packet.sCmd == 4) || (m_packet.sCmd == 9))
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
		if (m_client == -1)
		{
			return false;
		}
		return send(m_client, pData, nSzie, 0) > 0;
	}

	bool SendData(CPacket& pack)
	{
		if (m_client == -1)
		{
			return false;
		}
		//此处不加slepp丢包
		//Sleep(1);

		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}
	CPacket& GetPacket()
	{
		return m_packet;
	}

	void CloseClient()
	{
		if (m_client!=INVALID_SOCKET)
		{
			closesocket(m_client);
			m_client = INVALID_SOCKET;//-1
		}
	}
private:
	void* m_arg;
	SOCKET_CALLBACK m_callback;
	SOCKET m_socket;
	SOCKET m_client;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss)
	{

	}
	CServerSocket(const CServerSocket& ss)
	{
		m_client = ss.m_client;
		m_socket = ss.m_socket;
	}
	CServerSocket() 
	{
		m_client = INVALID_SOCKET;//-1
		if (InitSockEnv()==FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字环境"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_socket = socket(PF_INET, SOCK_STREAM, 0);
		//MessageBox(NULL, _T("成功初始化套接字环境"), _T("初始化成功！"), MB_OK);
	}
	~CServerSocket()
	{
		closesocket(m_socket);
		WSACleanup();
		//MessageBox(NULL, _T("成功析构"), _T("CServerSocket析构函数"), MB_OK);
	}
	BOOL InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data)!=0)
		{
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance()
	{
		if (m_instance!=NULL)
		{
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CServerSocket* m_instance;

	class CHelper {
	public:
		CHelper()
		{
			CServerSocket::getInstance();
		}
		~CHelper()
		{
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};


