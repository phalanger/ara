
#ifndef ARA_INTERNAL_STRFORMAT_H
#define ARA_INTERNAL_STRFORMAT_H

#include "string_traits.h"
#include "string_convert.h"

#include <string>
#include <type_traits>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <cstring>

namespace ara {
	namespace format {
		enum	INT_BASE {
			BASE8 = std::ios::oct,
			BASE10 = std::ios::dec,
			BASE16 = std::ios::hex
		};
		enum	CHAR_CASE {
			CHAR_UPCASE = 0,
			CHAR_LOWCASE = 1
		};
		enum	BASE_FLAG {
			HIDE_BASE = 0,
			SHOW_BASE = 1,
		};
		enum	POS_FLAG {
			HIDE_POS = 0,
			SHOW_POS = 1,
		};
		enum	FIX_FLAG {
			FIXED = 0,
			SCIENTIFIC = 1,
		};
		enum	POINT_FLAG {
			HIDE_POINT = 0,
			SHOW_POINT = 1,
		};
		enum	ADJUSTFIELD_FLAG {
			ADJUST_LEFT = std::ios::left,
			ADJUST_RIGHT = std::ios::right,
			ADJUST_INTERNAL = std::ios::internal,
		};
	}

	namespace internal {

		template<typename strType>
		class string_stream : protected std::basic_streambuf<typename strType::value_type, typename strType::traits_type>
							, public std::basic_ostream<typename strType::value_type, typename strType::traits_type>
		{
		public:
			typedef typename strType::value_type char_type;
			typedef typename strType::traits_type traits_type;
			typedef typename strType::size_type		size_type;
			typedef std::basic_streambuf<char_type, traits_type>	streambuf_parent;
			typedef std::basic_ostream<char_type, traits_type>		ostream_parent;

			string_stream(strType & buf) : buf_(buf), streambuf_parent(), ostream_parent((streambuf_parent*)this) {
				size_ = buf_.size();
				size_type rest = buf_.capacity() - size_;
				if (rest) {
					buf_.resize(size_ + rest);
					char_type * buf = const_cast<char_type *>(buf_.data());
					this->setp(buf + size_, buf + buf_.size());
				}
				else
					this->setp(nullptr, nullptr);
			}
			~string_stream() {
				size_ += this->pptr() - this->pbase();
				buf_.resize(size_);
			}
		protected:
			int 	sync() { return 0; }
			int 	overflow(int c) {
				size_ += this->pptr() - this->pbase();
				buf_.resize(size_);
				buf_ += static_cast<char_type>(c);
				++size_;
				buf_.resize(size_ + grow_);
				size_type maxsize = buf_.size();
				if (maxsize <= size_)
					return traits_type::eof();
				char_type * buf = const_cast<char_type *>(buf_.data());
				this->setp(buf + size_, buf + maxsize);
				if (grow_ < 1024)
					grow_ <<= 1;
				return c; 
			}
			strType	&		buf_;
			size_type		size_ = 0;
			size_type		grow_ = 16;
		};

		template<class T, class Enable = void>
		struct format_appender {
			typedef typename T::char_type	char_type;

			format_appender(T & stream) : stream_(stream) {}

			template<class T2>
			inline void	append(const T2 & t) {
				stream_ << t;
			}

			inline void	append(const char * ch) {
				append_str(ch, std::char_traits<char>::length(ch));
			}
			inline void	append(const wchar_t * ch) {
				append_str(ch, std::char_traits<wchar_t>::length(ch));
			}
			inline void	append(const char16_t * ch) {
				append_str(ch, std::char_traits<char16_t>::length(ch));
			}
			inline void	append(const char32_t * ch) {
				append_str(ch, std::char_traits<char32_t>::length(ch));
			}

			inline void	append_ch(char ch) {
				stream_ << static_cast<char_type>(ch);
			}

			template<class T2>
			inline void	append_str(const T2 * p, size_t nSize) {
				std::basic_string<char_type>	str;
				string_convert::append(str, p, nSize);
				stream_ << str;
			}

