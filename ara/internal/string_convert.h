
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

			string_appender(T & t) : str_(t) {}
			
			inline uint32_t & operator * () const { return ch_; }
			inline string_appender & operator ++ ()
			{
				traits::append(str_, static_cast<traits::value_type>(ch_));
				return *this;
			}
			string_appender operator ++ (int)
			{
				string_appender temp(str_);
				traits::append(str_, static_cast<traits::value_type>(ch_));
				return temp;
			}
			uint32_t	ch_;
			T & str_;
		};

		template<int>
		struct wchar_type {
			typedef char16_t	value_type;
		};
		template<>
		struct wchar_type<4> {
			typedef char32_t	value_type;
		};

		struct string_convert {

			template<typename typeTarStr>
			inline void	append(typeTarStr & str, const char * , const char * p, size_t n) {
				string_traits<typeTarStr>::append(str, p, n);
			}
			template<typename typeTarStr>
			inline void	append(typeTarStr & str, const char *, const char16_t * p, size_t n) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, reserve::size(str) + n * 3);
				utf8::unchecked::utf16to8(p, p + n, string_appender<typeTarStr>(str));
			}
			template<typename typeTarStr>
			inline void	append(typeTarStr & str, const char *, const char32_t * p, size_t n) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, reserve::size(str) + n * 3);
				utf8::unchecked::utf32to8(p, p + n, string_appender<typeTarStr>(str));
			}
			
			template<typename typeTarStr>
			inline void	append(typeTarStr & str, const char16_t *, const char * p, size_t n) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, reserve::size(str) + n);
				utf8::unchecked::utf8to16(p, p + n, string_appender<typeTarStr>(str));
			}
			template<typename typeTarStr>
			inline void	append(typeTarStr & str, const char16_t *, const char16_t * p, size_t n) {
				string_traits<typeTarStr>::append(str, p, n);
			}
			template<typename typeTarStr>
			inline void	append(typeTarStr & str, const char16_t *, const char32_t * p, size_t n) {
				typedef string_traits<typeTarStr>	traits;
				traits::reserve(str, reserve::size(str) + n * 3);
				utf8::unchecked::utf32to16(p, p + n, string_appender<typeTarStr>(str));
			}

			
			
			template<typename typeTarStr>
			inline void	append(typeTarStr & str, const wchar_t * p, size_t n) {
				typedef wchar_type<sizeof(wchar_t)>::value_type	value_type;
				append(str, static_cast<const value_type *>(p), n);
			}

		};

	}
}

#endif//ARA_INTERNAL_STRING_CONVERT_H

