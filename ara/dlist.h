
#ifndef ARA_DLIST_H
#define ARA_DLIST_H

#include "ara_def.h"

#include <functional>

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
	protected:
		T * next_ = nullptr;
		T * pre_ = nullptr;
	};
}

#endif ARA_DLIST_H
