
/*
	std::string strReal = "Hello world";
	ara::ref_string strRef = strReal;
*/

#ifndef ARA_BASE_REFSTRING_H
#define ARA_BASE_REFSTRING_H

#include "ara_def.h"

#include "internal/string_traits.h"
#include <map>
#include <functional>

namespace ara {

	template<typename Ch, typename chTraits = std::char_traits<Ch> >
	class ref_string_base
	{
	public:
		typedef size_t			size_type;
		typedef size_t			difference_type;
		typedef const Ch *		const_pointer;
		typedef const Ch &		const_reference;
		typedef Ch				value_type;
		typedef const Ch *		const_iterator;
		typedef const Ch *		iterator;
		typedef chTraits		traits_type;

		ref_string_base(void) : ptr_data_(&g_nCh), ptr_end_(&g_nCh) {}
		ref_string_base(const Ch * p, size_t nSize) : ptr_data_(p), ptr_end_(p + nSize) {}
		ref_string_base(const Ch * p, const Ch * pEnd) : ptr_data_(p), ptr_end_(pEnd) {}
		ref_string_base(const ref_string_base & s) : ptr_data_(s.ptr_data_), ptr_end_(s.ptr_end_) {}
		ref_string_base(const Ch * p) : ptr_data_(p), ptr_end_(p) {
			ptr_end_ += chTraits::length(p);
		}
		template<typename typeStr>
		explicit ref_string_base(const typeStr & s) :
			ptr_data_(string_traits<typeStr>::data(s)), ptr_end_(ptr_data_ + string_traits<typeStr>::size(s)) {}

		template<typename typeStr>
		ref_string_base(const typeStr & s, size_t off, size_t nCount = static_cast<size_t>(-1)) :
			ptr_data_(string_traits<typeStr>::data(s) + off), ptr_end_(ptr_data_ + std::min<size_t>(nCount, string_traits<typeStr>::size(s) - off)) {}

		int	compare(size_type pos1, size_type count1, const value_type * s2, size_type count2) const {

			size_t nMaxSize = size();
			if (pos1 > nMaxSize)
				pos1 = nMaxSize;
			if (count1 == npos || pos1 + count1 >= nMaxSize)
				count1 = nMaxSize - pos1;
			if (count2 == static_cast<size_t>(-1))
				count2 = chTraits::length(s2);

			size_t nS1 = count1;
			size_t nS2 = count2;
			size_t nCmpSize = nS1 > nS2 ? nS2 : nS1;
			int n = chTraits::compare(ptr_data_ + pos1, s2, nCmpSize);
			if (n != 0)
				return n;
			if (nS1 > nS2)
				return 1;
			else if (nS1 < nS2)
				return -1;
			return 0;
		}

		int	compare(const ref_string_base & s) const {
			size_t nS1 = size();
			size_t nS2 = s.size();
			size_t nCmpSize = nS1 > nS2 ? nS2 : nS1;
			int n = chTraits::compare(ptr_data_, s.ptr_data_, nCmpSize);
			if (n != 0)
				return n;
			if (nS1 > nS2)
				return 1;
			else if (nS1 < nS2)
				return -1;
			return 0;
		}

		bool operator==(const ref_string_base & s) const {
			return compare(s) == 0;
		}
		bool operator!=(const ref_string_base & s) const {
			return compare(s) != 0;
		}
		bool operator<(const ref_string_base & s) const {
			return compare(s) < 0;
		}
		bool operator>(const ref_string_base & s) const {
			return compare(s) > 0;
		}
		bool operator<=(const ref_string_base & s) const {
			return compare(s) <= 0;
		}
		bool operator>=(const ref_string_base & s) const {
			return compare(s) >= 0;
		}

		const ref_string_base & operator=(const ref_string_base & s) {
			if (this != &s) {
				ptr_data_ = s.ptr_data_;
				ptr_end_ = s.ptr_end_;
			}
			return *this;
		}

		const ref_string_base & assign(const ref_string_base & s) {
			return *this = s;
		}

		const ref_string_base & assign(const Ch * pBegin, const Ch * pEnd) {
			ptr_data_ = pBegin;
			ptr_end_ = pEnd;
			return *this;
		}

