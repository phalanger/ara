
#ifndef ARA_ASYNC_HTTPSERVER_H
#define ARA_ASYNC_HTTPSERVER_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/container/static_vector.hpp>

#include "httpbase.h"
#include "token.h"
#include "internal/async_httpsvr_pattern.h"

#include <cctype>


#if 0

	auto svr = ara::http::async_server::make( io );

	svr->add_dispatch("/", [](ara::http::request_ptr req, ara::http::respond_ptr res) {
		res->set_code(200, "OK").add_header("Content-type","text/html").write_full_data( 

			"<body>helloworld</body>"
		);
	})
	.add_port(8080);

	//or define custom pattern and handler

	class MyPattern : public ara::http::server_dispatch_pattern
	{
	public:
		virtual bool check_before_data(const ara::http::server_request & req) override {
			///
			return true;
		}
	};
	class MyHandle : public ara::http::server_handler
	{
	public:
		virtual void handle(ara::http::request_ptr req, ara::http::respond_ptr res) override {
			//Do something
			auto f = std::make_shared<ara::raw_file>();
			if (f->open("D:\\temp.txt").create().truncat().write_only().binary().done())
				req->read_body([f, res](boost::system::error_code ec, const char * data, size_t n) {
					if (ec)
						res->close();
					else if (n)
						f->write(data, n);
					else {
						f->close();
						res->set_code(200, "OK").add_header("Content-type","text/html").write_full_data( 
							"<body>helloworld</body>"
						);
					}
				});
			else
				res->close();
		}
	};
	svr->add_dispatch(std::make_shared<MyPattern>(), std::make_shared<MyHandle>(), std::string::npos);//handle body by myself

#endif

namespace ara {
	namespace http {
		
		class server_options
		{
		public:
			server_options() {}

			void				set_timeout(const timer_val & v) { time_out_ = v; }
			const timer_val &	get_timeout() const { return time_out_; }

			void		set_cache_size(size_t n) { cache_size_ = n; }
			size_t		get_cache_size() const { return cache_size_; }

		protected:
			timer_val	time_out_ = timer_val(10);
			size_t		cache_size_ = 64 * 1024;
		};

		class async_server;
		using async_server_ptr = std::shared_ptr<async_server>;
		using async_server_weak_ptr = std::weak_ptr<async_server>;

		using body_callback_func = std::function<void(const boost::system::error_code & ec, const char * p, size_t n)>;

		class async_server_request : public server_request {
		public:
			async_server_request() {}
			virtual ~async_server_request() {}
			virtual void	read_body(body_callback_func && func) = 0;
		};
		using request_ptr = std::shared_ptr<async_server_request>;

		class async_respond {
		public:
			async_respond() {}
			virtual ~async_respond() {}

			inline async_respond &	set_code(uint16_t code, const std::string & msg) { code_ = code; msg_ = msg; return *this; }
			inline async_respond &	add_header(const std::string & key, const std::string & val) { h_(key, val); return *this; }
			inline async_respond &	content_type(const std::string & type) { h_("Content-type", type); return *this; }
			inline async_respond &	html() { return content_type("text/html"); }
			inline async_respond &	text() { return content_type("text/plain"); }
			inline async_respond &	xml() { return content_type("text/xml"); }
			inline async_respond &	binary() { return content_type("application/octet-stream"); }
			inline async_respond &	json() { return content_type("application/json"); }

			inline uint16_t		get_code() const { return code_; }
			inline const std::string & get_msg() const { return msg_; }
			inline header & get_header_for_modify() { return h_; }
			inline const header & get_header() const { return h_; }

			void	write_full_data(const std::string & body) {
				write_full_data(body.data(), body.size());
			}
			virtual void	write_full_data(const void * data, size_t n) = 0;
			virtual void	write_chunk_data(const void * data, size_t n, std::function<void(boost::system::error_code & ec)> && func) = 0;
			virtual void	write_chunk_finished() = 0;
			virtual void	close() = 0;
		private:
			uint16_t		code_ = 0;
			std::string		msg_;
			header			h_;
		};

