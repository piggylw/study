#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

std::string GetErrInfo(int wsaErrCode)
{
	std::string ret;
	LPVOID lpMsgbuf = NULL;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgbuf, 0, NULL
	);
	ret = (char*)lpMsgbuf;
	LocalFree(lpMsgbuf);
	return ret;
}

bool CClientSocket::InitSocket()
{
	//校验
	if (m_socket != INVALID_SOCKET)CloseSocket();
	m_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (m_socket == -1)return false;

	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	//serv_adr.sin_addr.S_un.S_addr = inet_pton(strIPAddress.c_str());
	//inet_pton(AF_INET,strIPAddress.c_str(),(void*)&serv_adr.sin_addr);

	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("IP不存在");
		return false;
	}

	int ret = connect(m_socket, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1)
	{
		//AfxMessageBox("连接失败");
		TRACE("连接失败:%d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
		return false;
	}
	return true;
}

int CClientSocket::DealCommand()
{
	if (m_socket == -1)
	{
		return -1;
	}
	char* buffer = m_buffer.data();
	static size_t index = 0;
	while (true)
	{
		size_t len = recv(m_socket, buffer + index, BUFFER_SIZE - index, 0);
		if (((int)len <= 0) && ((int)index <= 0))
		{
			return -1;
		}
		index += len;
		len = index;
		m_packet = CPacket((BYTE*)buffer, len);
		if (len > 0)
		{
			memmove(buffer, buffer + len, index - len);
			index -= len;
			return m_packet.sCmd;
		}
	}
	return -1;
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
{
	if (m_socket == INVALID_SOCKET && m_hThread == INVALID_HANDLE_VALUE)
	{
		//if (InitSocket()==false)
		//{
		//	return false;
		//}
		m_hThread = (HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
	}
	m_lock.lock();
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
	m_mapAutoClosed.insert(std::pair<HANDLE,bool>(pack.hEvent,isAutoClosed));
	m_listSend.push_back(pack);
	m_lock.unlock();
	WaitForSingleObject(pack.hEvent, INFINITE);
	std::map<HANDLE, std::list<CPacket>&>::iterator it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end())
	{
		m_lock.lock();
		m_mapAck.erase(it);
		m_lock.unlock();
		return true;
	}
	return false;
}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	_endthread();
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	InitSocket();
	while (m_socket!=INVALID_SOCKET)
	{
		if (m_listSend.size() > 0)
		{
			TRACE("m_listSend.size()=%d\r\n", m_listSend.size());
			m_lock.lock();
			CPacket& head = m_listSend.front();
			m_lock.unlock();
			if (SendData(head) == false)
			{
				TRACE("发送失败\r\n");

				continue;
			}
			std::map<HANDLE, std::list<CPacket>&>::iterator it;
			it = m_mapAck.find(head.hEvent);
			std::map<HANDLE, bool>::iterator it0 = m_mapAutoClosed.find(head.hEvent);
			do
			{
				int len = recv(m_socket, pBuffer + index, BUFFER_SIZE - index, 0);
				if (len > 0 || index > 0)
				{
					index += len;
					size_t size = (size_t)index;
					CPacket pack((BYTE*)pBuffer, size);

					if (size > 0)//解出一个包了
					{
						pack.hEvent = head.hEvent;
						m_lock.lock();
						it->second.push_back(pack);
						m_lock.unlock();
						memmove(pBuffer, pBuffer + size, index - size);
						index -= size;
						if (it0->second==true)
						{
							SetEvent(head.hEvent);
							break;
						}
					}
				}
				else if (len <= 0 && index <= 0)
				{
					CloseSocket();
					SetEvent(head.hEvent);
					break;
				}
			} while (it0->second == false);
			m_lock.lock();
			m_mapAutoClosed.erase(it0);
			m_listSend.pop_front();
			m_lock.unlock();
			InitSocket();
		}
		Sleep(1);
		
	}
	CloseSocket();
}

bool CClientSocket::SendData(const CPacket& pack)
{
	TRACE("m_socket =%d\r\n", m_socket);
	if (m_socket == -1)
	{
		return false;
	}
	std::string strOut;
	pack.Data(strOut);
	TRACE("SendData(CPacket& pack) pack.size = %d\r\n", strOut.size());
	return send(m_socket, strOut.c_str(), strOut.size(), 0) > 0;
}
