
#ifndef ARA_INTERNAL_STRING_CONVERT_H
#define ARA_INTERNAL_STRING_CONVERT_H

#include "string_traits.h"
#include "../utf8.h"

#include <string>
#include <type_traits>

namespace ara {

	namespace internal {
		
		template<class T>
		struct string_appender {
			typedef string_traits<T>	traits;

			inline string_appender(T & t) : str_(t) {}
			inline string_appender(const string_appender & t) : str_(t.str_) {}

			inline string_appender & operator=(const string_appender & r) { return *this; }
			
			inline string_appender & operator * () { return *this; }
			inline string_appender & operator ++ () { return *this; }
			inline string_appender operator ++ (int) { return *this; }
			inline string_appender & operator=(char32_t ch) {
				traits::append(str_, static_cast<typename traits::value_type>(ch));
				return *this;
			}

			T & str_;
		};

		template<size_t>
		struct char_type {
			typedef char	value_type;
		};
		template<>
		struct char_type<2> {
			typedef char16_t	value_type;
		};
		template<>
		struct char_type<4> {
			typedef char32_t	value_type;
		};

		inline char32_t surrogate_to_utf32(char16_t high, char16_t low) {
			return (high << 10) + low - 0x35fdc00;
		}

		template <typename octet_iterator, typename u16bit_iterator>
		void convert_utf16_to_utf32(u16bit_iterator start, u16bit_iterator end, octet_iterator result) 	{
			while (start != end) {
				char16_t ch = *start++;
				if (!utf8::internal::is_surrogate(ch))
					*result++ = ch;
				else {
					if (utf8::internal::is_lead_surrogate(ch) && start != end && utf8::internal::is_trail_surrogate(*start))
						*result++ = surrogate_to_utf32(ch, *start);
				}
			}
		}

		template <typename octet_iterator, typename u32bit_iterator>
		void convert_utf32_to_utf16(u32bit_iterator start, u32bit_iterator end, octet_iterator result) 	{
			while (start != end) {
				char32_t ch = *start++;
				if (ch < 0x10000)
					*result++ = static_cast<char16_t>(ch);
				else if (ch <= 0x10FFFF) {
					*result++ = static_cast<char16_t>((ch >> 10) + 0xD7C0);
					*result++ = static_cast<char16_t>((ch & 0x3FF) + 0xDC00);
				}
			}
		}

		struct string_convert {

			template<typename typeTarStr>
			inline static void	append(typeTarStr & str , const char * p, size_t n, const char) {
				string_traits<typeTarStr>::append(str, p, n);
			}
			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char16_t * p, size_t n, const char) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, traits::size(str) + n * 3);
				utf8::unchecked::utf16to8(p, p + n, string_appender<typeTarStr>(str));
			}
			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char32_t * p, size_t n, const char) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, traits::size(str) + n * 3);
				utf8::unchecked::utf32to8(p, p + n, string_appender<typeTarStr>(str));
			}
			
			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char * p, size_t n, const char16_t) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, traits::size(str) + n);
				utf8::unchecked::utf8to16(p, p + n, string_appender<typeTarStr>(str));
			}
			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char16_t * p, size_t n, const char16_t) {
				string_traits<typeTarStr>::append(str, reinterpret_cast<const typename string_traits<typeTarStr>::value_type *>(p), n);
			}
			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char32_t * p, size_t n, const char16_t) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, traits::size(str) + n * 3);
				convert_utf32_to_utf16(p, p + n, string_appender<typeTarStr>(str));
			}

			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char * p, size_t n, const char32_t) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, traits::size(str) + n);
				utf8::unchecked::utf8to32(p, p + n, string_appender<typeTarStr>(str));
			}
			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char16_t * p, size_t n, const char32_t) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, traits::size(str) + n);
				convert_utf16_to_utf32(p, p + n, string_appender<typeTarStr>(str));
			}
			template<typename typeTarStr>
			inline static void	append(typeTarStr & str, const char32_t * p, size_t n, const char32_t) {
				string_traits<typeTarStr>::append(str, p, n);
			}

			template<typename typeTarStr, typename typeCh>
			inline static void	append(typeTarStr & str, const typeCh * p, size_t n) {
				using typeTarCh = typename char_type<sizeof(typename string_traits<typeTarStr>::value_type)>::value_type;
				using  typeSrcCh = typename char_type<sizeof(typeCh)>::value_type;
				append(str, reinterpret_cast<const typeSrcCh *>(p), n, typeTarCh() );
			}

			template<typename typeTarStr, typename typeSrcStr>
			inline static void	append(typeTarStr & str, const typeSrcStr & strSrc) {
				append(str, string_traits<typeSrcStr>::data(strSrc), string_traits<typeSrcStr>::size(strSrc));
			}
		};

	}
}

#endif//ARA_INTERNAL_STRING_CONVERT_H