		using respond_ptr = std::shared_ptr<async_respond>;

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
			virtual void handle(request_ptr, respond_ptr) = 0;
		};
		using server_handler_ptr = std::shared_ptr<server_handler>;
		class server_handler_func : public server_handler {
		public:
			server_handler_func(std::function<void(request_ptr, respond_ptr)> && func) : func_(func) {}
			virtual void handle(request_ptr req, respond_ptr res) override {
				func_(req, res);
			}
		private:
			std::function<void(request_ptr, respond_ptr)>	func_;
		};

		class async_server;
		using async_server_ptr = std::shared_ptr<async_server>;
		using async_server_weak_ptr = std::weak_ptr<async_server>;
		class async_server : public std::enable_shared_from_this<async_server> {
		public:
			static std::shared_ptr<async_server>		make(boost::asio::io_service & io, const server_options & opt) {
				return std::make_shared<async_server>(io, opt);
			}
			static std::shared_ptr<async_server>		make(boost::asio::io_service & io) {
				server_options opt;
				return std::make_shared<async_server>(io, opt);
			}

			server_options & get_options() { return options_; }
			const server_options & get_options() const { return options_; }

			async_server &	set_options(const server_options & opt) {
				options_ = opt;
				return *this;
			}

			async_server &	add_filter(server_filter_ptr p) {
				list_filter_.emplace_back(p);
				return *this;
			}

			async_server &	set_default_dispatch(server_handler_ptr handler, size_t max_body_size = 1024 * 1024 * 4) {
				default_dispatch_ = std::make_shared<dispatch>(nullptr, handler, max_body_size);
				return *this;
			}

			async_server &	add_dispatch(server_dispatch_pattern_ptr pattern, server_handler_ptr handler, size_t max_body_size = 1024 * 1024 * 4) {
				list_dispatch_.push_back( std::make_shared<dispatch>(nullptr, handler, max_body_size) );
				return *this;
			}
			async_server &	add_dispatch(const std::string & pattern, server_handler_ptr handler, size_t max_body_size = 1024 * 1024 * 4) {
				list_dispatch_.push_back( std::make_shared<dispatch>(std::make_shared<server_path_dispatch_pattern>(pattern), handler, max_body_size) );
				return *this;
			}
			async_server &	add_dispatch(server_dispatch_pattern_ptr pattern, std::function<void(request_ptr, respond_ptr)> && func, size_t max_body_size = 1024 * 1024 * 4) {
				return add_dispatch(pattern, std::make_shared<server_handler_func>(std::move(func)), max_body_size);
			}
			async_server &	add_dispatch(const std::string & pattern, std::function<void(request_ptr, respond_ptr)> && func, size_t max_body_size = 1024 * 1024 * 4) {
				return add_dispatch(pattern, std::make_shared<server_handler_func>(std::move(func)), max_body_size);
			}

			async_server & add_port(uint16_t port, const std::string & ip = "", size_t backlog = 64) {
				std::unique_ptr<svr_base> p(new svr_base(io_, ip, port, backlog));
				p->init();
				list_svr_.push_back(std::move(p));
				return *this;
			}
			//if bind fail, It will throw exception by boost::asio boost::asio::detail::throw_exception(e);
			async_server & add_port(uint16_t port, boost::asio::ssl::context & ssl_context, const std::string & ip = "", size_t backlog = 64) {
				std::unique_ptr<svr_base> p(new ssl_svr_base(io_, ip, port, backlog, ssl_context));
				p->init();
				list_svr_.push_back(std::move(p));
				return *this;
			}

			void	start() {
				for (auto & p : list_svr_)
					p->do_accept( shared_from_this() );
			}

			void	stop() {
				for (auto & p : list_svr_)
					p->stop();
			}

		public:

			async_server(boost::asio::io_service & io, const server_options & opt) : io_(io), options_(opt) {
			}

