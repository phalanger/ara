// Copyright 2006 Nemanja Trifunovic

/*
Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef ARA_UTF8_H
#define ARA_UTF8_H

#include "ara_def.h"

#include "internal/utf8_core.h"

namespace ara {
	namespace utf8 {
		template<typename typeChar> struct unicode_char {
			enum { utf_type = 32 };
		};
		template<> struct unicode_char<char> {
			enum { utf_type = 8 };
		};
		template<> struct unicode_char<char16_t> {
			enum { utf_type = 16 };
		};
		template<> struct unicode_char<wchar_t> {
			enum { utf_type = 16 };
		};

		// Base for the exceptions that may be thrown from the library
		class exception : public ::std::exception {
		};

		// Exceptions that may be thrown from the library functions.
		class invalid_code_point : public exception {
			char32_t cp;
		public:
			invalid_code_point(char32_t cp) : cp(cp) {}
			virtual const char* what() const throw() { return "Invalid code point"; }
			char32_t code_point() const { return cp; }
		};

		class invalid_utf8 : public exception {
			unsigned char u8;
		public:
			typedef unsigned char	char_type;
			invalid_utf8(unsigned char u) : u8(u) {}
			virtual const char* what() const throw() { return "Invalid UTF-8"; }
			unsigned char utf8_octet() const { return u8; }
		};

		class invalid_utf16 : public exception {
			char16_t u16;
		public:
			typedef char16_t	char_type;
			invalid_utf16(char16_t u) : u16(u) {}
			virtual const char* what() const throw() { return "Invalid UTF-16"; }
			char16_t utf16_word() const { return u16; }
		};

		class not_enough_room : public exception {
		public:
			virtual const char* what() const throw() { return "Not enough space"; }
		};

		//here is writed by cyt
		namespace policy {
			class skip_error
			{
			public:
				static inline bool is_code_point_valid(char32_t cp) {
					return true;
				}
				template <typename octet_iterator>
				static inline char32_t next(octet_iterator& it, octet_iterator end)
				{
					uint32_t cp = ara::utf8::internal::mask8(*it);
					typename std::iterator_traits<octet_iterator>::difference_type length = ara::utf8::internal::sequence_length(it);
					switch (length) {
					case 1:
						break;
					case 2:
						if (++it != end)
							cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
						break;
					case 3:
						if (++it != end) {
							cp = ((cp << 12) & 0xffff) + ((ara::utf8::internal::mask8(*it) << 6) & 0xfff);
							if (++it != end)
								cp += (*it) & 0x3f;
						}
						break;
					case 4:
						if (++it != end) {
							cp = ((cp << 18) & 0x1fffff) + ((ara::utf8::internal::mask8(*it) << 12) & 0x3ffff);
							if (++it != end) {
								cp += (ara::utf8::internal::mask8(*it) << 6) & 0xfff;
								if (++it != end)
									cp += (*it) & 0x3f;
							}
						}
						break;
					}
					++it;
					return cp;
				}
				template <typename octet_iterator>
				static inline bool go_prior(octet_iterator& it, octet_iterator start) {
					if (it == start)
						return false;
					// Go back until we hit either a lead octet or start
					while (ara::utf8::internal::is_trail(*(--it)))
						if (it == start)
							return false;
					return true;
				}
				static inline void check_utf16_trail_surrogate(char32_t) {
					return;
				}
				static inline void check_utf16_not_trail_surrogate(char32_t) {
					return;
				}
				template<typename iterator>
				static inline bool check_range(iterator & start, iterator & end, char32_t) {
					return (start != end);
				}
			};
			class skip_error_fast : public skip_error
			{
			public:
				template <typename octet_iterator>
				static inline char32_t next(octet_iterator& it, octet_iterator)
				{
					uint32_t cp = ara::utf8::internal::mask8(*it);
					typename std::iterator_traits<octet_iterator>::difference_type length = ara::utf8::internal::sequence_length(it);
					switch (length) {
					case 1:
						break;
					case 2:
						it++;
						cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
						break;
					case 3:
						++it;
						cp = ((cp << 12) & 0xffff) + ((ara::utf8::internal::mask8(*it) << 6) & 0xfff);
						++it;
						cp += (*it) & 0x3f;
						break;
					case 4:
						++it;
						cp = ((cp << 18) & 0x1fffff) + ((ara::utf8::internal::mask8(*it) << 12) & 0x3ffff);
						++it;
						cp += (ara::utf8::internal::mask8(*it) << 6) & 0xfff;
						++it;
						cp += (*it) & 0x3f;
						break;
					}
					++it;
					return cp;
				}
				template <typename octet_iterator>
				static inline bool go_prior(octet_iterator& it, octet_iterator) {
					while (ara::utf8::internal::is_trail(*(--it)));
					return true;
				}
				template<typename iterator>
				static inline bool check_range(iterator &, iterator &, char32_t) {
					return true;
				}
			};
			class exception_while_error
			{
			public:
				static inline bool is_code_point_valid(char32_t cp) {
					if (!ara::utf8::internal::is_code_point_valid(cp))
						throw invalid_code_point(cp);
					return true;
				}
				template <typename octet_iterator>
				static inline char32_t next(octet_iterator& it, octet_iterator end)
				{
					char32_t cp = 0;
					ara::utf8::internal::utf_error err_code = ara::utf8::internal::validate_next(it, end, cp);
					switch (err_code) {
					case ara::utf8::internal::UTF8_OK:
						break;
					case ara::utf8::internal::NOT_ENOUGH_ROOM:
						throw not_enough_room();
					case ara::utf8::internal::INVALID_LEAD:
					case ara::utf8::internal::INCOMPLETE_SEQUENCE:
					case ara::utf8::internal::OVERLONG_SEQUENCE:
						throw invalid_utf8(*it);
					case ara::utf8::internal::INVALID_CODE_POINT:
						throw invalid_code_point(cp);
					}
					return cp;
				}
				template <typename octet_iterator>
				static inline bool go_prior(octet_iterator& it, octet_iterator start) {
					// can't do much if it == start
					if (it == start)
						throw not_enough_room();
					// Go back until we hit either a lead octet or start
					while (ara::utf8::internal::is_trail(*(--it)))
						if (it == start)
							throw invalid_utf8(*it); // error - no lead byte in the sequence
					return true;
				}
				static inline void check_utf16_trail_surrogate(char32_t trail_surrogate) {
					if (!ara::utf8::internal::is_trail_surrogate(trail_surrogate))
						throw invalid_utf16(static_cast<char16_t>(trail_surrogate));
				}
				static inline void check_utf16_not_trail_surrogate(char32_t trail_surrogate) {
					if (ara::utf8::internal::is_trail_surrogate(trail_surrogate))
						throw invalid_utf16(static_cast<char16_t>(trail_surrogate));
				}
				template<typename iterator>
				static inline bool check_range(iterator & start, iterator & end, char32_t cp) {
					if (start == end)
						throw invalid_utf16(static_cast<uint16_t>(cp));
					return true;
				}
			};
		}

		//here is writed by Nemanja Trifunovic
		/// The library API - functions intended to be called by the users
		template <class utf8_policy = policy::skip_error, typename octet_iterator>
		octet_iterator append(char32_t cp, octet_iterator result)
		{
			if (!utf8_policy::is_code_point_valid(cp))
				return result;
			return ara::utf8::internal::append_rawchar(cp, result);
		}

		template <typename octet_iterator, typename output_iterator>
		output_iterator replace_invalid(octet_iterator start, octet_iterator end, output_iterator out, char32_t replacement)
		{
			while (start != end) {
				octet_iterator sequence_start = start;
				ara::utf8::internal::utf_error err_code = ara::utf8::internal::validate_next(start, end);
				switch (err_code) {
				case ara::utf8::internal::UTF8_OK:
					for (octet_iterator it = sequence_start; it != start; ++it)
						*out++ = *it;
					break;
				case ara::utf8::internal::NOT_ENOUGH_ROOM:
					throw not_enough_room();
				case ara::utf8::internal::INVALID_LEAD:
					out = ara::utf8::internal::append_rawchar(replacement, out);
					++start;
					break;
				case ara::utf8::internal::INCOMPLETE_SEQUENCE:
				case ara::utf8::internal::OVERLONG_SEQUENCE:
				case ara::utf8::internal::INVALID_CODE_POINT:
					out = ara::utf8::internal::append_rawchar(replacement, out);
					++start;
					// just one replacement mark for the sequence
					while (start != end && ara::utf8::internal::is_trail(*start))
						++start;
					break;
				}
			}
			return out;
		}

		template <typename octet_iterator, typename output_iterator>
		inline output_iterator replace_invalid(octet_iterator start, octet_iterator end, output_iterator out) {
			static const char32_t replacement_marker = ara::utf8::internal::mask16(0xfffd);
			return ara::utf8::replace_invalid(start, end, out, replacement_marker);
		}

		template <class utf8_policy = policy::skip_error, typename octet_iterator>
		char32_t next(octet_iterator& it, octet_iterator end) {
			return utf8_policy::next(it, end);
		}

		template <class utf8_policy = policy::skip_error, typename octet_iterator>
		char32_t peek_next(octet_iterator it, octet_iterator end) {
			return ara::utf8::next<utf8_policy, octet_iterator>(it, end);
		}

		template <class utf8_policy = policy::skip_error, typename octet_iterator>
		char32_t prior(octet_iterator& it, octet_iterator start) {
			octet_iterator end = it;
			if (!utf8_policy::go_prior(it, start))
				return 0;
			return ara::utf8::peek_next<utf8_policy, octet_iterator>(it, end);
		}

		template <class utf8_policy = policy::skip_error, typename octet_iterator, typename distance_type>
		void advance(octet_iterator& it, distance_type n, octet_iterator end)
		{
			for (distance_type i = 0; i < n; ++i)
				ara::utf8::next<utf8_policy, octet_iterator>(it, end);
		}

		template <class utf8_policy = policy::skip_error, typename octet_iterator>
		typename std::iterator_traits<octet_iterator>::difference_type
			distance(octet_iterator first, octet_iterator last)
		{
			typename std::iterator_traits<octet_iterator>::difference_type dist;
			for (dist = 0; first < last; ++dist)
				ara::utf8::next<utf8_policy, octet_iterator>(first, last);
			return dist;
		}

		template <class utf8_policy = policy::skip_error, typename u16bit_iterator, typename octet_iterator>
		octet_iterator utf16to8(u16bit_iterator start, u16bit_iterator end, octet_iterator result)
		{
			while (start != end) {
				char32_t t = *start;
				++start;
				char32_t cp = ara::utf8::internal::mask16(t);
				// Take care of surrogate pairs first
				if (ara::utf8::internal::is_lead_surrogate(cp)) {
					if (utf8_policy::check_range(start, end, cp)) {
						char32_t t2 = *start;
						++start;
						char32_t trail_surrogate = ara::utf8::internal::mask16(t2);
						utf8_policy::check_utf16_trail_surrogate(trail_surrogate);
						cp = (cp << 10) + trail_surrogate + ara::utf8::internal::SURROGATE_OFFSET;
					}
				}
				else 
					utf8_policy::check_utf16_not_trail_surrogate(cp);

				result = utf8::append<utf8_policy, octet_iterator>(cp, result);
			}
			return result;
		}

		template <class utf8_policy = policy::skip_error, typename u16bit_iterator, typename octet_iterator>
		u16bit_iterator utf8to16(octet_iterator start, octet_iterator end, u16bit_iterator result)
		{
			while (start != end) {
				char32_t cp = ara::utf8::next<utf8_policy, octet_iterator>(start, end);
				if (cp > 0xffff) { //make a surrogate pair
					*result++ = static_cast<uint16_t>((cp >> 10) + internal::LEAD_OFFSET);
					*result++ = static_cast<uint16_t>((cp & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
				}
				else
					*result++ = static_cast<uint16_t>(cp);
			}
			return result;
		}

		template <class utf8_policy = policy::skip_error, typename octet_iterator, typename u32bit_iterator>
		octet_iterator utf32to8(u32bit_iterator start, u32bit_iterator end, octet_iterator result)
		{
			while (start != end) {
				char32_t cp = *start;
				++start;
				result = ara::utf8::append<utf8_policy, octet_iterator>(cp, result);
			}

			return result;
		}

		template <class utf8_policy = policy::skip_error, typename octet_iterator, typename u32bit_iterator>
		u32bit_iterator utf8to32(octet_iterator start, octet_iterator end, u32bit_iterator result)
		{
			while (start != end)
				(*result++) = utf8::next<utf8_policy, octet_iterator>(start, end);

			return result;
		}

		template <class utf8_policy = policy::skip_error, typename u16bit_iterator, typename octet_iterator>
		octet_iterator utf16to32(u16bit_iterator start, u16bit_iterator end, octet_iterator result)
		{
			while (start != end) {
				char32_t t = *start;
				++start;
				char32_t cp = ara::utf8::internal::mask16(t);
				// Take care of surrogate pairs first
				if (ara::utf8::internal::is_lead_surrogate(cp)) {
					if (utf8_policy::check_range(start, end, cp)) {
						char32_t t2 = *start;
						++start;
						char32_t trail_surrogate = ara::utf8::internal::mask16(t2);
						utf8_policy::check_utf16_trail_surrogate(trail_surrogate);
						cp = (cp << 10) + trail_surrogate + ara::utf8::internal::SURROGATE_OFFSET;
					}
				}
				else
					utf8_policy::check_utf16_not_trail_surrogate(cp);

				(*result++) = cp;
			}
			return result;
		}

		template <class utf8_policy = policy::skip_error, typename u16bit_iterator, typename octet_iterator>
		u16bit_iterator utf32to16(octet_iterator start, octet_iterator end, u16bit_iterator result)
		{
			while (start != end) {
				char32_t cp = *start;
				++start;
				if (cp > 0xffff) { //make a surrogate pair
					*result++ = static_cast<uint16_t>((cp >> 10) + internal::LEAD_OFFSET);
					*result++ = static_cast<uint16_t>((cp & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
				}
				else
					*result++ = static_cast<uint16_t>(cp);
			}
			return result;
		}

		// The iterator class
		template <typename octet_iterator, class utf8_policy = policy::skip_error>
		class iterator : public std::iterator <std::bidirectional_iterator_tag, char32_t> {
			octet_iterator it;
			octet_iterator range_start;
			octet_iterator range_end;
		public:
			iterator() {}
			explicit iterator(const octet_iterator& octet_it,
				const octet_iterator& range_start,
				const octet_iterator& range_end) :
				it(octet_it), range_start(range_start), range_end(range_end)
			{
				if (it < range_start || it > range_end)
					throw std::out_of_range("Invalid utf-8 iterator position");
			}
			// the default "big three" are OK
			octet_iterator base() const { return it; }
			char32_t operator * () const
			{
				octet_iterator temp = it;
				return ara::utf8::next<utf8_policy, octet_iterator>(temp, range_end);
			}
			bool operator == (const iterator& rhs) const
			{
				if (range_start != rhs.range_start || range_end != rhs.range_end)
					throw std::logic_error("Comparing utf-8 iterators defined with different ranges");
				return (it == rhs.it);
			}
			bool operator != (const iterator& rhs) const
			{
				return !(operator == (rhs));
			}
			iterator& operator ++ ()
			{
				ara::utf8::next<utf8_policy, octet_iterator>(it, range_end);
				return *this;
			}
			iterator operator ++ (int)
			{
				iterator temp = *this;
				ara::utf8::next<utf8_policy, octet_iterator>(it, range_end);
				return temp;
			}
			iterator& operator -- ()
			{
				ara::utf8::prior<utf8_policy, octet_iterator>(it, range_start);
				return *this;
			}
			iterator operator -- (int)
			{
				iterator temp = *this;
				ara::utf8::prior<utf8_policy, octet_iterator>(it, range_start);
				return temp;
			}
		}; // class iterator

		namespace internal {
			template<class utf8_policy,size_t from, size_t to, typename typeStrFrom, typename typeStrTo>
			struct append_string {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy,8,8,typeStrFrom,typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					strTar.append( strSrc.begin(), strSrc.end() );
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 8, 16, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					ara::utf8::utf8to16<utf8_policy>(strSrc.begin(), strSrc.end(), std::back_inserter(strTar));
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 8, 32, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					ara::utf8::utf8to32<utf8_policy>(strSrc.begin(), strSrc.end(), std::back_inserter(strTar));
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 16, 16, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					strTar.append(strSrc.begin(), strSrc.end());
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 16, 8, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					ara::utf8::utf16to8<utf8_policy>(strSrc.begin(), strSrc.end(), std::back_inserter(strTar));
				}
				inline static size_t cal_size(size_t n) {
					return n * 3;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 16, 32, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					ara::utf8::utf16to32<utf8_policy>(strSrc.begin(), strSrc.end(), std::back_inserter(strTar));
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 32, 32, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					strTar.append(strSrc.begin(), strSrc.end());
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 32, 8, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					ara::utf8::utf32to8<utf8_policy>(strSrc.begin(), strSrc.end(), std::back_inserter(strTar));
				}
				inline static size_t cal_size(size_t n) {
					return n * 3;
				}
			};
			template<class utf8_policy, typename typeStrFrom, typename typeStrTo>
			struct append_string<utf8_policy, 32, 16, typeStrFrom, typeStrTo> {
				inline static void append( typeStrTo & strTar, const typeStrFrom & strSrc) {
					ara::utf8::utf32to16<utf8_policy>(strSrc.begin(), strSrc.end(), std::back_inserter(strTar));
				}
				inline static size_t cal_size(size_t n) {
					return n;
				}
			};
		}

		template<class utf8_policy = policy::skip_error
			, typename typeStrTo, typename typeStrFrom
			, typename charFrom = typename typeStrFrom::value_type
			, typename charTar = typename typeStrTo::value_type>
		typeStrTo & append(typeStrTo & strTar, const typeStrFrom & strSrc) {
			ara::utf8::internal::append_string<utf8_policy, unicode_char<charFrom>::utf_type, unicode_char<charTar>::utf_type, typeStrFrom, typeStrTo>::append(strTar, strSrc);
			return strTar;
		}
		template<typename typeStrTo
			, class utf8_policy = policy::skip_error
			, typename typeStrFrom
			, typename charFrom = typename typeStrFrom::value_type
			, typename charTar = typename typeStrTo::value_type>
		typeStrTo convert(const typeStrFrom & strSrc) {
			typeStrTo strTar;
			typedef ara::utf8::internal::append_string<utf8_policy, unicode_char<charFrom>::utf_type, unicode_char<charTar>::utf_type, typeStrFrom, typeStrTo>	helper;
			strTar.reserve(helper::cal_size(strSrc.length()));
			helper::append(strTar, strSrc);
			return strTar;
		}
	}
}

#endif // ARA_UTF8_H
