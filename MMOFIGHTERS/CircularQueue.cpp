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

//�ִ��� �־��ְ� return.
int CircularQueue::Enqueue(const char* pMessage, int size)
{
	int front = _Front;
	//��á�ٸ� return 0
	if ((_Rear + 1) % (_Capacity + 1) == front)
	{
		return 0;
	}

	//Front�� �ռ��ٸ�
	if (front <= _Rear)
	{
		// �ִ��� �������ְ� ��ȯ
		int rSize = _Capacity - _Rear + 1;
		int fSize = front - 1;

		//���� �����ִ°�� �� �ڴ� ��ĭ ��������.
		if (fSize < 0)
		{
			--rSize;
			fSize = 0;
		}

		// �ɰ����� ���� ���ϴ� ���, �ִ��� �ɰ��� �־���.
		if (size > fSize + rSize)
		{
			memcpy(_pBuffer + _Rear, pMessage, rSize);
			memcpy(_pBuffer, pMessage + rSize, fSize);
			_Rear = (_Rear + (fSize + rSize)) % (_Capacity + 1);
			return fSize + rSize;
		}

		//���� �� �ִ� ���
		//�� ������ ���� �� �ִ� ���
		if (size <= rSize)
		{
			memcpy(_pBuffer + _Rear, pMessage, size);
			_Rear = (_Rear + size) % (_Capacity + 1);
			return size;
		}

		//�ɰ��� �־�� �ϴµ�, ���� �� �ִ� ��� 
		fSize = size - rSize;
		memcpy(_pBuffer + _Rear, pMessage, rSize);
		memcpy(_pBuffer, pMessage + rSize, fSize);
		_Rear = (_Rear + (fSize + rSize)) % (_Capacity + 1);
		return fSize + rSize;
	}

	int cpySize = front - _Rear - 1;
	//�����ִ� ����� ����� ���
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
	int rear = _Rear;
	//����ִ� ���
	if (_Front == rear)
	{
		return 0;
	}

	if (_Front < rear)
	{
		int cpySize = rear - _Front;
		//��û�� ũ�⺸�� ���� �ִ°��
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
	//�ɰ����ϴ� ��� 
	cpySize = rear;
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
	int rear = _Rear;
	//����ִ� ���
	if (_Front == _Rear)
	{
		return 0;
	}

	if (_Front < rear)
	{
		int cpySize = rear - _Front;
		//��û�� ũ�⺸�� ���� �ִ°��
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
	//�ɰ����ϴ� ��� 
	cpySize = rear;
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