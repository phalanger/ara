
#ifndef ARA_ASYNC_HTTPCLIENT_H
#define ARA_ASYNC_HTTPCLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/array.hpp>

#include "httpbase.h"

#include <cctype>


/**
	auto client = http::async_client::make(io, ssl_context);

	auto c = client->request( http::request::make("https://my.website.com/?func=aaa", http::header("key1", "val1")("key2","val2"), "{json_ver:1}"),
		http::respond::make_simple([](int nCode, const std::string & strMsg, ara::http::header && h, std::string && strBody){
			if (nCode > 0)
				std::cout << "Body:" << strBody << std::endl;	
		})
	);


	auto req = http::request::make("https://www.163.com");
	auto res = http::respond::make_async();
	res->on_error([](const boost::system::error_code & ec){
		//handle error.
	}).on_header([](int code, const std::string & strMsg, ara::http::header & h, size_t nBodySize){
		//handle header.
	}).on_body([](std::string && strBody){
		//handle body.
	});
	client->request( req, res );
*/

namespace ara {
	namespace http {

		class async_client;
		typedef std::shared_ptr<async_client>	async_client_ptr;
		typedef std::weak_ptr<async_client>		async_client_weak_ptr;

		class client_options
		{
		public:
			client_options() {}

			void				set_timeout(const timer_val & v) { time_out_ = v; }
			const timer_val &	get_timeout() const { return time_out_; }

			void		set_verify_peer(bool b) { verify_peer_ = b; }
			bool		get_verify_peer() const { return verify_peer_; }

			void		set_cache_size(size_t n) { cache_size_ = n; }
			size_t		get_cache_size() const { return cache_size_; }

			void		set_redirect_count(size_t n) { redirect_count_ = n; }
			size_t		get_redirect_count() const { return redirect_count_; }
		protected:
			timer_val	time_out_ = timer_val(10);
			bool		verify_peer_ = false;
			size_t		cache_size_ = 64 * 1024;
			size_t		redirect_count_ = 5;
		};

		namespace respond {

			class client_respond_base : public std::enable_shared_from_this<client_respond_base>
			{
			public:
				virtual ~client_respond_base() {}
				virtual void on_finished(const boost::system::error_code & ec, async_client_ptr pClient) = 0;
				virtual void on_header(int nCode, const std::string & strMsg, ara::http::header && h, size_t nBodySize, async_client_ptr client) = 0;
				virtual void on_body(const void * pdata, size_t n, async_client_ptr client) = 0;
				virtual void on_redirect() = 0;
			public:
				inline void					_init(client_request_ptr r) { 	req_ = r; }
				inline client_request_ptr		_req() const { return req_; }
				inline std::string	&	_temp_data() { return temp_data_; }
				inline boost::asio::streambuf &	_streambuf() { return response_; }
				inline size_t		_get_body_size()	const { return body_size_; }
				inline size_t &		_recv_size() { return recv_size_; }
				inline void			_add_recv_size(size_t n) { recv_size_ += n; }
				inline void			_set_body_size(size_t n) { body_size_ = n; }
				inline int			_get_code() const { return code_; }
				inline void			_set_code(int n) { code_ = n; }
				inline bool			_is_https() const {	return req_->is_https(); }
				inline void			_set_redirect(size_t n) { redirect_count_ = n; }
				inline bool			_should_redirect(int nCode, bool check_only) { 
					if ((nCode / 100) != 3 || _temp_data().empty())
						return false;
					else if (!_req()->get_method().empty() && _req()->get_method() != "GET")
						return false;
					else if (redirect_count_ == 0)
						return false;
					if (!check_only)
						--redirect_count_;
					return true;
				}
				void			_clear() {
					temp_data_.clear();
					body_size_ = recv_size_ = 0;
					code_ = 0;
					response_.consume(response_.size());
				}
			private:
				client_request_ptr	req_;
				std::string			temp_data_;
				boost::asio::streambuf		response_;
				size_t			body_size_ = 0;
				size_t			recv_size_ = 0;
				int				code_ = 0;
				size_t			redirect_count_ = 5;
			};
		}//namespace respond

		typedef std::shared_ptr<respond::client_respond_base>	client_respond_ptr;
		typedef std::weak_ptr<respond::client_respond_base>		client_respond_weak_ptr;