			server_handler_ptr	find_handler(request_ptr req, size_t & nMaxSize) {
				for (auto & filter : list_filter_)
					filter->before_data(*req);
				for (auto & d : list_dispatch_) {
					if (d->pattern_->check_before_data(*req)) {
						nMaxSize = d->max_body_size_;
						return d->handler_;
					}
				}
				nMaxSize = default_dispatch_->max_body_size_;
				return default_dispatch_->handler_;
			}

			class async_server_request_imp_helper {
			public:
				virtual ~async_server_request_imp_helper() {}
				virtual void	read_body_imp(body_callback_func && func) = 0;
			};
			class async_server_request_imp : public async_server_request {
			public:
				async_server_request_imp(async_server_request_imp_helper & helper) : helper_(helper) {}
				virtual void	read_body(body_callback_func && func) override{
					helper_.read_body_imp(std::move(func));
				}
			private:
				async_server_request_imp_helper	& helper_;
			};
			class context_base : public async_respond, public async_server_request_imp_helper, public std::enable_shared_from_this<context_base> {
			public:
				context_base(async_server_weak_ptr p, boost::asio::io_service & io) 
					: async_respond(), svr_(p), timer_(io), strand_(io), io_(io) {
					req_ptr_ = std::make_shared<async_server_request_imp>( *this );
				}
				virtual ~context_base() {}

				virtual boost::asio::ip::tcp::socket::lowest_layer_type & socket() = 0;
				virtual void	handshake() = 0;

				virtual void	write_data_imp(const boost::container::static_vector<boost::asio::const_buffer, 3> & bufs) = 0;
				virtual void	write_full_data(const void * data, size_t n) override {
					func_chunk_output_callback_ = nullptr;
					output_respond_header(false, n);

					boost::container::static_vector<boost::asio::const_buffer, 3>		bufs;
					bufs.push_back(boost::asio::buffer(respond_header_.data(), respond_header_.size()));
					bufs.push_back(boost::asio::buffer(data, n));

					write_data_imp( bufs );
				}
				virtual void	write_chunk_data(const void * data, size_t n, std::function<void(boost::system::error_code & ec)> && func) override {

					if (respond_header_.empty()) {
						output_respond_header(true, 0);
						strext(respond_header_).printf("%x\r\n", n);
					} else
						strext(respond_header_).clear().printf("%x\r\n", n);

					func_chunk_output_callback_ = std::move(func);
					boost::container::static_vector<boost::asio::const_buffer, 3>	bufs;
					bufs.push_back(boost::asio::buffer(respond_header_.data(), respond_header_.size()));
					bufs.push_back(boost::asio::buffer(data, n));
					bufs.push_back(boost::asio::buffer("\r\n", 2));

					write_data_imp(bufs);
				}
				virtual void	write_chunk_finished() override {
					respond_header_.clear();
					func_chunk_output_callback_ = nullptr;
					boost::container::static_vector<boost::asio::const_buffer, 3>	bufs;
					bufs.push_back(boost::asio::buffer("0\r\n", 3));

					write_data_imp(bufs);
				}
				void	after_write_data(boost::system::error_code ec) {
					auto _lock = svr_.lock();
					if (!_lock)
						ec = boost::asio::error::operation_aborted;
					bool boFinishedAll = true;
					if (func_chunk_output_callback_) {
						std::function<void(boost::system::error_code & ec)> tmp;
						tmp.swap(func_chunk_output_callback_);
						tmp( ec );
						boFinishedAll = false;
					}
					if (ec)
						return close();
					else if (boFinishedAll) {
						reset_timeout(_lock->get_options().get_timeout());
						begin_http();
					}
				}

				void	output_respond_header(bool boChunk, size_t nLength) {
					if (get_code() == 0)
						set_code(200, "OK");

					auto & h = get_header_for_modify();

					if (!boChunk)
						strext(h["Content-Length"]).clear().printf("%u", nLength);
					else
						h["Transfer-Encoding"] = "chunked";

					if (h.find("Content-Type") == h.end())
						binary();

					const char * lpNewLine = "\r\n";
					respond_header_ = req_ptr_->get_version();
					respond_header_ += ' ';
					strext(respond_header_).append_int(get_code());
					respond_header_ += ' ';
					respond_header_ += get_msg();
					respond_header_ += lpNewLine;
					for (const auto & i : h) {
						respond_header_ += i.first;
						respond_header_ += ": ";
						respond_header_ += i.second;
						respond_header_ += lpNewLine;
					}
					respond_header_ += lpNewLine;
				}

