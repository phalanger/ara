#ifndef ARA_INTERNAL_URL_H
#define ARA_INTERNAL_URL_H

#include <string>
#include "../stringext.h"

namespace ara {
	namespace internal {
		namespace url {

			template <typename CharT>
			inline CharT hex_to_letter(CharT in) {
				switch (in) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					return in + '0';
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				default:
					return in - 10 + 'A';
				}
				return CharT();
			}

			template <typename CharT, class OutputIterator>
			void encode_char(CharT in, OutputIterator &out) {
				switch (in) {
				case 'a':
				case 'A':
				case 'b':
				case 'B':
				case 'c':
				case 'C':
				case 'd':
				case 'D':
				case 'e':
				case 'E':
				case 'f':
				case 'F':
				case 'g':
				case 'G':
				case 'h':
				case 'H':
				case 'i':
				case 'I':
				case 'j':
				case 'J':
				case 'k':
				case 'K':
				case 'l':
				case 'L':
				case 'm':
				case 'M':
				case 'n':
				case 'N':
				case 'o':
				case 'O':
				case 'p':
				case 'P':
				case 'q':
				case 'Q':
				case 'r':
				case 'R':
				case 's':
				case 'S':
				case 't':
				case 'T':
				case 'u':
				case 'U':
				case 'v':
				case 'V':
				case 'w':
				case 'W':
				case 'x':
				case 'X':
				case 'y':
				case 'Y':
				case 'z':
				case 'Z':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '-':
				case '.':
				case '_':
				case '*':
					out++ = in;
					break;
				default:
					out++ = '%';
					out++ = hex_to_letter((in >> 4) & 0x0f);
					out++ = hex_to_letter(in & 0x0f);
					;
				}
			}

			template <typename CharT>
			CharT letter_to_hex(CharT in, bool & boValid) {
				switch (in) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					boValid = true;
					return in - '0';
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					boValid = true;
					return in + 10 - 'a';
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					boValid = true;
					return in + 10 - 'A';
				}
				boValid = false;
				return CharT();
			}

			template <class InputIterator, class OutputIterator>
			OutputIterator encode(const InputIterator &in_begin, const InputIterator &in_end, const OutputIterator &out_begin) {
				InputIterator it = in_begin;
				OutputIterator out = out_begin;
				while (it != in_end) {
					ara::internal::url::encode_char(*it, out);
					++it;
				}
				return out;
			}

			template<class typeString>
			inline typeString encode(const typeString &input) {
				typeString encoded;
				encode(input.begin(), input.end(), std::back_inserter(encoded));
				return encoded;
			}

			template <class InputIterator, class OutputIterator>
			OutputIterator decode(const InputIterator &in_begin, const InputIterator &in_end, const OutputIterator &out_begin) {

				InputIterator it = in_begin;
				OutputIterator out = out_begin;
				bool boValid = false;
				while (it != in_end) {
					if (*it == '%') {
						if (++it == in_end)
							break;
						auto v0 = ara::internal::url::letter_to_hex(*it, boValid);
						if (!boValid) {
							*out++ = '%';
							continue;
						}
						if (++it == in_end)
							break;
						auto v1 = ara::internal::url::letter_to_hex(*it, boValid);
						if (boValid) {
							++it;
							*out++ = 0x10 * v0 + v1;
						} else
							*out++ = v0;
					}
					else if (*it == '+') {
						*out++ = ' ';
						++it;
					}
					else {
						*out++ = *it++;
					}
				}
				return out;
			}

			template<class typeString>
			inline typeString decoded(const typeString &input) {
				typeString decoded;
				decode(input.begin(), input.end(), std::back_inserter(decoded));
				return decoded;
			}

			template<class typeString>
			void split_url(const typeString & strURL, typeString & strSchema, typeString & strHost, typeString & strPath) {
				typename typeString::size_type p = strURL.find(ara::strext(std::string("://")).to<typeString>());
				if (p == typeString::npos) {
					strext(strSchema).clear().append(ara::strext(std::string("http")).to<typeString>());
					p = 0;
				}
				else {
					strSchema = strURL.substr(0, p);
					p += 3;
				}

				typename typeString::size_type p2 = strURL.find('/', p);
				if (p2 == typeString::npos) {
					strext(strPath).clear().append('/');
					strHost = strURL.substr(p);
					return;
				}
				else {
					strPath = strURL.substr(p2);
					strHost = strURL.substr(p, p2 - p);
				}
			}

		}//url
	}//internal
}//ara

#endif //ARA_INTERNAL_URL_H

