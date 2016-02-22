#ifndef ARA_FLAGS_H_201112
#define ARA_FLAGS_H_201112

#include "ara_def.h"

namespace ara {
	
template<typename valueType>
class flags
{
public:
	typedef		valueType	value_type;

	flags(void) : flags_num_() {}
	flags(valueType n) : flags_num_(n) {}
	flags(const flags & f) : flags_num_(f.flags_num_) {}
	flags(flags && f) : flags_num_(f.flags_num_) {}

	flags(std::initializer_list<valueType> l) : flags_num_() {
		for (auto it : l)
			flags_num_ |= it;
	}

	const flags & operator=(valueType llV) {
		flags_num_ = llV;
		return *this;
	}
	const flags & operator=(const flags & v) {
		flags_num_ = v.flags_num_;
		return *this;
	}

	inline	void	set_flags(valueType nFlags) {
		flags_num_ |= nFlag;
	}
	inline	void	clear_flags(valueType nFlags) {
		flags_num_ &= ~nFlag;
	}
	inline	void	set_flags_toggle(bool toggle,valueType nFlags) {
		if (toggle)
			set_flags(nFlag);
		else
			clear_flags(nFlag);
	}
	inline	bool	check(valueType nFlags) const {
		return (flags_num_ & nFlags) != 0;
	}
	inline	bool	check_marks(valueType nMarks, valueType nFlags) const {
		return (flags_num_ & nMarks) == nFlags;
	}
	inline	bool	check_all(valueType nFlag) const {
		return (flags_num_ & nFlag) == nFlag;
	}

	const valueType & get(void) const {
		return flags_num_;
	}

	void	swap(flags & f) {
		std::swap(flags_num_, f.flags_num_);
	}

	flags &	operator|=(const flags & f) {
		flags_num_ |= f.flags_num_;
		return *this;
	}

	flags &	operator&=(const flags & f) {
		flags_num_ &= f.flags_num_;
		return *this;
	}
	flags &	operator^=(const flags & f) {
		flags_num_ ^= f.flags_num_;
		return *this;
	}

	flags 	operator|(const flags & f) const {
		flags tmp(*this);
		tmp |= f.flags_num_;
		return tmp;
	}

	flags	operator&(const flags & f) const {
		flags tmp(*this);
		tmp &= f.flags_num_;
		return tmp;
	}
	flags 	operator^(const flags & f) const {
		flags tmp(*this);
		tmp ^= f.flags_num_;
		return tmp;
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		ar & flags_num_;
	}

protected:
	valueType		flags_num_;
};

}//namespace ara

#endif//ARA_BASE_FLAGS_H_201112
