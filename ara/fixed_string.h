

#ifndef ARA_BASE_FIXEDSTRING_H
#define ARA_BASE_FIXEDSTRING_H

#include "ara_def.h"

#include "internal/string_traits.h"
#include <map>
#include <functional>
#include <algorithm>
#include <stdexcept>

namespace ara {

	template<typename Ch, typename chTraits = std::char_traits<Ch> >
	class fixed_string_base
	{
	public:
		typedef size_t			size_type;
		typedef size_t			difference_type;
		typedef Ch				value_type;
		typedef const Ch *		const_pointer;
		typedef const Ch &		const_reference;
		typedef const Ch *		const_iterator;
		typedef Ch *			iterator;
		typedef Ch *			pointer;
		typedef Ch &			reference;
		typedef chTraits		traits_type;

		fixed_string_base(Ch * p, size_type nSize) : ptr_data_(p), ptr_data_end_(p), ptr_buf_end_(p + nSize) {}
		fixed_string_base(Ch * p, size_type nDataSize, size_type nBufSize) : ptr_data_(p), ptr_data_end_(p + nDataSize), ptr_buf_end_(p + nBufSize) {}
		fixed_string_base(Ch * p, Ch * pEnd) : ptr_data_(p), ptr_data_end_(p), ptr_buf_end_(pEnd) {}
		fixed_string_base(Ch * p, Ch * pDataEnd, Ch * pEnd) : ptr_data_(p), ptr_data_end_(pDataEnd), ptr_buf_end_(pEnd) {}
		fixed_string_base(const fixed_string_base & s) : ptr_data_(s.ptr_data_), ptr_data_end_(s.ptr_data_end_), ptr_buf_end_(s.ptr_buf_end_) {}

		int	compare(const Ch * s, size_type nS2 = npos) const {
			size_type nS1 = size();
			nS2 = (nS2 == npos ? chTraits::length(s) : nS2);
			size_type nCmpSize = nS1 > nS2 ? nS2 : nS1;
			int n = chTraits::compare(ptr_data_, s, nCmpSize);
			if (n != 0)
				return n;
			if (nS1 > nS2)
				return 1;
			else if (nS1 < nS2)
				return -1;
			return 0;
		}

		template<typename typeStr>
		inline int	compare(const typeStr & s, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) const {
			return compare(string_traits<typeStr>::data(s), string_traits<typeStr>::size(s));
		}
		template<typename typeStr>
		inline bool operator==(const typeStr & s) const {
			return compare(s) == 0;
		}
		template<typename typeStr>
		inline bool operator!=(const typeStr & s) const {
			return compare(s) != 0;
		}
		template<typename typeStr>
		inline bool operator<(const typeStr & s) const {
			return compare(s) < 0;
		}
		template<typename typeStr>
		inline bool operator>(const typeStr & s) const {
			return compare(s) > 0;
		}
		template<typename typeStr>
		inline bool operator<=(const typeStr & s) const {
			return compare(s) <= 0;
		}
		template<typename typeStr>
		inline bool operator>=(const typeStr & s) const {
			return compare(s) >= 0;
		}

		inline bool operator==(const Ch * s) const {
			return compare(s, chTraits::length(s)) == 0;
		}
		inline bool operator!=(const Ch * s) const {
			return compare(s, chTraits::length(s)) != 0;
		}
		inline bool operator>=(const Ch * s) const {
			return compare(s, chTraits::length(s)) >= 0;
		}
		inline bool operator<=(const Ch * s) const {
			return compare(s, chTraits::length(s)) <= 0;
		}
		inline bool operator>(const Ch * s) const {
			return compare(s, chTraits::length(s)) > 0;
		}
		inline bool operator<(const Ch * s) const {
			return compare(s, chTraits::length(s)) < 0;
		}

		inline const fixed_string_base & assign(const Ch * pBegin) {
			return assign(pBegin, chTraits::length(pBegin));
		}
		inline const fixed_string_base & assign(const Ch * pBegin, const Ch * pEnd) {
			return assign(pBegin, pEnd - pBegin);
		}
		inline const fixed_string_base & assign(const Ch * pBegin, size_type n) {
			if (n > capacity())
				n = capacity();
			if (n)
				chTraits::copy(ptr_data_, pBegin, n);
			ptr_data_end_ = ptr_data_ + n;
			return *this;
		}
		const fixed_string_base & assign(const fixed_string_base & s) {
			return assign(s.data(), s.size());
		}
		template<typename typeStr>
		inline const fixed_string_base & assign(const typeStr & s, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) {
			return assign(string_traits<typeStr>::data(s), string_traits<typeStr>::size(s));
		}
		template<typename typeStr>
		inline const fixed_string_base & operator=(const typeStr & s) {
			return assign(s);
		}
		inline const fixed_string_base & operator=(const Ch * s) {
			return assign(s);
		}