			inline void	append_str(const char_type * p, size_t nSize) {
				stream_.write(p, nSize);
			}

			template<class T2>
			void	append(const T2 & t, int nWidth, int chFill = '0'
					, format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_RIGHT) {

				std::ios::fmtflags nFlags = stream_.flags();
				stream_.setf(static_cast<std::ios::fmtflags>(nAdjust), std::ios::adjustfield);

				if (nWidth != -1)
					nWidth = static_cast<int>(stream_.width(nWidth));
				chFill = stream_.fill(chFill);

				stream_ << static_cast<T2>(t);

				stream_.flags(nFlags);
				if (nWidth != -1)
					stream_.width(nWidth);
				stream_.fill(chFill);
			}

			template<class T2>
			void	append(const T2 & t, format::INT_BASE nBase, int nWidth, int chFill = '0'
				, format::CHAR_CASE bUpcase = format::CHAR_UPCASE
				, format::POS_FLAG bShowPos = format::HIDE_POS
				, format::BASE_FLAG bShowBase = format::HIDE_BASE
				, format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_RIGHT) {

				std::ios::fmtflags nFlags = stream_.flags();
				stream_.setf(static_cast<std::ios::fmtflags>(nBase), std::ios::basefield);
				if (bShowPos == format::SHOW_POS)
					stream_.setf(std::ios::showpos);
				else
					stream_.unsetf(std::ios::showpos);
				if (bUpcase == format::CHAR_UPCASE)
					stream_.setf(std::ios::uppercase);
				else
					stream_.unsetf(std::ios::uppercase);
				if (bShowBase == format::SHOW_BASE)
					stream_.setf(std::ios::showbase);
				else
					stream_.unsetf(std::ios::showbase);
				stream_.setf(static_cast<std::ios::fmtflags>(nAdjust), std::ios::adjustfield);

				if (nWidth != -1)
					nWidth = static_cast<int>(stream_.width(nWidth));
				chFill = stream_.fill(chFill);

				stream_ << static_cast<T2>(t);

				stream_.flags(nFlags);
				if (nWidth != -1)
					stream_.width(nWidth);
				stream_.fill(chFill);
			}

			template<typename T2, int base = 10, bool boLowCase = false>
			inline void	append_int(T2 t) {
				std::ios::fmtflags nFlags = stream_.flags();

				if (base == 8) {
					stream_.setf(static_cast<std::ios::fmtflags>(format::BASE8), std::ios::basefield);
				} else if (base == 10) {
					stream_.setf(static_cast<std::ios::fmtflags>(format::BASE10), std::ios::basefield);
				} else if (base == 16) {
					stream_.setf(static_cast<std::ios::fmtflags>(format::BASE16), std::ios::basefield);
				}
				stream_ << static_cast<T2>(t);
				stream_.flags(nFlags);
			}

			template<typename typeDouble>
			void	append(const typeDouble & dbVal,
						format::FIX_FLAG bFixed,
						format::POINT_FLAG bShowPoint = format::SHOW_POINT,
						format::CHAR_CASE bUpcase = format::CHAR_UPCASE,
						format::POS_FLAG bShowPos = format::HIDE_POS,
						format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_LEFT,
							int nPrecision = -1, int nWidth = -1, int chFill = ' ') {
				std::ios::fmtflags nFlags = stream_.flags();
				if (bFixed == format::FIXED)
					stream_.setf(std::ios::fixed, std::ios::floatfield);
				else
					stream_.setf(std::ios::scientific, std::ios::floatfield);
				if (bShowPoint == format::SHOW_POINT)
					stream_.setf(std::ios::showpoint);
				else
					stream_.unsetf(std::ios::showpoint);
				if (bShowPos == format::SHOW_POS)
					stream_.setf(std::ios::showpos);
				else
					stream_.unsetf(std::ios::showpos);
				if (bUpcase == format::CHAR_UPCASE)
					stream_.setf(std::ios::uppercase);
				else
					stream_.unsetf(std::ios::uppercase);
				stream_.setf(static_cast<std::ios::fmtflags>(nAdjust), std::ios::adjustfield);

				if (nPrecision != -1)
					nPrecision = static_cast<int>(stream_.precision(nPrecision));
				if (nWidth != -1)
					nWidth = static_cast<int>(stream_.width(nWidth));
				chFill = stream_.fill(chFill);

				stream_ << static_cast<typeDouble>(dbVal);

				stream_.flags(nFlags);
				if (nPrecision != -1)
					stream_.precision(nPrecision);
				if (nWidth != -1)
					stream_.width(nWidth);
				stream_.fill(chFill);
			}

