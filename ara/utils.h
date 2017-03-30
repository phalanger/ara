
/*
	std::vector<int>	ary = { 1, 2, 3, 4, 5, 6 };
	for (int n : ara::reverse_range(ary)) {
		...//6 5 4 3 2 1
	}

*/
#ifndef ARA_UTILS_H
#define ARA_UTILS_H

#include "ara_def.h"
#include <functional>

namespace ara {

	template<bool B, class T1, class T2>
	struct select_type {
		typedef T2		type;
	};
	template<class T1, class T2>
	struct select_type<true, T1, T2> {
		typedef T1		type;
	};

	namespace internal {
		template<typename container>
		class reverse_range_helper {
		public:
			typedef typename select_type<std::is_const<container>::value,
				typename container::const_reverse_iterator,
				typename container::reverse_iterator>::type iterator;

			reverse_range_helper(container & c) : c_(c) {}
			reverse_range_helper(const reverse_range_helper & r) : c_(r.c_) {}

			inline iterator begin() {
				return c_.rbegin();
			}
			inline iterator end() {
				return c_.rend();
			}
		protected:

			container & c_;
		};
	}

	template<typename container>
	internal::reverse_range_helper<container>	reverse_range(container & c) {
		return internal::reverse_range_helper<container>(c);
	}


	///////////////////////////////////////////////

	class defer {
	public:
		defer(std::function<void()> && func) : func_(std::move(func)) {}
		~defer() { func_(); }
	protected:
		std::function<void()>	func_;
	};

	///////////////////////////////////////////////

	template<typename T>
	class static_empty {
	public:
		static const T	val;
	};
	template<typename T>
	const T	static_empty<T>::val;

	//////////////////////////////////////////////
	template<typename T>
	struct type_id {};
}

#define ARA_JOIN_2(a, b)			a##b
#define ARA_JOIN_1(a, b)			ARA_JOIN_2(a, b)
#define ARA_TMP_VAR(a)				ARA_JOIN_1(a, __LINE__)

#define ARA_DEFER(f)	ara::defer		ARA_TMP_VAR(__defer)([&](){ f });

#endif//ARA_UTILS_H
