
#ifndef ARA_VARIANT_H
#define ARA_VARIANT_H

#include "ref_string.h"
#include "key_string.h"
#include "stringext.h"
#include "internal/variant_convert.h"

#include <vector>
#include <map>
#include <functional>

namespace ara {

	class var {
		template<class T, size_t>
		struct var_holder {
			inline void	init(T v) { f_ = v; }
			inline void destroy() {}
			inline void set(T d) { f_ = d; }
			inline const T & get() const { return f_; }
			inline T & get() { return f_; }
		protected:
			T f_;
		};
		template<class T>
		struct var_holder<T, 4> {
			inline void	init(T v) { f_ = new T; *f_ = v; }
			inline void destroy() { delete f_; }
			inline void set(T d) { *f_ = d; }
			inline const T & get() const { return *f_; }
			inline T & get() { return *f_; }
		protected:
			T * f_;
		};
	public:
		typedef std::vector<var>			var_array;
		typedef std::map<key_string, var>	var_dict;

		enum TYPE {
			TYPE_NULL = 0x00,
			TYPE_BOOL = 0x01,
			TYPE_INT = 0x02,
			TYPE_INT64 = 0x03,
			TYPE_DOUBLE = 0x04,
			TYPE_STRING = 0x05,
			TYPE_ARRAY = 0x06,
			TYPE_DICT = 0x07,

			TYPE_MASK = 0x0f,
			TYPE_BOOL_FALSE = 0xf0,
			TYPE_CONST = 0x100,	//used only for string type. std::string or ara::ref_string
			TYPE_REF = 0x200,

			TYPE_CONST_STRING = TYPE_STRING | TYPE_CONST,
		};
		inline ~var() { destroy(); }

		inline void	set_null() {
			destroy();
			type_ = TYPE_NULL;
			ptr_ = 0;
		}

		var() : ptr_(0), type_(TYPE_NULL) {}
		var(bool b) : b_(b), type_(TYPE_BOOL) {}
		var(int8_t n) : i_(static_cast<int>(n)), type_(TYPE_INT) {}
		var(uint8_t n) : i_(static_cast<int>(n)), type_(TYPE_INT) {}
		var(int16_t n) : i_(static_cast<int>(n)), type_(TYPE_INT) {}
		var(uint16_t n) : i_(static_cast<int>(n)), type_(TYPE_INT) {}
		var(int32_t n) : i_(static_cast<int>(n)), type_(TYPE_INT) {}
		var(uint32_t n) : i_(static_cast<int>(n)), type_(TYPE_INT) {}
		var(int64_t n) : type_(TYPE_INT64) { i64_.init(n); }
		var(uint64_t n) : type_(TYPE_INT64) { i64_.init(static_cast<int64_t>(n)); }

		var(float n) : type_(TYPE_DOUBLE) { f_.init(static_cast<double>(n)); }
		var(double n) : type_(TYPE_DOUBLE) { f_.init(n); }

		var(const std::string & s) : str_(new std::string(s)), type_(TYPE_STRING) {}
		var(std::string && s) : str_(new std::string(std::move(s))), type_(TYPE_STRING) {}
		var(std::string * s) : str_(s), type_(TYPE_STRING) {}

		var(const char * s) : ref_str_(new ref_string(s)), type_(TYPE_CONST_STRING) {}
		var(const ref_string & s) : ref_str_(new ref_string(s)), type_(TYPE_CONST_STRING) {}
		var(ref_string && s) : ref_str_(new ref_string(s)), type_(TYPE_CONST_STRING) {}
		var(ref_string * s) : ref_str_(s), type_(TYPE_CONST_STRING) {}

		var(const var_array & s) : array_(new var_array(s)), type_(TYPE_ARRAY) {}
		var(var_array && s) : array_(new var_array(std::move(s))), type_(TYPE_ARRAY) {}
		var(var_array * s) : array_(s), type_(TYPE_ARRAY) {}

		var(const var_dict & s) : dict_(new var_dict(s)), type_(TYPE_DICT) {}
		var(var_dict && s) : dict_(new var_dict(std::move(s))), type_(TYPE_DICT) {}
		var(var_dict * s) : dict_(s), type_(TYPE_DICT) {}

		var(std::initializer_list<var> list) : array_(new var_array(std::move(list))), type_(TYPE_ARRAY) {}