				virtual void	read_body_imp(body_callback_func && func) override {
					if (max_body_size_ != std::string::npos) {
						const std::string & body = req_ptr_->get_body();
						func(boost::system::error_code(), body.data(), body.size());
					} else if (body_size_ != std::string::npos) {
						func_data_callback_ = std::move(func);
						on_body_content(boost::system::error_code());
					} else { //chunk mode
						func_data_callback_ = std::move(func);
						on_chunk_size(boost::system::error_code());
						//TODO
					}
				}

				void	on_body_data_fail(const boost::system::error_code & ec) {
					if (func_data_callback_) {
						body_callback_func tmp;
						tmp.swap(func_data_callback_);
						tmp(ec, nullptr, 0);
					}
					close();
				}
				void	on_body_data(const char * p, size_t n) {
					if (func_data_callback_)
						func_data_callback_(boost::system::error_code(), p, n);
					else if (n)
						req_ptr_->get_body_for_modify().append(p, n);
				}

				void	init_timeout_handle() {
					timer_.async_wait([self = shared_from_this(), this](const boost::system::error_code & ec) {
						if (ec.value() == boost::asio::error::operation_aborted)
							return;
						close();
					});
				}
				void	reset_timeout(const timer_val & tTimeout) {
					boost::system::error_code ec;
					timer_.expires_from_now(boost::posix_time::seconds(static_cast<long>(tTimeout.sec())) + boost::posix_time::microseconds(tTimeout.micro_sec()), ec);
				}
				virtual void	close() override {
					if (func_data_callback_) {
						body_callback_func tmp;
						tmp.swap(func_data_callback_);
						tmp(boost::asio::error::operation_aborted, nullptr, 0);
					}

					boost::system::error_code	ec;
					socket().cancel( ec );
					socket().close( ec );
					timer_.cancel( ec );
				}

				void		begin_http() {
					req_ptr_->clear();
					func_data_callback_ = nullptr;
					body_size_ = 0;
					max_body_size_ = 0;
					hander_ = nullptr;
					respond_header_.clear();
					func_chunk_output_callback_ = nullptr;

					auto _lock = svr_.lock();
					if ( !_lock )
						return close();
					read_header();
				}

				virtual void	read_header() = 0;
				void	on_header(const boost::system::error_code& ec) {
					auto _lock = svr_.lock();
					if (!_lock || ec)
						return close();
					auto & sock = socket();
					auto remote_info = sock.remote_endpoint();
					req_ptr_->set_remote_ip( remote_info.address().to_string() );
					req_ptr_->set_remote_port( remote_info.port() );
					auto local_info = sock.local_endpoint();
					req_ptr_->set_local_ip( local_info.address().to_string() );
					req_ptr_->set_local_port( local_info.port() );

					auto & h = req_ptr_->get_header_for_modify();
					std::istream req_stream(&req_);
					std::string header;
					const char * strSpace = " \t\r\n";

					if (std::getline(req_stream, header) && header != "\r") {
						token_string				token(header, strSpace);
						token_string::result_string	res;

						if (!token.next(res)) return close();
						req_ptr_->set_method(res.str());

						if (!token.next(res)) return close();
						req_ptr_->set_url(res.str());

						if (!token.next(res)) return close();
						req_ptr_->set_version(res.str());
					} else
						return close();
					while (std::getline(req_stream, header) && header != "\r") {
						std::string::size_type p = header.find(':');
						if (p == std::string::npos)
							h[ strext(header).trim(strSpace) ];
						else
							h[ strext(header.substr(0, p)).trim(ref_string(strSpace)) ] = strext(header.substr(p + 1)).trim(ref_string(strSpace));
					}//while

					auto it = h.find("Transfer-Encoding");
					if (it != h.end() && it->second == "chunked")	{
						body_size_ = std::string::npos;
					} else if ((it = h.find("Content-Length")) != h.end()) {
						body_size_ = strext(it->second).to_int<size_t>();
					} else 
						return close();
					req_ptr_->set_body_size(body_size_);

					hander_ = _lock->find_handler(req_ptr_, max_body_size_);
					if (max_body_size_ == static_cast<size_t>(-1))
						hander_->handle(req_ptr_, shared_from_this());
					else if (body_size_ == std::string::npos)
						on_chunk_size(ec);
					else if (body_size_ <= max_body_size_)
						on_body_content(ec);
					else
						return close();
				}

