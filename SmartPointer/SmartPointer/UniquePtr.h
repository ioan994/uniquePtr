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

   UniquePtr& operator=(UniquePtr&& i_uniquePtrOther)
   {
      if (this != &i_uniquePtrOther)
      {
         Reset(i_uniquePtrOther.Release());
         m_deleter = std::move(i_uniquePtrOther.m_deleter);
      }
      return *this;
   }

   UniquePtr& operator=(nullptr_t)
   {
      Reset();
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

   void Swap(UniquePtr& i_other)
   {
      std::swap(m_pointer, i_other.m_pointer);
   }

   T* operator->() const
   {
      return m_pointer;
   }

   T& operator*() const
   {
      return *m_pointer;
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

template <class T, class D>
void Swap(UniquePtr<T, D>& i_lhs, UniquePtr<T, D>& i_rhs)
{
   i_lhs.Swap(i_rhs);
}

template <class T, class D, class T2, class D2>
bool operator==(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return i_lhs.Get() == i_rhs.Get();
}

template <class T, class D, class T2, class D2>
bool operator!=(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return !(i_lhs == i_rhs);
}

template <class T, class D, class T2, class D2>
bool operator<(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return i_lhs.Get() < i_rhs.Get();
}

template <class T, class D, class T2, class D2>
bool operator<=(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return !(i_rhs < i_lhs);
}

template <class T, class D, class T2, class D2>
bool operator>(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return i_rhs < i_lhs;
}

template <class T, class D, class T2, class D2>
bool operator>=(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return !(i_lhs < i_rhs);
}

template <class T, class D>
bool operator==(const UniquePtr<T, D>& i_lhs, nullptr_t i_rhs)
{
   return !i_lhs.Get();
}

template <class T, class D>
bool operator==(nullptr_t i_lhs, const UniquePtr<T, D>& i_rhs)
{
   return i_rhs == i_lhs;
}

template <class T, class D>
bool operator!=(const UniquePtr<T, D>& i_lhs, nullptr_t i_rhs)
{
   return !(i_lhs == i_rhs);
}

template <class T, class D>
bool operator!=(nullptr_t i_lhs, const UniquePtr<T, D>& i_rhs)
{
   return i_rhs != i_lhs;
}

template <class T, class D>
bool operator<(const UniquePtr<T, D>& i_lhs, nullptr_t i_rhs)
{
   return i_lhs.Get() < i_rhs;
}

template <class T, class D>
bool operator<(nullptr_t i_lhs, const UniquePtr<T, D>& i_rhs)
{
   return i_lhs < i_rhs.Get();
}

template <class T, class D>
bool operator<=(const UniquePtr<T, D>& i_lhs, nullptr_t i_rhs)
{
   return !(i_rhs < i_lhs);
}

template <class T, class D>
bool operator<=(nullptr_t i_lhs, const UniquePtr<T, D>& i_rhs)
{
   return !(i_rhs < i_lhs);
}

template <class T, class D>
bool operator>(const UniquePtr<T, D>& i_lhs, nullptr_t i_rhs)
{
   return !(i_lhs <= i_rhs);
}

template <class T, class D>
bool operator>(nullptr_t i_lhs, const UniquePtr<T, D>& i_rhs)
{
   return !(i_lhs <= i_rhs);
}

template <class T, class D>
bool operator>=(const UniquePtr<T, D>& i_lhs, nullptr_t i_rhs)
{
   return !(i_lhs < i_rhs);
}

template <class T, class D>
bool operator>=(nullptr_t i_lhs, const UniquePtr<T, D>& i_rhs)
{
   return !(i_lhs < i_rhs);
}