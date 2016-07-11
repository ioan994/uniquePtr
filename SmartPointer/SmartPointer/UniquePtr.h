#pragma once

template<class T>
struct DefaultDeleter
{
   template<class TOther>
   DefaultDeleter(DefaultDeleter<TOther>&&){}
   DefaultDeleter() = default;

	void operator()(T* i_ptr) const
	{
		delete i_ptr;
	}
};

template <class T, class D = DefaultDeleter<T>>
class UniquePtr
{
public:

	explicit UniquePtr(T* i_pointer = nullptr) : m_pointer(i_pointer)
	{
	}

	UniquePtr(T* i_pointer, const D& i_deleter) : m_pointer(i_pointer), m_deleter(i_deleter)
	{
	}

   template <class TOther>
   UniquePtr(UniquePtr<TOther>&& i_uniquePtrOther) :
		m_pointer(i_uniquePtrOther.Release()),
      m_deleter(std::move(i_uniquePtrOther.GetDeleter()))
	{
	}

	UniquePtr& operator = (UniquePtr&& i_uniquePtrOther)
	{
		if (this != &i_uniquePtrOther)
		{
			Reset(i_uniquePtrOther.Release());
			m_deleter = std::move(i_uniquePtrOther.m_deleter);
		}
		return *this;
	}

	void Reset(T* i_pointer = nullptr)
	{
		if (m_pointer)
		{
			m_deleter(m_pointer);
		}
		m_pointer = i_pointer;
	}

	~UniquePtr()
	{
		Reset();
	}

	T* Get() const
	{
		return m_pointer;
	}

   T* Release()
   {
      T* ptr = m_pointer;
      m_pointer = nullptr;
      return ptr;
   }

   D& GetDeleter()
   {
      return m_deleter;
   }

	T* operator ->() const
	{
		return Get();
	}

	explicit operator bool() const
	{
		return m_pointer != nullptr;
	}

	UniquePtr(const UniquePtr&) = delete;
	UniquePtr& operator = (const UniquePtr&) = delete;

private:
	T* m_pointer;
	D m_deleter;
};

template <class T, class... TParams>
UniquePtr<T> MakeUnique(TParams&&... i_params)
{
   return UniquePtr<T>(new T(std::forward<TParams>(i_params)...));
}