				virtual void read_body_content(size_t nLeast) = 0;
				void on_body_content(const boost::system::error_code& ec) {
					auto _lock = svr_.lock();
					if (!_lock)
						return close();
					else if (ec)
						return on_body_data_fail(ec);

					auto d = req_.data();
					size_t nRest = boost::asio::buffer_size(d);
					const char * p = boost::asio::buffer_cast<const char *>(d);

					size_t nCopySize = std::min<size_t>(nRest , body_size_);
					on_body_data(p, nCopySize);
					req_.consume(nCopySize);
					body_size_ -= nCopySize;

					if (body_size_)
						read_body_content( std::min<size_t>(body_size_, _lock->get_options().get_cache_size()) );
					else
						finish_body();
				}

				virtual void read_chunk_size() = 0;
				void on_chunk_size(const boost::system::error_code& ec) 	{

					auto _lock = svr_.lock();
					if (!_lock)
						return close();
					else if (ec)
						return on_body_data_fail( ec );

					auto d = req_.data();
					ref_string	strData(boost::asio::buffer_cast<const char *>(d), boost::asio::buffer_size(d));
					ref_string::size_type p = strData.find('\n');

					if (p == ref_string::npos) {
						return read_chunk_size();
					} else {
						std::istream response_stream(&req_);
						size_t nChunkSize = 0;
						response_stream >> std::hex >> nChunkSize;
						std::string dummy;
						std::getline(response_stream, dummy);

						if (req_ptr_->get_body().size() + nChunkSize > max_body_size_)
							return close();

						on_read_chunk_data(ec, nChunkSize == 0, nChunkSize);
					}
				}

				virtual void read_chunk_data(bool boLastPackage, size_t nLeast) = 0;
				void on_read_chunk_data(const boost::system::error_code& ec, bool boLastPackage, size_t nRestSize) {

					auto _lock = svr_.lock();
					if (!_lock)
						return close();
					else if (ec)
						return on_body_data_fail(ec);

					auto d = req_.data();
					ref_string	strData(boost::asio::buffer_cast<const char *>(d), boost::asio::buffer_size(d));

					if (nRestSize)	{
						size_t nCopyData = std::min<size_t>(strData.size(), nRestSize);
						on_body_data(strData.data(), nCopyData);
						req_.consume(nCopyData);
						nRestSize -= nCopyData;
					}
					if (nRestSize == 0)
						on_read_chunk_data_end(ec, boLastPackage);
					else
						read_chunk_data(boLastPackage, nRestSize);
				}

				virtual void read_chunk_end_data(bool boLastPackage) = 0;
				void on_read_chunk_data_end(const boost::system::error_code& ec, bool boLastPackage) {

					auto _lock = svr_.lock();
					if (!_lock)
						return close();
					else if (ec)
						return on_body_data_fail(ec);

					auto d = req_.data();
					ref_string	strData(boost::asio::buffer_cast<const char *>(d), boost::asio::buffer_size(d));
					ref_string::size_type p = strData.find('\n');

					if (p != ref_string::npos) {
						req_.consume(p + 1);
						if (boLastPackage)
							return finish_body();
						else
							return on_chunk_size(ec);
					} else
						read_chunk_end_data(boLastPackage);
				}

