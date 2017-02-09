
///		double link list template
///		example:
///			class MyData : public ara::dlist<MyData>
///			{
///			public:
///				MyData(int n = 0) : data_(n) {}
///				int	data_;
///			};
///			MyData		root;
///			root.as_root();
///			
///			MyData		node1(1);
///			node1.append_after(root);	//root ->  node1

#ifndef ARA_DLIST_H
#define ARA_DLIST_H

#include "ara_def.h"

#include <functional>
#include <iterator>

namespace ara {
	template<typename T>
	class dlist
	{
	public:
		dlist() {}
		~dlist() { 
			unlink(); 
		}

		dlist(const dlist &) = delete;
		dlist(dlist && r) = delete;

		void       as_root() {
			next_ = pre_ = (T *)this;
		}
		T * get_next() const {
			return next_;
		}
		T * get_pre() const {
			return pre_;
		}

		bool    alone() const {
			return next_ == NULL;
		}
		bool    root_empty() const {
			return next_ == (T *)this;
		}

		void        unlink() {
			if (alone())
				return;
			next_->pre_ = pre_;
			pre_->next_ = next_;
			pre_ = next_ = NULL;
		}

		void        append_after(T & t) {
			next_ = t.next_;
			pre_ = &t;
			next_->pre_ = (T *)this;
			t.next_ = (T *)this;
		}
		void        append_before(T & t) {
			pre_ = t.pre_;
			next_ = &t;
			pre_->next_ = (T *)this;
			t.pre_ = (T *)this;
		}

		class forward_iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
		public:
			explicit forward_iterator(dlist * p) : p_(p) {}
			forward_iterator(const forward_iterator & rhs) : p_(rhs.p_) {}

			T & operator * () const {
				return p_->data();
			}
			bool operator == (const forward_iterator & rhs) const {
				return p_ == rhs.p_;
			}
			bool operator != (const forward_iterator& rhs) const {
				return p_ != rhs.p_;
			}
			forward_iterator& operator ++ () {
				p_ = p_->get_next();
				return *this;
			}
			forward_iterator operator ++ (int) {
				forward_iterator temp(p_);
				p_ = p_->get_next();
				return temp;
			}
			forward_iterator & operator --() {
				p_ = p_->get_pre();
				return *this;
			}
			forward_iterator operator -- (int) {
				forward_iterator temp = *this;
				p_ = p_->get_pre();
				return temp;
			}
		protected:
			dlist	*	p_ = nullptr;
		};
		class const_forward_iterator : public std::iterator<std::bidirectional_iterator_tag, const T> {
		public:
			explicit const_forward_iterator(const dlist * p) : p_(p) {}
			const_forward_iterator(const const_forward_iterator & rhs) : p_(rhs.p_) {}
			const_forward_iterator(const forward_iterator & rhs) : p_(rhs.p_) {}

			const T & operator * () const {
				return p_->data();
			}
			bool operator == (const const_forward_iterator & rhs) const {
				return p_ == rhs.p_;
			}
			bool operator != (const const_forward_iterator& rhs) const {
				return p_ != rhs.p_;
			}
			const_forward_iterator& operator ++ () {
				p_ = p_->get_next();
				return *this;
			}
			const_forward_iterator operator ++ (int) {
				const_forward_iterator temp(p_);
				p_ = p_->get_next();
				return temp;
			}
			const_forward_iterator & operator --() {
				p_ = p_->get_pre();
				return *this;
			}
			const_forward_iterator operator -- (int) {
				const_forward_iterator temp = *this;
				p_ = p_->get_pre();
				return temp;
			}
		protected:
			const dlist	* p_ = nullptr;
		};
		class reverse_iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
		public:
			explicit reverse_iterator(dlist * p) : p_(p) {}
			reverse_iterator(const reverse_iterator & rhs) : p_(rhs.p_) {}

			T & operator * () const {
				return p_->data();
			}
			bool operator == (const reverse_iterator & rhs) const {
				return p_ == rhs.p_;
			}
			bool operator != (const reverse_iterator& rhs) const {
				return p_ != rhs.p_;
			}
			reverse_iterator& operator ++ () {
				p_ = p_->get_pre();
				return *this;
			}
			reverse_iterator operator ++ (int) {
				reverse_iterator temp(p_);
				p_ = p_->get_pre();
				return temp;
			}
			reverse_iterator & operator --() {
				p_ = p_->get_next();
				return *this;
			}
			reverse_iterator operator -- (int) {
				reverse_iterator temp = *this;
				p_ = p_->get_next();
				return temp;
			}
		protected:
			dlist	*	p_ = nullptr;
		};
		class const_reverse_iterator : public std::iterator<std::bidirectional_iterator_tag, const T> {
		public:
			explicit const_reverse_iterator(const dlist * p) : p_(p) {}
			const_reverse_iterator(const const_reverse_iterator & rhs) : p_(rhs.p_) {}
			const_reverse_iterator(const reverse_iterator & rhs) : p_(rhs.p_) {}

			const T & operator * () const {
				return p_->data();
			}
			bool operator == (const const_reverse_iterator & rhs) const {
				return p_ == rhs.p_;
			}
			bool operator != (const const_reverse_iterator& rhs) const {
				return p_ != rhs.p_;
			}
			const_reverse_iterator& operator ++ () {
				p_ = p_->get_pre();
				return *this;
			}
			const_reverse_iterator operator ++ (int) {
				const_reverse_iterator temp(p_);
				p_ = p_->get_pre();
				return temp;
			}
			const_reverse_iterator & operator --() {
				p_ = p_->get_next();
				return *this;
			}
			const_reverse_iterator operator -- (int) {
				const_reverse_iterator temp = *this;
				p_ = p_->get_next();
				return temp;
			}
		protected:
			const dlist	* p_ = nullptr;
		};

		forward_iterator	begin() {
			return forward_iterator(get_next());
		}
		forward_iterator	end() {
			return forward_iterator(this);
		}
		const_forward_iterator	begin() const {
			return const_forward_iterator(get_next());
		}
		const_forward_iterator	end() const {
			return const_forward_iterator(this);
		}
		const_forward_iterator	cbegin() const {
			return const_forward_iterator(get_next());
		}
		const_forward_iterator	cend() const {
			return const_forward_iterator(this);
		}

		reverse_iterator	rbegin() {
			return reverse_iterator(get_pre());
		}
		reverse_iterator	rend() {
			return reverse_iterator(this);
		}
		const_reverse_iterator	rbegin() const {
			return const_reverse_iterator(get_pre());
		}
		const_reverse_iterator	rend() const {
			return const_reverse_iterator(this);
		}
		const_reverse_iterator	crbegin() const {
			return const_reverse_iterator(get_pre());
		}
		const_reverse_iterator	crend() const {
			return const_reverse_iterator(this);
		}

		T	& data() {
			return *(reinterpret_cast<T *>(this));
		}
		const T	& data() const {
			return *(reinterpret_cast<const T *>(this));
		}

	protected:
		T * next_ = nullptr;
		T * pre_ = nullptr;
	};
}

#endif//ARA_DLIST_H
