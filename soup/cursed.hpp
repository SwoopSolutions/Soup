#pragma once

#include "base.hpp"
#include "utility.hpp" // SOUP_MOVE_RETURN

NAMESPACE_SOUP
{
	template <typename Desired, typename Actual>
	[[nodiscard]] Desired ub_cast(Actual a)
	{
		union {
			Actual a;
			Desired d;
		} u{.a = a };
		SOUP_MOVE_RETURN(u.d);
	}

	template <typename Desired, typename Actual>
	[[nodiscard]] Desired ub_pointer_cast(Actual a)
	{
		static_assert(sizeof(Desired) == sizeof(Actual));
		return ub_cast<Desired, Actual>(a);
	}


	template <typename StructT, typename FieldT>
	uintptr_t offsetofp(FieldT StructT::* ptr)
	{
		return reinterpret_cast<uintptr_t>(&(ub_pointer_cast<StructT*>(nullptr)->*ptr));
	}


	template<typename Tag>
	struct PrivateMemberResult
	{
		typedef typename Tag::type type;
		static type ptr;
	};

	template<typename Tag>
	typename PrivateMemberResult<Tag>::type PrivateMemberResult<Tag>::ptr;

	template<typename Tag, typename Tag::type p>
	struct PrivateMemberGetter : PrivateMemberResult<Tag>
	{
		struct PrivateMemberFiller
		{
			PrivateMemberFiller()
			{
				PrivateMemberResult<Tag>::ptr = p;
			}
		};
		static PrivateMemberFiller filler_obj;
	};

	template<typename Tag, typename Tag::type p>
	typename PrivateMemberGetter<Tag, p>::PrivateMemberFiller PrivateMemberGetter<Tag, p>::filler_obj;

	#define SOUP_PREPARE_MEMBER(StructT, MemberT, member_name) \
	struct PrivateMemberTag_ ## StructT ## _ ## member_name { typedef MemberT StructT::* type; }; \
	NAMESPACE_SOUP { template struct PrivateMemberGetter<PrivateMemberTag_ ## StructT ## _ ## member_name, &StructT::member_name>; }

	#define SOUP_GET_MEMBER(StructT, member_name) (PrivateMemberResult<PrivateMemberTag_ ## StructT ## _ ## member_name>::ptr)
}