		var(const var & r) : ptr_(r.ptr_), type_(r.type_) {
			switch (type_) {
			case TYPE_INT64:
				i64_.init(r.i64_.get());	break;
			case TYPE_DOUBLE:
				f_.init(r.f_.get());	break;
			case TYPE_STRING:
				str_ = new std::string(*r.str_);	break;
			case TYPE_CONST_STRING:
				ref_str_ = new ref_string(*r.ref_str_);	break;
			case TYPE_ARRAY:
				array_ = new var_array(*r.array_);	break;
			case TYPE_DICT:
				dict_ = new var_dict(*r.dict_);	break;
			default:
				break;
			}
		}
		var(var && r) : ptr_(r.ptr_), type_(r.type_) {
			r.type_ = TYPE_NULL;
			r.ptr_ = 0;
		}

		template<typename T>
		var(const key_string & k, const T & t) : dict_(new var_dict), type_(TYPE_DICT) {
			var tmp(t);
			(*dict_)[k].swap( tmp );
		}
		template<typename T>
		var(const key_string & k, T && t) : dict_(new var_dict), type_(TYPE_DICT) {
			var tmp(std::move(t));
			(*dict_)[k].swap( tmp );
		}

		var & ref(const var & v) {
			destroy();
			type_ = v.type_;
			switch (type_) {
			case TYPE_INT64:
				i64_.init(v.i64_.get());	break;
			case TYPE_DOUBLE:
				f_.init(v.f_.get());	break;
			case TYPE_STRING:
			case TYPE_CONST_STRING:
			case TYPE_ARRAY:
			case TYPE_DICT:
				ptr_ = v.ptr_;
				type_ |= TYPE_REF;	break;
			default:
				ptr_ = v.ptr_;
				break;
			}
			return *this;
		}

		inline TYPE	get_type() const {
			return static_cast<TYPE>(type_ & TYPE_MASK);
		}

		inline bool	is_null() const { return type_ == TYPE_NULL; }
		inline bool is_bool() const { return get_type() == TYPE_BOOL; }
		inline bool is_int() const { return get_type() == TYPE_INT; }
		inline bool is_int64() const { return get_type() == TYPE_INT64; }
		inline bool is_double() const { return get_type() == TYPE_DOUBLE; }
		inline bool is_string() const { return get_type() == TYPE_STRING; }
		inline bool is_ref_string() const { return (type_ & (~TYPE_REF)) == (TYPE_CONST_STRING); }
		inline bool is_std_string() const { return (type_ & (~TYPE_REF)) == TYPE_STRING; }
		inline bool is_array() const { return get_type() == TYPE_ARRAY; }
		inline bool is_dict() const { return get_type() == TYPE_DICT; }

		bool	get_bool() const { check_type(TYPE_BOOL); return b_; }
		bool &	get_bool_modify() { check_type(TYPE_BOOL); return b_; }
		int		get_int() const { check_type(TYPE_INT); return i_; }
		int	&	get_int_modify() { check_type(TYPE_INT); return i_; }
		int64_t		get_int64() const { check_type(TYPE_INT64); return i64_.get(); }
		int64_t	&	get_int64_modify() { check_type(TYPE_INT64); return i64_.get(); }
		double_t		get_double() const { check_type(TYPE_DOUBLE); return f_.get(); }
		double_t	&	get_double_modify() { check_type(TYPE_DOUBLE); return f_.get(); }
		ref_string	get_string() const {
			check_type(TYPE_STRING);
			if (type_ & TYPE_CONST)
				return *ref_str_;
			else
				return ref_string(*str_);
		}
		std::string &	get_string_modify() { 
			check_type(TYPE_STRING); 
			if (type_ & TYPE_CONST) {
				std::string	r(ref_str_->str());
				if ((type_ & TYPE_REF) == 0)
					delete ref_str_;
				str_ = new std::string(std::move(r));
				type_ = TYPE_STRING;
			}
			return *str_;
		}
		const var_array	&	get_array() const { check_type(TYPE_ARRAY); return *array_; }
		var_array	&	get_array_modify() { check_type(TYPE_ARRAY); return *array_; }
		const var_dict	&	get_dict() const { check_type(TYPE_DICT); return *dict_; }
		var_dict	&	get_dict_modify() { check_type(TYPE_DICT); return *dict_; }

		template<typename T>
		inline T 	get() const { return get_imp(ara::type_id<T>()); }

