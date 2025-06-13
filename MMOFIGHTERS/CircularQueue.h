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
			int front = _Front;
			int rear = _Rear;

			if ((rear + 1) % (_Capacity + 1) == front)
			{
				return _Capacity;
			}

			if (front <= rear)
			{
				return (rear - front);
			}
			else
			{
				return (_Capacity - front + 1 + rear);
			}

		}
		inline int GetRemainingSize() const
		{
			int front = _Front;
			int rear = _Rear;
			//비어 있다면
			if (front == rear)
			{
				return _Capacity;
			}
			// 꽉찬 경우도 커버함.
			if (front < rear)
			{
				return (front)+(_Capacity - rear);
			}
			else
			{
				return (front - rear - 1);
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
			int front = _Front;
			int rear = _Rear;
			//Front가 0인 경우, R이 Capacity칸을 비워야하기 때문에.. +1을 못해줌.
			if (front == 0)
			{
				return _Capacity - rear;
			}
			if (front <= rear)
			{
				//Rear가 가리키는 값은 항상 비어있어야함.
				return _Capacity - rear + 1;
			}
			return front - rear - 1;
		}

		inline void MoveRear(int size)
		{
			_Rear = (_Rear + size) % (_Capacity + 1);
			return;
		}

		inline int GetDirect_DequeueSize() const
		{
			int front = _Front;
			int rear = _Rear;

			if (front <= rear)
			{
				return (rear - front);
			}
			return (_Capacity - front + 1);
		}

		inline void MoveFront(int size)
		{
			_Front = (_Front + size) % (_Capacity + 1);
			return;
		}

		void clear();
	public:
		int _Front;
		int _Rear;
		int _Capacity;
		enum {
			DEFAULT_SIZE = 1024,
		};
		char* _pBuffer;
	};
}