			void	reserve(size_t n) {}

			T & stream_;
		};

		template<class T>
		struct format_appender<T, typename std::enable_if<is_string<T>::value>::type> {
			format_appender(T & str) : str_(str) {}

			typedef string_traits<T>					typeStrTraits;
			typedef typename typeStrTraits::value_type	typeCh;
			typedef string_stream<T>    typeStream;

			inline void	append_ch(char ch) {
				str_ += static_cast<typeCh>(ch);
			}

			template<class T2>
			inline void	append(const T2 & t) {
				typeStream	out(str_);
				out << t;
			}

			inline void	append(const char * ch) {
				append_str(ch, std::char_traits<char>::length(ch));
			}
			inline void	append(const wchar_t * ch) {
				append_str(ch, std::char_traits<wchar_t>::length(ch));
			}
			inline void	append(const char16_t * ch) {
				append_str(ch, std::char_traits<char16_t>::length(ch));
			}
			inline void	append(const char32_t * ch) {
				append_str(ch, std::char_traits<char32_t>::length(ch));
			}

			template<class T2, typename std::enable_if_t<std::is_integral<T2>::value>>
			inline void	append(const T2 & t) {
				append_int<T2, 10, false>(t);
			}

			template<class T2>
			inline void	append_str(const T2 * p, size_t nSize) {
				string_convert::append(str_, p, nSize);
			}

			template<typename T2, int base = 10, bool boLowCase = false, typename std::enable_if<std::is_signed<T2>::value>::type>
			void	append_int(T2 t) {

				static const char * Number_Low = "0123456789abcdef";
				static const char * Number_Up = "0123456789ABCDEF";
				bool boNegative = false;
				const char * Number = boLowCase ? Number_Low : Number_Up;

				if (t == 0) {
					typeStrTraits::append(str_, 1, static_cast<typeCh>(Number[0]));
					return;
				}
				else if (t < 0) {
					boNegative = true;
					t = -t;
				}

				const	size_t	bufsize = 72;
				typeCh	buf[bufsize];
				typeCh * p = buf + bufsize;
				while (t) {
					*(--p) = static_cast<typeCh>(Number[t % static_cast<T2>(base)]);
					t /= static_cast<T2>(base);
				}
				if (boNegative)
					*(--p) = static_cast<typeCh>('-');
				typeStrTraits::append(str_, p, buf + bufsize - p);
			}

			template<typename T2, int base = 10, bool boLowCase = false>
			void	append_int(T2 t) {

					static const char * Number_Low = "0123456789abcdef";
					static const char * Number_Up = "0123456789ABCDEF";
					const char * Number = boLowCase ? Number_Low : Number_Up;

					if (t == 0) {
						typeStrTraits::append(str_, 1, static_cast<typeCh>(Number[0]));
						return;
					}

					const	size_t	bufsize = 72;
					typeCh	buf[bufsize];
					typeCh * p = buf + bufsize;
					while (t) {
						*(--p) = static_cast<typeCh>(Number[t % static_cast<T2>(base)]);
						t /= static_cast<T2>(base);
					}
					typeStrTraits::append(str_, p, buf + bufsize - p);
			}

