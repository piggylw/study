#pragma once
#include <string>
#include <atlimage.h>
class CTool
{
public:
	static int Bytes2Image(CImage& image, const std::string& strBffer)
	{

		BYTE* pData = (BYTE*)strBffer.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL)
		{
			TRACE("HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0)\r\n");
			Sleep(1);
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMem, true, &pStream);
		if (hRet == S_OK)
		{
			ULONG length = 0;
			pStream->Write(pData, strBffer.size(),&length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			if ((HBITMAP)image != NULL)
				image.Destroy();
			image.Load(pStream);
		}
		pStream->Release();
		GlobalFree(hMem);
		return hRet;
	}
};

