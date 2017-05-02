
#ifndef ARA_TEST_ASYNC_HELPER_H_20170527
#define ARA_TEST_ASYNC_HELPER_H_20170527

#include <mutex>
#include <list>
#include <string>

class async_error
{
public:
	void	set_error(std::string && s) {
		std::lock_guard<std::mutex>		_guard(lock_);
		info_list_.emplace_back(std::move(s));
	}
	std::string get_error() const {
		std::string		str;
		for (auto & it : info_list_) {
			str += it;
			str += "\r\n";
		}
		return str;
	}

	void	trace(const std::string & s) {
		std::lock_guard<std::mutex>		_guard(lock_);
		trace_info_ += s;
	}

	const std::string & get_trace() const {
		return trace_info_;
	}

protected:
	std::mutex	lock_;
	std::list<std::string>	info_list_;
	std::string				trace_info_;
};

#endif//ARA_TEST_ASYNC_HELPER_H_20170527
