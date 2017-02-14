/*
	enum MY_EVENT {
		EVENT_INIT,
		EVENT_OPEN_THE_DOOR,
		EVENT_OPEN_THE_WINDOW
	};

	ara::event<MY_EVENT>		n(EVENT_INIT);

	thread1 {
		if (n.wait(EVENT_OPEN_THE_DOOR))
			foo();
	}

	thread2 {
		foo();
		n.singal_all(EVENT_OPEN_THE_WINDOW);
	}

	or 

	ara::event<int>		num(0);
	thread 1 {
		foo();
		num.inc_signal_one();
	}
	thread 2 {
		foo();
		num.inc_signal_one();
	}

	num.wait(2);	to wait for two thread all finished.

*/

#ifndef ARA_EVENT_H
#define ARA_EVENT_H

#include "datetime.h"

#include <mutex>
#include <condition_variable>

namespace ara {

	template<typename EVType>
	class event {
	public:
		event(EVType n = EVType()) : n_(n) {}

		bool	wait(EVType n, const timer_val & w = timer_val::max_time) {

			std::unique_lock<std::mutex> lk(lock_);
			if (n_ == n)
				return true;
			else if (w == timer_val::zero)
				return false;

			if (w == timer_val::max_time) {
				while (n_ != n)
					cond_.wait(lk);
			}
			else {
				auto exp = std::chrono::system_clock::now() + w.to_duration();
				while (n_ != n)
					if (cond_.wait_until(lk, exp) == std::cv_status::timeout)
						break;
			}
			return n_ == n;
		}

		template<class _Clock, class _Duration>
		bool	wait_until(EVType n, const std::chrono::time_point<_Clock, _Duration>& abstime) {

			std::unique_lock<std::mutex> lk(lock_);
			if (n_ == n)
				return true;
			while (n_ != n) {
				if (cond_.wait_until(lk, abstime) == std::cv_status::timeout)
					break;
			}
			return n_ == n;
		}

		void	signal_all(EVType n) {
			std::unique_lock<std::mutex> lk(lock_);
			n_ = n;
			cond_.notify_all();
		}
		void	signal_one(EVType n) {
			std::unique_lock<std::mutex> lk(lock_);
			n_ = n;
			cond_.notify_one();
		}

		void	inc_signal_all(EVType n = 1) {
			std::unique_lock<std::mutex> lk(lock_);
			n_ += n;
			cond_.notify_all();
		}
		void	inc_signal_one(EVType n = 1) {
			std::unique_lock<std::mutex> lk(lock_);
			n_ += n;
			cond_.notify_one();
		}

		EVType	value() const {
			return n_;
		}
		void	reset(EVType n) {
			n_ = n;
		}
	protected:
		std::mutex				lock_;
		std::condition_variable	cond_;
		EVType					n_;
	};

	template<class Rep, class Period>
	void	sleep(const std::chrono::duration<Rep, Period> & v) {
		ara::event<int> ev(0);
		auto now = std::chrono::system_clock::now();
		ev.wait_until(1, now + v);
	}
}

#endif//ARA_EVENT_H