			template<class T2>
			inline void	append(const T2 & t, int nWidth, int chFill = '0'
				, format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_RIGHT) {

				typeStream	out(str_);

				out.setf(static_cast<std::ios::fmtflags>(nAdjust), std::ios::adjustfield);

				if (nWidth != -1)
					out.width(nWidth);
				out.fill(chFill);

				out << t;
			}

			template<class T2>
			void	append(const T2 & t, format::INT_BASE nBase, int nWidth, int chFill = '0'
						, format::CHAR_CASE bUpcase = format::CHAR_UPCASE
						, format::POS_FLAG bShowPos = format::HIDE_POS
						, format::BASE_FLAG bShowBase = format::HIDE_BASE
						, format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_RIGHT) {

				typeStream	out(str_);

				out.setf(static_cast<std::ios::fmtflags>(nBase), std::ios::basefield);
				if (bShowPos == format::SHOW_POS)
					out.setf(std::ios::showpos);
				else
					out.unsetf(std::ios::showpos);
				if (bUpcase == format::CHAR_UPCASE)
					out.setf(std::ios::uppercase);
				else
					out.unsetf(std::ios::uppercase);
				if (bShowBase == format::SHOW_BASE)
					out.setf(std::ios::showbase);
				else
					out.unsetf(std::ios::showbase);
				out.setf(static_cast<std::ios::fmtflags>(nAdjust), std::ios::adjustfield);

				if (nWidth != -1)
					out.width(nWidth);
				out.fill(chFill);

				out << static_cast<T2>(t);
			}

			template<typename typeDouble>
			void	append(const typeDouble & dbVal,
							format::FIX_FLAG bFixed,
							format::POINT_FLAG bShowPoint = format::SHOW_POINT,
							format::CHAR_CASE bUpcase = format::CHAR_UPCASE,
							format::POS_FLAG bShowPos = format::HIDE_POS,
							format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_LEFT,
							int nPrecision = -1, int nWidth = -1, int chFill = ' ') {

				typeStream	out(str_);
				if (bFixed == format::FIXED)
					out.setf(std::ios::fixed, std::ios::floatfield);
				else
					out.setf(std::ios::scientific, std::ios::floatfield);
				if (bShowPoint == format::SHOW_POINT)
					out.setf(std::ios::showpoint);
				else
					out.unsetf(std::ios::showpoint);
				if (bShowPos == format::SHOW_POS)
					out.setf(std::ios::showpos);
				else
					out.unsetf(std::ios::showpos);
				if (bUpcase == format::CHAR_UPCASE)
					out.setf(std::ios::uppercase);
				else
					out.unsetf(std::ios::uppercase);
				out.setf(static_cast<std::ios::fmtflags>(nAdjust), std::ios::adjustfield);

				if (nPrecision != -1)
					out.precision(nPrecision);
				if (nWidth != -1)
					out.width(nWidth);
				out.fill(chFill);

				out << static_cast<typeDouble>(dbVal);
			}

			void	reserve(size_t n) {
				str_.reserve(str_.size() + n);
			}

			T & str_;
		};
	}//internal

	template<class T>
	struct str_format
	{
	public:
		typedef internal::format_appender<T>	appender;
		str_format(T & t) : out_(t) {}

		template<class ch, typename...TypeList>
		str_format &	printf(const ch * s, TypeList... t2) {
			size_t nLength = std::char_traits<ch>::length(s);
			out_.reserve(nLength);
			printf_imp(s, nLength, std::forward<TypeList>(t2)...);
			return *this;
		}

	protected:
		template<typename char_type>
		size_t	append_prefix(const char_type * fmt, size_t nSize) {
			size_t i = 0;
			for (; i < nSize; ++i) {
				if (fmt[i] == '%' && i + 1 < nSize)
					break;
			}
			out_.append_str(fmt, i);
			return i;
		}