		template<typename T, typename keyType>
		inline T  get(const keyType & key, const T & defaultVal) const {
			return get_path_imp(key_string::ref(key), defaultVal);
		}
		template<typename keyType>
		inline const std::string &	get(const keyType & key, const std::string & defaultVal) const {
			return get_path_imp(key_string::ref(key), defaultVal);
		}
		template<typename keyType>
		inline const std::string &	get(const keyType & key, const ref_string & defaultVal) const {
			return get_path_imp(key_string::ref(key), defaultVal);
		}
		template<typename keyType>
		inline const var &	get(const keyType & key, const var & defaultVal) const {
			return get_path_imp(key_string::ref(key), defaultVal);
		}

		template<typename T, typename Convertor = internal::default_variant_convert>
		T	to() const {
			switch (get_type()) {
			case TYPE_NULL:
				return Convertor::to(ara::type_id<T>());
			case TYPE_BOOL:
				return Convertor::to(ara::type_id<T>(), b_);
			case TYPE_INT:
				return Convertor::to(ara::type_id<T>(), i_);
			case TYPE_INT64:
				return Convertor::to(ara::type_id<T>(), i64_.get());
			case TYPE_DOUBLE:
				return Convertor::to(ara::type_id<T>(), f_.get());
			case TYPE_STRING:
				if (type_ & TYPE_CONST)
					return Convertor::to(ara::type_id<T>(), *ref_str_);
				else
					return Convertor::to(ara::type_id<T>(), *str_);
			default:
				break;
			}
			return T();
		}
		template<typename T, typename keyType, typename Convertor = internal::default_variant_convert>
		T	find_and_convert_to(const keyType & key, const T & nDefault) const {
			check_type(TYPE_DICT);
			auto it = get_dict().find(key);
			if (it == get_dict().end())
				return  nDefault;
			else
				return it->second.to<T>();
		}

		var_dict	&	to_dict() { 
			if (get_type() != TYPE_DICT)
				convert_to_type(TYPE_DICT);
			return *dict_; 
		}
		var_array&	to_array() {
			if (get_type() != TYPE_ARRAY)
				convert_to_type(TYPE_ARRAY);
			return *array_;
		}

		size_t	array_size() const {
			return (get_type() == TYPE_ARRAY) ? array_->size() : 0;
		}
		size_t	dict_size() const {
			return (get_type() == TYPE_DICT) ? dict_->size() : 0;
		}

		const var &	operator[](size_t index) const {
			check_type(TYPE_ARRAY);
			return (*array_)[index];
		}
		var &	operator[](size_t index) {
			if (get_type() != TYPE_ARRAY) 
				convert_to_type(TYPE_ARRAY); 
			return (*array_)[index];
		}
		const var &	operator[](const key_string & key) const {
			check_type(TYPE_ARRAY);
			auto it = dict_->find(key);
			if (it == dict_->end())
				return static_empty<var>::val;
			return it->second;
		}
		var &	operator[](const key_string & key) {
			if (get_type() != TYPE_DICT)
				convert_to_type(TYPE_DICT);
			return (*dict_)[key];
		}

		template<typename T>
		var & operator=(const T & v) {
			var tmp(v);
			swap(tmp);
			return *this;
		}
		template<typename T>
		var & operator=(T && v) {
			var tmp( std::forward<T>(v) );
			swap(tmp);
			return *this;
		}

		template<typename T>
		var & operator()(const key_string & key, const T & v) {
			var tmp(v);
			(*this)[key].swap( tmp );
			return *this;
		}
		template<typename T>
		var & operator()(const key_string & key, T && v) {
			var tmp(std::move(v));
			(*this)[key].swap( tmp );
			return *this;
		}

		void	swap(var & v) {
			if (&v == this)
				return;
			std::swap(v.ptr_, ptr_);
			std::swap(v.type_, type_);
		}

	protected:
		void	destroy() {
			switch (type_) {
			case TYPE_INT64:
				i64_.destroy();	break;
			case TYPE_DOUBLE:
				f_.destroy();	break;
			case TYPE_STRING:
				delete str_;	break;
			case TYPE_CONST_STRING:
				delete ref_str_;	break;
			case TYPE_ARRAY:
				delete array_;	break;
			case TYPE_DICT:
				delete dict_;	break;
			default:
				break;
			}
		}
		static const char * get_type_name(TYPE n) {
			switch (n) {
			case TYPE_NULL:
				return "NULL";
			case TYPE_BOOL:
				return "BOOL";
			case TYPE_INT:
				return "INT";
			case TYPE_INT64:
				return "INT64";
			case TYPE_DOUBLE:
				return "DOUBLE";
			case TYPE_STRING:
			case TYPE_CONST_STRING:
				return "STRING";
			case TYPE_ARRAY:
				return "ARRAY";
			case TYPE_DICT:
				return "DICT";
			default:
				break;
			}
			return "";
		}