		size_t		size(void) const {
			return ptr_end_ - ptr_data_;
		}
		size_t		length(void) const {
			return ptr_end_ - ptr_data_;
		}
		bool		empty(void) const {
			return ptr_data_ == ptr_end_;
		}
		const Ch *	data(void) const {
			return ptr_data_;
		}
		Ch			operator[](size_t nIndex) const {
			return ptr_data_[nIndex];
		}
		Ch			at(size_t nIndex) const {
			return ptr_data_[nIndex];
		}
		const_iterator	begin(void) const {
			return ptr_data_;
		}
		const_iterator	end(void) const {
			return ptr_end_;
		}

		ref_string_base	substr(size_t nOff, size_t nC = npos) const {
			size_t nMaxSize = size();
			if (nOff > nMaxSize)
				nOff = nMaxSize;
			if (nC == npos || nOff + nC > nMaxSize)
				nC = nMaxSize - nOff;
			return ref_string_base(ptr_data_ + nOff, ptr_data_ + nOff + nC);
		}

		ref_string_base & erase(size_t nOff, size_t nC = npos) {
			if (nOff != 0 && nC != npos)
				throw std::bad_function_call();
			else if (nOff == 0) {
				if (nC < size())
					ptr_data_ += nC;
				else
					clear();
			}
			else if (nOff < size())
				ptr_end_ = ptr_data_ + nOff;
			return *this;
		}

		ref_string_base	substring(int nBegin, int nEnd = -1) const {
			int nMaxSize = static_cast<int>(size());
			if (nBegin > nMaxSize)
				nBegin = nMaxSize;
			else if (nBegin < 0) {
				if (-nBegin > nMaxSize)
					nBegin = nMaxSize;
				else
					nBegin = nMaxSize + nBegin;
			}
			if (nEnd >= nMaxSize)
				nEnd = nMaxSize;
			else if (nEnd < 0) {
				if (-nEnd > nMaxSize)
					nEnd = 0;
				else
					nEnd = nMaxSize + nEnd + 1;
			}
			if (nEnd < nBegin)
				nEnd = nBegin;

			return ref_string_base(ptr_data_ + nBegin, ptr_data_ + nEnd);
		}

		size_t		find(Ch ch, size_t nOff = 0) const {
			size_t nMySize = size();
			if (nOff == npos || nOff > nMySize)
				return npos;
			const Ch * p = chTraits::find(ptr_data_ + nOff, nMySize - nOff, ch);
			return p == 0 ? npos : p - ptr_data_;
		}

		size_t		find(const Ch * s, size_t nOff, size_t nSize) const {
			size_t	nMysize = size();
			if (nSize == 0 && nOff <= nMysize)	// null string always matches (if inside string)
				return (nOff);

			size_type Nm;
			if (nOff < nMysize && nSize <= (Nm = nMysize - nOff)) {
				const Ch * pUptr, *pVptr;
				size_t	nCheckSize = (nSize - 1);
				for (Nm -= nCheckSize, pVptr = ptr_data_ + nOff;
					(pUptr = chTraits::find(pVptr, Nm, *s)) != 0;
					Nm -= pUptr - pVptr + 1, pVptr = pUptr + 1)
					if (chTraits::compare(pUptr + 1, s + 1, nCheckSize) == 0)
						return (pUptr - ptr_data_);
			}
			return (npos);	// no match
		}
		size_t		find(const ref_string_base & s, size_t nOff = 0) const {
			return find(s.data(), nOff, s.size());
		}

		size_t		rfind(Ch ch, size_t nOff = npos) const {
			if (nOff == 0)
				return npos;
			size_t nMaxSize = size();
			const Ch * p = ptr_data_ + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > ptr_data_)
				if (*(--p) == ch)
					return p - ptr_data_;
			return npos;
		}

		size_t		rfind(const Ch * s, size_t nOff, size_t nSize) const {
			size_t nMaxSize = size();
			if (s == nullptr || *s == 0)
				return nOff < nMaxSize ? nOff : nMaxSize;

			if (nSize <= nMaxSize) {
				const Ch * p = ptr_data_ + (nOff < nMaxSize - nSize ? nOff : nMaxSize - nSize);
				size_t	nCheckSize = (nSize - 1);

				for (;; --p)
					if (*p == *s && chTraits::compare(p + 1, s + 1, nCheckSize) == 0)
						return p - ptr_data_;
					else if (p == ptr_data_)
						break;
			}

			return (npos);	// no match
		}
		size_t		rfind(const ref_string_base & s, size_t nOff = npos) const {
			return rfind(s.data(), nOff, s.size());
		}

