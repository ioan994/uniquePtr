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
   using element_type = typename std::conditional<std::is_array<T>::value, typename std::remove_extent<T>::type, T>::type;
   using pointer = element_type*;
   using deleter_type = typename std::decay<D>::type;

   PointerStorage(pointer i_pointer, const deleter_type& i_deleter) :
      m_pointer(i_pointer),
      m_deleter(i_deleter)
   {
   }

   PointerStorage(pointer i_pointer, deleter_type&& i_deleter) :
      m_pointer(i_pointer),
      m_deleter(std::move(i_deleter))
   {
   }

   deleter_type& get_deleter()
   {
      return m_deleter;
   }

   pointer m_pointer;
   deleter_type m_deleter;
};

template<class T, class D>
struct PointerStorage < T, D, true > : public D
{
   using element_type = typename std::conditional<std::is_array<T>::value, typename std::remove_extent<T>::type, T>::type;
   using pointer = element_type*;
   using deleter_type = typename std::decay<D>::type;

   PointerStorage(pointer i_pointer, const deleter_type& i_deleter) : m_pointer(i_pointer)
   {
   }

   PointerStorage(pointer i_pointer, deleter_type&& i_deleter) : m_pointer(i_pointer)
   {
   }

   deleter_type& get_deleter()
   {
      return *this;
   }

   pointer m_pointer;
};

template <class T, class D = DefaultDeleter<T>>
class UniquePtr : public PointerStorage<T, D, std::is_empty<D>::value>
{
public:

   explicit UniquePtr(pointer i_pointer = nullptr) : PointerStorage(i_pointer, deleter_type())
   {
   }

   UniquePtr(pointer i_pointer, const deleter_type& i_deleter) : PointerStorage(i_pointer, i_deleter)
   {
   }

   UniquePtr(pointer i_pointer, deleter_type&& i_deleter) : PointerStorage(i_pointer, std::move(i_deleter))
   {
   }

   template <class TOther>
   UniquePtr(UniquePtr<TOther>&& i_uniquePtrOther) :
      PointerStorage(i_uniquePtrOther.release(), std::move(i_uniquePtrOther.get_deleter()))
   {
   }

   UniquePtr& operator=(UniquePtr&& i_uniquePtrOther)
   {
      if (this != &i_uniquePtrOther)
      {
         reset(i_uniquePtrOther.release());
         get_deleter() = std::move(i_uniquePtrOther.get_deleter());
      }
      return *this;
   }

   UniquePtr& operator=(nullptr_t)
   {
      reset();
      return *this;
   }

   void reset(pointer i_pointer = nullptr)
   {
      if (m_pointer)
      {
         get_deleter()(m_pointer);
      }
      m_pointer = i_pointer;
   }

   ~UniquePtr()
   {
      reset();
   }

   pointer get() const
   {
      return m_pointer;
   }

   pointer release()
   {
      pointer ptr = m_pointer;
      m_pointer = nullptr;
      return ptr;
   }

   void swap(UniquePtr& i_other)
   {
      std::swap(m_pointer, i_other.m_pointer);
   }

   pointer operator->() const
   {
      return m_pointer;
   }

   element_type& operator*() const
   {
      return *m_pointer;
   }

   template < typename = typename std::enable_if< std::is_array<T>::value >::type >
   element_type& operator[](size_t i_index) const
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
void swap(UniquePtr<T, D>& i_lhs, UniquePtr<T, D>& i_rhs)
{
   i_lhs.swap(i_rhs);
}

template <class T, class D, class T2, class D2>
bool operator==(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return i_lhs.get() == i_rhs.get();
}

template <class T, class D, class T2, class D2>
bool operator!=(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return !(i_lhs == i_rhs);
}

template <class T, class D, class T2, class D2>
bool operator<(const UniquePtr<T, D>& i_lhs, const UniquePtr<T2, D2>& i_rhs)
{
   return i_lhs.get() < i_rhs.get();
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
   return !i_lhs.get();
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
   return i_lhs.get() < i_rhs;
}

template <class T, class D>
bool operator<(nullptr_t i_lhs, const UniquePtr<T, D>& i_rhs)
{
   return i_lhs < i_rhs.get();
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