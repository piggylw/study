#pragma once
#include "pch.h"
#include "framework.h"

class CServerSocket
{
public:
	static CServerSocket* getInstance()
	{
		if (m_instance == NULL)//��̬����û��thisָ�룬���ܷ��ʳ�Ա����
		{
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket()
	{
		
		//У��
		if (m_socket == -1)return false;

		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.S_un.S_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9999);
		//��
		if (bind(m_socket, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		{
			return false;
		}
		//����
		if (listen(m_socket, 1) == -1)
		{
			return false;
		}
		return true;
	}
	bool AcceptClient()
	{
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_socket, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1)return false;
		return true;
	}

	int DealCommand()
	{
		if (m_client==-1)
		{
			return -1;
		}
		char buffer[1024]="";
		while (true)
		{
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len<=0)
			{
				return -1;
			}
			//TODO
		}
	}

	bool SendData(const char* pData, int nSzie)
	{
		if (m_client == -1)
		{
			return false;
		}
		return send(m_client, pData, nSzie, 0) > 0;
	}

private:
	SOCKET m_socket;
	SOCKET m_client;
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
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_socket = socket(PF_INET, SOCK_STREAM, 0);
		//MessageBox(NULL, _T("�ɹ���ʼ���׽��ֻ���"), _T("��ʼ���ɹ���"), MB_OK);
	}
	~CServerSocket()
	{
		closesocket(m_socket);
		WSACleanup();
		//MessageBox(NULL, _T("�ɹ�����"), _T("CServerSocket��������"), MB_OK);
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


