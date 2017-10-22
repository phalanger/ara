
#ifndef ARA_ASYNC_HTTPSERVER_H
#define ARA_ASYNC_HTTPSERVER_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/array.hpp>

#include "httpbase.h"
#include "internal/async_httpsvr_pattern.h"

#include <cctype>


#if 0

	ara::http::async_server svr(io, ssl_context);
	svr.add_dispatch_data("/*", [](ara::http::async_request_ptr req, ara::http::async_respond_ptr res) {
		
		if (req->get_url() == "/") {
			res->set_code(200, "OK")("Content-type","text/html").write_full_data("<body>helloworld</body>");
		}

	}).add_port(80).start();

#endif

namespace ara {
	namespace http {
		
		class async_server;
		using async_server_ptr = std::shared_ptr<async_server>;
		using async_server_weak_ptr = std::weak_ptr<async_server>;

		class async_server_request : public server_request {
		public:
			async_server_request() {}
			virtual ~async_server_request() {}
			virtual void	need_body(void * data, size_t n, std::function<void(const boost::system::error_code & ec, size_t n)> && func) = 0;
		};
		using async_server_request_ptr = std::shared_ptr<async_server_request>;

		class async_respond {
		public:
			async_respond(async_server_weak_ptr svr) : svr_(svr) {}

			async_respond &	set_code(uint16_t code, const std::string & msg) { code_ = code; msg_ = msg; return *this; }
			async_respond &	add_header(const std::string & key, const std::string & val) { h_(key, val); return *this; }
			header & get_header() { return h_; }
			const header & get_header() const { return h_; }

			void	write_full_data(const void * data, size_t n) {
				//TODO
			}
			void	write_full_data(const std::string & body) {
				write_full_data(body.data(), body.size());
			}

			void	write_chunk_data(const void * data, size_t n, std::function<void(boost::system::error_code & ec)> && func) {
				//TODO
			}
			void	write_chunk_finished() {
				//TODO
			}
			void	close() {
				//TODO
			}

		public:
			async_server_weak_ptr	get_svr_ptr() { return svr_; }
		private:
			async_server_weak_ptr	svr_;
			uint16_t		code_;
			std::string		msg_;
			header			h_;
		};

		using async_respond_ptr = std::shared_ptr<async_respond>;

		class server_filter {
		public:
			enum FILTER_RESULT {
				OK,
				SKIP_NEXT,
				REJECT,
			};
			virtual ~server_filter() {}
			virtual FILTER_RESULT	before_data(server_request & req) = 0;
		};
		using server_filter_ptr = std::shared_ptr<server_filter>;

		class server_handler {
		public:
			virtual ~server_handler() {}
			virtual void handle(async_server_request_ptr, async_respond_ptr) = 0;
		};
		using server_handler_ptr = std::shared_ptr<server_handler>;

		class async_server;
		using async_server_ptr = std::shared_ptr<async_server>;
		using async_server_weak_ptr = std::weak_ptr<async_server>;
		class async_server : public std::enable_shared_from_this<async_server> {
		public:

			std::shared_ptr<async_server>		make(boost::asio::io_service & io) {
				return std::make_shared<async_server>(io);
			}

			async_server &	add_filter(server_filter_ptr p) {
				list_filter_.emplace_back(p);
				return *this;
			}

			async_server &	set_default_dispatch(server_handler_ptr handler, size_t max_body_size = 1024 * 1024 * 4) {
				default_dispatch_ = std::make_shared<dispatch>(nullptr, handler, max_body_size);
				return *this;
			}

			async_server &	add_dispatch_data(server_dispatch_pattern_ptr pattern, server_handler_ptr handler, size_t max_body_size = 1024 * 1024 * 4) {
				list_dispatch_.push_back( std::make_shared<dispatch>(nullptr, handler, max_body_size) );
				return *this;
			}

			async_server &	add_dispatch_data(const std::string & pattern, server_handler_ptr handler, size_t max_body_size = 1024 * 1024 * 4) {
				list_dispatch_.push_back( std::make_shared<dispatch>(std::make_shared<server_path_dispatch_pattern>(pattern), handler, max_body_size) );
				return *this;
			}

