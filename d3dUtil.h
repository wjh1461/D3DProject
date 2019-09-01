#pragma once

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <memory>
#include <array>
#include <unordered_map>
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
public:
	
	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
	
	
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, 
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
	
	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint, const std::string& target);
		
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


/*
 기하구조 보조 구조체
 이 구조체는 MeshGeometry가 대표하는 기하구조 그룹(메시)의 부분 구간, 부분 메시를 정의한다.
 부분 메시는 하나의 정점/색인 버퍼에 여러 개의 기하구조가 들어 있는 경우에 쓰인다. 이 구조체는
 정점/색인 버퍼에 저장된 메시의 부분 메시를 그리는 데 필요한 오프셋들과 자료를 제공한다. 
*/
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// 이 부분 메시가 정의하는 기하구조의 경계 상자(bounding box).
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	// 이 메시를 이름으로 조회할 수 있도록 이름을 부여한다.
	std::string Name;

	// 시스템 메모리 복사본. 정점/색인 형식이 범용적일 수 있으므로
	// 블로브(ID3DBlob)를 사용한다.
	// 실제로 사용할 때에는 클라이언트에서 적절한 캐스팅해야 한다.
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> VPosBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> VColorBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VPosBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> VColorBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VPosBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> VColorBufferUploader = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// 버퍼들에 관한 자료.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;

	UINT VPosByteStride = 0;
	UINT VPosBufferByteSize = 0;
	UINT VColorByteStride = 0;
	UINT VColorBufferByteSize = 0;

	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// 한 MeshGeometry 인스턴스의 한 정점/색인 버퍼에 여러 개의
	// 기하구조를 담을 수 있다.
	// 부분 메시들을 개별적으로 그릴 수 있도록, 부분 메시 기하구조들을 컨테이너에 담아 둔다.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	// 연습문제 2번 추가 부분
	D3D12_VERTEX_BUFFER_VIEW VPosBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VPosBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VPosByteStride;
		vbv.SizeInBytes = VPosBufferByteSize;

		return vbv;
	}

	// 연습문제 2번 추가 부분
	D3D12_VERTEX_BUFFER_VIEW VColorBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VColorBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VColorByteStride;
		vbv.SizeInBytes = VColorBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	// 자료를 GPU에 모두 올린 후에는 메모리를 해제해도 된다.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;

		// 연습문제 2번 추가 부분
		VPosBufferUploader = nullptr;
		VColorBufferUploader = nullptr;

		IndexBufferUploader = nullptr;
	}
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

