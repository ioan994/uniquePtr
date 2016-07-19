#pragma once

#include <memory>

template<class T>
struct DefaultDeleter
{
   template<class TOther, class = std::enable_if_t<std::is_convertible<TOther *, T *>::value>>
   DefaultDeleter(const DefaultDeleter<TOther>&)
   {
   }

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

   template <class TOtherType>
   void operator()(TOtherType*) const = delete;
};

template<class T, class D, bool noDeleter>
struct PointerStorage
{
   using element_type = T;
   using pointer = typename std::_Get_deleter_pointer_type<element_type, D>::type;
   using deleter_type = D;

   PointerStorage(pointer i_pointer, deleter_type i_deleter) :
      m_pointer(i_pointer),
      m_deleter(i_deleter)
   {
   }

   PointerStorage(pointer i_pointer) : m_pointer(i_pointer)
   {
   }

   std::remove_reference_t<deleter_type>& get_deleter()
   {
      return m_deleter;
   }

   const std::remove_reference_t<deleter_type>& get_deleter() const
   {
      return m_deleter;
   }

   pointer m_pointer;
   deleter_type m_deleter;
};

template<class T, class D>
struct PointerStorage < T, D, true > : public D
{
   using element_type = T;
   using pointer = typename std::_Get_deleter_pointer_type<element_type, D>::type;
   using deleter_type = D;

   PointerStorage(pointer i_pointer) : m_pointer(i_pointer)
   {
   }

   PointerStorage(pointer i_pointer, deleter_type i_deleter) : m_pointer(i_pointer)
   {
   }

   std::remove_reference_t<deleter_type>& get_deleter()
   {
      return *this;
   }

   const std::remove_reference_t<deleter_type>& get_deleter() const
   {
      return *this;
   }

   pointer m_pointer;
};

template <class T, class D = DefaultDeleter<T>>
class UniquePtr : public PointerStorage<T, D, std::is_empty<D>::value>
{
public:

   UniquePtr() : PointerStorage(nullptr)
   {
   }

   UniquePtr(nullptr_t) : PointerStorage(nullptr)
   {
   }

   explicit UniquePtr(pointer i_pointer) : PointerStorage(i_pointer)
   {
   }

   UniquePtr(pointer i_pointer,
      std::conditional_t<
         std::is_reference<D>::value, D, const std::remove_reference_t<D>
      > i_deleter) : PointerStorage(i_pointer, i_deleter)
   {
   }

   UniquePtr(pointer i_pointer, std::remove_reference_t<D>&& i_deleter) : PointerStorage(i_pointer, std::move(i_deleter))
   {
   }

   UniquePtr(UniquePtr&& i_other) : PointerStorage(i_other.release(), std::move(i_other.get_deleter()))
   {
   }

   template <class TOtherPtr, class TOtherDeleter,
      class = std::enable_if_t<
         !std::is_array<TOtherPtr>::value &&
         std::is_convertible<UniquePtr<TOtherPtr, TOtherDeleter>::pointer, pointer>::value &&
         ((std::is_reference<D>::value && std::is_same<D, TOtherDeleter>::value) ||
         (!std::is_reference<D>::value && std::is_convertible<TOtherDeleter, D>::value))
      >>
   UniquePtr(UniquePtr<TOtherPtr, TOtherDeleter>&& i_uniquePtrOther) :
      PointerStorage(i_uniquePtrOther.release(), std::forward<TOtherDeleter>(i_uniquePtrOther.get_deleter()))
   {
   }

   UniquePtr& operator=(UniquePtr&& i_uniquePtrOther)
   {
      if (this != &i_uniquePtrOther)
      {
         reset(i_uniquePtrOther.release());
         get_deleter() = std::forward<D>(i_uniquePtrOther.get_deleter());
      }
      return *this;
   }

   template<class TPointerOther, class TDeleterOther,
      class = std::enable_if_t<
         !std::is_array<TPointerOther>::value &&
         std::is_convertible<UniquePtr<TPointerOther, TDeleterOther>::pointer, pointer>::value &&
         std::is_assignable<D&, TDeleterOther&&>::value
      >>
   UniquePtr& operator=(UniquePtr<TPointerOther, TDeleterOther>&& i_uniquePtrOther)
   {
      reset(i_uniquePtrOther.release());
      get_deleter() = std::forward<TDeleterOther>(i_uniquePtrOther.get_deleter());
      return *this;
   }

   UniquePtr& operator=(nullptr_t)
   {
      reset();
      return *this;
   }

   void reset(pointer i_pointer = nullptr)
   {
      pointer temp = m_pointer;
      m_pointer = i_pointer;

      if (temp)
      {
         get_deleter()(temp);
      }
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

   explicit operator bool() const
   {
      return m_pointer != nullptr;
   }

   UniquePtr(const UniquePtr&) = delete;
   UniquePtr& operator = (const UniquePtr&) = delete;
};

template <class T, class D>
class UniquePtr<T[], D> : public PointerStorage<T, D, std::is_empty<D>::value>
{
public:

   UniquePtr() : PointerStorage(nullptr)
   {
   }

   UniquePtr(nullptr_t) : PointerStorage(nullptr)
   {
   }

   explicit UniquePtr(pointer i_pointer) : PointerStorage(i_pointer)
   {
   }

   UniquePtr(pointer i_pointer,
      std::conditional_t<
         std::is_reference<D>::value,
         D,
         const std::remove_reference_t<D>&
      > i_deleter) : PointerStorage(i_pointer, i_deleter)
   {
   }

   UniquePtr(pointer i_pointer, std::remove_reference_t<D>&& i_deleter) : PointerStorage(i_pointer, std::move(i_deleter))
   {
   }

   UniquePtr(UniquePtr&& i_other) : PointerStorage(i_other.release(), std::move(i_other.get_deleter()))
   {
   }

   UniquePtr& operator=(UniquePtr&& i_uniquePtrOther)
   {
      if (this != &i_uniquePtrOther)
      {
         reset(i_uniquePtrOther.release());
         get_deleter() = std::forward<D>(i_uniquePtrOther.get_deleter());
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
      pointer temp = m_pointer;
      m_pointer = i_pointer;

      if (temp)
      {
         get_deleter()(temp);
      }
   }

   void reset(nullptr_t)
   {
      reset();
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

template <class T, class... TParams, class = std::enable_if_t<!std::is_array<T>::value>>
UniquePtr<T> MakeUnique(TParams&&... i_params)
{
   return UniquePtr<T>(new T(std::forward<TParams>(i_params)...));
}

template <class T, class = std::enable_if_t<std::is_array<T>::value && std::extent<T>::value == 0>>
UniquePtr<T> MakeUnique(size_t i_size)
{
   return UniquePtr<T>(new std::remove_extent_t<T>[i_size]());
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