		class http_control
		{
		public:
			virtual ~http_control() {}
			virtual void stop() = 0;
		};
		typedef std::shared_ptr<http_control>	http_control_ptr;
		typedef std::weak_ptr<http_control>		http_control_weak_ptr;

		class async_client : public std::enable_shared_from_this<async_client>
		{
		public:
			class async_client_control : public http_control
			{
			public:
				async_client_control(async_client_ptr p) : p_(p) {}
				virtual void stop() {
					p_->stop();
				}
			protected:
				async_client_ptr	p_;
			};

			static std::shared_ptr<async_client>	make(boost::asio::io_service & io, boost::asio::ssl::context & ssl_context) {
				return std::make_shared<async_client>(io, ssl_context);
			}

			async_client &	set_options(const client_options & opt) {
				options_ = opt;
				if (opt.get_verify_peer())
					socket_ptr()->set_verify_mode(boost::asio::ssl::verify_peer);
				else
					socket_ptr()->set_verify_mode(boost::asio::ssl::verify_none);

				return *this;
			}

			http_control_ptr	request(client_request_ptr req, client_respond_ptr res, const client_options & opt) {
				return set_options(opt).request(req, res);
			}

			http_control_ptr	request(client_request_ptr req, client_respond_ptr res) {

				auto self = shared_from_this();
				res->_init(req);
				res->_set_redirect(options_.get_redirect_count());

				std::shared_ptr<async_client_control>	pControl = std::make_shared<async_client_control>(self);
				retry_count_ = 1;
				if (req->has_body_callback()) {
					stop();
					retry_count_ = 0;
				}
				_go(res);

				return pControl;
			}

			void stop() {
				if (is_connect_)
					close_all();
			}

			boost::asio::ssl::stream<boost::asio::ip::tcp::socket> & ssl_stream_socket() {
				return *socket_ptr();
			}
		public:
			void	_go(client_respond_ptr res) {
				if (is_connect_) {
					if (res->_req()->get_server() != last_server_)
						close_all();
					else if (last_time_ != 0 && time(NULL) >= keep_alive_expire_time_)
						if (keep_alive_used_count_)
							--keep_alive_used_count_;
						else
							close_all();
				}

				auto self = shared_from_this();
				reset_timeout();
				timer_.async_wait(strand_.wrap([self, this, res](const boost::system::error_code & ec) {
					if (ec.value() == boost::asio::error::operation_aborted)
						return;
					close_all();
					on_net_fail(boost::asio::error::timed_out, res);
				}));

				if (is_connect_) {
					handle_handshake(boost::system::error_code(), res);
					return;
				}

				std::string strServices;
				if (res->_req()->get_port() != 0)
					strext(strServices).append_int(res->_req()->get_port());
				else
					strServices = res->_is_https() ? "https" : "http";

				boost::asio::ip::tcp::resolver::query query(res->_req()->get_server(), strServices);

				resolver_.async_resolve(query, strand_.wrap([self, this, res](const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
					handle_resolve(err, endpoint_iterator, res);
				}));
			}

			void	_get_data(client_respond_ptr res) {
				res->_recv_size() = 0;
				size_t n = res->_get_body_size();
				if (n == 0)
					on_respond_finished( res );
				else if (n == std::string::npos)	//chunk
					handle_read_chunk_size(res);
				else
					handle_read_content(boost::system::error_code(), res);
			}

			async_client(boost::asio::io_service & io, boost::asio::ssl::context & ssl_context)
				: io_(io), resolver_(io), ssl_context_(ssl_context), timer_(io), strand_(io) {
			}
		protected:
			void close_all() {
				is_connect_ = false;
				last_server_.clear();
				boost::system::error_code ec;
				timer_.cancel(ec);
				if (socket_) {
					socket_->lowest_layer().close(ec);
					socket_ = nullptr;
				}
			}

			void reset_timeout()	{
				auto tTimeout = options_.get_timeout();
				boost::system::error_code ec;
				timer_.expires_from_now(boost::posix_time::seconds(static_cast<long>(tTimeout.sec())) + boost::posix_time::microseconds(tTimeout.micro_sec()), ec);
			}

			void on_net_fail(const boost::system::error_code& err, client_respond_ptr res, bool can_retry = false) {
				if (can_retry && retry_count_ > 0) {
					--retry_count_;
					close_all();
					_go(res);
				}
				else {
					close_all();
					retry_count_ = 0;
					handle_http_respond(err, res);
				}
			}

