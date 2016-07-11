#pragma once

template<class T>
struct DefaultDeleter
{
   template<class TOther>
   DefaultDeleter(const DefaultDeleter<TOther>&){}
   DefaultDeleter() = default;

   void operator()(T* i_ptr) const
   {
      delete i_ptr;
   }
};

template<class T>
struct DefaultDeleter<T[]>
{
   template<class TOther>
   DefaultDeleter(const DefaultDeleter<TOther>&){}
   DefaultDeleter() = default;

   void operator()(T* i_ptr) const
   {
      delete[] i_ptr;
   }
};

template<class T, class D, bool noDeleter>
struct PointerStorage
{
   using ElementType = typename std::conditional<std::is_array<T>::value, typename std::remove_extent<T>::type, T>::type;
   using Pointer = ElementType*;
   using DeleterType = typename std::decay<D>::type;

   PointerStorage(Pointer i_pointer, const DeleterType& i_deleter) :
      m_pointer(i_pointer),
      m_deleter(i_deleter)
   {
   }

   PointerStorage(Pointer i_pointer, DeleterType&& i_deleter) :
      m_pointer(i_pointer),
      m_deleter(std::move(i_deleter))
   {
   }

   DeleterType& GetDeleter()
   {
      return m_deleter;
   }

   Pointer m_pointer;
   DeleterType m_deleter;
};

template<class T, class D>
struct PointerStorage < T, D, true > : public D
{
   using ElementType = typename std::conditional<std::is_array<T>::value, typename std::remove_extent<T>::type, T>::type;
   using Pointer = ElementType*;
   using DeleterType = typename std::decay<D>::type;

   PointerStorage(Pointer i_pointer, const DeleterType& i_deleter) : m_pointer(i_pointer)
   {
   }

   PointerStorage(Pointer i_pointer, DeleterType&& i_deleter) : m_pointer(i_pointer)
   {
   }

   DeleterType& GetDeleter()
   {
      return *this;
   }

   Pointer m_pointer;
};

template <class T, class D = DefaultDeleter<T>>
class UniquePtr : public PointerStorage<T,D, std::is_same<D, DefaultDeleter<T>>::value>
{
public:

   explicit UniquePtr(Pointer i_pointer = nullptr) : PointerStorage(i_pointer, DeleterType())
   {
   }

   UniquePtr(Pointer i_pointer, const DeleterType& i_deleter) : PointerStorage(i_pointer, i_deleter)
   {
   }

   UniquePtr(Pointer i_pointer, DeleterType&& i_deleter) : PointerStorage(i_pointer, std::move(i_deleter))
   {
   }

   template <class TOther>
   UniquePtr(UniquePtr<TOther>&& i_uniquePtrOther) :
      PointerStorage(i_uniquePtrOther.Release(), std::move(i_uniquePtrOther.GetDeleter()))
   {
   }

   UniquePtr& operator=(UniquePtr&& i_uniquePtrOther)
   {
      if (this != &i_uniquePtrOther)
      {
         Reset(i_uniquePtrOther.Release());
         GetDeleter() = std::move(i_uniquePtrOther.GetDeleter());
      }
      return *this;
   }

   UniquePtr& operator=(nullptr_t)
   {
      Reset();
      return *this;
   }

   void Reset(Pointer i_pointer = nullptr)
   {
      if (m_pointer)
      {
         GetDeleter()(m_pointer);
      }
      m_pointer = i_pointer;
   }

   ~UniquePtr()
   {
      Reset();
   }

   Pointer Get() const
   {
      return m_pointer;
   }

   Pointer Release()
   {
      Pointer ptr = m_pointer;
      m_pointer = nullptr;
      return ptr;
   }

   void Swap(UniquePtr& i_other)
   {
      std::swap(m_pointer, i_other.m_pointer);
   }

   Pointer operator->() const
   {
      return m_pointer;
   }

   ElementType& operator*() const
   {
      return *m_pointer;
   }

   template < typename = typename std::enable_if< std::is_array<T>::value >::type >
   ElementType& operator[](size_t i_index) const
   {
      return m_pointer[i_index];
   }

   explicit operator bool() const
   {
      return m_pointer != nullptr;
   }

   UniquePtr(const UniquePtr&) = delete;
   UniquePtr& operator = (const UniquePtr&) = delete;
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