		inline const fixed_string_base & reset(Ch * pBegin, Ch * pDataEnd, Ch * pEnd) {
			ptr_data_ = pBegin;
			ptr_data_end_ = pDataEnd;
			ptr_buf_end_ = pEnd;
			return *this;
		}

		inline size_type		size(void) const {
			return ptr_data_end_ - ptr_data_;
		}
		inline size_type		length(void) const {
			return ptr_data_end_ - ptr_data_;
		}
		inline bool		empty(void) const {
			return ptr_data_ == ptr_data_end_;
		}
		inline const Ch *	data(void) const {
			return ptr_data_;
		}
		inline Ch			operator[](size_type nIndex) const {
			return ptr_data_[nIndex];
		}
		inline Ch			at(size_type nIndex) const {
			if (nIndex >= size())
				throw std::out_of_range("fixed_string");
			return ptr_data_[nIndex];
		}
		inline const_iterator	begin(void) const {
			return ptr_data_;
		}
		inline const_iterator	end(void) const {
			return ptr_data_end_;
		}
		inline size_type		capacity(void) const {
			return ptr_buf_end_ - ptr_data_;
		}
		inline iterator	begin(void) {
			return ptr_data_;
		}
		iterator	end(void) {
			return ptr_data_end_;
		}

		fixed_string_base	substr(size_type nOff, size_type nC = npos) const {
			size_type nMaxSize = size();
			if (nOff > nMaxSize)
				nOff = nMaxSize;
			if (nC > nMaxSize || nOff + nC > nMaxSize)
				nC = nMaxSize - nOff;
			return fixed_string_base(ptr_data_ + nOff, ptr_data_ + nOff + nC, ptr_buf_end_);
		}

		inline fixed_string_base & erase(size_type nOff, size_type nC = npos) {
			size_type nSize = size();
			if (nOff > nSize)
				nOff = nSize;
			if (nC > nSize || nC + nOff > nSize)
				nC = nSize - nOff;
			size_type nRest = nSize - nOff - nC;
			if (nRest)
				chTraits::move(ptr_data_ + nOff, ptr_data_ + nOff + nC, nRest);
			ptr_data_end_ = ptr_data_ + nOff + nRest;
			return *this;
		}

		inline fixed_string_base	substring(int nBegin, int nEnd = -1) const {
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

			return fixed_string_base(ptr_data_ + nBegin, ptr_data_ + nEnd, ptr_buf_end_);
		}

		inline size_type		find(Ch ch, size_type nOff = 0) const {
			size_type nMySize = size();
			if (nOff == npos || nOff > nMySize)
				return npos;
			const Ch * p = chTraits::find(ptr_data_ + nOff, nMySize - nOff, ch);
			return p == 0 ? npos : p - ptr_data_;
		}

