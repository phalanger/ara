
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
			typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				return typeStrTraits::substr(str_, 0, p + 1);
			return str_;
		}
		string_ext &		trim_right_inplace(const typeConstStr & chSet) {
			typeStrTraits::size_type p = typeStrTraits::find_last_not_of(str_, chSet.data(), chSet.size(), 0);
			if (p != typeStrTraits::npos)
				str_ = typeStrTraits::substr(str_, 0, p + 1);
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