		template<typename char_type>
		size_t	printf_imp(const char_type * fmt, size_t nSize) {
			out_.append_str(fmt, nSize);
			return nSize;
		}
		template<typename char_type, typename T1>
		size_t    printf_imp(const char_type * fmt, size_t nSize, const T1 & t1) {
			size_t i = append_prefix(fmt, nSize);
			if (i + 1 >= nSize)
				return nSize;
			i += append_fmt_val(fmt + i, nSize - i, t1);
			if (i >= nSize)
				return nSize;
			out_.append_str(fmt + i, nSize - i);
			return i;
		}

		template<typename char_type, typename T1>
		size_t	append_fmt_val(const char_type * fmt, size_t nSize, const T1 & t1, typename std::enable_if<std::is_integral<T1>::value>::type * a= nullptr) {
			size_t i = 1;	//fmt[0] is %
			char_type	ch = fmt[i];
			switch (ch) {
			case 'v': case 'u': case 's': case 'f':
				++i;	out_.append(t1);	break;
			case 'd':
				++i;	out_.template append_int<T1, 10, false>(t1);	break;
			case 'x':
				++i;	out_.template append_int<T1, 16, false>(t1);	break;
			case 'X':
				++i;	out_.template append_int<T1, 16, true>(t1);	break;

			case '\0': case ',': case ' ': case '\r': case '\n': case '\t':
				out_.append_ch(static_cast<char>(t1));
				break;
			default:
				i += output_fmt_val_with_format(fmt + i, nSize - i, t1);
				break;
			}
			return i;
		}
		template<typename char_type, typename T1>
		size_t	append_fmt_val(const char_type * fmt, size_t nSize, const T1 & t1, typename std::enable_if<!std::is_integral<T1>::value>::type * a = nullptr) {
			size_t i = 1;	//fmt[0] is %
			char_type	ch = fmt[i];
			switch (ch) {
			case 'v': case 'u': case 's': case 'f':case 'd':case 'x':case 'X':
				++i;	out_.append(t1);	break;
			case '\0': case ',': case ' ': case '\r': case '\n': case '\t':
				out_.append(t1);
				break;
			default:
				i += output_fmt_val_with_format(fmt + i, nSize - i, t1);
				break;
			}
			return i;
		}

#define CHECK_CH( ch , action)  else if ( *fmt == (ch) ) { ++fmt; action; }
#define CHECK_LOWCASE( c )    ( ch == c ? format::CHAR_LOWCASE : format::CHAR_UPCASE)

		template<typename char_type, typename T1>
		size_t output_fmt_val_with_format(const char_type * fmt, size_t n, const T1 & t1
											, typename std::enable_if<std::is_arithmetic<T1>::value>::type * = nullptr) {
			int                 nWidth = -1;
			int                 nPrecision = -1;
			char_type           chFill = ' ';
			const char_type * pBegin = fmt;
			const char_type * pEnd = fmt + n;
			format::POS_FLAG bShowPos = format::HIDE_POS;
			format::BASE_FLAG bShowBase = format::HIDE_BASE;
			format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_RIGHT;

			for(;;) {
				if (fmt >= pEnd)
					break;
				CHECK_CH('#', bShowBase = format::SHOW_BASE)
				CHECK_CH('-', nAdjust = format::ADJUST_LEFT)
				CHECK_CH('+', bShowPos = format::SHOW_POS)
				CHECK_CH(' ', chFill = ' ')
				CHECK_CH('0', chFill = '0')
				else
					break;
			}
			if (fmt < pEnd && *fmt >= '0' && *fmt <= '9') {
				nWidth = 0;
				for (; fmt < pEnd && *fmt >= '0' && *fmt <= '9'; ++fmt)
					nWidth = nWidth * 10 + *fmt - '0';
			}
			if (fmt < pEnd && *fmt == '.') {
				nPrecision = 0;
				for (++fmt; fmt < pEnd && *fmt >= '0' && *fmt <= '9'; ++fmt)
					nPrecision = nPrecision * 10 + *fmt - '0';
			}

			while (fmt < pEnd && *fmt == 'l')
				++fmt;
			if (fmt < pEnd)
			{
				const char_type ch = *fmt;
				if (ch && !std::isspace(ch))
					++fmt;

				if (ch == 'f') {
					out_.append(t1, format::FIXED, format::SHOW_POINT, format::CHAR_LOWCASE, bShowPos, nAdjust, nPrecision, nWidth, chFill);
				}
				else if (ch == 'e' || ch == 'E') {
					out_.append(t1, format::SCIENTIFIC, format::SHOW_POINT, CHECK_LOWCASE('e'), bShowPos, nAdjust, nPrecision, nWidth, chFill);
				}
				else if (ch == 'x' || ch == 'X') {
					out_.append(t1, format::BASE16, nWidth, chFill, CHECK_LOWCASE('x'), bShowPos, bShowBase, nAdjust);
				}
				else if (ch == 'o' || ch == 'O') {
					out_.append(t1, format::BASE8, nWidth, chFill, CHECK_LOWCASE('o'), bShowPos, bShowBase, nAdjust);
				}
				else if (ch == 'd' || ch == 'u') {
					out_.append(t1, format::BASE10, nWidth, chFill, format::CHAR_LOWCASE, bShowPos, bShowBase, nAdjust);
				}
				else {
					out_.append(t1, nWidth, chFill, nAdjust);
				}
			}
			return fmt - pBegin;
		}

