﻿#pragma once
#include "d3dUtil.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : mIsConstantBuffer(isConstantBuffer)
	{
		mElementByteSize = sizeof(T);

		/*
		 상수 버퍼 원소의 크기는 반드시 256바이트의 배수이어야 한다.
		 이는 하드웨어가 m * 256바이트 오프셋에서 시작하는 n * 256바이트 길이의 상수 자료만 볼 수 있기 때문이다.
		 typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
		 UINT64 OffsetInBytes; // multiple of 256
		 UINT   SizeInBytes;   // multiple of 256
		 } D3D12_CONSTANT_BUFFER_VIEW_DESC;
		 */
		if (isConstantBuffer)
		{
			mElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));
		}

		ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mUploadBuffer)));

		ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));

		/*
		 자원을 다 사용하기 전에는 대응을 해제할 필요가 없다.
		 그러나, 자원을 GPU가 사용하는 중에는 CPU에서 자원을 갱신하지 않아야 한다.(따라서 반드시 동기화 기법을 사용해야 한다).
		*/
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

	~UploadBuffer()
	{
		if (mUploadBuffer != nullptr)
		{
			mUploadBuffer->Unmap(0, nullptr);
		}

		mMappedData = nullptr;
	}

	ID3D12Resource* Resource() const
	{
		return mUploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
	BYTE* mMappedData = nullptr;

	UINT mElementByteSize = 0;
	bool mIsConstantBuffer = false;
};
