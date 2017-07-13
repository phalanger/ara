
#ifndef ARA_TOKEN_H
#define ARA_TOKEN_H

#include "ref_string.h"
#include "internal/string_traits.h"

#include <iterator>

namespace ara {

	namespace internal {
		template<class ch>
		class token_splitor {
		public:
			typedef ref_string_base<ch>			ref_string_t;

			token_splitor(const ch * splitor) : s_(splitor) {}

			inline bool	operator==(ch ch) const {
				return s_.find(ch) != ref_string_t::npos;
			}
		protected:
			ref_string_t	s_;
		};
	}

	template<class typeString, class splitor = internal::token_splitor<typename string_traits<typeString>::value_type> >
	class token_base {
	public:
		typedef string_traits<typeString>			stringTraits;
		typedef typename stringTraits::value_type	value_type;
		typedef ref_string_base<value_type>			ref_string_t;
		typedef ref_string_t						result_string;

		class iterator : public std::iterator<std::forward_iterator_tag, ref_string_t> {
		public:
			iterator() {}
			iterator(token_base & b) : b_(&b) {
				go_next();
			}
			iterator(const iterator & i) : b_(i.b_), res_(i.res_) {}

			const ref_string_t & operator*() const {
				return res_;
			}

			iterator & operator++() {
				go_next();
				return *this;
			}

			iterator operator++(int) {
				iterator res(*this);
				go_next();
				return res;
			}

			bool operator==(const iterator & i) const {
				return b_ == i.b_;
			}
			bool operator!=(const iterator & i) const {
				return b_ != i.b_;
			}
		protected:
			inline void go_next() {
				if (b_ && !b_->next(res_))
					b_ = nullptr;
			}
			token_base	*	b_ = nullptr;
			ref_string_t	res_;
		};
		typedef iterator	const_iterator;

		template<class T>
		token_base(const typeString & str, T t) : splitor_(t) {
			begin_ = stringTraits::data(str);
			end_ = begin_ + stringTraits::size(str);
		}
		template<class T>
		token_base(const value_type * str, T t) : splitor_(t) {
			begin_ = str;
			end_ = begin_ + stringTraits::traits_type::length(str);
		}

		bool	next(result_string & res) {
			while (begin_ != end_ && splitor_ == *begin_)
				++begin_;
			if (begin_ == end_)
				return false;
			const value_type * save = begin_;
			while (begin_ != end_ && !(splitor_ == *begin_))
				++begin_;
			res = ref_string_t(save, begin_);
			return true;
		}

	protected:
		const value_type *					begin_ = nullptr;
		const value_type *					end_ = nullptr;
		splitor								splitor_;
	};

	typedef token_base<std::string>		token_string;
}


#endif//ARA_TOKEN_H
