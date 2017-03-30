#ifndef ARA_INTERNAL_VARIANT_CONVERT_H
#define ARA_INTERNAL_VARIANT_CONVERT_H

#include "../ref_string.h"
#include "../stringext.h"
#include <string>
#include <sstream>

namespace ara {
	namespace internal {
		struct default_variant_convert {
			static const std::string	 &	to(ara::type_id<std::string> id) { return ara::static_empty<std::string>::val; }
			static std::string		to(ara::type_id<std::string> id, bool bo) { return bo ? "true" : "false"; }
			static std::string		to(ara::type_id<std::string> id, int n) { std::string e; strext(e).append_int(n); return e; }
			static std::string		to(ara::type_id<std::string> id, int64_t n) { std::string e; strext(e).append_int(n); return e; }
			static std::string		to(ara::type_id<std::string> id, double bo) {
				std::stringstream s;
				s << bo;
				return s.str();
			}
			static std::string		to(ara::type_id<std::string> id, const ref_string & s) { return s.str(); }
			static const std::string	&	to(ara::type_id<std::string> id, const std::string & s) { return s; }

			static int8_t		to(ara::type_id<int8_t>) { return 0; }
			static int8_t		to(ara::type_id<int8_t>, bool bo) { return bo ? 1 : 0; }
			static int8_t		to(ara::type_id<int8_t>, int n) { return static_cast<int8_t>(n); }
			static int8_t		to(ara::type_id<int8_t>, int64_t n) { return static_cast<int8_t>(n); }
			static int8_t		to(ara::type_id<int8_t>, double n) { return static_cast<int8_t>(n); }
			static int8_t		to(ara::type_id<int8_t>, const ref_string & s) { return strext(s).to_int<int8_t>(); }
			static int8_t		to(ara::type_id<int8_t>, const std::string & s) { return strext(s).to_int<int8_t>(); }
		};

		struct strict_variant_convert {
		};

		struct display_variant_convert {
		};
	}
}

#endif//ARA_INTERNAL_VARIANT_CONVERT_H
