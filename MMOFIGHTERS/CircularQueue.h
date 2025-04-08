#pragma once

namespace Common
{
	class CircularQueue
	{
	public:
		CircularQueue();
		CircularQueue(int size);
		~CircularQueue()
		{
			delete[] _pBuffer;
		}

		inline int GetCurrentSize() const
		{
			if ((_Rear + 1) % (_Capacity + 1) == _Front)
			{
				return _Capacity;
			}

			if (_Front <= _Rear)
			{
				return (_Rear - _Front);
			}
			else
			{
				return (_Capacity - _Front + 1 + _Rear);
			}

		}
		inline int GetRemainingSize() const
		{
			//비어 있다면
			if (_Front == _Rear)
			{
				return _Capacity;
			}
			// 꽉찬 경우도 커버함.
			if (_Front < _Rear)
			{
				return (_Front)+(_Capacity - _Rear);
			}
			else
			{
				return (_Front - _Rear - 1);
			}
		}

		int Enqueue(const char* pMessage, int size);
		int Dequeue(char* out, int size);
		int Peek(char* out, int size) const;

		inline char* GetBufferPtr()
		{
			return _pBuffer;
		}

		inline char* GetRearPtr()
		{
			return &_pBuffer[_Rear];
		}
		inline char* GetFrontPtr()
		{
			return &_pBuffer[_Front];
		}

		inline int GetDirect_EnqueueSize() const
		{
			//Front가 0인 경우, R이 Capacity칸을 비워야하기 때문에.. +1을 못해줌.
			if (_Front == 0)
			{
				return _Capacity - _Rear;
			}
			if (_Front <= _Rear)
			{
				//Rear가 가리키는 값은 항상 비어있어야함.
				return _Capacity - _Rear + 1;
			}
			return _Front - _Rear - 1;
		}
		inline void MoveRear(int size)
		{
			_Rear = (_Rear + size) % (_Capacity + 1);
			return;
		}

		inline int GetDirect_DequeueSize() const
		{
			if (_Front <= _Rear)
			{
				return (_Rear - _Front);
			}
			return (_Capacity - _Front + 1);
		}
		inline void MoveFront(int size)
		{
			_Front = (_Front + size) % (_Capacity + 1);
			return;
		}

		void clear();
	private:
		int _Front;
		int _Rear;
		int _Capacity;
		enum {
			DEFAULT_SIZE = 1024,
		};
		char* _pBuffer;
	};
}