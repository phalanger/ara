
#ifndef ARA_JSON_H
#define ARA_JSON_H

#include "variant.h"
#include <list>

#include "rapidjson/rapidjson.h"

namespace ara {

	namespace internal {

		template<bool boCanRef>
		class	ara_json_handler {
		public:
			ara_json_handler(var & doc) {
				list_stack_.push_back(&doc);
				doc.set_null();
			}

			bool Null() {
				auto last = get_last();
				if (last == nullptr)
					return false;
				last->set_null();
				return true;
			}
			bool Bool(bool b) { return assign(b); }
			bool Int(int i) { return assign(i); }
			bool Uint(unsigned i) { return assign(i); }
			bool Int64(int64_t i) { return assign(i); }
			bool Uint64(uint64_t i) { return assign(i); }
			bool Double(double d) { return assign(d); }
			bool RawNumber(const char * str, size_t length, bool copy) {
				if (boCanRef && !copy)
					return assign(ref_string(str, length));
				return assign(std::string(str, length));
			}
			bool String(const char * str, size_t length, bool copy) {
				if (boCanRef && !copy)
					return assign(ref_string(str, length));
				return assign(std::string(str, length));
			}
			bool StartObject() {
				auto last = get_last();
				if (last == nullptr)
					return false;
				last->to_dict();
				list_stack_.push_back(last);
				return true;
			}
			bool Key(const char * str, size_t length, bool copy) {
				if (boCanRef && !copy)
					key_ = key_string::ref(str, length);
				else
					key_ = key_string::copy(str, length);
				return true;
			}
			bool EndObject(size_t memberCount) {
				if (list_stack_.empty())
					return false;
				list_stack_.pop_back();
				return true;
			}
			bool StartArray() {
				auto last = get_last();
				if (last == nullptr)
					return false;
				last->to_array();
				list_stack_.push_back(last);
				return true;
			}
			bool EndArray(size_t elementCount) {
				if (list_stack_.empty())
					return false;
				list_stack_.pop_back();
				return true;
			}
		protected:
			template<typename T>
			bool		assign(T && t) {
				auto last = get_last();
				if (last == nullptr)
					return false;
				*last = std::move(t);
				return true;
			}
			var *	get_last() {
				if (list_stack_.empty())
					return nullptr;
				var * last = list_stack_.back();
				if (last->is_dict())
					return &((*last)[key_]);
				else if (last->is_array()) {
					auto & ary = last->get_array_modify();
					ary.push_back(var());
					return &(ary.back());
				}
				return last;
			}
			key_string		key_;
			std::list<var *>	list_stack_;
		};

		template <typename Encoding>
		struct ARAGenericStringStream {
			typedef typename Encoding::Ch Ch;

			ARAGenericStringStream(const Ch *src, size_t nSize) : src_(src), head_(src), end_(src + nSize) {}

			Ch Peek() const { return src_ == end_ ? 0 : *src_; }
			Ch Take() { return src_ == end_ ? 0 : *src_++; }
			size_t Tell() const { return static_cast<size_t>(src_ - head_); }

			Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
			void Put(Ch) { RAPIDJSON_ASSERT(false); }
			void Flush() { RAPIDJSON_ASSERT(false); }
			size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

			const Ch* src_;     //!< Current read position.
			const Ch* head_;    //!< Original head of the string.
			const Ch* end_;    //!< Original head of the string.
		};

		template <typename Encoding>
		struct ARAGenericInsituStringStream {
			typedef typename Encoding::Ch Ch;

			ARAGenericInsituStringStream(Ch *src, size_t nSize) : src_(src), dst_(0), head_(src), end_(src + nSize) {}

			// Read
			Ch Peek() const { return src_ == end_ ? 0 : *src_; }
			Ch Take() { return src_ == end_ ? 0 : *src_++; }
			size_t Tell() { return static_cast<size_t>(src_ - head_); }

			// Write
			void Put(Ch c) { RAPIDJSON_ASSERT(dst_ != 0); *dst_++ = c; }

			Ch* PutBegin() { return dst_ = src_; }
			size_t PutEnd(Ch* begin) { return static_cast<size_t>(dst_ - begin); }
			void Flush() {}

			Ch* Push(size_t count) { Ch* begin = dst_; dst_ += count; return begin; }
			void Pop(size_t count) { dst_ -= count; }

			Ch* src_;
			Ch* dst_;
			Ch* head_;
			Ch* end_;
		};

		/////////////////////////////////////////////////////////////////////

		template <typename Encoding>
		class ARAGenericStringBuffer {
		public:
			typedef typename Encoding::Ch Ch;
			typedef std::basic_string<Ch, std::char_traits<Ch>>		typeStr;

