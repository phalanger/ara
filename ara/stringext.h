
#ifndef ARA_STRINGEXT_H
#define ARA_STRINGEXT_H

#include "ara_def.h"

#include <string>
#include <type_traits>

#include "ref_string.h"
#include "internal/string_traits.h"
#include "internal/string_convert.h"
#include "internal/strformat.h"

namespace ara {

	template<class typeStr>
	class string_ext
	{
	public:
		using typeOrgStr = std::remove_const_t<typeStr>;
		using typeStrTraits = string_traits<typeOrgStr>;
		using typeRefStr = ref_string_base<typename typeStrTraits::value_type, typename  typeStrTraits::traits_type>;

		string_ext(typeStr & s) : str_(s) {}
		string_ext(const string_ext & s) : str_(s._str_) {}

 		auto begin() { return typeStrTraits::begin(str_); }
 		auto end() { return typeStrTraits::end(str_); }
		typename typeStrTraits::const_iterator begin() const { return typeStrTraits::begin(str_); }
		typename typeStrTraits::const_iterator end() const { return typeStrTraits::end(str_); }

		typeStr & str() { return str_; }
		const typeStr & str() const { return str_; }

		typeStr		trim_left(const typeRefStr & chSet) const {
			typeStrTraits::size_type p = typeStrTraits::find_first_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				return typeStrTraits::substr(str_, p, typeStrTraits::npos);
			return str_;
		}
		string_ext &		trim_left_inplace(const typeRefStr & chSet) {
			typeStrTraits::size_type p = typeStrTraits::find_first_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				str_ = typeStrTraits::substr(str_, p, typeStrTraits::npos);
			return *this;
		}
		typeStr		trim_right(const typeRefStr & chSet) const {
			typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), typeStrTraits::npos);
			if (p != typeStrTraits::npos)
				return typeStrTraits::substr(str_, 0, p + 1);
			return str_;
		}
		string_ext &		trim_right_inplace(const typeRefStr & chSet) {
			typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), typeStrTraits::npos);
			if (p != typeStrTraits::npos)
				str_ = typeStrTraits::substr(str_, 0, p + 1);
			return *this;
		}

		typeStr		trim(const typeRefStr & chSet) const {
			typeOrgStr res = trim_left(chSet);
			string_ext<typeOrgStr>	ext(res);
			ext.trim_right_inplace(chSet);
			return res;
		}
		string_ext &		trim_inplace(const typeRefStr & chSet) {
			trim_left_inplace(chSet);
			return trim_right_inplace(chSet);
		}

		template<typename T, int base = 10>
			T	to_int() const {
				T t = 0;
				auto p = typeStrTraits::data(str_);
				auto end = p + typeStrTraits::size(str_);
				bool boNegative = false;
				if (p != end && *p == '-') {
					boNegative = true;
					++p;
				}
				for (; p != end; ++p) {
					auto ch = *p;
					if (ch >= '0' && ch <= '9') {
						t *= base;
						t += ch - '0';
					} else if (base == 16 && ch >= 'a' && ch <= 'f') {
						t *= base;
						t += ch - 'a' + 10;
					} else if (base == 16 && ch >= 'A' && ch <= 'F') {
						t *= base;
						t += ch - 'A' + 10;
					} else
						break;
				}
				return boNegative ? (-t) : t;
			}
		template<typename T,int base = 10, bool boLowCase = false>
			string_ext &	append_int(T t) {
				internal::format_appender<typeStr>	appender(str_);
				appender.append_int<T, base, boLowCase>(t);
				return *this;
			}

		template<class T>
		inline string_ext & append(const T & t, typename std::enable_if<is_string<T>::value>::type * p = 0) {
			internal::string_convert::append(str_, t);
			return *this;
		}

		template<class T>
		inline string_ext & append(const T * p, typename std::enable_if<is_char<T>::value>::type * p2 = 0) {
			internal::string_convert::append(str_, ref_string_base<T>(p));
			return *this;
		}

		template<class T, typename = std::enable_if<is_string<T>::value>::type >
		inline string_ext & operator+=(const T & t) {
			internal::string_convert::append(str_, t);
			return *this;
		}
		template<class T, typename = std::enable_if<is_char<T>::value>::type >
		inline string_ext & operator+=(const T * p) {
			internal::string_convert::append(str_, ref_string_base<T>(p));
			return *this;
		}

		template<class T, typename std::enable_if<std::is_same<T, typeStr>::value>::type * p = 0>
		inline const T & to() const {
			return str_;
		}
		template<class T>
		inline T to(typename std::enable_if<is_string<T>::value>::type * p = 0) const {
			T		res;
			string_ext<T>(res).append(str_);
			return res;
		}
		template<class T>
		inline T to(typename std::enable_if<std::is_integral<T>::value>::type * p = 0) const {
			return to_int<T>();
		}

		template<class ch, typename...TypeList>
		string_ext &	printf(const ch * s, const TypeList&... t2) {
			str_format<typeStr>		f(str_);
			f.printf(s , t2...);
			return *this;
		}

	protected:
		typeStr	&	str_;
	};

	template<class typeStr>
	string_ext<typeStr>	strext(typeStr & s) {
		return string_ext<typeStr>(s);
	}

	template<class strType, class ch, typename...TypeList>
	strType	str_printf(const ch * s, const TypeList&... t2) {
		strType		str;
		str_format<strType>		f(str);
		f.printf(s, t2...);
		return str;
	}
}

#endif // !ARA_STRINGEXT_H