		size_t		find_first_of(const Ch * s, size_t nOff, size_t nSize) const {
			if (nOff == npos)
				return npos;
			const Ch * p = ptr_data_ + nOff;
			for (; p < ptr_end_; ++p)
				if (chTraits::find(s, nSize, *p) != 0)
					return p - ptr_data_;
			return npos;
		}
		size_t		find_first_of(const ref_string_base & s, size_t nOff = 0) const {
			return find_first_of(s.data(), nOff, s.size());
		}

		size_t		find_first_not_of(const Ch * s, size_t nOff, size_t nSize) const {
			if (nOff == npos)
				return npos;
			const Ch * p = ptr_data_ + nOff;
			for (; p < ptr_end_; ++p)
				if (chTraits::find(s, nSize, *p) == 0)
					return p - ptr_data_;
			return npos;
		}
		size_t		find_first_not_of(const ref_string_base & s, size_t nOff = 0) const {
			return find_first_not_of(s.data(), nOff, s.size());
		}

		size_t		find_last_of(const Ch * s, size_t nOff, size_t nSize) const {
			if (nOff == 0)
				return npos;
			size_t nMaxSize = size();
			const Ch * p = ptr_data_ + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > ptr_data_)
				if (chTraits::find(s, nSize, *(--p)) != 0)
					return p - ptr_data_;
			return npos;
		}
		size_t		find_last_of(const ref_string_base & s, size_t nOff = npos) const {
			return find_last_of(s.data(), nOff, s.size());
		}

		size_t		find_last_not_of(const Ch * s, size_t nOff, size_t nSize) const {
			if (nOff == 0)
				return npos;
			size_t nMaxSize = size();
			const Ch * p = ptr_data_ + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > ptr_data_)
				if (chTraits::find(s, nSize, *(--p)) == 0)
					return p - ptr_data_;
			return npos;
		}
		size_t		find_last_not_of(const ref_string_base & s, size_t nOff = npos) const {
			return find_last_not_of(s.data(), nOff, s.size());
		}

		std::basic_string<value_type> str() const {
			return std::basic_string<value_type>(data(), size());
		}
		template<typename typeString>
		typeString  as() const {
			return typeString(data(), size());
		}

		void	swap(const ref_string_base & s) {
			std::swap(ptr_data_, s.ptr_data_);
			std::swap(ptr_end_, s.ptr_end_);
		}

		void	clear(void) {
			ptr_data_ = ptr_end_ = &g_nCh;
		}

		static size_t	npos;
	private:
		static const Ch	g_nCh;
		const Ch *		ptr_data_;
		const Ch *		ptr_end_;
	};

	//////////////////////////////////////////////////////////////////////////

	template<typename Ch, typename chTraits>
	const Ch ref_string_base<Ch, chTraits>::g_nCh = Ch();

	template<typename Ch, typename chTraits>
	size_t	ref_string_base<Ch, chTraits>::npos = static_cast<size_t>(-1);

	template<typename Ch, typename chTraits, typename chTraits2>
	std::basic_ostream<Ch, chTraits> & operator<<(std::basic_ostream<Ch, chTraits> & o, const ref_string_base<Ch, chTraits2> & s)
	{
		typename ref_string_base<Ch, chTraits2>::const_iterator it(s.begin()), itEnd(s.end());
		for (; it != itEnd; ++it)
			o << (Ch)(*it);
		return o;
	}

	template<typename Ch, typename chTraits, typename chTraits2>
	std::basic_string<Ch, chTraits> operator+(const std::basic_string<Ch, chTraits> & s, const ref_string_base<Ch, chTraits2> & s2)
	{
		std::basic_string<Ch, chTraits>  s3 = s;
		s3.append(s2.data(), s2.size());
		return s3;
	}

	template<typename Ch, typename chTraits, typename chTraits2>
	std::basic_string<Ch, chTraits> & operator+=(std::basic_string<Ch, chTraits> & s, const ref_string_base<Ch, chTraits2> & s2)
	{
		return s.append(s2.data(), s2.size());
	}

	typedef ref_string_base<char, std::char_traits<char> >			ref_string;
	typedef ref_string_base<char16_t, std::char_traits<char16_t> >	ref_u16string;
	typedef ref_string_base<char32_t, std::char_traits<char32_t> >	ref_u32string;
	typedef ref_string_base<wchar_t, std::char_traits<wchar_t> >	ref_wstring;

	template<typename Ch, typename chTraits>
	struct is_string<ref_string_base<Ch, chTraits>> : public std::true_type {};

}//namespace ara {

#endif//ARA_BASE_REFSTRING_H