		void	check_type(TYPE n) const {
			if (n != get_type())
				throw std::invalid_argument(ara::printf<std::string>("type is %v not %v", get_type_name(get_type()), get_type_name(n)));
		}
		void	convert_to_type(TYPE n) {
			destroy();
			switch (n)
			{
			case TYPE_NULL:
				break;
			case TYPE_BOOL:
				b_ = false;	break;
			case TYPE_INT:
				i_ = 0;		break;
			case TYPE_INT64:
				i64_.init(0);	break;
			case TYPE_DOUBLE:
				f_.init(0.0);	break;
			case TYPE_STRING:
				str_ = new std::string;	break;
			case TYPE_CONST_STRING:
				ref_str_ = new ref_string;	break;
			case TYPE_ARRAY:
				array_ = new var_array;	break;
			case ara::var::TYPE_DICT:
				dict_ = new var_dict;	break;
			default:
				break;
			}
			type_ = n;
		}
		inline bool	get_imp(ara::type_id<bool> a) const {	return get_bool(); }
		inline int8_t	get_imp(ara::type_id<int8_t> a) const { return static_cast<int8_t>(get_int()); }
		inline uint8_t	get_imp(ara::type_id<uint8_t> a) const { return static_cast<uint8_t>(get_int()); }
		inline int16_t	get_imp(ara::type_id<int16_t> a) const { return static_cast<int16_t>(get_int()); }
		inline uint16_t	get_imp(ara::type_id<uint16_t> a) const { return static_cast<uint16_t>(get_int()); }
		inline int32_t	get_imp(ara::type_id<int32_t> a) const { return static_cast<int32_t>(get_int()); }
		inline uint32_t	get_imp(ara::type_id<uint32_t> a) const { return static_cast<uint32_t>(get_int()); }
		inline int64_t	get_imp(ara::type_id<int64_t> a) const { return static_cast<int64_t>(get_int64()); }
		inline uint64_t	get_imp(ara::type_id<uint64_t> a) const { return static_cast<uint64_t>(get_int64()); }
		inline double	get_imp(ara::type_id<double> a) const { return static_cast<double>(get_double()); }
		inline float	get_imp(ara::type_id<float> a) const { return static_cast<float>(get_double()); }
		inline ref_string	get_imp(ara::type_id<ref_string> a) const { return get_string(); }
		inline const std::string & get_imp(ara::type_id<std::string> a) const {
			if ( is_ref_string() )
				throw std::invalid_argument("type is REF_STRING not STRING");
			else if ( !is_string() )
				throw std::invalid_argument(ara::printf<std::string>("type is %v not STRING", get_type_name(get_type())));
			return *str_;
		}

		template<typename T>
		inline T  get_path_imp(const key_string & key, const T & defaultVal) const {
			if ( !is_dict() )
				return defaultVal;
			auto it = dict_->find(key);
			if (it == dict_->end())
				return defaultVal;
			return it->second.get_imp(ara::type_id<T>());
		}
		const std::string & get_path_imp(const key_string & key, const std::string & defaultVal) const {
			if ( !is_dict() )
				return defaultVal;
			auto it = dict_->find(key);
			if (it == dict_->end())
				return defaultVal;
			else if (it->second.is_ref_string())
				throw std::invalid_argument("type is REF_STRING not STRING");
			it->second.check_type(TYPE_STRING);
			return *(it->second.str_);
		}
		ref_string	get_path_imp(const key_string & key, const ref_string & defaultVal) const {
			if (!is_dict())
				return defaultVal;
			auto it = dict_->find(key);
			if (it == dict_->end())
				return defaultVal;
			else if (it->second.is_string())
				return ref_string(*it->second.str_);
			it->second.check_type(TYPE_STRING);
			return *(it->second.ref_str_);
		}
		const var &	get_path_imp(const key_string & key, const var & defaultVal) const {
			if (!is_dict())
				return defaultVal;
			auto it = dict_->find(key);
			if (it == dict_->end())
				return defaultVal;
			return it->second;
		}

		union {
			bool	b_;
			int		i_;
			var_holder<int64_t,sizeof(void *)>	i64_;
			var_holder<double,sizeof(void *)>	f_;
			ref_string *	ref_str_;
			std::string	*	str_;
			var_array	*	array_;
			var_dict *		dict_;
			void *			ptr_;
		};
		int		type_;
	};
}

#endif//ARA_VARIANT_H
