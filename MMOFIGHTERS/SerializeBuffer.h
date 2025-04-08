#pragma once

#include <string.h>
#include "NetDefine.h"

namespace Common
{
	class SerializeBuffer
	{
	public:
		SerializeBuffer()
			: _Failbit(false),
			_WritePos(0),
			_ReadPos(0),
			_Capacity(DEFAULT_CAPACITY)
		{
			_pBuffer = new char[DEFAULT_CAPACITY + 1];
		}
		SerializeBuffer(int capcity)
			:_Failbit(false),
			_WritePos(0),
			_ReadPos(0),
			_Capacity(capcity)
		{
			_pBuffer = new char[capcity + 1];
		}
		~SerializeBuffer()
		{
			delete[] _pBuffer;
		}

		inline int getData(char* dst, size_t size)
		{
			int cpySize = size;
			//요청된 사이즈가 갖고있는 사이즈보다 크다.
			//dst는 size보다 크다고 가정한다.
			if (size > _WritePos - _ReadPos)
			{
				cpySize = _WritePos - _ReadPos;
			}
			memcpy(dst, _pBuffer + _ReadPos, cpySize);
			_ReadPos += cpySize;
			return cpySize;
		}
		inline int setData(char* src, size_t size)
		{
			int cpySize = size;
			if ((_Capacity - _WritePos) < size)
			{
				cpySize = _Capacity - _WritePos;
			}
			memcpy(_pBuffer + _WritePos, src, cpySize);
			_WritePos += cpySize;
			return cpySize;
		}
		inline int getCapacity() const
		{
			return _Capacity;
		}
		inline int getUsedSize() const
		{
			return (_WritePos - _ReadPos);
		}
		inline bool checkFailBit() const
		{
			return _Failbit;
		}
		inline void resetFailBit()
		{
			_Failbit = false;
			return;
		}
		
		inline char* getBufferPtr()
		{
			return (_pBuffer + _ReadPos);
		}
		
		inline int moveWritePos(size_t size)
		{
			if (_Capacity < size + _WritePos)
			{
				return -1;
			}
			_WritePos += size;
			return size;
		}
		inline int moveReadPos(size_t size)
		{
			if (_Capacity < _ReadPos + size)
			{
				return -1;
			}
			_ReadPos += size;
			return size;
		}
		inline void clear()
		{
			_Failbit = false;
			_WritePos = 0;
			_ReadPos = 0;
		}
		
		inline void reserveHeader()
		{
			_WritePos += sizeof(NetLib::NETWORK_HEADER);
			return;
		}

		//PayLoadLen을 변경하는코드
		inline void setPayLoadLen(char payload_len)
		{
			char* curRead = _pBuffer + _ReadPos;
			*curRead++ = NetLib::SIGNITURE;
			*curRead = payload_len;
		}

		inline SerializeBuffer& operator>> (unsigned char& value)
		{
			//비어있는 경우
			if (_ReadPos == _WritePos)
			{
				_Failbit = true;
				return *this;
			}
			value = _pBuffer[_ReadPos];
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (bool& value)
		{
			if (_ReadPos == _WritePos)
			{
				_Failbit = true;
				return *this;
			}
			value = _pBuffer[_ReadPos];
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (char& value)
		{
			if (_ReadPos == _WritePos)
			{
				_Failbit = true;
				return *this;
			}
			value = _pBuffer[_ReadPos];
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (short& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<short*>(_pBuffer + _ReadPos);
			//memcpy(&value, _pBuffer + _ReadPos, sizeof(value));
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (unsigned short& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<unsigned short*>(_pBuffer + _ReadPos);
			//memcpy(&value, _pBuffer + _ReadPos, sizeof(value));
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (int& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<int*>(_pBuffer + _ReadPos);
			//memcpy(&value, _pBuffer + _ReadPos, sizeof(value));
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (unsigned int& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<unsigned int*>(_pBuffer + _ReadPos);
			//memcpy(&value, _pBuffer + _ReadPos, sizeof(value));
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (long& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<long*>(_pBuffer + _ReadPos);
			//memcpy(&value, _pBuffer + _ReadPos, sizeof(value));
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (unsigned long& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<unsigned long*>(_pBuffer + _ReadPos);
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (float& fvalue)
		{
			if (_WritePos - _ReadPos < sizeof(fvalue))
			{
				_Failbit = true;
				return *this;
			}
			fvalue = *reinterpret_cast<float*>(_pBuffer + _ReadPos);
			_ReadPos += sizeof(fvalue);
			return *this;
		}
		inline SerializeBuffer& operator>> (double& dvalue)
		{
			if (_WritePos - _ReadPos < sizeof(dvalue))
			{
				_Failbit = true;
				return *this;
			}
			dvalue = *reinterpret_cast<double*>(_pBuffer + _ReadPos);
			//memcpy(&dvalue, _pBuffer + _ReadPos, sizeof(dvalue));
			_ReadPos += sizeof(dvalue);
			return *this;
		}
		inline SerializeBuffer& operator>> (long long& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<long long*>(_pBuffer + _ReadPos);
			_ReadPos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator>> (unsigned long long& value)
		{
			if (_WritePos - _ReadPos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			value = *reinterpret_cast<unsigned long long*>(_pBuffer + _ReadPos);
			_ReadPos += sizeof(value);
			return *this;
		}

		inline SerializeBuffer& operator<< (const unsigned char value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			_pBuffer[_WritePos] = value;
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const char value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			_pBuffer[_WritePos] = value;
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const bool value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			_pBuffer[_WritePos] = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;

		}
		inline SerializeBuffer& operator<< (const short value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<short*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const unsigned short value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<unsigned short*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const int value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<int*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const unsigned int value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<unsigned int*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const long value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<long*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const unsigned long value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<unsigned long*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const float fvalue)
		{
			if (_Capacity - _WritePos < sizeof(fvalue))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<float*>(_pBuffer + _WritePos) = fvalue;
			//memcpy(_pBuffer + _WritePos, &fvalue, sizeof(fvalue));
			_WritePos += sizeof(fvalue);
			return *this;
		}
		inline SerializeBuffer& operator<< (const double dvalue)
		{
			if (_Capacity - _WritePos < sizeof(dvalue))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<double*>(_pBuffer + _WritePos) = dvalue;
			//memcpy(_pBuffer + _WritePos, &dvalue, sizeof(dvalue));
			_WritePos += sizeof(dvalue);
			return *this;
		}
		inline SerializeBuffer& operator<< (const long long value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<long long*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
		inline SerializeBuffer& operator<< (const unsigned long long value)
		{
			if (_Capacity - _WritePos < sizeof(value))
			{
				_Failbit = true;
				return *this;
			}
			*reinterpret_cast<unsigned long long*>(_pBuffer + _WritePos) = value;
			//memcpy(_pBuffer + _WritePos, &value, sizeof(value));
			_WritePos += sizeof(value);
			return *this;
		}
	private:
		bool _Failbit;
		int _WritePos;
		int _ReadPos;
		int _Capacity;

		enum
		{
			DEFAULT_CAPACITY = 1400,
		};

		char* _pBuffer;
	};
}
