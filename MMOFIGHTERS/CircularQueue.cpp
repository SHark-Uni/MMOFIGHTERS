#include <string.h>

#include "CircularQueue.h"
using namespace Common;

CircularQueue::CircularQueue()
	:_Front(0)
	, _Rear(0)
	, _Capacity(DEFAULT_SIZE)
{
	_pBuffer = new char[DEFAULT_SIZE + 1];
}

CircularQueue::CircularQueue(int size)
	:_Front(0)
	, _Rear(0)
	, _Capacity(size)
{

	_pBuffer = new char[size + 1];

}

//최대한 넣어주고 return.
int CircularQueue::Enqueue(const char* pMessage, int size)
{
	//꽉찼다면 return 0
	if ((_Rear + 1) % (_Capacity + 1) == _Front)
	{
		return 0;
	}

	//Front가 앞선다면
	if (_Front <= _Rear)
	{
		// 최대한 복사해주고 반환
		int rSize = _Capacity - _Rear + 1;
		int fSize = _Front - 1;

		//앞이 꽉차있는경우 맨 뒤는 한칸 비워줘야함.
		if (fSize < 0)
		{
			--rSize;
			fSize = 0;
		}

		// 쪼개서라도 넣지 못하는 경우, 최대한 쪼개서 넣어줌.
		if (size > fSize + rSize)
		{
			memcpy(_pBuffer + _Rear, pMessage, rSize);
			memcpy(_pBuffer, pMessage + rSize, fSize);
			return fSize + rSize;
		}

		//넣을 수 있는 경우
		//뒷 공간에 넣을 수 있는 경우
		if (size <= rSize)
		{
			memcpy(_pBuffer + _Rear, pMessage, size);
			_Rear = (_Rear + size) % (_Capacity + 1);
			return size;
		}

		//쪼개서 넣어야 하는데, 넣을 수 있는 경우 
		fSize = size - rSize;
		memcpy(_pBuffer + _Rear, pMessage, rSize);
		memcpy(_pBuffer, pMessage + rSize, fSize);
		_Rear = (_Rear + (fSize + rSize)) % (_Capacity + 1);
		return fSize + rSize;
	}

	int cpySize = _Front - _Rear - 1;
	//남이있는 사이즈가 충분한 경우
	if (size < cpySize)
	{
		cpySize = size;
	}
	memcpy(_pBuffer + _Rear, pMessage, cpySize);
	_Rear += cpySize;
	return cpySize;
}

int CircularQueue::Dequeue(char* out, int size)
{
	//비어있는 경우
	if (_Front == _Rear)
	{
		return 0;
	}

	if (_Front < _Rear)
	{
		int cpySize = _Rear - _Front;
		//요청한 크기보다 많이 있는경우
		if (size < cpySize)
		{
			cpySize = size;
		}
		memcpy(out, _pBuffer + _Front, cpySize);
		_Front += cpySize;
		return cpySize;
	}

	int fSize = _Capacity - _Front + 1;
	int cpySize;

	if (size < fSize)
	{
		memcpy(out, _pBuffer + _Front, size);
		_Front = (_Front + size) % (_Capacity + 1);
		return size;
	}
	//쪼개야하는 경우 
	cpySize = _Rear;
	if (size - fSize < cpySize)
	{
		cpySize = size - fSize;
	}
	memcpy(out, _pBuffer + _Front, fSize);
	memcpy(out + fSize, _pBuffer, cpySize);

	_Front = (_Front + fSize + cpySize) % (_Capacity + 1);
	return fSize + cpySize;
}

int CircularQueue::Peek(char* out, int size) const
{
	//비어있는 경우
	if (_Front == _Rear)
	{
		return 0;
	}

	if (_Front < _Rear)
	{
		int cpySize = _Rear - _Front;
		//요청한 크기보다 많이 있는경우
		if (size < cpySize)
		{
			cpySize = size;
		}
		memcpy(out, _pBuffer + _Front, cpySize);
		return cpySize;
	}

	int fSize = _Capacity - _Front + 1;
	int cpySize;

	if (size < fSize)
	{
		memcpy(out, _pBuffer + _Front, size);
		return size;
	}
	//쪼개야하는 경우 
	cpySize = _Rear;
	if (size - fSize < cpySize)
	{
		cpySize = size - fSize;
	}
	memcpy(out, _pBuffer + _Front, fSize);
	memcpy(out + fSize, _pBuffer, cpySize);
	return fSize + cpySize;
}

void CircularQueue::clear()
{
	_Rear = _Front = 0;
}