			ARAGenericStringBuffer(typeStr & str) : str_(str), old_size_(str_.size()) {}

			inline void Put(Ch c) { str_ += c; }
			inline void Flush() {}

			inline void Clear() { str_.resize(old_size_); }
			inline void ShrinkToFit() {}

			Ch* Push(size_t count) {
				size_t n = str_.size();
				str_.resize(n + count);
				return const_cast<Ch *>(str_.data() + n);
			}
			void Pop(size_t count) {
				size_t n = str_.size();
				if (n > count && n - count >= old_size_)
					str_.resize(n - count);
				else
					Clear();
			}

			const Ch* GetString() const {
				// Push and pop a null terminator. This is safe.
				return str_.c_str();
			}

			size_t GetSize() const { return str_.size(); }

		private:
			typeStr		& str_;
			size_t		old_size_ = 0;
		};

	}//interanl
}//ara

#include "rapidjson/stream.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

RAPIDJSON_NAMESPACE_BEGIN

	template <typename Encoding>
	struct StreamTraits<ara::internal::ARAGenericStringStream<Encoding> > {
		enum { copyOptimization = 1 };
	};

RAPIDJSON_NAMESPACE_END

namespace ara {
	namespace internal {

		template<typename Ch>
		struct json_mem_encoding {
			using encoding_type = RAPIDJSON_NAMESPACE::ASCII<Ch>;
		};
		template<>
		struct json_mem_encoding<char> {
			using encoding_type = RAPIDJSON_NAMESPACE::UTF8<>;
		};
		template<>
		struct json_mem_encoding<wchar_t> {
			using encoding_type = RAPIDJSON_NAMESPACE::UTF16<wchar_t>;
		};
		template<>
		struct json_mem_encoding<char16_t> {
			using encoding_type = RAPIDJSON_NAMESPACE::UTF16<char16_t>;
		};
		template<>
		struct json_mem_encoding<char32_t> {
			using encoding_type = RAPIDJSON_NAMESPACE::UTF32<char32_t>;
		};

		class VarWriter
		{
		public:
			template<class typeWriter>
			static bool	output(const var & v, typeWriter & w) {
				switch (v.get_type()) {
				case var::TYPE_NULL:
					return w.Null();
				case var::TYPE_BOOL:
					return w.Bool(v.get_bool());
				case var::TYPE_INT:
					return w.Int(v.get_int());
				case var::TYPE_INT64:
					return w.Int64(v.get_int64());
				case var::TYPE_DOUBLE:
					return w.Double(v.get_double());
				case var::TYPE_STRING:
				case var::TYPE_CONST_STRING:
				{
					auto s = v.get_string();
					return w.String(s.data(), static_cast<RAPIDJSON_NAMESPACE::SizeType>(s.size()), false);
				}
				case var::TYPE_ARRAY:
				{
					if (!w.StartArray())
						return false;
					const auto & a = v.get_array();
					RAPIDJSON_NAMESPACE::SizeType count = 0;
					for (const auto & i : a) {
						if (!output(i, w))
							break;
						++count;
					}
					return w.EndArray(count);
				}
				case var::TYPE_DICT:
				{
					if (!w.StartObject())
						return false;
					const auto & a = v.get_dict();
					RAPIDJSON_NAMESPACE::SizeType count = 0;
					for (const auto & i : a) {
						const auto & key = i.first;
						if (!w.Key(key.data(), static_cast<RAPIDJSON_NAMESPACE::SizeType>(key.size())))
							break;
						else if (!output(i.second, w))
							break;
						++count;
					}
					return w.EndObject(count);
				}
				default:
					break;
				}
				return false;
			}
		};
	}//internal

	class json{
	public:
		using utf8 = RAPIDJSON_NAMESPACE::UTF8<>;
		using utf16 = RAPIDJSON_NAMESPACE::UTF16<>;
		using utf16le = RAPIDJSON_NAMESPACE::UTF16LE<>;
		using utf16be = RAPIDJSON_NAMESPACE::UTF16BE<>;
		using utf32be = RAPIDJSON_NAMESPACE::UTF32BE<>;
		using utf32le = RAPIDJSON_NAMESPACE::UTF32LE<>;
		using ascii = RAPIDJSON_NAMESPACE::ASCII<>;

