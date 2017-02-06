

#ifndef ARA_BASE_FIXEDSTRING_H
#define ARA_BASE_FIXEDSTRING_H

#include "ara_def.h"

#include "internal/string_traits.h"
#include <map>
#include <functional>
#include <algorithm>

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

		fixed_string_base(Ch * p, size_type nSize) : m_pCh(p), m_pChEnd(p), m_pChBufEnd(p + nSize) {}
		fixed_string_base(Ch * p, Ch * pEnd) : m_pCh(p), m_pChEnd(p), m_pChBufEnd(pEnd) {}
		fixed_string_base(Ch * p, Ch * pDataEnd, Ch * pEnd) : m_pCh(p), m_pChEnd(pDataEnd), m_pChBufEnd(pEnd) {}
		fixed_string_base(const fixed_string_base & s) : m_pCh(s.m_pCh), m_pChEnd(s.m_pChEnd), m_pChBufEnd(s.m_pChBufEnd) {}

		int	compare(const Ch * s, size_type nS2) const {
			size_type nS1 = size();
			size_type nCmpSize = nS1 > nS2 ? nS2 : nS1;
			int n = chTraits::compare(m_pCh, s, nCmpSize);
			if (n != 0)
				return n;
			if (nS1 > nS2)
				return 1;
			else if (nS1 < nS2)
				return -1;
			return 0;
		}

		template<typename typeStr>
		inline int	compare(const typeStr & s) const {
			return compare(s.data(), s.size());
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
				chTraits::copy(m_pCh, pBegin, n);
			m_pChEnd = m_pCh + n;
			return *this;
		}

		template<typename typeStr>
		inline const fixed_string_base & operator=(const typeStr & s) {
			return assign(s.data(), s.size());
		}
		inline const fixed_string_base & operator=(const fixed_string_base & s) {
			if (&s != this)
				return assign(s.data(), s.size());
			return *this;
		}
		inline const fixed_string_base & operator=(const Ch * s) {
			return assign(s);
		}

		inline const fixed_string_base & reset(const Ch * pBegin, const Ch * pDataEnd, const Ch * pEnd) {
			m_pCh = pBegin;
			m_pChEnd = pDataEnd;
			m_pChBufEnd = pEnd;
			return *this;
		}

		inline size_type		size(void) const {
			return m_pChEnd - m_pCh;
		}
		inline size_type		length(void) const {
			return m_pChEnd - m_pCh;
		}
		inline bool		empty(void) const {
			return m_pCh == m_pChEnd;
		}
		inline const Ch *	data(void) const {
			return m_pCh;
		}
		inline Ch			operator[](size_type nIndex) const {
			return m_pCh[nIndex];
		}
		inline Ch			at(size_type nIndex) const {
			return m_pCh[nIndex];
		}
		inline const_iterator	begin(void) const {
			return m_pCh;
		}
		inline const_iterator	end(void) const {
			return m_pChEnd;
		}
		inline size_type		capacity(void) const {
			return m_pChBufEnd - m_pCh;
		}
		inline iterator	begin(void) {
			return m_pCh;
		}
		iterator	end(void) {
			return m_pChEnd;
		}

		fixed_string_base	substr(size_type nOff, size_type nC = npos) const {
			size_type nMaxSize = size();
			if (nOff > nMaxSize)
				nOff = nMaxSize;
			if (nC > nMaxSize || nOff + nC > nMaxSize)
				nC = nMaxSize - nOff;
			return fixed_string_base(m_pCh + nOff, m_pCh + nOff + nC, m_pChBufEnd);
		}

		inline fixed_string_base & erase(size_type nOff, size_type nC = npos) {
			size_type nSize = size();
			if (nOff > nSize)
				nOff = nSize;
			if (nC > nSize || nC + nOff > nSize)
				nC = nSize - nOff;
			size_type nRest = nSize - nOff - nC;
			if (nRest)
				chTraits::move(m_pCh + nOff, m_pCh + nOff + nC, nRest);
			m_pChEnd = m_pCh + nOff + nRest;
			return *this;
		}

		inline fixed_string_base	substring(int nBegin, int nEnd = -1) const {
			int nMaxSize = static_cast<int>(size());
			if (nBegin > nMaxSize)
				nBegin = nMaxSize;
			else if (nBegin < 0) {
				if (-nBegin > nMaxSize)
					nBegin = 0;
				else
					nBegin = nMaxSize + nBegin;
			}
			if (nEnd >= nMaxSize)
				nEnd = nMaxSize;
			else if (nEnd < 0) {
				if (-nEnd > nMaxSize)
					nEnd = 0;
				else
					nEnd = nMaxSize + nEnd;
			}
			if (nEnd < nBegin)
				nEnd = nBegin;

			return fixed_string_base(m_pCh + nBegin, m_pCh + nEnd, m_pChBufEnd);
		}

		inline size_type		find(Ch ch, size_type nOff = 0) const {
			size_type nMySize = size();
			if (nOff == npos || nOff > nMySize)
				return npos;
			const Ch * p = chTraits::find(m_pCh + nOff, nMySize - nOff, ch);
			return p == 0 ? npos : p - m_pCh;
		}

		size_type		find(const Ch * s, size_type nOff, size_type nSize) const {
			size_type	nMysize = size();
			if (nSize == 0 && nOff <= nMysize)	// null string always matches (if inside string)
				return (nOff);

			size_type Nm;
			if (nOff < nMysize && nSize <= (Nm = nMysize - nOff)) {
				const Ch * pUptr, *pVptr;
				size_type	nCheckSize = (nSize - 1);
				for (Nm -= nCheckSize, pVptr = m_pCh + nOff;
					(pUptr = chTraits::find(pVptr, Nm, *s)) != 0;
					Nm -= pUptr - pVptr + 1, pVptr = pUptr + 1)
					if (chTraits::compare(pUptr + 1, s + 1, nCheckSize) == 0)
						return (pUptr - m_pCh);
			}
			return (npos);	// no match
		}
		inline size_type		find(const fixed_string_base & s, size_type nOff = 0) const {
			return find(s.data(), nOff, s.size());
		}

		size_type		rfind(Ch ch, size_type nOff = npos) const {
			if (nOff == 0)
				return npos;
			size_type nMaxSize = size();
			const Ch * p = m_pCh + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > m_pCh)
				if (*(--p) == ch)
					return p - m_pCh;
			return npos;
		}

		size_type		rfind(const Ch * s, size_type nOff, size_type nSize) const {
			size_type nMaxSize = size();
			if (s == nullptr || *s == 0)
				return nOff < nMaxSize ? nOff : nMaxSize;

			if (nSize <= nMaxSize) {
				const Ch * p = m_pCh + (nOff < nMaxSize - nSize ? nOff : nMaxSize - nSize);
				size_type	nCheckSize = (nSize - 1);

				for (;; --p)
					if (*p == *s && chTraits::compare(p + 1, s + 1, nCheckSize) == 0)
						return p - m_pCh;
					else if (p == m_pCh)
						break;
			}

			return (npos);	// no match
		}
		inline size_type		rfind(const fixed_string_base & s, size_type nOff = 0) const {
			return rfind(s.data(), nOff, s.size());
		}

		size_type		find_first_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == npos)
				return npos;
			const Ch * p = m_pCh + nOff;
			for (; p < m_pChEnd; ++p)
				if (chTraits::find(s, nSize, *p) != 0)
					return p - m_pCh;
			return npos;
		}
		inline size_type		find_first_of(const fixed_string_base & s, size_type nOff = 0) const {
			return find_first_of(s.data(), nOff, s.size());
		}

		size_type		find_first_not_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == npos)
				return npos;
			const Ch * p = m_pCh + nOff;
			for (; p < m_pChEnd; ++p)
				if (chTraits::find(s, nSize, *p) == 0)
					return p - m_pCh;
			return npos;
		}
		inline size_type		find_first_not_of(const fixed_string_base & s, size_type nOff = 0) const {
			return find_first_not_of(s.data(), nOff, s.size());
		}

		size_type		find_last_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == 0)
				return npos;
			size_type nMaxSize = size();
			const Ch * p = m_pCh + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > m_pCh)
				if (chTraits::find(s, nSize, *(--p)) != 0)
					return p - m_pCh;
			return npos;
		}
		inline size_type		find_last_of(const fixed_string_base & s, size_type nOff = npos) const {
			return find_last_of(s.data(), nOff, s.size());
		}

		size_type		find_last_not_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == 0)
				return npos;
			size_type nMaxSize = size();
			const Ch * p = m_pCh + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > m_pCh)
				if (chTraits::find(s, nSize, *(--p)) == 0)
					return p - m_pCh;
			return npos;
		}
		inline size_type		find_last_not_of(const fixed_string_base & s, size_type nOff = npos) const {
			return find_last_not_of(s.data(), nOff, s.size());
		}

		inline std::basic_string<value_type> str() const {
			return std::basic_string<value_type>(data(), size());
		}
		template<typename typeString>
		inline typeString  as() const {
			return typeString(data(), size());
		}

		inline void	swap(const fixed_string_base & s) {
			std::swap(m_pCh, s.m_pCh);
			std::swap(m_pChEnd, s.m_pChEnd);
			std::swap(m_pChBufEnd, s.m_pChBufEnd);
		}

		inline void	clear(void) {
			m_pCh = m_pChEnd = m_pChBufEnd = nullptr;
		}


		inline fixed_string_base & append(Ch ch) {
			if (m_pChEnd != m_pChBufEnd)
				*(m_pChEnd++) = ch;
			return *this;
		}
		fixed_string_base & append(size_type count, Ch ch) {
			size_type n = std::min<size_type>( count, capacity() - size());
			for (size_type i = 0;i < n; ++i)
				*(m_pChEnd++) = ch;
			return *this;
		}
		inline fixed_string_base & append(const Ch * ch) {
			return append(ch, chTraits::length(ch));
		}
		inline fixed_string_base & append(const Ch * ch, size_type n) {
			size_type nCopy = std::min<size_type>(n, capacity() - size());
			if (nCopy) {
				chTraits::copy(m_pChEnd, ch, nCopy);
				m_pChEnd += nCopy;
			}
			return *this;
		}
		template<typename typeStr>
		inline fixed_string_base & append(const typeStr & s) {
			size_type nCopy = std::min<size_type>(s.size(), capacity() - size());
			if (nCopy) {
				chTraits::move(m_pChEnd, s.data(), nCopy);
				m_pChEnd += nCopy;
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
				chTraits::move(m_pCh + nWhere + count, m_pCh + nWhere, nToMove);
			if (count)
				chTraits::copy(m_pCh + nWhere, s, count);

			m_pChEnd = m_pCh + nWhere + count + nToMove;
			return (*this);
		}

		fixed_string_base & insert(size_type nWhere, const Ch * s) {
			return insert(nWhere, s, chTraits::length(s));
		}

		template<typename typeStr>
		fixed_string_base & insert(size_type nWhere, const typeStr & srcStr, size_type off = 0, size_type count = npos) {
			size_type nSrcSize = srcStr.size();
			if (off > nSrcSize)
				off = nSrcSize;
			if (count > nSrcSize || off + count > nSrcSize)
				count = nSrcSize - off;
			return insert(nWhere, srcStr.data() + off, count);
		}

		void	reserve(size_type) {}

		void	resize(size_type n, Ch ch = 0) {
			if (n > capacity())
				n = capacity();
			if (n < size())
				m_pChEnd = m_pCh + n;
			else
				append(n - size(), ch);
		}

		static size_type	npos;
	private:
		Ch *		m_pCh;
		Ch *		m_pChEnd;
		Ch *		m_pChBufEnd;
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