				void finish_body() {
					respond_header_.clear();
					func_chunk_output_callback_ = nullptr;
					boost::system::error_code	ec;
					timer_.cancel(ec);

					if (func_data_callback_) {
						body_callback_func tmp;
						tmp.swap(func_data_callback_);
						tmp(boost::system::error_code(), nullptr, 0);
					}
					else
						hander_->handle(req_ptr_, shared_from_this());
				}

				async_server_weak_ptr			svr_;
				request_ptr						req_ptr_;
				
				boost::asio::deadline_timer		timer_;
				boost::asio::strand				strand_;
				boost::asio::io_service &		io_;
				boost::asio::streambuf			req_;
				size_t							body_size_ = 0;
				size_t							max_body_size_ = 0;
				server_handler_ptr				hander_;
				body_callback_func				func_data_callback_;
				std::string						respond_header_;
				std::function<void(boost::system::error_code & ec)>	func_chunk_output_callback_;
			};
			class context : public context_base  {
			public:
				context(async_server_weak_ptr p, boost::asio::io_service & io) :
					context_base(p, io), socket_(io) {}
				
				virtual void	handshake() override {
					begin_http();
				}
				virtual void read_header() override {
					req_ptr_->set_https( false );
					boost::asio::async_read_until(socket_, req_, "\r\n\r\n", strand_.wrap(std::bind(&context_base::on_header, shared_from_this(), std::placeholders::_1)));
				}
				virtual void read_chunk_size() override {
					boost::asio::async_read_until(socket_, req_, "\n", strand_.wrap(std::bind(&context_base::on_chunk_size, shared_from_this(), std::placeholders::_1)));
				}
				virtual void read_body_content(size_t nLeast) override {
					boost::asio::async_read(socket_, req_, boost::asio::transfer_at_least(nLeast), strand_.wrap(std::bind(&context_base::on_body_content, shared_from_this(), std::placeholders::_1)));
				}
				virtual void read_chunk_data(bool boLastPackage, size_t nLeast) override {
					boost::asio::async_read(socket_, req_, boost::asio::transfer_at_least(nLeast), 
						strand_.wrap(std::bind(&context_base::on_read_chunk_data, shared_from_this(), std::placeholders::_1, boLastPackage, nLeast)));
				}
				virtual void read_chunk_end_data(bool boLastPackage) override {
					boost::asio::async_read_until(socket_, req_, "\n",
						strand_.wrap(std::bind(&context_base::on_read_chunk_data_end, shared_from_this(), std::placeholders::_1, boLastPackage)));
				}
				virtual void write_data_imp(const boost::container::static_vector<boost::asio::const_buffer, 3> & bufs) override {
					boost::asio::async_write(socket_, bufs, strand_.wrap(std::bind(&context_base::after_write_data, shared_from_this(), std::placeholders::_1)));
				}

				boost::asio::ip::tcp::socket::lowest_layer_type & socket()	{ return socket_.lowest_layer(); }
				boost::asio::ip::tcp::socket	socket_;
			};
			class ssl_context : public context_base {
			public:
				ssl_context(async_server_weak_ptr p, boost::asio::io_service & io, boost::asio::ssl::context & ssl_context) :
					context_base(p, io), ssl_context_(ssl_context), socket_(io, ssl_context) {}

