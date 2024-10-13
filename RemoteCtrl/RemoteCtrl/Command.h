#pragma once
#include <map>
#include "ServerSocket.h"
#include <atlimage.h>
#include <direct.h>
#include "CTool.h"
#include <stdio.h>
#include <io.h>
#include "resource.h"
#include "LockDialog.h"
#include "Packet.h"
#include <list>
class CCommand
{
public:
	CCommand();
	~CCommand();
	int ExecuteCmd(int nCmd, std::list<CPacket>& lstPacket,CPacket& inPacket);
    static void RunCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        CCommand* thiz = (CCommand*)arg;
        if (status > 0)
        {
            int ret = thiz->ExecuteCmd(status, lstPacket, inPacket);

            if ( ret!= 0)
            {
                TRACE("ִ������ʧ��:%d ret=%d\r\n",status,ret);
            }
        }
        else
        {
            MessageBox(NULL, _T("�޷�ACCEPT���Զ�����"), _T("ACCEPT�û�ʧ�ܣ�"), MB_OK | MB_ICONERROR);
        }
    }
protected:
	typedef int (CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& inPacket);
	std::map<int, CMDFUNC> m_mapFunction;
    CLockDialog dlg;
    unsigned threadid;
protected:
    static unsigned __stdcall threadLockDlg(void* arg)
    {
        CCommand* thiz = (CCommand*)arg;
        thiz->threadLockDlgMain();
        //_endthread();
        _endthreadex(0);
        return 0;
    }

    void threadLockDlgMain()
    {
        dlg.Create(IDD_DIALOG_INFO, NULL);
        dlg.ShowWindow(SW_SHOW);
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
        dlg.MoveWindow(rect);
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC_INFO);
        if (pText)
        {
            CRect rtText;
            pText->GetWindowRect(&rtText);
            int nWdith = rtText.Width();
            int x = (rect.right - nWdith) / 2;
            int nHeight = rtText.Height();
            int y = (rect.bottom - nHeight) / 2;
            pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
        }
        //�����ö�
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

        //������귶Χ
        ShowCursor(false);
        ShowWindow(FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
        //CRect rect;
        //dlg.GetWindowRect(rect);
        rect.left = 0;
        rect.top = 0;
        rect.right = 1;
        rect.bottom = 1;
        ClipCursor(rect);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_KEYDOWN)
            {
                if (msg.wParam == 0x41)//0x41-a 0x1B-esc
                {
                    break;
                }

            }
            //else if (msg.message == WM_RBUTTONDOWN)
            //{
            //    CStatic* info = (CStatic*)dlg.GetDlgItem(IDC_STATIC_INFO);
            //    info->SetWindowTextW(_T("���"));
            //}
        }
        ClipCursor(NULL);
        ShowCursor(true);
        ShowWindow(FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
        dlg.DestroyWindow();
    }

    int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket)//1-A�� 2-B 3-C .....26-z
    {
        std::string result;
        for (int i = 1; i <= 26; i++)
        {
            if (_chdrive(i) == 0)
            {
                //if (result.size()>0)
                //{
                //    result += ",";
                //}
                //result += 'A' + i - 1;
                result += 'A' + i - 1;
                result += ",";
            }
        }
        TRACE("MakeDriverInfo() result=%s\r\n", result.c_str());
        lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }
    int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        //std::list<FILEINFO>  listFileInfos;

        if (_chdir(strPath.c_str()) != 0)
        {
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            OutputDebugString(_T("���ʲ���Ŀ¼"));
            return -2;
        }
        _finddata_t fdata;
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1)
        {
            OutputDebugString(_T("û���ļ�"));
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            return -3;
        }
        do
        {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
        } while (_findnext(hfind, &fdata) == 0);

        FILEINFO finfo;
        finfo.HasNext = FALSE;
        lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));

        return 0;
    }
    int RunFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        CPacket pack(3, NULL, 0);
        lstPacket.push_back(CPacket(3, NULL, 0));
        return 0;
    }

    int DownLoadFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        long long data = 0;
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
        if (err != 0)
        {
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            return -1;
        }
        if (pFile != NULL)
        {
            fseek(pFile, 0, SEEK_END);
            data = _ftelli64(pFile);
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            fseek(pFile, 0, SEEK_SET);

            char buffer[1024] = "";
            size_t rlen = 0;
            do
            {
                rlen = fread(buffer, 1, 1024, pFile);
                lstPacket.push_back(CPacket(4, (BYTE*)buffer, rlen));
            } while (rlen >= 1024);
            fclose(pFile);
        }
        lstPacket.push_back(CPacket(4, NULL, 0));
        return 0;
    }

    int MouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        MOUSEEV mouse;
        memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEV));

        DWORD nFlags = 0;
        switch (mouse.nButton) {
        case 0://���
            nFlags = 1;
            break;
        case 1://�Ҽ�
            nFlags = 2;
            break;
        case 2://�м�
            nFlags = 4;
            break;
        case 4://û�а���
            nFlags = 8;
            break;
        }
        if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        switch (mouse.nAction)
        {
        case 0://����
            nFlags |= 0x10;
            break;
        case 1://˫��
            nFlags |= 0x20;
            break;
        case 2://����
            nFlags |= 0x40;
            break;
        case 3://�ſ�
            nFlags |= 0x80;
            break;
        default:
            break;
        }
        TRACE("mouse event : %08X x %d y %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
        switch (nFlags)
        {
        case 0x21://���˫��
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://�������
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://�������
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://����ſ�
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://�Ҽ�˫��
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12://�Ҽ�����
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://�Ҽ�����
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://�Ҽ��ſ�
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://�м�˫��
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14://�м�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44://�м�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://�м��ſ�
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://����������ƶ�
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }

        lstPacket.push_back(CPacket(5, NULL, 0));
        
        return 0;
    }

    int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        CImage screen;
        HDC hScreen = ::GetDC(NULL);
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
        int nWidth = GetDeviceCaps(hScreen, HORZRES);
        int nHeight = GetDeviceCaps(hScreen, VERTRES);
        screen.Create(nWidth, nHeight, nBitPerPixel);
        //BitBlt(screen.GetDC(), 0, 0, 1920,1080,hScreen,0,0,SRCCOPY);
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
        TRACE("nwidth=%d nheight=%d\r\n", nWidth, nHeight);
        ReleaseDC(NULL, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
        if (hMem == NULL)return -1;
        IStream* pStream = NULL;
        int ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
        if (ret == S_OK)
        {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);
            PBYTE pData = (PBYTE)GlobalLock(hMem);
            SIZE_T nSize = GlobalSize(hMem);
            lstPacket.push_back(CPacket(6, pData, nSize));
            TRACE("SendScreen() nsize=%d\r\n", nSize);
            GlobalUnlock(hMem);
        }

        //screen.Save(_T("test2024.png"), Gdiplus::ImageFormatPNG);
        //screen.Save(_T("test2024j.jpg"), Gdiplus::ImageFormatJPEG);
        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();

        return 0;
    }
    int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE))
        {
            //_beginthread(threadLockDlg, 0, NULL);
            _beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
        }
        lstPacket.push_back(CPacket(7, NULL, 0));
        return 0;
    }

    int UnlockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        //SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 0x01E0001);���ܿ��̷߳�����Ϣ
        PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);
        lstPacket.push_back(CPacket(8, NULL, 0));
        return 0;
    }

    int DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        DeleteFileA(strPath.c_str());
        lstPacket.push_back(CPacket(9, NULL, 0));
        return 0;
    }

    int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        lstPacket.push_back(CPacket(1981, NULL, 0));
        return 0;
    }
};

