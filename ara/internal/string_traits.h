
#ifndef ARA_INTERNAL_STRING_TRAITS_H
#define ARA_INTERNAL_STRING_TRAITS_H

#include "default_string_traits.h"
#include "stdstring_traits.h"

namespace ara {
	template<class Ch, int base>
	struct is_valid_number_char {
		static bool yes(Ch ch) {
			return ch == '-' || (ch >= '0' && ch <= '9');
		}
	};
	template<class Ch>
	struct is_valid_number_char<Ch,16> {
		static bool yes(Ch ch) {
			return ch == '-' || (ch >= '0' && ch <= '9') || (ch >='a' && ch <= 'f') || (ch >='A' && ch <= 'F');
		}
	};
	template<class Ch>
	struct is_valid_number_char<Ch,8> {
		static bool yes(Ch ch) {
			return ch == '-' || (ch >= '0' && ch <= '7');
		}
	};
	template<class Ch>
	struct is_valid_number_char<Ch,2> {
		static bool yes(Ch ch) {
			return ch == '-' || (ch >= '0' && ch <= '1');
		}
	};
}

#endif//ARA_INTERNAL_STRING_TRAITS_H
