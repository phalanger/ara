
#ifndef ARA_PROMISE_H
#define ARA_PROMISE_H

#include "ara_def.h"

#include <functional>

namespace ara {

	namespace internal {

		template<typename typeCallback>
		class ara_promise
		{
		public:
			using err_CallBack = std::function<void(int)>;
			using exception_CallBack = std::function<void(std::exception &)>;

			ara_promise(typeCallback && func) : func_(std::move(func)) {}

			ara_promise &	on_error(err_CallBack && func) {
				err_func_ = std::move(func)£»
					return *this;
			}
			ara_promise & on_exception(exception_CallBack && func) {
				exception_func_ = std::move(func);
				return *this;
			}

		protected:
			typeCallback		func_;
			err_CallBack		err_func_;
			exception_CallBack	exception_func_;
		};
	}

}

#endif ARA_PROMISE_H