				boost::asio::ip::tcp::socket::lowest_layer_type & socket()	{ return socket_.lowest_layer(); }
				virtual void	handshake() override {
					socket_.async_handshake(boost::asio::ssl::stream_base::server, strand_.wrap([self = shared_from_this(), this](const boost::system::error_code& ec) {
						if ( ec )
							return close();
						begin_http();
					}));
				}
				virtual void read_header() override {
					req_ptr_->set_https( true );
					boost::asio::async_read_until(socket_, req_, "\r\n\r\n", strand_.wrap(std::bind(&context_base::on_header, shared_from_this(), std::placeholders::_1)));
				}
				virtual void read_chunk_size() override {
					boost::asio::async_read_until(socket_, req_, "\n", strand_.wrap(std::bind(&context_base::on_chunk_size, shared_from_this(), std::placeholders::_1)));
				}
				virtual void read_body_content(size_t nLeast) override {
					boost::asio::async_read(socket_, req_, boost::asio::transfer_at_least(nLeast), strand_.wrap(std::bind(&context_base::on_body_content, shared_from_this(), std::placeholders::_1)));
				}
				virtual void read_chunk_data(bool boLastPackage, size_t nLeast) override {
					boost::asio::async_read(socket_, req_, boost::asio::transfer_at_least(nLeast), 
						strand_.wrap(std::bind(&context_base::on_read_chunk_data, shared_from_this(), std::placeholders::_1, boLastPackage, nLeast)));
				}
				virtual void read_chunk_end_data(bool boLastPackage) override {
					boost::asio::async_read_until(socket_, req_, "\n",
						strand_.wrap(std::bind(&context_base::on_read_chunk_data_end, shared_from_this(), std::placeholders::_1, boLastPackage)));
				}
				virtual void write_data_imp(const boost::container::static_vector<boost::asio::const_buffer, 3> & bufs) override {
					boost::asio::async_write(socket_, bufs, strand_.wrap(std::bind(&context_base::after_write_data, shared_from_this(), std::placeholders::_1)));
				}

				boost::asio::ssl::context & ssl_context_;
				boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
			};


			class svr_base {
			public:
				svr_base(boost::asio::io_service & io, const std::string & ip, uint16_t port, size_t bl) : ip_(ip), port_(port), backlog_(bl),acceptor_(io) {}
				virtual ~svr_base() {}

				void init() {
					boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip_.c_str()), port_);
					boost::asio::ip::tcp::no_delay option(true);
					boost::asio::socket_base::reuse_address option2(true);

					acceptor_.open(endpoint.protocol());
					acceptor_.set_option(option);
					acceptor_.set_option(option2);
					acceptor_.bind(endpoint);
				}

				void	do_accept(async_server_ptr parent) {
					std::shared_ptr<context_base> pContext = make_context(parent, acceptor_.get_io_service());
					acceptor_.async_accept(pContext->socket(), [pContext, this](boost::system::error_code ec) {
						auto lock = pContext->svr_.lock();
						if (lock == nullptr || ec)
							return;
						pContext->init_timeout_handle();
						pContext->reset_timeout(lock->get_options().get_timeout());
						pContext->handshake();
						do_accept(lock);
					});
				}
				void	stop() {
					acceptor_.cancel();
					acceptor_.close();
				}

				virtual std::shared_ptr<context_base>	make_context(async_server_weak_ptr p, boost::asio::io_service & io) {
					return std::make_shared<context>(p, io);
				}

				std::string		ip_;
				uint16_t		port_;
				size_t			backlog_;
				boost::asio::ip::tcp::acceptor acceptor_;
			};

			class ssl_svr_base : public svr_base {
			public:
				ssl_svr_base(boost::asio::io_service & io, const std::string & ip, uint16_t port, size_t bl, boost::asio::ssl::context & ssl_context) 
					: svr_base(io, ip, port, bl), ssl_context_(ssl_context) {}

				virtual std::shared_ptr<context_base>	make_context(async_server_weak_ptr p, boost::asio::io_service & io) {
					return std::make_shared<ssl_context>(p, io, ssl_context_);
				}

				boost::asio::ssl::context & ssl_context_;
			};

		private:
			
			async_server_weak_ptr  weak_from_this() {
				return shared_from_this();
			}

			class dispatch;
			using dispatch_ptr = std::shared_ptr<dispatch>;
			class dispatch {
			public:
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

			boost::asio::io_service & io_;

			std::list<server_filter_ptr>			list_filter_;
			std::list<std::unique_ptr<svr_base>>	list_svr_;
			dispatch_ptr							default_dispatch_;
			std::list<dispatch_ptr>					list_dispatch_;
			server_options							options_;
		};

	}
}


#endif//ARA_ASYNC_HTTPSERVER_H
