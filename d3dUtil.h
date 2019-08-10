#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <string>
#include <cassert>
#include "d3dx12.h"

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}


class d3dUtil
{
};

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};



#ifndef ThrowIfFailed
#define ThrowIfFailed(x)	\
{	\
	HRESULT hr__ = (x);	\
	std::wstring wfn = AnsiToWString(__FILE__);	\
	if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); }	\
}	
#endif // !ThrowIfFaild

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