			void handle_http_respond(const boost::system::error_code& err, client_respond_ptr res)	{
				if (keep_alive_used_count_ == 0)
					close_all();
				else
				{
					boost::system::error_code ec;
					timer_.cancel(ec);
				}
				res->on_finished(err, shared_from_this());
			}

			void on_respond_finished(client_respond_ptr res) {
				if (res->_should_redirect(res->_get_code(), false)) {	//redirect
					res->_req()->set_full_url(res->_temp_data());
					res->_clear();
					res->on_redirect();
					_go(res);
					return;
				}
				handle_http_respond(boost::system::error_code(), res);
			}


			void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator, client_respond_ptr res) {
				if (err)
					return	on_net_fail(err, res);

				auto self = shared_from_this();
				auto pSocket = socket_ptr();
				boost::asio::async_connect(pSocket->lowest_layer(), endpoint_iterator, strand_.wrap([self, this, pSocket, res](const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
					handle_connect(err, res);
				}));
			}

			void handle_connect(const boost::system::error_code& err, client_respond_ptr res) 	{
				if (err)
					return	on_net_fail(err, res);

				if (res->_is_https()) {
					auto self = shared_from_this();
					auto pSocket = socket_ptr();
					pSocket->async_handshake(boost::asio::ssl::stream_base::client, strand_.wrap([pSocket, self, this, res](const boost::system::error_code& err) {
						handle_handshake(err, res);
					}));
				}
				else {
					handle_handshake(boost::system::error_code(), res);
				}
			}

			void handle_handshake(const boost::system::error_code& err, client_respond_ptr res) {
				if (err)
					return	on_net_fail(err, res);

				is_connect_ = true;
				last_server_ = res->_req()->get_server();

				if (res->_is_https())
					send_request_header(*socket_ptr(), res);
				else
					send_request_header(socket_ptr()->next_layer(), res);
			}

			template<class typeSocket>
			void	send_request_header(typeSocket & s, client_respond_ptr res) {
				std::string & str = res->_temp_data();
				str.clear();

				auto req = res->_req();
				strext(str).printf("%v %v HTTP/%v\r\nHost: %v\r\n"
									, (req->get_method().empty() ? std::string("GET") : req->get_method())
									, req->get_url()
									, req->get_http_version()
									, req->get_server());

				const header & h = req->get_header();
				for (const auto & it : h)
					strext(str).printf("%v: %v\r\n", it.first , it.second);

				if (req->is_body_chunk_mode())
					str += "Transfer-Encoding: chunked\r\n";
				else if (!req->get_body().empty() && h.find("Content-Length") == h.end())
					strext(str).printf("Content-Length: %v\r\n", req->get_body().size());
				str += "\r\n";

				auto self = shared_from_this();
				auto pSocket = socket_ptr();
				boost::asio::async_write(s, boost::asio::buffer(str), strand_.wrap([&s, self, pSocket, this, res](const boost::system::error_code& err, size_t) {
					if (err)
						on_net_fail(err, res, true);
					else
						send_request_body(s, res);
				}));
			}

			template<class typeSocket>
			void	send_request_body(typeSocket & s, client_respond_ptr res) 	{
				res->_temp_data().clear();	//clear the header cache data
				auto self = shared_from_this();
				auto pSocket = socket_ptr();

				if (!res->_req()->get_body().empty()) {
					boost::asio::async_write(s, boost::asio::buffer(res->_req()->get_body()), strand_.wrap([self, pSocket, this, res](const boost::system::error_code& err, size_t) {
						if (err)
							on_net_fail(err, res, true);
						else
							to_read_status_line(res);
					}));
				} else if (res->_req()->has_body_callback()) {
					if (res->_req()->is_body_chunk_mode())
						send_request_body_chunk(s, res);
					else {
						res->_set_body_size(res->_req()->get_body_callback_size());
						send_request_body_callback(s, res);
					}
						
				} else
					to_read_status_line(res);
			}

