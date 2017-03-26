
#ifndef ARA_VARIANT_H
#define ARA_VARIANT_H

#include "ref_string.h"
#include "key_string.h"

#include <vector>
#include <map>
#include <functional>

namespace ara {

	class var {
		template<class T, size_t>
		struct var_holder{
			inline void	init(T v) { f_ = v; }
			inline void destroy() {}
			inline void set(T d) { f_ = d; }
			inline T get() const { return f_; }
		protected:
			T f_;
		};
		template<class T>
		struct var_holder<T, 4>{
			inline void	init(T v) { f_ = new T; *f_ = v; }
			inline void destroy() { delete f_; }
			inline void set(T d) { *f_ = d; }
			inline T get() const { return *f_; }
		protected:
			T * f_;
		};
	public:
		typedef std::vector<var>			var_array;
		typedef std::map<key_string, var>	var_dict;

		enum TYPE {
			TYPE_NULL = 0x00,
			TYPE_BOOL = 0x01,
			TYPE_BOOL_FALSE = 0x02,
			TYPE_INT = 0x03,
			TYPE_INT64 = 0x04,
			TYPE_DOUBLE = 0x05,
			TYPE_STRING = 0x06,
			TYPE_REF_STRING = 0x07,
			TYPE_ARRAY = 0x08,
			TYPE_DICT = 0x09,

			TYPE_MASK = 0xff,
			TYPE_REF = 0x100,
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
		
		var(const var_array & s) : array_(new var_array(s)), type_(TYPE_ARRAY) {}
		var(var_array && s) : array_(new var_array(std::move(s))), type_(TYPE_ARRAY) {}
		var(var_array * s) : array_(s), type_(TYPE_ARRAY) {}

		var(const var_dict & s) : dict_(new var_dict(s)), type_(TYPE_DICT) {}
		var(var_dict && s) : dict_(new var_dict(std::move(s))), type_(TYPE_DICT) {}
		var(var_dict * s) : dict_(s), type_(TYPE_DICT) {}

		var(const var & r) : ptr_(r.ptr_), type_(r.type_) {
			switch (type_) {
			case TYPE_INT64:
				i64_.init( r.i64_.get() );	break;
			case TYPE_DOUBLE:
				f_.init( r.f_.get() );	break;
			case TYPE_STRING:
				str_ = new std::string(*r.str_);	break;
			case TYPE_REF_STRING:
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

		inline bool	is_null() const {
			return type_ == TYPE_NULL;
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
			case TYPE_REF_STRING:
				delete ref_str_;	break;
			case TYPE_ARRAY:
				delete array_;	break;
			case TYPE_DICT:
				delete dict_;	break;
			default:
				break;
			}
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

#endif ARA_VARIANT_H
