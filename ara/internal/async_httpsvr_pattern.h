#ifndef ARA_ASYNC_HTTPSVR_PATTERN_H_20171021
#define ARA_ASYNC_HTTPSVR_PATTERN_H_20171021


#include "../httpbase.h"

namespace ara {
	namespace http {


		class server_path_dispatch_pattern : public server_dispatch_pattern {
		public:
			server_path_dispatch_pattern(const std::string & pattern) : pattern_(pattern) {}

			bool check_before_data(const server_request & req) {
				return req.get_url().find(pattern_) == 0;
			}
		private:
			std::string		pattern_;
		};



	}
}

#endif//ARA_ASYNC_HTTPSVR_PATTERN_H_20171021