		template<typename char_type, typename T1>
		size_t output_fmt_val_with_format(const char_type * fmt, size_t n, const T1 & t1
									, typename std::enable_if<!std::is_arithmetic<T1>::value>::type * = nullptr) {
			int                 nWidth = -1;
			int                 nPrecision = -1;
			char_type           chFill = ' ';
			const char_type * pBegin = fmt;
			const char_type * pEnd = fmt + n;
			format::POS_FLAG bShowPos = format::HIDE_POS;
			format::BASE_FLAG bShowBase = format::HIDE_BASE;
			format::ADJUSTFIELD_FLAG nAdjust = format::ADJUST_RIGHT;

			for (;;) {
				if (fmt >= pEnd)
					break;
				CHECK_CH('#', bShowBase = format::SHOW_BASE)
				CHECK_CH('-', nAdjust = format::ADJUST_LEFT)
				CHECK_CH('+', bShowPos = format::SHOW_POS)
				CHECK_CH(' ', chFill = ' ')
				CHECK_CH('0', chFill = '0')
				else
					break;
			}

			if (fmt < pEnd && *fmt >= '0' && *fmt <= '9') {
				nWidth = 0;
				for (; fmt < pEnd && *fmt >= '0' && *fmt <= '9'; ++fmt)
					nWidth = nWidth * 10 + *fmt - '0';
			}
			if (fmt < pEnd && *fmt == '.') {
				nPrecision = 0;
				for (++fmt; fmt < pEnd && *fmt >= '0' && *fmt <= '9'; ++fmt)
					nPrecision = nPrecision * 10 + *fmt - '0';
			}

			while (fmt < pEnd && *fmt == 'l')
				++fmt;
			if (fmt < pEnd)
			{
				const char_type ch = *fmt;
				if (ch && !std::isspace(ch))
					++fmt;
				out_.append(t1, nWidth, chFill, nAdjust);
			}
			return fmt - pBegin;
		}
		template<typename char_type, typename T1, typename...TypeList>
		size_t printf_imp(const char_type * fmt, size_t nSize, T1 && t1, TypeList... t2) {
			size_t i = append_prefix(fmt, nSize);
			if (i + 1 >= nSize)
				return nSize;
			i += append_fmt_val(fmt + i, nSize - i, t1);
			if (i >= nSize)
				return nSize;
			return i + printf_imp(fmt + i, nSize - i, std::forward<TypeList>(t2)...);
		}

		appender		out_;
	};
}

#endif//ARA_INTERNAL_STRFORMAT_H