		enum ParseFlag {
			kParseNoFlags = 0,              //!< No flags are set.
			kParseInsituFlag = 1,           //!< In-situ(destructive) parsing.
			kParseValidateEncodingFlag = 2, //!< Validate encoding of JSON strings.
			kParseIterativeFlag = 4,        //!< Iterative(constant complexity in terms of function call stack size) parsing.
			kParseStopWhenDoneFlag = 8,     //!< After parsing a complete JSON root from stream, stop further processing the rest of stream. When this flag is used, parser will not generate kParseErrorDocumentRootNotSingular error.
			kParseFullPrecisionFlag = 16,   //!< Parse number in full precision (but slower).
			kParseCommentsFlag = 32,        //!< Allow one-line (//) and multi-line (/**/) comments.
			kParseNumbersAsStringsFlag = 64,    //!< Parse all numbers (ints/doubles) as strings.
			kParseTrailingCommasFlag = 128, //!< Allow trailing commas at the end of objects and arrays.
			kParseNanAndInfFlag = 256,      //!< Allow parsing NaN, Inf, Infinity, -Inf and -Infinity as doubles.
			kParseDefaultFlags = kParseNoFlags,  //!< Default parse flags. Can be customized by defining RAPIDJSON_PARSE_DEFAULT_FLAGS
			kParseRef = 1024,
		};

	
		template<typename Char>
		inline static bool	parse(var & v, const Char * str, size_t n = 0) {
			return generic_parse<kParseNoFlags, typename ara::internal::json_mem_encoding<Char>::encoding_type>(v, str, n == 0 ? std::char_traits<Char>::length(str) : n);
		}
		template<typename Char>
		inline static var	parse(const Char * str, size_t n = 0) {
			var t;
			parse(t, str, n);
			return t;
		}

		template<typename typeStr, typename = typename std::enable_if<is_string<typeStr>::value>::type>
		inline static bool	parse(var & v, const typeStr & str) {
			typedef typename string_traits<typeStr>::value_type	Char;
			return generic_parse<kParseNoFlags, typename ara::internal::json_mem_encoding<Char>::encoding_type>(v, string_traits<typeStr>::data( str ), string_traits<typeStr>::size( str ));
		}
		template<typename typeStr, typename = typename std::enable_if<is_string<typeStr>::value>::type>
		inline static var	parse(const typeStr & str) {
			var t;
			parse(t, str);
			return t;
		}

		template<typename typeStr, typename = typename std::enable_if<is_string<typeStr>::value>::type>
		inline static bool	parse(var & v, typeStr && str) {
			typedef typename string_traits<typeStr>::value_type	Char;
			return generic_parse<kParseInsituFlag, typename ara::internal::json_mem_encoding<Char>::encoding_type>(v, const_cast<Char *>(string_traits<typeStr>::data(str)), string_traits<typeStr>::size(str));
		}
		template<typename typeStr, typename = typename std::enable_if<is_string<typeStr>::value>::type>
		inline static var	parse(typeStr && str) {
			var t;
			parse(t, std::move(str));
			return t;
		}

		template<typename Char>
		inline static bool	parse(var & v, Char * str, size_t n = 0) {
			return generic_parse<kParseInsituFlag, typename ara::internal::json_mem_encoding<Char>::encoding_type>(v, str, n == 0 ? std::char_traits<Char>::length(str) : n);
		}
		template<typename Char>
		inline static var	parse(Char * str, size_t n = 0) {
			var t;
			parse(t, str, n);
			return t;
		}

		/////////////////////////////////////////////////////////////////////////////////

		inline static bool	parse_ref(var & v, const char * str, size_t n = 0) {
			return generic_parse<kParseRef>(v, str, n == 0 ? std::char_traits<char>::length(str) : n);
		}
		inline static var	parse_ref(const char * str, size_t n = 0) {
			var t;
			parse_ref(t, str, n);
			return t;
		}

		template<typename typeStr, typename = typename std::enable_if<is_string<typeStr>::value>::type>
		inline static bool	parse_ref(var & v, const typeStr & str) {
			typedef typename string_traits<typeStr>::value_type	Char;
			return generic_parse<kParseRef, typename ara::internal::json_mem_encoding<Char>::encoding_type>(v, string_traits<typeStr>::data( str ), string_traits<typeStr>::size( str ));
		}
		template<typename typeStr, typename = typename std::enable_if<is_string<typeStr>::value>::type>
		inline static var	parse_ref(const typeStr & str) {
			var t;
			parse_ref(t, str);
			return t;
		}

		inline static bool	parse_ref(var & v, std::string && str) {
			return generic_parse<kParseInsituFlag|kParseRef>(v, const_cast<char *>(str.data()), str.size());
		}
		inline static var	parse_ref(std::string && str) {
			var t;
			parse_ref(t, std::move(str));
			return t;
		}

		inline static bool	parse_ref(var & v, char * str, size_t n = 0) {
			return generic_parse<kParseInsituFlag|kParseRef>(v, str, n == 0 ? std::char_traits<char>::length(str) : n);
		}
		inline static var	parse_ref(char * str, size_t n = 0) {
			var t;
			parse_ref(t, str, n);
			return t;
		}

		/////////////////////////////////////////////////////////////////