			template<class typeSocket>
			void	send_request_body_chunk(typeSocket & s, client_respond_ptr res) {
				std::string & str = res->_temp_data();
				
				const size_t nPrefixSize = 128;
				size_t nCacheSize = options_.get_cache_size();
				str.resize(nPrefixSize + nCacheSize + 2);
				char * pBegin = const_cast<char *>(str.data());
				char * pData = pBegin + nPrefixSize;

				size_t res_size = res->_req()->get_body(pData, nCacheSize);
				size_t prefix_size = ara::snprintf(pBegin, nPrefixSize, "%X\r\n", res_size);
				auto self = shared_from_this();
				auto pSocket = socket_ptr();
				reset_timeout();
				
				if (res_size == 0) {
					pBegin[prefix_size++] = '\r';
					pBegin[prefix_size++] = '\n';
					boost::asio::async_write(s, boost::asio::buffer(pBegin, prefix_size), strand_.wrap([self, pSocket, this, res](const boost::system::error_code& err, size_t) {
						if (err)
							on_net_fail(err, res, true);
						else
							to_read_status_line(res);
					}));
				} else {
					pData[res_size++] = '\r';
					pData[res_size++] = '\n';

					boost::array<boost::asio::mutable_buffer, 2> bufs = {
						boost::asio::buffer(pBegin, prefix_size),
						boost::asio::buffer(pData, res_size)
					};
					boost::asio::async_write(s, bufs, strand_.wrap([&s, self, pSocket, this, res](const boost::system::error_code& err, size_t) {
						if (err)
							on_net_fail(err, res, true);
						else
							send_request_body_chunk(s, res);
					}));
				}
			}

			template<class typeSocket>
			void	send_request_body_callback(typeSocket & s, client_respond_ptr res) {
				reset_timeout();
				size_t nCacheSize = std::min<size_t>(options_.get_cache_size(), res->_get_body_size());
				if (nCacheSize == 0) {
					to_read_status_line(res);
				} else {
					std::string & str = res->_temp_data();
					str.resize(nCacheSize);
					char * pBegin = const_cast<char *>(str.data());
					size_t res_size = res->_req()->get_body(pBegin, nCacheSize);
					if (res_size == 0) {
						to_read_status_line(res);
					} else {
						auto self = shared_from_this();
						auto pSocket = socket_ptr();

						boost::asio::async_write(s, boost::asio::buffer(pBegin, res_size), strand_.wrap([&s, self, pSocket, this, res](const boost::system::error_code& err, size_t n) {
							if (err)
								on_net_fail(err, res, true);
							else {
								res->_set_body_size(res->_get_body_size() - n);
								send_request_body_callback(s, res);
							}
						}));
					}
				}
			}

			void to_read_status_line(client_respond_ptr res) {

				// Read the response status line.
				auto self = shared_from_this();
				auto pSocket = socket_ptr();
				auto func = strand_.wrap([self, pSocket, this, res](const boost::system::error_code& err, size_t) {
					handle_read_status_line(err, res);
				});
				res->_temp_data().clear();	//clear cache data
				if (res->_is_https())
					boost::asio::async_read_until(*pSocket, res->_streambuf(), "\r\n", std::move(func));
				else
					boost::asio::async_read_until(pSocket->next_layer(), res->_streambuf(), "\r\n", std::move(func));
			}

			void handle_read_status_line(const boost::system::error_code& err, client_respond_ptr res) {
				if (err)
					return	on_net_fail(err, res, true);
				else
					reset_timeout();

				// Check that response is OK.
				std::string		http_version, status_msg;
				int				code = 0;

				std::istream response_stream(&res->_streambuf());
				response_stream >> http_version;
				response_stream >> code;
				std::getline(response_stream, status_msg);

				strext(status_msg).trim_inplace(" \r\t\n");
				res->_set_code(code);
				res->_temp_data().swap(status_msg);

				if (!response_stream || http_version.substr(0, 5) != "HTTP/")
					return on_net_fail(err, res);

				// Read the response headers, which are terminated by a blank line.
				auto self = shared_from_this();
				auto pSocket = socket_ptr();
				auto func = strand_.wrap([self, pSocket, this, res](const boost::system::error_code& err, size_t) {
					handle_read_headers(err, res);
				});
				if (res->_is_https())
					boost::asio::async_read_until(*pSocket, res->_streambuf(), "\r\n\r\n", std::move(func));
				else
					boost::asio::async_read_until(pSocket->next_layer(), res->_streambuf(), "\r\n\r\n", std::move(func));
			}

