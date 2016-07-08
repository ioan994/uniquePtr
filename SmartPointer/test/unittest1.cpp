#include "stdafx.h"
#include "CppUnitTest.h"
#include "UniquePtr.h"

#include <type_traits>
#include <memory>
#include <functional>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace
{
	struct DummyWithDestructor
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

		TEST_METHOD(TestConvertsToTrueIfStorePointer)
		{
			UniquePtr<int> uniquePtr(new int);
			
			Assert::IsTrue(static_cast<bool>(uniquePtr));
		}

		TEST_METHOD(TestConvertsToTrueIfStoreNull)
		{
			UniquePtr<int> uniquePtr;

			Assert::IsFalse(static_cast<bool>(uniquePtr));
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
				UniquePtr<int, decltype(deleter)> (new int, deleter);
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
			bool destructionCalled = false;
			UniquePtr<DummyWithDestructor> uniquePtr(new DummyWithDestructor(destructionCalled));
			uniquePtr = std::move(uniquePtr);

			Assert::IsFalse(destructionCalled);
		}
	};
}