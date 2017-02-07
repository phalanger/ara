

#ifndef ARA_KEY_STRING_H
#define ARA_KEY_STRING_H

#include "ara_def.h"

#include "internal/string_traits.h"
#include <map>
#include <functional>
#include <atomic>

namespace ara {
	
	namespace internal {
		template<size_t>
		struct detect_int {
			typedef		uint8_t		int_type; 
		};
		template<>
		struct detect_int<4> {
			typedef		uint16_t	int_type;
		};
		template<>
		struct detect_int<8> {
			typedef		uint32_t	int_type;
		};
	}

	template<typename Ch, typename chTraits = std::char_traits<Ch> >
	class key_string_base
	{
	public:
		enum DATA_TYPE {
			TYPE_CONST,
			TYPE_BUF,
			TYPE_STORE,
		};
		typedef size_t			size_type;
		typedef size_t			difference_type;
		typedef const Ch *		const_pointer;
		typedef const Ch &		const_reference;
		typedef Ch				value_type;
		typedef const Ch *		const_iterator;
		typedef const Ch *		iterator;
		typedef chTraits		traits_type;

		void clear() {
			destroy_data();
			ptr_ = &g_nCh;
			type_ = TYPE_CONST;
			len_ = 0;
		}
		~key_string_base() {
			destroy_data();
		}

		key_string_base() : ptr_(&g_nCh), type_(TYPE_CONST), len_(0) {}
		key_string_base(const Ch * p, size_t nSize, bool boCopy = true) { set_data(p, nSize, boCopy); }
		key_string_base(const Ch * p) { set_data(p, chTraits::length(p), false); }
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		explicit key_string_base(const typeStr & s) {
			set_data(string_traits<typeStr>::data(s), string_traits<typeStr>::size(s), true);
		}
		key_string_base(const key_string_base & s) : ptr_(s.ptr_), type_(s.type_), len_(s.len_) {
			if (s.type_ == TYPE_STORE)
				++(store_ptr_->ref_count_);
		}

