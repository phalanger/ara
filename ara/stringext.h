
/*
	std::string		str;
	auto s = ara::strext(str);
	s.printf("hello : %d\n", 100);
	std::string s2 = s.trim();
	s.trim_left_inplace(" \r\n;");

	auto s2 = ara::str_printf<std::string>("name:%s\n", str);
	ara::stream_printf(std::cout, "%d + %d = %d\n", 1 , 2, 1 + 2);

	auto s3 = ara::printf<std::string>("%d + %d = %d", 1, 2, 3);

	std::stringstream s4;
	ara::printf(s4, "%d - %d = %d", 3, 2, 1);
*/

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
		using value_type = typename typeStrTraits::value_type;
		using typeRefStr = ref_string_base<value_type, typename  typeStrTraits::traits_type>;

		string_ext(typeStr & s) : str_(s) {}
		string_ext(const string_ext & s) : str_(s.str_) {}

		auto begin() { return typeStrTraits::begin(str_); }
		auto end() { return typeStrTraits::end(str_); }
		typename typeStrTraits::const_iterator begin() const { return typeStrTraits::begin(str_); }
		typename typeStrTraits::const_iterator end() const { return typeStrTraits::end(str_); }

		typeStr & str() { return str_; }
		const typeStr & str() const { return str_; }

		typeStr		trim_left(const typeRefStr & chSet) const {
			typename typeStrTraits::size_type p = typeStrTraits::find_first_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				return typeStrTraits::substr(str_, p, typeStrTraits::npos);
			return str_;
		}
		string_ext &		trim_left_inplace(const typeRefStr & chSet) {
			typename typeStrTraits::size_type p = typeStrTraits::find_first_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				str_ = typeStrTraits::substr(str_, p, typeStrTraits::npos);
			return *this;
		}
		typeStr		trim_right(const typeRefStr & chSet) const {
			typename typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), typeStrTraits::npos);
			if (p != typeStrTraits::npos)
				return typeStrTraits::substr(str_, 0, p + 1);
			return str_;
		}
		string_ext &		trim_right_inplace(const typeRefStr & chSet) {
			typename typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), typeStrTraits::npos);
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

		template<class T, typename = typename std::enable_if<is_string<T>::value>::type >
		inline string_ext & operator+=(const T & t) {
			internal::string_convert::append(str_, t);
			return *this;
		}
		template<class T, typename = typename std::enable_if<is_char<T>::value>::type >
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
		string_ext &	printf(const ch * s, TypeList... t2) {
			str_format<typeStr>		f(str_);
			f.printf(s , std::forward<TypeList>(t2)...);
			return *this;
		}

	protected:
		typeStr	&	str_;
	};

	template<class typeStr>
	inline string_ext<typeStr>	strext(typeStr & s) {
		return string_ext<typeStr>(s);
	}
	
	template<class typeStr>
	inline string_ext<const typeStr>	strext(const typeStr & s) {
		return string_ext<const typeStr>(s);
	}

	template<class strType, class ch, typename...TypeList>
	inline strType	str_printf(const ch * s, TypeList... t2) {
		strType		str;
		str_format<strType>		f(str);
		f.printf(s, std::forward<TypeList>(t2)...);
		return str;
	}

	template<class Stream>
	inline str_format<Stream> stream_printf(Stream & out) {
		return str_format<Stream>(out);
	}

	template<class Stream, class ch, typename...TypeList >
	inline Stream & stream_printf(Stream & out, const ch * s, TypeList... t2) {
		str_format<Stream>(out).printf(s, std::forward<TypeList>(t2)...);
		return out;
	}

	template<class strType, class ch, typename...TypeList>
	inline strType	printf(const ch * s, TypeList... t2) {
		return str_printf<strType>(s, std::forward<TypeList>(t2)...);
	}
	template<class streamChar, class streamTraits, class ch, typename...TypeList>
	inline std::basic_ostream<streamChar, streamTraits> & printf(std::basic_ostream<streamChar, streamTraits> & out, const ch * s, TypeList... t2) {
		return stream_printf(out, s, std::forward<TypeList>(t2)...);
	}
}

#endif // !ARA_STRINGEXT_H

