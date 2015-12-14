
#ifndef ARA_STRINGEXT_H
#define ARA_STRINGEXT_H

#include <string>
#include <type_traits>

#include "const_string.h"
#include "internal/string_traits.h"

namespace ara {

	template<class typeStr>
	class string_ext
	{
	public:
		using typeOrgStr = std::remove_const_t<typeStr>;
		using typeStrTraits = string_traits<typeOrgStr>;
		using typeConstStr = const_string_base<typename typeStrTraits::value_type, typename  typeStrTraits::traits_type>;

		string_ext(typeStr & s) : str_(s) {}
		string_ext(const string_ext & s) : str_(s._str_) {}

 		auto begin() { return typeStrTraits::begin(str_); }
 		auto end() { return typeStrTraits::end(str_); }
		typename typeStrTraits::const_iterator begin() const { return typeStrTraits::begin(str_); }
		typename typeStrTraits::const_iterator end() const { return typeStrTraits::end(str_); }

		typeStr		trim_left(const typeConstStr & chSet) const {
			typeStrTraits::size_type p = typeStrTraits::find_first_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				return typeStrTraits::substr(str_, p, typeStrTraits::npos);
			return str_;
		}
		string_ext &		trim_left_inplace(const typeConstStr & chSet) {
			typeStrTraits::size_type p = typeStrTraits::find_first_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				str_ = typeStrTraits::substr(str_, p, typeStrTraits::npos);
			return *this;
		}
		typeStr		trim_right(const typeConstStr & chSet) const {
			typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), typeStrTraits::npos);
			if (p != typeStrTraits::npos)
				return typeStrTraits::substr(str_, 0, p + 1);
			return str_;
		}
		string_ext &		trim_right_inplace(const typeConstStr & chSet) {
			typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), typeStrTraits::npos);
			if (p != typeStrTraits::npos)
				str_ = typeStrTraits::substr(str_, 0, p + 1);
			return *this;
		}

		typeStr		trim(const typeConstStr & chSet) const {
			typeStr res = trim_left(chSet);
			string_ext	ext(res);
			ext.trim_right_inplace(chSet);
			return res;
		}
		string_ext &		trim_inplace(const typeConstStr & chSet) {
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

				static const char * Number_Low = "0123456789abcdef";
				static const char * Number_Up  = "0123456789ABCDEF";
				typedef typename typeStrTraits::value_type	typeCh;
				bool boNegative = false;
				const char * Number = boLowCase ? Number_Low : Number_Up;

				if (t == 0) {
					typeStrTraits::append(str_, 1, static_cast<typeCh>(Number[0]));
					return;
				} else if (std::is_signed<T>::value && t < 0) {
					boNegative = true;
					t = -t;
				}
				
				const	size_t	bufsize = 72;
				typeCh	buf[bufsize];
				typeCh * p = buf + bufsize;
				while (t) {
					*(--p) = static_cast<typeCh>(Number[t % static_cast<T>(base)]);
					t /= static_cast<T>(base);
				}
				if (boNegative)
					*(--p) = static_cast<typeCh>('-');
				typeStrTraits::append(str_, p, buf + bufsize - p);
				return *this;
			}

	protected:
		typeStr	&	str_;
	};

	template<class typeStr>
	string_ext<typeStr>	strext(typeStr & s) {
		return string_ext<typeStr>(s);
	}

}

#endif // !ARA_STRINGEXT_H