		static key_string_base copy(const Ch * ch, size_type l = npos) {
			return key_string_base(ch, l == npos ? chTraits::length(ch) : l, true);
		}
		static key_string_base ref(const Ch * ch, size_type l = npos) {
			return key_string_base(ch, l == npos ? chTraits::length(ch) : l, false);
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		static key_string_base copy(const typeStr & s) {
			return key_string_base(string_traits<typeStr>::data(s), string_traits<typeStr>::size(s), true);
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		static key_string_base ref(const typeStr & s) {
			return key_string_base(string_traits<typeStr>::data(s), string_traits<typeStr>::size(s), false);
		}

		int	compare(const Ch * s, size_type nS2) const {
			size_type nS1 = size();
			size_type nCmpSize = nS1 > nS2 ? nS2 : nS1;
			int n = chTraits::compare(data(), s, nCmpSize);
			if (n != 0)
				return n;
			if (nS1 > nS2)
				return 1;
			else if (nS1 < nS2)
				return -1;
			return 0;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline int	compare(const typeStr & s) const {
			return compare(string_traits<typeStr>::data(s), string_traits<typeStr>::size(s));
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline bool operator==(const typeStr & s) const {
			return compare(s) == 0;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline bool operator!=(const typeStr & s) const {
			return compare(s) != 0;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline bool operator<(const typeStr & s) const {
			return compare(s) < 0;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline bool operator>(const typeStr & s) const {
			return compare(s) > 0;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline bool operator<=(const typeStr & s) const {
			return compare(s) <= 0;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
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

		const key_string_base & assign(const key_string_base & s) {
			if (&s != this) {
				destroy_data();
				ptr_ = s.ptr_;
				type_ = s.type_;
				len_ = s.len_;
				if (s.type_ == TYPE_STORE)
					++(store_ptr_->ref_count_);
			}
			return *this;
		}
		const key_string_base & assign(const Ch * c, size_type n = npos, bool boCopy = true) {
			destroy_data();
			set_data(c, n == npos ? chTraits::length(c) : n, boCopy);
			return *this;
		}
		const key_string_base & operator=(const key_string_base & s) {
			return assign(s);
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline const key_string_base & operator=(const typeStr & s) {
			return assign(string_traits<typeStr>::data(s), string_traits<typeStr>::size(s));
		}
		const key_string_base & operator=(const Ch * s) {
			return assign(s, npos, false);
		}

		inline size_type		size(void) const {
			return len_;
		}
		inline size_type		length(void) const {
			return len_;
		}
		inline bool		empty(void) const {
			return len_ == 0;
		}
		const Ch *	data(void) const {
			return type_ == TYPE_BUF ? buf_ : (type_ == TYPE_CONST ? ptr_ : store_ptr_->buf_);
		}
		inline Ch			operator[](size_type nIndex) const {
			return data()[nIndex];
		}
		inline Ch			at(size_type nIndex) const {
			return data()[nIndex];
		}
		inline const_iterator	begin(void) const {
			return data();
		}
		inline const_iterator	end(void) const {
			return data() + size();
		}

		key_string_base	substr(size_t nOff, size_t nC = npos) const {
			size_t nMaxSize = size();
			if (nOff > nMaxSize)
				nOff = nMaxSize;
			if (nC == npos || nOff + nC > nMaxSize)
				nC = nMaxSize - nOff;
			return key_string_base(data() + nOff, nC, true);
		}

		key_string_base	substring(int nBegin, int nEnd = -1) const {
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

			return key_string_base(data() + nBegin, nEnd - nBegin, true);
		}

		size_type		find(Ch ch, size_type nOff = 0) const {
			size_t nMySize = size();
			if (nOff == npos || nOff > nMySize)
				return npos;
			const Ch * buf = data();
			const Ch * p = chTraits::find(buf + nOff, nMySize - nOff, ch);
			return p == 0 ? npos : p - buf;
		}

		size_type		find(const Ch * s, size_type nOff, size_type nSize) const {
			size_t	nMysize = size();
			if (nSize == 0 && nOff <= nMysize)	// null string always matches (if inside string)
				return (nOff);

			size_type Nm;
			if (nOff < nMysize && nSize <= (Nm = nMysize - nOff)) {
				const Ch * pUptr, *pVptr;
				const Ch * buf = data();
				size_type	nCheckSize = (nSize - 1);
				for (Nm -= nCheckSize, pVptr = buf + nOff;
					(pUptr = chTraits::find(pVptr, Nm, *s)) != 0;
					Nm -= pUptr - pVptr + 1, pVptr = pUptr + 1)
					if (chTraits::compare(pUptr + 1, s + 1, nCheckSize) == 0)
						return (pUptr - buf);
			}
			return (npos);	// no match
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline size_type		find(const typeStr & s, size_type nOff = 0) const {
			return find(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		rfind(Ch ch, size_type nOff = npos) const {
			if (nOff == 0)
				return npos;
			size_t nMaxSize = size();
			const Ch * buf = data();
			const Ch * p = buf + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > buf)
				if (*(--p) == ch)
					return p - buf;
			return npos;
		}

		size_type		rfind(const Ch * s, size_type nOff, size_type nSize) const {
			size_t nMaxSize = size();
			if (s == nullptr || *s == 0)
				return nOff < nMaxSize ? nOff : nMaxSize;

			if (nSize <= nMaxSize) {
				const Ch * buf = data();
				const Ch * p = buf + (nOff < nMaxSize - nSize ? nOff : nMaxSize - nSize);
				size_t	nCheckSize = (nSize - 1);

				for (;; --p)
					if (*p == *s && chTraits::compare(p + 1, s + 1, nCheckSize) == 0)
						return p - buf;
					else if (p == buf)
						break;
			}
			return (npos);	// no match
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline size_type		rfind(const typeStr & s, size_type nOff = 0) const {
			return rfind(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_first_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == npos)
				return npos;
			const Ch * buf = data();
			const Ch * buf_end = buf + size();
			const Ch * p = buf + nOff;
			for (; p < buf_end; ++p)
				if (chTraits::find(s, nSize, *p) != 0)
					return p - buf;
			return npos;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline size_type		find_first_of(const typeStr & s, size_type nOff = 0) const {
			return find_first_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_first_not_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == npos)
				return npos;
			const Ch * buf = data();
			const Ch * buf_end = buf + size();
			const Ch * p = buf + nOff;
			for (; p < buf_end; ++p)
				if (chTraits::find(s, nSize, *p) == 0)
					return p - buf;
			return npos;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline size_type		find_first_not_of(const typeStr & s, size_type nOff = 0) const {
			return find_first_not_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_last_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == 0)
				return npos;
			size_type nMaxSize = size();
			const Ch * buf = data();
			const Ch * p = buf + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > buf)
				if (chTraits::find(s, nSize, *(--p)) != 0)
					return p - buf;
			return npos;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline size_type		find_last_of(const typeStr & s, size_type nOff = npos) const {
			return find_last_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		size_type		find_last_not_of(const Ch * s, size_type nOff, size_type nSize) const {
			if (nOff == 0)
				return npos;
			size_type nMaxSize = size();
			const Ch * buf = data();
			const Ch * p = buf + (nOff > nMaxSize ? nMaxSize : nOff);
			while (p > buf)
				if (chTraits::find(s, nSize, *(--p)) == 0)
					return p - buf;
			return npos;
		}
		template<typename typeStr, typename std::enable_if<is_string<typeStr>::value>::type>
		inline size_type		find_last_not_of(const typeStr & s, size_type nOff = npos) const {
			return find_last_not_of(string_traits<typeStr>::data(s), nOff, string_traits<typeStr>::size(s));
		}

		inline std::basic_string<value_type, chTraits> str() const {
			return std::basic_string<value_type, chTraits>(data(), size());
		}
		template<typename typeString>
		inline typeString  as() const {
			return string_traits<typeString>::make(data(), size());
		}

		void	swap(const key_string_base & s) {
			std::swap(ptr_, s.ptr_);
			std::swap(type_, s.type_);
			std::swap(len_, s.len_);
		}

		inline size_type		ref_count() const {
			return (type_ == TYPE_STORE ? static_cast<size_type>(store_ptr_->ref_count_) : 0);
		}
		inline DATA_TYPE		data_type() const {
			return static_cast<DATA_TYPE>(type_);
		}

		static size_type	npos;
	private:
		typedef	 typename internal::detect_int<sizeof(Ch *)>::int_type	int_type;
		static const Ch			g_nCh;

		class _store {
		public:
			std::atomic_int		ref_count_;
			Ch					buf_[0];
		};
		union {
			_store	*		store_ptr_;
			const Ch *		ptr_;
			Ch				buf_[sizeof(Ch *)];
		};
		int_type				type_ = TYPE_CONST;
		int_type				len_ = 0;

		void destroy_data() {
			if (type_ == TYPE_STORE) {
				if (--store_ptr_->ref_count_ == 0)
					free(store_ptr_);
				store_ptr_ = nullptr;
			}
		}
		void	set_data(const Ch * data, size_t n, bool copy) {
			len_ = static_cast<int_type>(n);
			if (n * sizeof(Ch) <= sizeof(Ch *)) {
				traits_type::copy(buf_, data, n);
				type_ = TYPE_BUF;
			}
			else if (!copy) {
				ptr_ = data;
				type_ = TYPE_CONST;
			}
			else {
				void * buf = malloc(sizeof(_store) + n * sizeof(Ch));
				store_ptr_ = new(buf)_store;
				traits_type::copy(store_ptr_->buf_, data, n);
				store_ptr_->ref_count_ = 1;
				type_ = TYPE_STORE;
			}
		}
	};

	//////////////////////////////////////////////////////////////////////////

	template<typename Ch, typename chTraits>
	const Ch key_string_base<Ch, chTraits>::g_nCh = Ch();

	template<typename Ch, typename chTraits>
	typename key_string_base<Ch, chTraits>::size_type key_string_base<Ch, chTraits>::npos = static_cast<typename key_string_base<Ch, chTraits>::size_type>(-1);

	template<typename Ch, typename chTraits, typename chTraits2>
	std::basic_ostream<Ch, chTraits> & operator<<(std::basic_ostream<Ch, chTraits> & o, const key_string_base<Ch, chTraits2> & s)
	{
		typename key_string_base<Ch, chTraits2>::const_iterator it(s.begin()), itEnd(s.end());
		for (; it != itEnd; ++it)
			o << (Ch)(*it);
		return o;
	}

	typedef key_string_base<char, std::char_traits<char> >			key_string;
	typedef key_string_base<char16_t, std::char_traits<char16_t> >	key_u16string;
	typedef key_string_base<char32_t, std::char_traits<char32_t> >	key_u32string;
	typedef key_string_base<wchar_t, std::char_traits<wchar_t> >	key_wstring;

	template<typename Ch, typename chTraits>
	struct is_string<key_string_base<Ch, chTraits>> : public std::true_type {};

}//namespace ara {

#endif //ARA_KEY_STRING_H

