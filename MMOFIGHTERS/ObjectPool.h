#pragma once
#include <utility>
#include <new>

namespace Common
{
	template<typename T, size_t BucketCount, bool usePlacementNew>
	class ObjectPool
	{
	};
	//objectpool 
	template<typename T, size_t BucketCount>
	class ObjectPool<T, BucketCount, false>
	{
	public:
		typedef char _BYTE;
		ObjectPool()
			:_CurBucket(nullptr)
			, _CurSlot(nullptr)
			, _LastSlot(nullptr)
			, _FreeList(nullptr)
		{
			//warmming Up Pool
			allocateBucket_with_default_constructor();
		}
		~ObjectPool()
		{
			Slot* cur = _CurBucket;
			while (cur != nullptr)
			{
				Slot* prev = cur->_pNext;
				operator delete(reinterpret_cast<void*>(cur));
				cur = prev;
			}
		}
		inline bool empty() const
		{
			return (_FreeList == nullptr);
		}
		T* allocate()
		{
			T* ret;
			if (_FreeList != nullptr)
			{
				ret = reinterpret_cast<T*>(&_FreeList->_Data);
				_FreeList = _FreeList->_pNext;
				return ret;
			}

			//다 소모함 
			if (_CurSlot > _LastSlot)
			{
				//allocateBucket();
				allocateBucket_with_default_constructor();
			}

			ret = reinterpret_cast<T*>(&_CurSlot->_Data);
			++_CurSlot;
			return ret;
		}
		template<class... Args>
		T* allocate(Args&&... args)
		{
			T* ret;
			if (_FreeList != nullptr)
			{
				ret = reinterpret_cast<T*>(&_FreeList->_Data);
				_FreeList = _FreeList->_pNext;
				return ret;
			}

			//다 소모함 
			if (_CurSlot > _LastSlot)
			{
				//allocateBucket();
				allocateBucket_with_constructor(std::forward<Args>(args)...);
			}
			ret = reinterpret_cast<T*>(&_CurSlot->_Data);
			_CurSlot++;
			return ret;
		}
		void deAllocate(T* addr)
		{
			if (addr != nullptr)
			{
				Slot* retSlot = reinterpret_cast<Slot*>(reinterpret_cast<_BYTE*>(addr) - sizeof(Slot*));
				retSlot->_pNext = _FreeList;
				_FreeList = retSlot;
				return;
			}
		}
	
	private:
		struct Slot
		{
			struct Slot* _pNext;
			T _Data;
		};
		void allocateBucket_with_default_constructor()
		{
			Slot* newBucket = reinterpret_cast<Slot*>(operator new(sizeof(Slot) * (BucketCount + 1)));

			newBucket->_pNext = _CurBucket;

			_CurBucket = newBucket;
			_CurSlot = _CurBucket + 1;
			_LastSlot = newBucket + (BucketCount);

			char* tmp = reinterpret_cast<char*>(_CurSlot) + sizeof(Slot*);
			size_t padding = (reinterpret_cast<char*>(_CurSlot) + sizeof(Slot)) - (tmp + sizeof(T));
			size_t offset = sizeof(T) + sizeof(Slot*) + padding;
			Slot* BucketEnd = (_LastSlot + 1);

			while (reinterpret_cast<Slot*>(tmp) < BucketEnd)
			{
				new (static_cast<void*>(tmp)) T();
				tmp += offset;
			}
		}
		template<class... Args>
		void allocateBucket_with_constructor(Args&&... args)
		{
			Slot* newBucket = reinterpret_cast<Slot*>(operator new(sizeof(Slot) * (BucketCount + 1)));
			//맨 앞을 더미로 쓸꺼임.

			newBucket->_pNext = _CurBucket;

			_CurBucket = newBucket;
			_CurSlot = _CurBucket + 1;
			_LastSlot = newBucket + (BucketCount);

			char* tmp = reinterpret_cast<char*>(_CurSlot) + sizeof(Slot*);
			size_t padding = (reinterpret_cast<char*>(_CurSlot) + sizeof(Slot)) - (tmp + sizeof(T));
			size_t offset = sizeof(T) + sizeof(Slot*) + padding;
			Slot* BucketEnd = (_LastSlot + 1);

			while (reinterpret_cast<Slot*>(tmp) < _LastSlot)
			{
				new (static_cast<void*>(tmp)) T(std::forward<Args>(args)...);
				tmp = tmp + offset;
			}
		}

		Slot* _CurBucket;
		Slot* _CurSlot;
		Slot* _LastSlot;
		Slot* _FreeList;
	};

	template<typename T, size_t BucketCount>
	class ObjectPool<T, BucketCount, true>
	{
	public:
		typedef char _BYTE;
		ObjectPool()
			:_CurBucket(nullptr)
			, _CurSlot(nullptr)
			, _LastSlot(nullptr)
			, _FreeList(nullptr)
		{
			//warmming Up Pool
			allocateBucket();
		}
		~ObjectPool()
		{
			Slot* cur = _CurBucket;
			while (cur != nullptr)
			{
				Slot* prev = cur->_pNext;
				operator delete(reinterpret_cast<void*>(cur));
				cur = prev;
			}
		}

		inline bool empty() const
		{
			return (_FreeList == nullptr);
		}
		template<class... Args>
		T* allocate_PlacementNew(Args&&... args)
		{
			T* element;
			if (_FreeList != nullptr)
			{
				element = &_FreeList->_Data;
				_FreeList = _FreeList->_pNext;
				new (static_cast<void*>(element)) T(std::forward<Args>(args)...);
				return element;
			}

			if (_CurSlot >= _LastSlot)
			{
				allocateBucket();
			}
			element = reinterpret_cast<T*>(&_CurSlot->_Data);
			new (static_cast<void*>(element)) T(std::forward<Args>(args)...);
			return element;
		}
		void deallocate_Destructor(T* pMemory)
		{
			pMemory->~T();
			deAllocate(pMemory);
		}
		void deAllocate(T* addr)
		{
			if (addr != nullptr)
			{
				Slot* retSlot = reinterpret_cast<Slot*>(reinterpret_cast<_BYTE*>(addr) - sizeof(Slot*));
				retSlot->_pNext = _FreeList;
				_FreeList = retSlot;
				return;
			}
		}
	private:
		struct Slot
		{
			struct Slot* _pNext;
			T _Data;
		};
		void allocateBucket()
		{
			Slot* newBucket = reinterpret_cast<Slot*>(operator new(sizeof(Slot) * (BucketCount + 1)));
			newBucket->_pNext = _CurBucket;
			_CurBucket = newBucket;
			_CurSlot = _CurBucket + 1;
			_LastSlot = newBucket + (BucketCount + 1);
		}

		Slot* _CurBucket;
		Slot* _CurSlot;
		Slot* _LastSlot;
		Slot* _FreeList;
	};
}