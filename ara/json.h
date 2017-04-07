
#ifndef ARA_JSON_H
#define ARA_JSON_H

#include "variant.h"
#include <list>

#include "rapidjson/reader.h"
#include "rapidjson/writer.h"

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
			bool String(const char * str, size_t length, bool copy) {
				if (boCanRef && !copy)
					return assign(ref_string(str, length));
				return assign( std::string(str, length) );
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
		struct GenericStringStream {
			typedef typename Encoding::Ch Ch;

			GenericStringStream(const Ch *src, size_t nSize) : src_(src), head_(src), end_(src + nSize) {}

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
		struct GenericInsituStringStream {
			typedef typename Encoding::Ch Ch;

			GenericInsituStringStream(Ch *src, size_t nSize) : src_(src), dst_(0), head_(src), end_(src + nSize) {}

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
	}

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
			kParseRef = 64,
			kParseDefaultFlags = kParseNoFlags  //!< Default parse flags. Can be customized by defining RAPIDJSON_PARSE_DEFAULT_FLAGS
		};

	
		static bool	parse(var & v, const char * str, size_t n) {
			return generic_parse<kParseNoFlags>(v, str, n);
		}
		static var	parse(const char * str, size_t n) {
			var t;
			generic_parse<kParseNoFlags>(t, str, n);
			return t;
		}

		static bool	parse(var & v, const std::string & str) {
			return generic_parse<kParseNoFlags>(v, str.data(), str.size());
		}
		static var	parse(const std::string & str) {
			var t;
			generic_parse<kParseNoFlags>(t, str.data(), str.size());
			return t;
		}

		static bool	parse(var & v, std::string && str) {
			return generic_parse<kParseInsituFlag>(v, const_cast<char *>(str.data()), str.size());
		}
		static var	parse(std::string && str) {
			var t;
			generic_parse<kParseInsituFlag>(t, const_cast<char *>(str.data()), str.size());
			return t;
		}

		static bool	parse(var & v, char * str, size_t n) {
			return generic_parse<kParseInsituFlag>(v, str, n);
		}
		static var	parse(char * str, size_t n) {
			var t;
			generic_parse<kParseInsituFlag>(t, str, n);
			return t;
		}

		/////////////////////////////////////////////////////////////////////////////////

		static bool	parse_ref(var & v, const char * str, size_t n) {
			return generic_parse<kParseRef>(v, str, n);
		}
		static var	parse_ref(const char * str, size_t n) {
			var t;
			generic_parse<kParseRef>(t, str, n);
			return t;
		}

		static bool	parse_ref(var & v, const std::string & str) {
			return generic_parse<kParseRef>(v, str.data(), str.size());
		}
		static var	parse_ref(const std::string & str) {
			var t;
			generic_parse<kParseRef>(t, str.data(), str.size());
			return t;
		}

		static bool	parse_ref(var & v, std::string && str) {
			return generic_parse<kParseInsituFlag|kParseRef>(v, const_cast<char *>(str.data()), str.size());
		}
		static var	parse_ref(std::string && str) {
			var t;
			generic_parse<kParseInsituFlag|kParseRef>(t, const_cast<char *>(str.data()), str.size());
			return t;
		}

		static bool	parse_ref(var & v, char * str, size_t n) {
			return generic_parse<kParseInsituFlag|kParseRef>(v, str, n);
		}
		static var	parse_ref(char * str, size_t n) {
			var t;
			generic_parse<kParseInsituFlag|kParseRef>(t, str, n);
			return t;
		}

		/////////////////////////////////////////////////////////////////

		template <unsigned parseFlags, typename SourceEncoding = ara::json::utf8, typename TargetEncoding = ara::json::utf8>
			static bool		generic_parse(var & v, const typename SourceEncoding::Ch * str, size_t nSize) {

				ara::internal::GenericStringStream<SourceEncoding> s(str, nSize);
				RAPIDJSON_NAMESPACE::GenericReader<SourceEncoding, TargetEncoding> reader;
				ara::internal::ara_json_handler<(parseFlags & kParseRef) != 0>	handler(v);
				auto parseResult_ = reader.template Parse<parseFlags>(s, handler);
				if (parseResult_ || parseResult_ == RAPIDJSON_NAMESPACE::kParseErrorDocumentEmpty)
					return true;

				return false;
			}
		template <unsigned parseFlags, typename SourceEncoding = ara::json::utf8, typename TargetEncoding = ara::json::utf8>
			static bool		generic_parse(var & v, typename SourceEncoding::Ch * str, size_t nSize) {

				ara::internal::GenericInsituStringStream<SourceEncoding> s(str, nSize);
				RAPIDJSON_NAMESPACE::GenericReader<SourceEncoding, TargetEncoding> reader;
				ara::internal::ara_json_handler<(parseFlags & kParseRef) != 0>	handler(v);
				auto parseResult_ = reader.template Parse<parseFlags | kParseInsituFlag>(s, handler);
				if (parseResult_ || parseResult_ == RAPIDJSON_NAMESPACE::kParseErrorDocumentEmpty)
					return true;

				return false;
			}
	};
}

inline ara::var operator "" _json(const char * p, size_t n) {
	return ara::json::parse(p, n);
}


#endif // ARA_JSON_H

