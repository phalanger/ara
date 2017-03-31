#ifndef ARA_INTERNAL_VARIANT_CONVERT_H
#define ARA_INTERNAL_VARIANT_CONVERT_H

#include "../ref_string.h"
#include "../stringext.h"
#include <string>
#include <sstream>

namespace ara {
	namespace internal {
		struct default_variant_convert {

			//To string
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

			//To bool
			static bool				to(ara::type_id<bool> id) { return false; }
			static bool				to(ara::type_id<bool> id, bool bo) { return bo; }
			static bool				to(ara::type_id<bool> id, int n) { return n != 0; }
			static bool				to(ara::type_id<bool> id, int64_t n) { return n != 0; }
			static bool				to(ara::type_id<bool> id, double bo) { return !(bo <  std::numeric_limits<double>::epsilon() && bo > -std::numeric_limits<double>::epsilon()); }
			static bool				to(ara::type_id<bool> id, const ref_string & s) { return !s.empty(); }
			static bool				to(ara::type_id<bool> id, const std::string & s) { return !s.empty(); }

			//To int/int64/...
			template<typename T>
			static T		to(ara::type_id<T>, typename std::enable_if<std::is_integral<T>::value>::type * = 0) { return 0; }
			template<typename T>
			static T		to(ara::type_id<T>, bool bo, typename std::enable_if<std::is_integral<T>::value>::type * = 0) { return bo ? 1 : 0; }
			template<typename T>
			static T		to(ara::type_id<T>, int n, typename std::enable_if<std::is_integral<T>::value>::type * = 0) { return static_cast<T>(n); }
			template<typename T>
			static T		to(ara::type_id<T>, int64_t n, typename std::enable_if<std::is_integral<T>::value>::type * = 0) { return static_cast<T>(n); }
			template<typename T>
			static T		to(ara::type_id<T>, double n, typename std::enable_if<std::is_integral<T>::value>::type * = 0) { return static_cast<T>(n); }
			template<typename T>
			static T		to(ara::type_id<T>, const ref_string & s, typename std::enable_if<std::is_integral<T>::value>::type * = 0) { return strext(s).to_int<T>(); }
			template<typename T>
			static T		to(ara::type_id<T>, const std::string & s, typename std::enable_if<std::is_integral<T>::value>::type * = 0) { return strext(s).to_int<T>(); }

			//To double/float
			template<typename T>
			static T		to(ara::type_id<T>, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) { return 0.0; }
			template<typename T>
			static T		to(ara::type_id<T>, bool bo, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) { return bo ? 1.0 : 0.0; }
			template<typename T>
			static T		to(ara::type_id<T>, int n, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) { return static_cast<T>(n); }
			template<typename T>
			static T		to(ara::type_id<T>, int64_t n, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) { return static_cast<T>(n); }
			template<typename T>
			static T		to(ara::type_id<T>, double n, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) { return static_cast<T>(n); }
			template<typename T>
			static T		to(ara::type_id<T>, const ref_string & s, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) {
				std::stringstream	o(s.str());
				T r = 0.0;
				o >> r;
				return r;
			}
			template<typename T>
			static T		to(ara::type_id<T>, const std::string & s, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) {
				std::stringstream	o(s);
				T r = 0.0;
				o >> r;
				return r;
			}
		};

		struct display_variant_convert : public default_variant_convert {
			static std::string	 to(ara::type_id<std::string> id) { return "null"; }
		};
	}
}

#endif//ARA_INTERNAL_VARIANT_CONVERT_H
