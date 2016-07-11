#include "stdafx.h"
#include "CppUnitTest.h"
#include "UniquePtr.h"

#include <type_traits>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace
{
   struct Dummy
   {
      virtual ~Dummy(){}
   };
   struct DummyWithDestructor : public Dummy
   {
      DummyWithDestructor(bool& i_destructorCalled) : m_destructorCalled(i_destructorCalled)
      {
      }
      ~DummyWithDestructor()
      {
         m_destructorCalled = true;
      }

      bool& m_destructorCalled;
   };

   struct DestructorCallCounter
   {
      DestructorCallCounter()
      {
         m_destructorCallsCount = 0;
      }

      ~DestructorCallCounter()
      {
         ++m_destructorCallsCount;
      }

      static int m_destructorCallsCount;
   };
   int DestructorCallCounter::m_destructorCallsCount = 0;

   template<class T>
   T* Get(const UniquePtr<T>& i_unique)
   {
      return i_unique.Get();
   }

   nullptr_t Get(nullptr_t)
   {
      return nullptr;
   }

   template <class Left, class Right>
   void TestUniquePtrOperators(Left i_lhsPointer, Right i_rhsPointer)
   {
      Assert::AreEqual(i_lhsPointer == i_rhsPointer, Get(i_lhsPointer) == Get(i_rhsPointer), L"Operator == yields incorrect result.");
      Assert::AreEqual(i_lhsPointer != i_rhsPointer, Get(i_lhsPointer) != Get(i_rhsPointer), L"Operator != yields incorrect result.");
      Assert::AreEqual(i_lhsPointer < i_rhsPointer, Get(i_lhsPointer) < Get(i_rhsPointer), L"Operator < yields incorrect result.");
      Assert::AreEqual(i_lhsPointer <= i_rhsPointer, Get(i_lhsPointer) <= Get(i_rhsPointer), L"Operator <= yields incorrect result.");
      Assert::AreEqual(i_lhsPointer > i_rhsPointer, Get(i_lhsPointer) > Get(i_rhsPointer), L"Operator > yields incorrect result.");
      Assert::AreEqual(i_lhsPointer >= i_rhsPointer, Get(i_lhsPointer) >= Get(i_rhsPointer), L"Operator >= yields incorrect result.");
   }
}

