
#pragma once

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include <string>


#ifdef _DEBUG
	#define DEBUG_VAR(wstrDebug,var)		std::wstring wstrDebug =ara::strext(jsVar::saveToString(var)).to<std::wstring>();
#else
	#define DEBUG_VAR(cstrDebug,var)
#endif

class jsVar : public rapidjson::Document
{
public:
	typedef rapidjson::Document	typeParent;

	bool	loadFromString(const std::string & str)	{
		Parse(str.c_str());
		return true;
	}
	bool	saveToString(std::string & str) const {
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		this->Accept(writer);
		str = buffer.GetString();
		return true;
	}
	static std::string	saveToString(const rapidjson::Value & val){
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		val.Accept(writer);
		return buffer.GetString();
	}

	template<typename V>
	jsVar & set(const char * strKey, const V & v) {
		if (!IsObject())
			SetObject();
		this->AddMember(rapidjson::StringRef(strKey), rapidjson::Value(v), this->GetAllocator());
		return *this;
	}
	jsVar & set(const char * strKey, const std::string & v) {
		if (!IsObject())
			SetObject();
		rapidjson::Value ra(v.data(), v.size());
		this->AddMember(rapidjson::StringRef(strKey), std::move(ra), this->GetAllocator());
		return *this;
	}
	jsVar & set(const char * strKey, const char * v) {
		if (!IsObject())
			SetObject();
		this->AddMember(rapidjson::StringRef(strKey), rapidjson::StringRef(v), this->GetAllocator());
		return *this;
	}

	template<typename V>
	jsVar & set(rapidjson::Value & val, const char * strKey, const V & v) {
		if (!val.IsObject())
			val.SetObject();
		val.AddMember(rapidjson::StringRef(strKey), rapidjson::Value(v), this->GetAllocator());
		return *this;
	}
	jsVar & set(rapidjson::Value & val, const char * strKey, const std::string & v) {
		if (!val.IsObject())
			val.SetObject();
		rapidjson::Value ra(v.data(), v.size());
		val.AddMember(rapidjson::StringRef(strKey), std::move(ra), this->GetAllocator());
		return *this;
	}
	jsVar & set(rapidjson::Value & val, const char * strKey, const char * v) {
		if (!val.IsObject())
			val.SetObject();
		val.AddMember(rapidjson::StringRef(strKey), rapidjson::StringRef(v), this->GetAllocator());
		return *this;
	}
	jsVar & setObj(const char * strKey, rapidjson::Value & val)
	{
		if (!IsObject())
			SetObject();
		this->AddMember(rapidjson::StringRef(strKey), val, this->GetAllocator());
		return *this;
	}
	rapidjson::Value & addObj(const char * strKey)
	{
		if (!IsObject())
			SetObject();
		rapidjson::Value	v;
		this->AddMember(rapidjson::StringRef(strKey), v, this->GetAllocator());
		auto it = FindMember(rapidjson::StringRef(strKey));
		return it->value;
	}
	rapidjson::Value & addObj(rapidjson::Value & val, const char * strKey) {
		if (!val.IsObject())
			val.SetObject();
		rapidjson::Value	v;
		val.AddMember(rapidjson::StringRef(strKey), v, this->GetAllocator());
		auto it = val.FindMember(rapidjson::StringRef(strKey));
		return it->value;
	}
	rapidjson::Value & addArray(rapidjson::Value & val, const char * strKey)
	{
		// add array object to val with the key strkey
		rapidjson::Value & valAry = addObj(val, strKey);
		valAry.SetArray();
		return valAry;
	}
	rapidjson::Value & addArray(const char * strKey)
	{
		// add array object to root with the key strkey
		rapidjson::Value & valAry = addObj(strKey);
		valAry.SetArray();
		return valAry;
	}
	rapidjson::Value & pushObjToArray(rapidjson::Value & valAry)
	{
		if (!valAry.IsArray())
			valAry.SetArray();
		rapidjson::Value	dummy;
		size_t nIndex = valAry.Size();
		valAry.PushBack(std::move(dummy), this->GetAllocator());
		return valAry[nIndex];
	}

	std::string		getString(const char * strKey, const char * default_type) {
		return getString(*this, strKey, default_type);
	}
	const rapidjson::Value & getObj(const char * strKey) {
		return getObj(*this, strKey);
	}
	static std::string		getString(const rapidjson::Value & val, const char * strKey, const char * default_type) {
		if (!val.IsObject())
			return default_type;
		auto it = val.FindMember(rapidjson::StringRef(strKey));
		if (it == val.MemberEnd())
			return default_type;
		else if (!it->value.IsString())
			return default_type;
		return it->value.GetString();
	}
	template<typename typeInt>
	static typeInt		getInt(const rapidjson::Value & val, const char * strKey, typeInt default_type) {
		if (!val.IsObject())
			return default_type;
		auto it = val.FindMember(rapidjson::StringRef(strKey));
		if (it == val.MemberEnd())
			return default_type;
		else if (it->value.IsInt())
			return static_cast<typeInt>(it->value.GetInt());
		else if (it->value.IsInt64())
			return static_cast<typeInt>(it->value.GetInt64());
		else if (it->value.IsUint())
			return static_cast<typeInt>(it->value.GetUint());
		else if (it->value.IsUint64())
			return static_cast<typeInt>(it->value.GetUint64());
		else if (it->value.IsString())
		{
			std::string s(it->value.GetString(), it->value.GetStringLength());
			return (typeInt)ara::strext(ara::ref_string(s.data())).to<int>();
		}
		return default_type;
	}
	static bool		getBool(const rapidjson::Value & val, const char * strKey, bool default_type) {
		if (!val.IsObject())
			return default_type;
		auto it = val.FindMember(rapidjson::StringRef(strKey));
		if (it == val.MemberEnd())
			return default_type;
		else if (!it->value.IsBool())
			return default_type;
		return it->value.GetBool();
	}
	static double		getDouble(const rapidjson::Value & val, const char * strKey, double default_type) {
		if (!val.IsObject())
			return default_type;
		auto it = val.FindMember(rapidjson::StringRef(strKey));
		if (it == val.MemberEnd())
			return default_type;
		else if (it->value.IsInt())
			return static_cast<double>(it->value.GetInt());
		else if (it->value.IsInt64())
			return static_cast<double>(it->value.GetInt64());
		else if (it->value.IsUint())
			return static_cast<double>(it->value.GetUint());
		else if (it->value.IsUint64())
			return static_cast<double>(it->value.GetUint64());
		else if (!it->value.IsDouble())
			return default_type;
		return it->value.GetDouble();
	}
	static	const rapidjson::Value & getObj(const rapidjson::Value & val, const char * strKey) {
		static const rapidjson::Value	dummy;
		if (!val.IsObject())
			return dummy;
		auto it = val.FindMember(rapidjson::StringRef(strKey));
		if (it == val.MemberEnd())
			return dummy;
		return it->value;
	}
};

