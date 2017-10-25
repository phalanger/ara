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

		template<class Pattern>
		struct server_dispatch_pattern_builder : public std::false_type {
		};


		template<>
		struct server_dispatch_pattern_builder<const char *> : public std::true_type {
			static server_dispatch_pattern_ptr	build(const char * p) {
				return std::make_shared<server_path_dispatch_pattern>(p);
			}
		};
		template<>
		struct server_dispatch_pattern_builder<const std::string &> : public std::true_type {
			static server_dispatch_pattern_ptr	build(const std::string & p) {
				return std::make_shared<server_path_dispatch_pattern>(p);
			}
		};
	}
}

#endif//ARA_ASYNC_HTTPSVR_PATTERN_H_20171021