			async_server & add_port(uint16_t port, size_t block = 64) {
				//TODO
				return *this;
			}
			async_server & add_port(uint16_t port, boost::asio::ssl::context & ssl_context, size_t block = 64) {
				//TODO
				return *this;
			}

			void	start() {
				//TODO
			}

			void	stop() {
				//TODO
			}

		public:

			async_server(boost::asio::io_service & io) : io_(io) {
			}

			class svr_base {
			public:
				svr_base(boost::asio::io_service & io, const std::string & ip, uint16_t port, size_t bl) : ip_(ip), port_(port), backlog_(bl),acceptor_(io) {}
				virtual ~svr_base() {}

				std::string		ip_;
				uint16_t		port_;
				size_t			backlog_;
				boost::asio::ip::tcp::acceptor acceptor_;
			};

			class ssl_svr_base : public svr_base {
			public:
				ssl_svr_base(boost::asio::io_service & io, const std::string & ip, uint16_t port, size_t bl, boost::asio::ssl::context & ssl_context) 
					: svr_base(io, ip, port, bl), ssl_context_(ssl_context) {}

				boost::asio::ssl::context & ssl_context_;
			};

			class async_server_request_imp : public async_server_request {
			public:
				virtual void	need_body(void * data, size_t n, std::function<void(const boost::system::error_code & ec, size_t n)> && func) {

				}
			};

			class instance_base : public async_respond {
			public:
				instance_base(async_server_weak_ptr p, boost::asio::io_service & io) 
					: async_respond(p), timer_(io), strand_(io), io_(io) {
					req_ptr_ = std::make_shared<async_server_request_imp>();
				}
				virtual ~instance_base() {}

				virtual boost::asio::ip::tcp::socket::lowest_layer_type & socket() = 0;

				async_server_request_ptr		req_ptr_;
				
				boost::asio::deadline_timer		timer_;
				boost::asio::strand				strand_;
				boost::asio::io_service &		io_;
				boost::asio::streambuf			req_;
			};
			class instance : public instance_base  {
			public:
				instance(async_server_weak_ptr p, boost::asio::io_service & io) :
					instance_base(p, io), socket_(io) {}
				
				boost::asio::ip::tcp::socket::lowest_layer_type & socket()	{ return socket_.lowest_layer(); }

				boost::asio::ip::tcp::socket	socket_;
			};
			class ssl_instance : public instance_base {
			public:
				ssl_instance(async_server_weak_ptr p, boost::asio::io_service & io, boost::asio::ssl::context & ssl_context) :
					instance_base(p, io), ssl_context_(ssl_context), socket_(io, ssl_context) {}

				boost::asio::ip::tcp::socket::lowest_layer_type & socket()	{ return socket_.lowest_layer(); }

				boost::asio::ssl::context & ssl_context_;
				boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
			};

		private:
			
			async_server_weak_ptr  weak_from_this() {
				return shared_from_this();
			}

			struct dispatch {
				dispatch(server_dispatch_pattern_ptr pattern, server_handler_ptr handler, size_t max_body_size) 
					: pattern_(pattern), handler_(handler), max_body_size_(max_body_size) {}
				dispatch(dispatch && r) : pattern_(r.pattern_), handler_(r.handler_),max_body_size_(r.max_body_size_)  {}
				dispatch(const dispatch & r) : pattern_(r.pattern_), handler_(r.handler_),max_body_size_(r.max_body_size_)  {}
				dispatch() {}

				dispatch & operator=(const dispatch & r) {
					if (&r != this) {
						pattern_ = r.pattern_;
						handler_ = r.handler_;
						max_body_size_ = r.max_body_size_;
					}
					return *this;
				}

				server_dispatch_pattern_ptr		pattern_;
				server_handler_ptr				handler_;
				size_t							max_body_size_ = 1024 * 1024 * 4;
			};
			using dispatch_ptr = std::shared_ptr<dispatch>;


			boost::asio::io_service & io_;

			std::list<server_filter_ptr>			list_filter_;
			std::list<std::unique_ptr<svr_base>>	list_svr_;
			dispatch_ptr							default_dispatch_;
			std::list<dispatch_ptr>					list_dispatch_;
		};

	}
}


#endif//ARA_ASYNC_HTTPSERVER_H