namespace test
{
   TEST_CLASS(UniquePtrTest)
   {
   public:

      TEST_METHOD(TestGetStoredPointer)
      {
         int* ptr = new int;
         UniquePtr<int> uniquePtr(ptr);

         Assert::AreEqual(ptr, uniquePtr.Get());
      }

      TEST_METHOD(TestInitWithNull)
      {
         UniquePtr<int> uniquePtr;

         Assert::IsNull(uniquePtr.Get());
      }

      TEST_METHOD(TestReleaseReturnStoredPointerAndStoreNull)
      {
         int val;
         UniquePtr<int> uniquePtr(&val);

         Assert::AreEqual(&val, uniquePtr.Release(), L"Release returned wrong pointer.");
         Assert::IsNull(uniquePtr.Get(), L"Object still store value");
      }

      TEST_METHOD(TestDestructorCalledWhenOutOfScope)
      {
         bool destructorCalled = false;

         {
            UniquePtr<DummyWithDestructor>(new DummyWithDestructor(destructorCalled));
         }

         Assert::IsTrue(destructorCalled);
      }

      // These two fail in VS2013 and older because of compiler issue.
      // See https://connect.microsoft.com/VisualStudio/feedback/details/819202/std-is-assignable-and-std-is-constructible-give-wrong-value-for-deleted-members.
      //TEST_METHOD(TestIsNotCopyConstructible)
      //{
      //	Assert::IsFalse(std::is_copy_constructible<UniquePtr<int>>::value);
      //}

      //TEST_METHOD(TestIsNotCopyAssignable)
      //{
      //	Assert::IsFalse(std::is_copy_assignable<UniquePtr<int>>::value);
      //}

      TEST_METHOD(TestAssingWithNull)
      {
         bool destructorCalled = false;
         UniquePtr<DummyWithDestructor> unique(new DummyWithDestructor(destructorCalled));
         
         unique = nullptr;

         Assert::IsTrue(destructorCalled, L"Destructor was not called.");
         Assert::IsNull(unique.Get(), L"Object still holds pointer.");
      }

      TEST_METHOD(TestDestructorCalledAfterResetAndStorePointer)
      {
         bool destructorCalled = false;
         UniquePtr<DummyWithDestructor> uniquePtr(new DummyWithDestructor(destructorCalled));
         bool otherDummyDestructorCalled = false;
         auto otherDummy = new DummyWithDestructor(otherDummyDestructorCalled);

         uniquePtr.Reset(otherDummy);

         Assert::IsTrue(uniquePtr.Get() == otherDummy, L"Object store wrong pointer.");
         Assert::IsTrue(destructorCalled, L"Destructor was not called.");
      }

      TEST_METHOD(TestStoreNullAfterResetWithNoParameters)
      {
         UniquePtr<int> uniquePtr(new int);

         uniquePtr.Reset();

         Assert::IsNull(uniquePtr.Get());
      }

      TEST_METHOD(TestMoveConstruction)
      {
         bool destructorCalled = false;
         auto dummy = new DummyWithDestructor(destructorCalled);

         UniquePtr<DummyWithDestructor> rhs(dummy);
         UniquePtr<DummyWithDestructor> lhs = std::move(rhs);

         Assert::IsNull(rhs.Get(), L"Right hand side object still holds pointer.");
         Assert::IsTrue(dummy == lhs.Get(), L"Pointer was not moved to left hand side object.");
         Assert::IsFalse(destructorCalled, L"Destructor was called for stored object.");
      }

      TEST_METHOD(TestMoveAssign)
      {
         bool lhsDestructorCalled = false;
         auto lhsDummy = new DummyWithDestructor(lhsDestructorCalled);
         UniquePtr<DummyWithDestructor> lhs(lhsDummy);
         bool rhsDestructorCalled = false;
         auto rhsDummy = new DummyWithDestructor(rhsDestructorCalled);
         UniquePtr<DummyWithDestructor> rhs(rhsDummy);

         lhs = std::move(rhs);

         Assert::IsNull(rhs.Get(), L"Right hand side object still holds pointer.");
         Assert::IsTrue(rhsDummy == lhs.Get(), L"Pointer was not moved to left hand side object.");
         Assert::IsFalse(rhsDestructorCalled, L"Destructor was called for object stored in the right hand side unique ptr.");
         Assert::IsTrue(lhsDestructorCalled, L"Destructor was not called for object stored in the left hand side unique ptr.");
      }

      TEST_METHOD(TestConvertions)
      {
         UniquePtr<int> uniquePtr(new int);
         UniquePtr<int> uniquePtrEmpty;

         Assert::IsTrue(static_cast<bool>(uniquePtr));
         Assert::IsFalse(static_cast<bool>(uniquePtrEmpty));
      }

      TEST_METHOD(TestCustomDeleter)
      {
         bool customDestructorCalled = false;
         auto deleter = [&customDestructorCalled](int* i_ptr)
         {
            customDestructorCalled = true;
            delete i_ptr;
         };

         {
            UniquePtr<int, decltype(deleter)>(new int, deleter);
         }

         Assert::IsTrue(customDestructorCalled);
      }

      TEST_METHOD(TestAccessToMemberField)
      {
         bool destructorCalled = false;
         UniquePtr<DummyWithDestructor> uniquePtr(new DummyWithDestructor(destructorCalled));

         Assert::IsFalse(uniquePtr->m_destructorCalled);
      }

      TEST_METHOD(TestSelfMoveDoesNothing)
      {
         bool destructorCalled = false;
         UniquePtr<DummyWithDestructor> uniquePtr(new DummyWithDestructor(destructorCalled));

         uniquePtr = std::move(uniquePtr);

         Assert::IsFalse(destructorCalled);
         Assert::IsNotNull(uniquePtr.Get());
      }

      TEST_METHOD(TestDestructorIsNotCalledForNull)
      {
         bool destructorCalled = false;
         auto deleter = [&destructorCalled](int*){destructorCalled = true; };

         {
            UniquePtr<int, decltype(deleter)> unique(nullptr, deleter);
         }

         Assert::IsFalse(destructorCalled);
      }

      TEST_METHOD(TestSupportInheritedObjects)
      {
         bool wasDestructorCalled = false;
         bool wasDestructorCalled2 = false;

         {
            UniquePtr<DummyWithDestructor> rhsUnique(new DummyWithDestructor(wasDestructorCalled));
            UniquePtr<Dummy> lhsUnique = std::move(rhsUnique);

            UniquePtr<Dummy> lhsUnique2 = UniquePtr<DummyWithDestructor>(new DummyWithDestructor(wasDestructorCalled2));
         }

         Assert::IsTrue(wasDestructorCalled);
         Assert::IsTrue(wasDestructorCalled2);
      }

      TEST_METHOD(TestMakeUnique)
      {
         using BoolInt = std::pair < bool, int > ;
         const bool testBool = true;
         const int testInt = 42;

         auto unique = MakeUnique<BoolInt>(testBool, testInt);

         Assert::AreEqual(unique->first, testBool);
         Assert::AreEqual(unique->second, testInt);
      }

      TEST_METHOD(TestSwap)
      {
         int* ptr1 = new int;
         int* ptr2 = new int;
         UniquePtr<int> unique1(ptr1);
         UniquePtr<int> unique2(ptr2);

         unique1.Swap(unique2);

         Assert::IsTrue(unique1.Get() == ptr2);
         Assert::IsTrue(unique2.Get() == ptr1);
      }

      TEST_METHOD(TestOperators)
      {
         int* ptr = new int;
         UniquePtr<int> lesserUniquePtr(ptr);
         UniquePtr<int> greaterUniquePtr(ptr + 1);

         TestUniquePtrOperators<UniquePtr<int>&, UniquePtr<int>&>(lesserUniquePtr, lesserUniquePtr);
         TestUniquePtrOperators<UniquePtr<int>&, UniquePtr<int>&>(lesserUniquePtr, greaterUniquePtr);
         TestUniquePtrOperators<UniquePtr<int>&, UniquePtr<int>&>(greaterUniquePtr, lesserUniquePtr);
         TestUniquePtrOperators<nullptr_t, UniquePtr<int>&>(nullptr, lesserUniquePtr);
         TestUniquePtrOperators<UniquePtr<int>&, nullptr_t>(lesserUniquePtr, nullptr);

         greaterUniquePtr.Release();
      }

      TEST_METHOD(TestUniquePtrWithArray)
      {
         {
            UniquePtr<DestructorCallCounter[]> unique(new DestructorCallCounter[5]);
         }

         Assert::AreEqual(5, DestructorCallCounter::m_destructorCallsCount);
      }

      TEST_METHOD(TestIndexOperator)
      {
         int* array = new int[2];
         array[0] = 70;
         array[1] = 2;

         UniquePtr<int[]> unique(array);

         Assert::AreEqual(70, unique[0]);
         Assert::AreEqual(2, unique[1]);
      }

      TEST_METHOD(TestUniquePtrSize)
      {
         Assert::AreEqual(sizeof(int*), sizeof(UniquePtr<int>));
      }
   };
}