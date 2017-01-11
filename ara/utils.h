
/*
	std::vector<int>	ary = { 1, 2, 3, 4, 5, 6 };
	for (int n : ara::reverse_range(ary)) {
		...//6 5 4 3 2 1
	}

*/
#ifndef ARA_UTILS_H
#define ARA_UTILS_H

#include "ara_def.h"

namespace ara {

	namespace internal {
		template<typename container>
		class reverse_range_helper {
		public:
			reverse_range_helper(container & c) : c_(c) {}
			reverse_range_helper(const reverse_range_helper & r) : c_(r.c_) {}

			inline auto begin() {
				return c_.rbegin();
			}
			inline auto end() {
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


}

#endif//ARA_UTILS_H
