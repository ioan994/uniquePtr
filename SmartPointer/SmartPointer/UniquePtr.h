#pragma once

template<class T>
struct DefaultDeleter
{
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

	UniquePtr(UniquePtr&& i_uniquePtrOther) :
		m_pointer(i_uniquePtrOther.Release()),
		m_deleter(std::move(i_uniquePtrOther.m_deleter))
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

	T* operator ->() const
	{
		return Get();
	}

	explicit operator bool() const
	{
		return m_pointer != nullptr;
	}

	T* Release()
	{
		T* ptr = m_pointer;
		m_pointer = nullptr;
		return ptr;
	}

	UniquePtr(const UniquePtr&) = delete;
	UniquePtr& operator = (const UniquePtr&) = delete;

private:
	T* m_pointer;
	D m_deleter;
};