		template<typename typeString>
		inline static typeString	save(const var & v) {

			typeString		strStore;
			typedef typename typeString::value_type	Char;

			generic_save<Char, std::char_traits<Char>, ara::json::utf8, typename ara::internal::json_mem_encoding<Char>::encoding_type >(v, strStore);
			return strStore;
		}

		template<typename typeString>
		inline static bool	save_to(const var & v, typeString & strStore) {

			typedef typename typeString::value_type	Char;
			return generic_save<Char, std::char_traits<Char>, ara::json::utf8, typename ara::internal::json_mem_encoding<Char>::encoding_type >(v, strStore);
		}

		/////////////////////////////////////////////////////////////////

		template<typename typeString>
		inline static typeString	pretty_save(const var & v, int indentChar = ' ', unsigned indentCharCount = 4) {

			typeString		strStore;
			typedef typename typeString::value_type	Char;

			generic_pretty_save<Char, std::char_traits<Char>, ara::json::utf8, typename ara::internal::json_mem_encoding<Char>::encoding_type >(v, strStore, indentChar, indentCharCount);
			return strStore;
		}

		template<typename typeString>
		inline static bool	pretty_save_to(const var & v, typeString & strStore, int indentChar = ' ', unsigned indentCharCount = 4) {

			typedef typename typeString::value_type	Char;
			return generic_pretty_save<Char, std::char_traits<Char>, ara::json::utf8, typename ara::internal::json_mem_encoding<Char>::encoding_type >(v, strStore, indentChar, indentCharCount);
		}

		/////////////////////////////////////////////////////////////////

		template <unsigned parseFlags, typename SourceEncoding = ara::json::utf8, typename TargetEncoding = ara::json::utf8>
			static bool		generic_parse(var & v, const typename SourceEncoding::Ch * str, size_t nSize) {

				ara::internal::ARAGenericStringStream<SourceEncoding> s(str, nSize);
				RAPIDJSON_NAMESPACE::GenericReader<SourceEncoding, TargetEncoding> reader;
				ara::internal::ara_json_handler<(parseFlags & kParseRef) != 0>	handler(v);
				auto parseResult_ = reader.template Parse<parseFlags>(s, handler);
				if (parseResult_ || parseResult_ == RAPIDJSON_NAMESPACE::kParseErrorDocumentEmpty)
					return true;

				return false;
			}
		template <unsigned parseFlags, typename SourceEncoding = ara::json::utf8, typename TargetEncoding = ara::json::utf8>
			static bool		generic_parse(var & v, typename SourceEncoding::Ch * str, size_t nSize) {

				ara::internal::ARAGenericInsituStringStream<SourceEncoding> s(str, nSize);
				RAPIDJSON_NAMESPACE::GenericReader<SourceEncoding, TargetEncoding> reader;
				ara::internal::ara_json_handler<(parseFlags & kParseRef) != 0>	handler(v);
				auto parseResult_ = reader.template Parse<parseFlags | kParseInsituFlag>(s, handler);
				if (parseResult_ || parseResult_ == RAPIDJSON_NAMESPACE::kParseErrorDocumentEmpty)
					return true;

				return false;
			}

		/////////////////////////////////////////////////////////////
		template<typename Char, typename CharTraits = std::char_traits<Char>, typename SourceEncoding = ara::json::utf8, typename TargetEncoding = ara::json::utf8>
			inline static bool	generic_save(const var & v, std::basic_string<Char, CharTraits>	& strStore) {

				typedef ara::internal::ARAGenericStringBuffer<TargetEncoding>		araStringBuffer;
				araStringBuffer	buf(strStore);
				RAPIDJSON_NAMESPACE::Writer<araStringBuffer, SourceEncoding, TargetEncoding> writer(buf);

				return internal::VarWriter::output(v, writer);
			}

		/////////////////////////////////////////////////////////////
		template<typename Char, typename CharTraits = std::char_traits<Char>, typename SourceEncoding = ara::json::utf8, typename TargetEncoding = ara::json::utf8>
			inline static bool	generic_pretty_save(const var & v, std::basic_string<Char, CharTraits> & strStore, typename SourceEncoding::Ch indentChar = ' ', unsigned indentCharCount = 4) {

				typedef ara::internal::ARAGenericStringBuffer<TargetEncoding>		araStringBuffer;
				araStringBuffer		buf(strStore);
				RAPIDJSON_NAMESPACE::PrettyWriter<araStringBuffer, SourceEncoding, TargetEncoding> writer(buf);
				writer.SetIndent(indentChar, indentCharCount);

				return internal::VarWriter::output(v, writer);
			}
	};
}

inline ara::var operator "" _json(const char * p, size_t n) {
	return ara::json::parse(p, n);
}

#endif // ARA_JSON_H