			void handle_read_headers(const boost::system::error_code& err, client_respond_ptr res) {
				if (err)
					return	on_net_fail(err, res);

				// Process the response headers.
				std::istream response_stream(&res->_streambuf());
				std::string sheader, sKey;
				header		h;

				while (std::getline(response_stream, sheader) && sheader != "\r") {
					std::string::size_type p = sheader.find(':');
					std::string sVal;
					if (sheader.empty())
						continue;
					else if (ara::isspace(sheader[0])) {
						std::string & val = h[sKey];
						val += "\r\n";
						val += sheader;
						continue;
					} else if (p == std::string::npos) {
						sKey = sheader;
					} else {
						std::string::size_type p2 = sheader.find_first_not_of(" \t", p + 1);
						if (p2 == std::string::npos) {
							sKey = sheader;
						} else {
							std::string::size_type p3 = sheader.find_last_not_of(" \t\r\n");
							if (p3 == std::string::npos) {
								sKey = sheader.substr(0, p);
								sVal = sheader.substr(p2);
							} else {
								sKey = sheader.substr(0, p);
								sVal = sheader.substr(p2, p3 - p2 + 1);
							}
						}
					}
					h.insert(std::make_pair(std::move(sKey), std::move(sVal)));
				}

				auto it = h.find("Transfer-Encoding");
				if (it != h.end() && it->second == "chunked")
					res->_set_body_size( std::string::npos );
				else if ((it = h.find("Content-Length")) != h.end())
					res->_set_body_size( strext(it->second).to_int<size_t>() );
				else
					res->_set_body_size(0);

				time_t	tTimeout = 600;
				size_t	nCountSetting = size_t(-1);
				it = h.find("connection");
				if (it != h.end() && it->second == "close")
					tTimeout = 0;
				else {
					it = h.find("keep-alive");
					if (it != h.end()) {

						const std::string & strSub = it->second;

						std::string::size_type p2 = strSub.find("timeout=");
						if ( p2 != std::string::npos )
							tTimeout = strext(strSub).find_int<time_t>(p2 + 8);

						p2 = strSub.find("max=");
						if ( p2 != std::string::npos )
							nCountSetting = strext(strSub).find_int<size_t>(p2 + 4);
					}//if (it != h.end())
				}//else

				if (tTimeout == 0) {
					keep_alive_expire_time_ = 0;
					keep_alive_used_count_ = 0;
				} else {
					keep_alive_expire_time_ = time(NULL) + tTimeout;
					keep_alive_used_count_ = nCountSetting;
				}

				std::string status_msg;
				status_msg.swap(res->_temp_data());
				if ((res->_get_code() / 100) == 3) {
					res->_temp_data() = h["Location"];
				}
				res->on_header(res->_get_code(), status_msg, std::move(h), res->_get_body_size(), shared_from_this());
			}

			void handle_read_content(const boost::system::error_code& err, client_respond_ptr res)	{

				if (err)	{
					if (err != boost::asio::error::eof)
						on_net_fail(err, res);
					else
						on_respond_finished( res );
					return;
				}
				else
					reset_timeout();

				auto self = shared_from_this();
				auto pSocket = socket_ptr();
				size_t nRestSize = res->_get_body_size() - res->_recv_size();
				size_t nCopy = std::min<size_t>(res->_streambuf().size(), nRestSize);
				res->on_body(boost::asio::buffer_cast<const char *>(res->_streambuf().data()), nCopy, self);
				res->_streambuf().consume(nCopy);
				nRestSize -= nCopy;
				if (nRestSize == 0)
					return on_respond_finished(res);
				
				auto func = strand_.wrap([self, pSocket, this, res](const boost::system::error_code& err, size_t) {
					handle_read_content(err, res);
				});

				if (nRestSize > options_.get_cache_size())
					nRestSize = options_.get_cache_size();
				if (res->_is_https())
					boost::asio::async_read(*pSocket, res->_streambuf(), boost::asio::transfer_at_least(nRestSize), std::move(func));
				else
					boost::asio::async_read(pSocket->next_layer(), res->_streambuf(), boost::asio::transfer_at_least(nRestSize), std::move(func));
			}

