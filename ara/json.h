
#ifndef ARA_RAPIDJSON_H
#define ARA_RAPIDJSON_H

#include "stringext.h"

#include "../3rd/rapidjson/include/rapidjson/reader.h"
#include "../3rd/rapidjson/include/rapidjson/writer.h"
#include "../3rd/rapidjson/include/rapidjson/document.h"

RAPIDJSON_NAMESPACE_BEGIN

	template <typename iterator, typename Char = typename iterator::value_type>
	struct ara_json_string_stream {
		typedef Char Ch;
		ara_json_string_stream(const iterator beg, const iterator end) : src_(beg), head_(beg), end_(end) {}

		Ch Peek() const { return  src_ == end_ ? 0 : (*src_); }
		Ch Take() { Ch c = (src_ == end_) ? 0 : *src_; ++src_; return c; }
		size_t Tell() const { return static_cast<size_t>(src_ - head_); }

		Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
		void Put(Ch) { RAPIDJSON_ASSERT(false); }
		void Flush() { RAPIDJSON_ASSERT(false); }
		size_t PutEnd(Ch *) { RAPIDJSON_ASSERT(false); return 0; }

		iterator src_;     //!< Current read position.
		iterator head_;    //!< Original head of the string.
		iterator end_;
	};

	template <typename iterator, typename Ch>
	struct StreamTraits<ara_json_string_stream<iterator,Ch>> {
		enum { copyOptimization = 1 };
	};

	template <typename Char>
	struct ara_json_insitu_string_stream {
		typedef Char Ch;
		ara_json_insitu_string_stream(Ch * beg, Ch * end) : src_(beg), dst_(0), head_(beg), end_(end) {}

		Ch Peek() const { return  src_ == end_ ? 0 : (*src_); }
		Ch Take() { Ch c = (src_ == end_) ? 0 : *src_; ++src_; return c; }
		size_t Tell() { return static_cast<size_t>(src_ - head_); }

		// Write
		void Put(Ch c) { RAPIDJSON_ASSERT(dst_ != 0); *dst_++ = c; }

		Ch* PutBegin() { return dst_ = src_; }
		size_t PutEnd(Ch* begin) { return static_cast<size_t>(dst_ - begin); }
		void Flush() {}

		Ch* Push(size_t count) { Ch* begin = dst_; dst_ += count; return begin; }
		void Pop(size_t count) { dst_ -= count; }

		Ch * src_;     //!< Current read position.
		Ch * dst_;
		Ch * head_;    //!< Original head of the string.
		Ch * end_;
	};

	template <typename Ch>
	struct StreamTraits<ara_json_insitu_string_stream<Ch>> {
		enum { copyOptimization = 1 };
	};
	RAPIDJSON_NAMESPACE_END

namespace ara {

	template<typename Encoding, typename Allocator>
	class jsvar_imp
	{
	public:
		typedef rapidjson::GenericValue<Encoding, Allocator>		jsvalue;
		typedef rapidjson::GenericDocument<Encoding, Allocator>		jsdoc;
		typedef typename Encoding::Ch								Ch;

		template<typename StackAllocator = rapidjson::CrtAllocator>
			jsvar_imp(rapidjson::GenericDocument<Encoding, Allocator,StackAllocator> & doc) : val_(doc), at_(doc.GetAllocator()) {}
		jsvar_imp(jsvalue & val, Allocator & at) : val_(val), at_(at) {}
		jsvar_imp(const jsvar_imp & js) : val_(js.val_), at_(js.at_) {}

		~jsvar_imp() {}

