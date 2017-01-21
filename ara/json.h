
#ifndef ARA_RAPIDJSON_H
#define ARA_RAPIDJSON_H

#include "ara_def.h"

#include "../3rd/rapidjson/include/rapidjson/reader.h"
#include "../3rd/rapidjson/include/rapidjson/writer.h"
#include "../3rd/rapidjson/include/rapidjson/document.h"

namespace ara {

	template<typename Encoding, typename Allocator>
	class jsvar_imp
	{
	public:
		typedef rapidjson::GenericValue<Encoding, Allocator>		jsvalue;
		typedef rapidjson::GenericDocument<Encoding, Allocator>		jsdoc;

		template<typename StackAllocator = rapidjson::CrtAllocator>
			jsvar_imp(rapidjson::GenericDocument<Encoding, Allocator,StackAllocator> & doc) : val_(doc), at_(doc.GetAllocator()) {}
		jsvar_imp(jsvalue & val, Allocator & at) : val_(val), at_(at) {}
		jsvar_imp(const jsvar_imp & js) : val_(js.val_), at_(js.at_) {}

		~jsvar_imp() {}

		static jsdoc	parse(const std::string & s);
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

	private:
		jsvalue	&	val_;
		Allocator	& at_;
	};

	typedef jsvar_imp<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>		jsvar;
}

#endif // ARA_RAPIDJSON_H

