
#ifndef ARA_INTERNAL_DEFAULT_STRING_TRAITS_H
#define ARA_INTERNAL_DEFAULT_STRING_TRAITS_H

#include <string>

namespace ara {

	template<class T>
	class string_traits
	{
	public:
		typedef typename std::remove_reference<T>::type	base_type;
		typedef typename base_type::value_type	value_type;
		typedef typename base_type::size_type	size_type;
		typedef typename base_type::traits_type	traits_type;
		typedef typename base_type::iterator	iterator;
		typedef typename base_type::const_iterator	const_iterator;

		static const size_type	npos;

		static inline iterator	begin(base_type & s) {
			return s.begin();
		}
		static inline iterator	end(base_type & s) {
			return s.end();
		}
		static inline const_iterator	begin(const base_type & s) {
			return s.begin();
		}
		static inline const_iterator	end(const base_type & s) {
			return s.end();
		}
		static inline const value_type * data(const base_type & s) {
			return s.data();
		}
		static inline value_type * data_force_to_modify(const base_type & s) {
			return const_cast<value_type *>(s.data());
		}
		static inline size_type	size(const base_type & s) {
			return s.size();
		}
		static void clear(base_type & s) {
			s.clear();
		}
		static void resize(base_type & s, size_type n) {
			s.resize(n);
		}
		static void reserve(base_type & s, size_type n) {
			s.reserve(n);
		}
		static bool empty(const base_type & s) {
			return s.empty();
		}
		static inline size_type capacity(base_type & s) {
			return s.capacity();
		}

		static inline int compare(const base_type & s, size_type pos1, size_type count1, const value_type * s2, size_type count2) {
			return s.compare(pos1, count1, s2, count2);
		}

		static inline size_type	find(const base_type & s, value_type ch, size_type off) {
			return s.find(ch, off);
		}
		static inline size_type	find(const base_type & s, const value_type * pattern, size_type pattern_size, size_type off) {
			return s.find(pattern, off, pattern_size);
		}
		static inline size_type	rfind(const base_type & s, value_type ch, size_type off) {
			return s.rfind(ch, off);
		}
		static inline size_type	rfind(const base_type & s, const value_type * pattern, size_type pattern_size, size_type off) {
			return s.rfind(pattern, off, pattern_size);
		}

		static inline size_type	find_first_of(const base_type & s, const value_type * pattern, size_type pattern_size, size_type off) {
			return s.find_first_of(pattern, off, pattern_size);
		}
		static inline size_type	find_first_not_of(const base_type & s, const value_type * pattern, size_type pattern_size, size_type off) {
			return s.find_first_not_of(pattern, off, pattern_size);
		}
		static inline size_type	find_last_of(const base_type & s, const value_type * pattern, size_type pattern_size, size_type off) {
			return s.find_last_of(pattern, off, pattern_size);
		}
		static inline size_type	find_last_not_of(const base_type & s, const value_type * pattern, size_type pattern_size, size_type off) {
			return s.find_last_not_of(pattern, off, pattern_size);
		}
		static inline base_type substr(const base_type & s, size_type off, size_type count) {
			return s.substr(off, count);
		}
		static inline void append(base_type & s, const value_type * ch, size_type size) {
			s.append(ch, size);
		}
		static inline void append(base_type & s, size_type size, value_type ch) {
			s.append(size, ch);
		}
		static inline void append(base_type & s, value_type ch) {
			s += ch;
		}
		static inline void append(base_type & s, const base_type & s2) {
			s += s2;
		}
		static inline base_type make(const value_type * ch, size_type size) {
			return base_type(ch, size);
		}
	};
	template<class T>
	const typename string_traits<T>::size_type	string_traits<T>::npos = string_traits<T>::base_type::npos;

	template<class T>
	struct is_string : public std::false_type {};
}
#endif // ARA_INTERNAL_DEFAULT_STRING_TRAITS_H