		size_type		find(const Ch * s, size_type nOff = 0, size_type nSize = npos) const {
			size_type	nMysize = size();
			if (nSize == npos)
				nSize = chTraits::length(s);
			if (nSize == 0 && nOff <= nMysize)	// null string always matches (if inside string)
				return (nOff);

			size_type Nm;
			if (nOff < nMysize && nSize <= (Nm = nMysize - nOff)) {
				const Ch * pUptr, *pVptr;
				size_type	nCheckSize = (nSize - 1);
				for (Nm -= nCheckSize, pVptr = ptr_data_ + nOff;
					(pUptr = chTraits::find(pVptr, Nm, *s)) != 0;
					Nm -= pUptr - pVptr + 1, pVptr = pUptr + 1)
					if (chTraits::compare(pUptr + 1, s + 1, nCheckSize) == 0)
						return (pUptr - ptr_data_);
			}
			return (npos);	// no match
		}
		template<typename typeStr>
		inline size_type		find(const typeStr & s, size_type nOff = 0, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) const {
			return find(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		rfind(Ch ch, size_type nOff = npos) const {
			if (nOff == 0)
				return npos;
			size_type nMaxSize = size();
			const Ch * p = ptr_data_ + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > ptr_data_)
				if (*(--p) == ch)
					return p - ptr_data_;
			return npos;
		}

		size_type		rfind(const Ch * s, size_type nOff = npos, size_type nSize = npos) const {
			size_type nMaxSize = size();
			if (s == nullptr || *s == 0)
				return nOff < nMaxSize ? nOff : npos;
			if (nSize == npos)
				nSize = chTraits::length(s);

			if (nSize <= nMaxSize) {
				const Ch * p = ptr_data_ + (nOff < nMaxSize - nSize ? nOff : nMaxSize - nSize);
				size_type	nCheckSize = (nSize - 1);

				for (;; --p)
					if (*p == *s && chTraits::compare(p + 1, s + 1, nCheckSize) == 0)
						return p - ptr_data_;
					else if (p == ptr_data_)
						break;
			}

			return (npos);	// no match
		}
		template<typename typeStr>
		inline size_type		rfind(const typeStr & s, size_type nOff = npos, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) const {
			return rfind(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_first_of(const Ch * s, size_type nOff = 0, size_type nSize = npos) const {
			if (nOff >= size())
				return npos;
			if (nSize == npos)
				nSize = chTraits::length(s);
			const Ch * p = ptr_data_ + nOff;
			for (; p < ptr_data_end_; ++p)
				if (chTraits::find(s, nSize, *p) != 0)
					return p - ptr_data_;
			return npos;
		}
		template<typename typeStr>
		inline size_type		find_first_of(const typeStr & s, size_type nOff = 0, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) const {
			return find_first_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_first_not_of(const Ch * s, size_type nOff = 0, size_type nSize = npos) const {
			if (nOff >= size())
				return npos;
			if (nSize == npos)
				nSize = chTraits::length(s);
			const Ch * p = ptr_data_ + nOff;
			for (; p < ptr_data_end_; ++p)
				if (chTraits::find(s, nSize, *p) == 0)
					return p - ptr_data_;
			return npos;
		}
		template<typename typeStr>
		inline size_type		find_first_not_of(const typeStr & s, size_type nOff = 0, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) const {
			return find_first_not_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_last_of(const Ch * s, size_type nOff = npos, size_type nSize = npos) const {
			if (nOff == 0)
				return npos;
			if (nSize == npos)
				nSize = chTraits::length(s);
			size_type nMaxSize = size();
			const Ch * p = ptr_data_ + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > ptr_data_)
				if (chTraits::find(s, nSize, *(--p)) != 0)
					return p - ptr_data_;
			return npos;
		}
		template<typename typeStr>
		inline size_type		find_last_of(const typeStr & s, size_type nOff = npos, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) const {
			return find_last_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_last_not_of(const Ch * s, size_type nOff = npos, size_type nSize = npos) const {
			if (nOff == 0)
				return npos;
			if (nSize == npos)
				nSize = chTraits::length(s);
			size_type nMaxSize = size();
			const Ch * p = ptr_data_ + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > ptr_data_)
				if (chTraits::find(s, nSize, *(--p)) == 0)
					return p - ptr_data_;
			return npos;
		}
		template<typename typeStr>
		inline size_type		find_last_not_of(const typeStr & s, size_type nOff = npos, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) const {
			return find_last_not_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		inline std::basic_string<value_type, chTraits> str() const {
			return std::basic_string<value_type, chTraits>(data(), size());
		}
		template<typename typeString>
		inline typeString  as() const {
			return string_traits<typeString>::make(data(), size());
		}

		inline void	swap(fixed_string_base & s) {
			std::swap(ptr_data_, s.ptr_data_);
			std::swap(ptr_data_end_, s.ptr_data_end_);
			std::swap(ptr_buf_end_, s.ptr_buf_end_);
		}

		inline void	clear(void) {
			ptr_data_end_ = ptr_data_;
		}


		inline fixed_string_base & append(Ch ch) {
			if (ptr_data_end_ != ptr_buf_end_)
				*(ptr_data_end_++) = ch;
			return *this;
		}
		fixed_string_base & append(size_type count, Ch ch) {
			size_type n = std::min<size_type>(count, capacity() - size());
			for (size_type i = 0; i < n; ++i)
				*(ptr_data_end_++) = ch;
			return *this;
		}
		inline fixed_string_base & append(const Ch * ch) {
			return append(ch, chTraits::length(ch));
		}
		inline fixed_string_base & append(const Ch * ch, size_type n) {
			size_type nCopy = std::min<size_type>(n, capacity() - size());
			if (nCopy) {
				chTraits::copy(ptr_data_end_, ch, nCopy);
				ptr_data_end_ += nCopy;
			}
			return *this;
		}
		template<typename typeStr>
		inline fixed_string_base & append(const typeStr & s, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) {
			size_type nCopy = std::min<size_type>(string_traits<typeStr>::size(s), capacity() - size());
			if (nCopy) {
				chTraits::move(ptr_data_end_, string_traits<typeStr>::data(s), nCopy);
				ptr_data_end_ += nCopy;
			}
			return *this;
		}

		inline fixed_string_base & operator+=(Ch ch) {
			return append(ch);
		}
		inline fixed_string_base & operator+=(const Ch * ch) {
			return append(ch);
		}
		template<typename typeStr>
		inline fixed_string_base & operator+=(const typeStr & s) {
			return append(s);
		}

		fixed_string_base & insert(size_type nWhere, const Ch * s, size_type count) {

			size_type nSize = size();
			size_type nMaxSize = capacity();

			if (nWhere > nSize)
				nWhere = nSize;
			size_type nToMove = nSize - nWhere;

			if (nWhere + count > nMaxSize)
				count = nMaxSize - nWhere;

			if (nWhere + count + nToMove > nMaxSize)
				nToMove = nMaxSize - nWhere - count;

			if (nToMove)
				chTraits::move(ptr_data_ + nWhere + count, ptr_data_ + nWhere, nToMove);
			if (count)
				chTraits::copy(ptr_data_ + nWhere, s, count);

			ptr_data_end_ = ptr_data_ + nWhere + count + nToMove;
			return (*this);
		}

		fixed_string_base & insert(size_type nWhere, const Ch * s) {
			return insert(nWhere, s, chTraits::length(s));
		}

		template<typename typeStr>
		fixed_string_base & insert(size_type nWhere, const typeStr & srcStr, size_type off = 0, size_type count = npos
			, typename std::enable_if<is_string<typeStr>::value>::type * = nullptr) {
			size_type nSrcSize = string_traits<typeStr>::size(srcStr);
			if (off > nSrcSize)
				off = nSrcSize;
			if (count > nSrcSize || off + count > nSrcSize)
				count = nSrcSize - off;
			return insert(nWhere, string_traits<typeStr>::data(srcStr) + off, count);
		}

		void	reserve(size_type) {}

		void	resize(size_type n, Ch ch = 0) {
			if (n > capacity())
				n = capacity();
			if (n < size())
				ptr_data_end_ = ptr_data_ + n;
			else
				append(n - size(), ch);
		}

		const Ch *	c_str() {
			if (ptr_data_end_ == ptr_buf_end_)
				*(ptr_data_end_ - 1) = Ch();
			else
				*(ptr_data_end_) = Ch();
			return ptr_data_;
		}

		static size_type	npos;
	private:
		Ch *		ptr_data_;
		Ch *		ptr_data_end_;
		Ch *		ptr_buf_end_;
	};

	//////////////////////////////////////////////////////////////////////////

	template<typename Ch, typename chTraits>
	typename fixed_string_base<Ch, chTraits>::size_type	fixed_string_base<Ch, chTraits>::npos = static_cast<typename fixed_string_base<Ch, chTraits>::size_type>(-1);

	template<typename Ch, typename chTraits, typename chTraits2>
	std::basic_ostream<Ch, chTraits> & operator<<(std::basic_ostream<Ch, chTraits> & o, const fixed_string_base<Ch, chTraits2> & s)
	{
		typename fixed_string_base<Ch, chTraits2>::const_iterator it(s.begin()), itEnd(s.end());
		for (; it != itEnd; ++it)
			o << (Ch)(*it);
		return o;
	}

	typedef fixed_string_base<char, std::char_traits<char> >			fixed_string;
	typedef fixed_string_base<char16_t, std::char_traits<char16_t> >	fixed_u16string;
	typedef fixed_string_base<char32_t, std::char_traits<char32_t> >	fixed_u32string;
	typedef fixed_string_base<wchar_t, std::char_traits<wchar_t> >		fixed_wstring;

	template<typename Ch, typename chTraits>
	struct is_string<fixed_string_base<Ch, chTraits>> : public std::true_type {};

}//namespace ara {

#endif //ARA_BASE_FIXEDSTRING_H