			void handle_read_chunk_size(client_respond_ptr res)	{

				auto d = res->_streambuf().data();
				ref_string	strData(boost::asio::buffer_cast<const char *>(d), boost::asio::buffer_size(d));
				ref_string::size_type p = strData.find('\n');

				if (p == ref_string::npos)	{
					auto self = shared_from_this();
					auto pSocket = socket_ptr();
					auto func = strand_.wrap([self, pSocket, this, res](const boost::system::error_code& err, size_t) {
						if (err)
							return on_net_fail(err, res);
						else
							handle_read_chunk_size(res);
					});
					if (res->_is_https())
						boost::asio::async_read_until(*pSocket, res->_streambuf(), "\n", std::move(func));
					else
						boost::asio::async_read_until(pSocket->next_layer(), res->_streambuf(), "\n", std::move(func));
					return;
				} else {
					std::istream response_stream(&res->_streambuf());
					size_t nChunkSize = 0;
					response_stream >> std::hex >> nChunkSize;
					std::string dummy;
					std::getline(response_stream, dummy);

					handle_read_chunk_data(nChunkSize == 0, nChunkSize, res);
				}
			}

			void handle_read_chunk_data(bool boLastPackage, size_t nChunkSize, client_respond_ptr res) {
				auto d = res->_streambuf().data();
				ref_string	strData(boost::asio::buffer_cast<const char *>(d), boost::asio::buffer_size(d));

				if (nChunkSize)	{
					size_t nCopy = std::min<size_t>(strData.size(), nChunkSize);
					res->on_body(strData.data(), nCopy, shared_from_this());
					res->_streambuf().consume(nCopy);
					nChunkSize -= nCopy;
					reset_timeout();
				}

				if (nChunkSize == 0)
					handle_read_chunk_data_end(boLastPackage, res);
				else
				{
					auto self = shared_from_this();
					auto pSocket = socket_ptr();
					auto func = strand_.wrap([self, pSocket, this, res, boLastPackage, nChunkSize](const boost::system::error_code& err, size_t) {
						if (err)
							on_net_fail(err, res);
						else
							handle_read_chunk_data(boLastPackage, nChunkSize, res);
					});
					if (res->_is_https())
						boost::asio::async_read(*pSocket, res->_streambuf(), boost::asio::transfer_at_least(1), std::move(func));
					else
						boost::asio::async_read(pSocket->next_layer(), res->_streambuf(), boost::asio::transfer_at_least(1), std::move(func));
				}
			}

			void handle_read_chunk_data_end(bool boLastPackage, client_respond_ptr res) {
				auto d = res->_streambuf().data();
				ref_string	strData(boost::asio::buffer_cast<const char *>(d), boost::asio::buffer_size(d));
				ref_string::size_type p = strData.find('\n');

				if (p != ref_string::npos)	{
					res->_streambuf().consume(p + 1);

					if (boLastPackage)
						return on_respond_finished( res );
					else
						return handle_read_chunk_size(res);
				} else 	{
					auto self = shared_from_this();
					auto pSocket = socket_ptr();
					auto func = strand_.wrap([self, pSocket, this, res, boLastPackage](const boost::system::error_code & e, size_t nSize) {
						if (e)
							on_net_fail(e, res);
						else
							handle_read_chunk_data_end(boLastPackage, res);
					});
					if (res->_is_https())
						boost::asio::async_read_until(*pSocket, res->_streambuf(), "\n", std::move(func));
					else
						boost::asio::async_read_until(pSocket->next_layer(), res->_streambuf(), "\n", std::move(func));
				}
			}

			std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_ptr() {
				if (socket_ == nullptr)
					socket_ = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_, ssl_context_);
				return socket_;
			}