		template<typename Ch>
		static jsdoc	parse(const Ch * s) {
			jsdoc	t;
			t.Parse(s);
			return t;
		}
		template<typename Ch>
		static jsdoc	parse(Ch * s) {
			jsdoc	t;
			t.ParseInsitu(s);
			return t;
		}
		template<typename typeStr>
		static jsdoc	parse(const typeStr & s, typename std::enable_if<ara::is_string<typeStr>::value, void>::type * dummy = nullptr) {
			const Ch * ch = string_traits<typeStr>::data(s);
			size_t n = string_traits<typeStr>::size(s);
			rapidjson::ara_json_string_stream<const Ch *, Ch>	stream(ch, ch + n);
			jsdoc	t;
			t.template ParseStream<rapidjson::kParseDefaultFlags, Encoding>(stream);
			return t;
		}
		static jsdoc	parse(std::basic_string<Ch> && str) {
			Ch * ch = const_cast<Ch *>(str.data());
			size_t n = str.size();
			rapidjson::ara_json_insitu_string_stream<Ch>	stream(ch, ch + n);
			jsdoc	t;
			t.template ParseStream<rapidjson::kParseInsituFlag, Encoding>(stream);
			return t;
		}

		static jsdoc	init() { return jsdoc(); }

		inline bool	is_null() const { return val_.IsNull(); }
		inline bool	is_number() const { return val_.IsNumber(); }
		inline bool	is_bool() const { return val_.IsBool(); }
		inline bool	is_string() const { return val_.IsString(); }
		inline bool	is_object() const { return val_.IsObject(); }
		inline bool	is_array() const { return val_.IsArray(); }
		inline bool	is_int() const { return val_.IsInt(); }
		inline bool	is_uint() const { return val_.IsUint(); }
		inline bool	is_int64() const { return val_.IsInt64(); }
		inline bool	is_uint64() const { return val_.IsUint64(); }
		inline bool	is_double() const { return val_.IsDouble(); }
		inline bool is_true() const { return val_.IsTrue(); }
		inline bool is_false() const { return val_.IsFalse(); }

		bool append_to_string(std::basic_string<Ch> & str) const {
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			if (!val_.Accept(writer))
				return false;
			str.append(buffer.GetString(), buffer.GetSize());
			return true;
		}

		std::basic_string<Ch> to_string() const {
			std::basic_string<Ch> res;
			append_to_string(res);
			return res;
		}

		template<typename V>
		jsvar_imp & set(const Ch * strKey, const V & v) {
			if (!val_.IsObject())
				val_.SetObject();
			val_.AddMember(rapidjson::StringRef(strKey), rapidjson::Value(v), at_);
			return *this;
		}
		jsvar_imp & set(const Ch * strKey, const std::basic_string<Ch> & v) {
			if (!val_.IsObject())
				val_.SetObject();
			rapidjson::Value ra(v.data(), v.size());
			val_.AddMember(rapidjson::StringRef(strKey), std::move(ra), at_);
			return *this;
		}
		jsvar_imp & set(const char * strKey, const char * v) {
			if (!val_.IsObject())
				val_.SetObject();
			val_.AddMember(rapidjson::StringRef(strKey), rapidjson::StringRef(v), at_);
			return *this;
		}
		jsvar_imp & set(const char * strKey, rapidjson::Value && val) {
			if (!val_.IsObject())
				val_.SetObject();
			val_.AddMember(rapidjson::StringRef(strKey), val, at_);
			return *this;
		}
		jsvar_imp & set(const char * strKey, jsvar_imp && val) {
			if (!val_.IsObject())
				val_.SetObject();
			val_.AddMember(rapidjson::StringRef(strKey), val.val_, at_);
			return *this;
		}

		inline bool has_member(const char * strKey) const {
			auto it = val_.FindMember(rapidjson::StringRef(strKey));
			return it != val_.MemberEnd();
		}

		inline size_t member_count() const {
			return val_.MemberCount();
		}

		jsvar_imp & member(const char * strKey) {
			if (!val_.IsObject())
				val_.SetObject();
			jsvalue	& v = val_.AddMember(rapidjson::StringRef(strKey), v, at_);
			return jsvar_imp(v, at_);
		}


	private:
		jsvalue	&	val_;
		Allocator	& at_;
	};

	typedef jsvar_imp<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>		jsvar;
}

#endif // ARA_RAPIDJSON_H