			boost::asio::io_service &		io_;
			client_options					options_;
			boost::asio::ip::tcp::resolver	resolver_;
			std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;
			boost::asio::ssl::context	&	ssl_context_;
			boost::asio::deadline_timer		timer_;
			boost::asio::io_context::strand	strand_;
			bool							is_connect_ = false;
			std::string						last_server_;
			size_t							retry_count_ = 2;
			time_t							last_time_ = 0;
			time_t							keep_alive_expire_time_ = 0;
			size_t							keep_alive_used_count_ = static_cast<size_t>(-1);

		};

		namespace respond {

			class simple_client_respond : public client_respond_base
			{
			public:
				simple_client_respond(std::function<void(int nCode, const std::string & strMsg, ara::http::header && h, std::string && strBody)> && func) : func_(std::move(func)) {}

				virtual void on_finished(const boost::system::error_code & ec, async_client_ptr client) {
					if (func_) {
						decltype(func_)	temp;
						temp.swap(func_);
						int nCode = ec ? (-ec.value()) : _get_code();
						temp(nCode, status_msg_, std::move(respond_header_), std::move(respond_body_));
					}
				}
				virtual void on_header(int nCode, const std::string & strMsg, ara::http::header && h, size_t nBodySize, async_client_ptr client) {
					respond_header_ = std::move(h);
					status_msg_ = strMsg;
					if (nBodySize + 1 > 1)
						respond_body_.reserve(nBodySize);
					client->_get_data(shared_from_this());
				}
				virtual void on_body(const void * pdata, size_t n, async_client_ptr client) {
					respond_body_.append(static_cast<const char *>(pdata), n);
					_add_recv_size( n );
				}
				virtual void on_redirect() {
					respond_header_.clear();
					status_msg_.clear();
					respond_body_.clear();
				}
			protected:
				std::function<void(int nCode, const std::string & strMsg, ara::http::header && h, std::string && strBody)>	func_;
				ara::http::header	respond_header_;
				std::string			status_msg_;
				std::string			respond_body_;
			};

			inline std::shared_ptr<simple_client_respond>		make_simple(std::function<void(int nCode, const std::string & strMsg, ara::http::header && h, std::string && strBody)> && func) {
				return std::make_shared<simple_client_respond>(std::move(func));
			}

			///////////////////////////////////////////////////////////

			class async_client_respond : public client_respond_base
			{
			public:
				async_client_respond() {}

				async_client_respond &	on_error(std::function<void(const boost::system::error_code & ec)> && func) { err_func_ = std::move(func); return *this; }
				async_client_respond &	on_header(std::function<bool(int code, const std::string & strMsg, ara::http::header && h, size_t nBodySize)> && func) { header_func_ = std::move(func); return *this; }
				async_client_respond &	on_body(std::function<void(std::string && strBody)> && func, size_t nMaxBodySize = std::string::npos) { body_string_func_ = std::move(func); max_body_size_ = nMaxBodySize; return *this; }
				async_client_respond &	on_body(std::function<void(const void * p, size_t n)> && func) { body_stream_func_ = std::move(func); return *this; }

				virtual void on_finished(const boost::system::error_code & ec, async_client_ptr pClient) {
					if (ec) {
						if (err_func_) {
							decltype(err_func_)		temp;
							temp.swap(err_func_);
							temp(ec);
						}
						return;
					}
					if (body_string_func_) {
						decltype(body_string_func_)	temp;
						temp.swap(body_string_func_);
						temp(std::move(body_));
					}
					if (body_stream_func_) {
						decltype(body_stream_func_)	temp;
						temp.swap(body_stream_func_);
						temp(nullptr, 0);
					}
				}
				virtual void on_header(int nCode, const std::string & strMsg, ara::http::header && h, size_t nBodySize, async_client_ptr client) {
					if (_should_redirect(nCode, true))
						return;
					if (header_func_) {
						decltype(header_func_) temp;
						temp.swap(header_func_);
						if (!temp(nCode, strMsg, std::move(h), nBodySize))
							return;
					}
					client->_get_data(shared_from_this());
				}
				virtual void on_body(const void * pdata, size_t n, async_client_ptr client) {
					if (body_string_func_) {
						if (body_.size() < max_body_size_) {
							body_.append(static_cast<const char *>(pdata), std::min<size_t>(max_body_size_ - body_.size(), n));
						}
					}
					if (body_stream_func_)
						body_stream_func_(pdata, n);
				}
				virtual void on_redirect() {
					body_.clear();
				}

			protected:
				std::function<void(const boost::system::error_code & ec)>											err_func_;
				std::function<bool(int code, const std::string & strMsg, ara::http::header && h, size_t nBodySize)>	header_func_;
				std::function<void(std::string && strBody)>															body_string_func_;
				std::function<void(const void * p, size_t n)>														body_stream_func_;
				size_t				max_body_size_ = 1024 * 1024;
				std::string			body_;
			};

			inline std::shared_ptr<async_client_respond>		make_async() {
				return std::make_shared<async_client_respond>();
			}
		}//namespace respond
	}//namespace http
}//namespace ara

#endif//ARA_ASYNC_HTTPCLIENT_H
