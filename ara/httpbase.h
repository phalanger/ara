
#ifndef ARA_HTTPBASE_H
#define ARA_HTTPBASE_H

#include "datetime.h"
#include "stringext.h"
#include "token.h"
#include "internal/url.h"

#include <map>
#include <list>
#include <functional>
#include <mutex>

namespace ara {
	namespace http {

		class header : public std::map<std::string, std::string, nocase_string_compare<std::string>>
		{
		public:
			typedef	std::map<std::string, std::string, nocase_string_compare<std::string>>	typeParent;
			typedef typeParent::const_iterator const_iterator;
			typedef typeParent::iterator	iterator;
			header() {}
			header(const std::string & key, const std::string & val) {
				this->insert(std::make_pair(key, val));
			}
			header & operator()(const std::string & key, const std::string & val) {
				this->insert(std::make_pair(key, val));
				return *this;
			}
		};

		class params : public std::list<std::pair<std::string, std::string>>
		{
		public:
			typedef	std::list<std::pair<std::string, std::string>>	typeParent;
			typedef typeParent::const_iterator const_iterator;
			typedef typeParent::iterator	iterator;
			params() {}
			params(const std::string & key, const std::string & val) {
				this->push_back(std::make_pair(key, val));
			}
			params(const std::string & key, std::string && val) {
				this->push_back(std::make_pair(key, std::move(val)));
			}
			template<typename typeObj>
			params(const std::string & key, const typeObj & val) {
				std::string strVal;
				strext(strVal).append(val);
				this->push_back(std::make_pair(key, strVal));
			}

			params & operator()(const std::string & key, const std::string & val) {
				this->push_back(std::make_pair(key, val));
				return *this;
			}
			params & operator()(const std::string & key, std::string && val) {
				this->push_back(std::make_pair(key, std::move(val)));
				return *this;
			}
			template<typename typeObj>
			params & operator()(const std::string & key, const typeObj & val) {
				std::string strVal;
				strext(strVal).append(val);
				this->push_back(std::make_pair(key, std::move(strVal)));
				return *this;
			}

			std::string to_string() const {
				std::string		res;
				for (const auto & it : *this) {
					if (!res.empty())
						res.append(1, '&');
					res.append(internal::url::encoded(it.first));
					res.append(1, '=');
					res.append(internal::url::encoded(it.second));
				}
				return res;
			}
			void from_string(const std::string & str) {
				for (auto item : token_base<std::string, char>(str, '&')) {
					auto p = item.find('=');
					if (p == ref_string::npos)
						this->push_back(std::make_pair(internal::url::decoded(item.str()), std::string()));
					else
						this->push_back(std::make_pair(item.substr(0, p).str(), item.substr(p + 1).str()));
				}
			}
		};

		class client_request;
		typedef std::shared_ptr<client_request>		client_request_ptr;
		typedef std::weak_ptr<client_request>		client_request_weak_ptr;
		class client_request
		{
		public:
			typedef std::function<size_t(void * p, size_t nBuf)>	body_callback_func;

			client_request(bool boHttps, const std::string & strServer, int nPort, const std::string & strURL) : is_https_(boHttps), server_(strServer), url_(strURL) {
				if (nPort == 0)
					port_ = boHttps ? 443 : 80;
				else
					port_ = nPort;
			}

			client_request &	set_full_url(const std::string & strFullURL) {
				std::string		strHost, strSchema, strPath;
				ara::internal::url::split_url(strFullURL, strSchema, strHost, strPath);
				int nPort = 0;
				std::string::size_type p = strHost.find(':');
				if (p != std::string::npos) {
					nPort = strext(strHost).to_int<int>(p + 1);
					strHost = strHost.substr(0, p);
				}
				is_https_ = (strSchema == "https");
				server_ = strHost;
				url_ = strPath;
				port_ = nPort;
				return *this;
				
			}

			inline client_request &	set_method(const std::string & strMethod) {
				method_ = strMethod;
				return *this;
			}

			inline client_request &		set_header(header && h) {
				header_ = std::move(h);
				return *this;
			}

			inline client_request &		add_header(header && h) {
				for (auto & it : h) {
					header_.insert(std::move(it));
				}
				return *this;
			}
			inline client_request &		add_header(const header & h) {
				for (auto & it : h) {
					header_.insert( it );
				}
				return *this;
			}

			inline client_request &		set_header(const std::string & strHeaderKey, const std::string & strHeaderItem) {
				header_[strHeaderKey] = strHeaderItem;
				return *this;
			}
			inline client_request &		set_header(const std::string & strHeaderKey, std::string && strHeaderItem) {
				header_[strHeaderKey] = std::move(strHeaderItem);
				return *this;
			}
			inline client_request &		set_http_version(const std::string & strVer) {
				version_ = strVer;
				return *this;
			}

			inline bool is_https() const { return is_https_; }
			inline const std::string & get_server() const { return server_; }
			inline const std::string & get_method() const { return method_; }
			inline const std::string & get_url() const { return url_; }
			inline int				get_port() const { return port_; }
			inline const std::string & get_http_version() const { return version_; }

			inline void		nobody() {}
			inline void		body(const std::string & strBody) {
				string_body(std::string(strBody));
			}
			inline void		body(std::string && strBody) {
				string_body(std::move(strBody));
			}
			void		body(body_callback_func && func, size_t nSize = std::string::npos) {
				if (method_.empty())
					method_ = "POST";
				if (header_.find("Content-Type") == header_.end())
					header_["Content-Type"] = "x-application/octet-stream";

				body_callback_size_ = nSize;
				auto slen = strext(header_["Content-Length"]);
				slen.clear();
				if (nSize != std::string::npos)
					slen.append_int(nSize);
				body_func_ = std::move(func);
			}

			inline const header &		get_header() const { return header_; }
			inline const std::string&	get_body() const { return body_; }
			inline bool			has_body_callback() const { return body_func_ != nullptr; }
			inline bool			is_body_chunk_mode() const { return body_func_ != nullptr && body_callback_size_ == std::string::npos; }
			inline size_t		get_body(void * p, size_t nBuf) { return body_func_(p, nBuf); }
			inline size_t		get_body_callback_size() const { return body_callback_size_; }
		protected:
			void	string_body(std::string && strBody) {
				if (strBody.empty())
					return nobody();

				body_ = std::move(strBody);
				if (method_.empty())
					method_ = "POST";

				if (header_.find("Content-Type") == header_.end())
					header_["Content-Type"] = "x-application/octet-stream";

				strext(header_["Content-Length"]).clear().append_int(strBody.size());
			}

			bool					is_https_ = false;
			std::string				server_;
			std::string				method_;
			std::string				url_;
			std::string				version_ = "1.1";
			int						port_ = 0;
			header					header_;
			std::string				body_;
			body_callback_func		body_func_;
			size_t					body_callback_size_ = 0;
		};


		class server_request;
		typedef std::shared_ptr<server_request>		server_request_ptr;
		typedef std::weak_ptr<server_request>		server_request_weak_ptr;
		class server_request {
		public:
			server_request() {}

			inline bool	is_https() const { return is_https_; }
			inline void	set_https(bool b) { is_https_ = b; }

			inline const std::string & get_remote_ip() const { return peer_ip_; }
			inline void set_remote_ip(const std::string & ip) { peer_ip_ = ip; }

			inline const std::string & get_local_ip() const { return local_ip_; }
			inline void set_local_ip(const std::string & ip) { local_ip_ = ip; }

			inline uint16_t get_local_port() const { return local_port_; }
			inline void set_local_port(uint16_t n) { local_port_ = n; }

			inline uint16_t get_remote_port() const { return remote_port_; }
			inline void set_remote_port(uint16_t n) { remote_port_ = n; }

			inline const std::string & get_url() const { return url_; }
			inline void set_url(const std::string & u) { url_ = u; }

			inline const std::string & get_method() const { return method_; }
			inline void set_method(const std::string & s) { method_ = s; }

			inline const std::string & get_version() const { return version_; }
			inline void set_version(const std::string & s) { version_ = s; }

			inline const header & get_header() const { return h_; }
			inline header & get_header_for_modify() { return h_; }

			inline const std::string & get_body() const { return body_; }
			inline std::string & get_body_for_modify() { return body_; }

			inline size_t	get_body_size() const { return body_size_; }
			inline void		set_body_size(size_t n) { body_size_ = n; }

			void	clear() {
				is_https_ = false;
				peer_ip_.clear();
				local_ip_.clear();
				local_port_ = 0;
				remote_port_ = 0;
				url_.clear();
				method_.clear();
				version_.clear();
				body_.clear();
				h_.clear();
				body_size_ = 0;
			}
		private:
			bool					is_https_ = false;
			std::string		peer_ip_, local_ip_;
			uint16_t		local_port_ = 0, remote_port_ = 0;
			std::string		url_, method_, version_, body_;
			header			h_;
			size_t			body_size_ = 0;
		};

		class request {
		public:
			static inline client_request_ptr		make() {
				return std::make_shared<client_request>(false, "127.0.0.1", 80, "/");
			}
			static inline client_request_ptr		make(bool boHttps, const std::string & strServer, int nPort, const std::string & strURL) {
				return std::make_shared<client_request>(boHttps, strServer, nPort, strURL);
			}
			static client_request_ptr		make(const std::string & strFullURL) {
				std::string		strHost, strSchema, strPath;
				ara::internal::url::split_url(strFullURL, strSchema, strHost, strPath);
				int nPort = 0;
				std::string::size_type p = strHost.find(':');
				if (p != std::string::npos) {
					nPort = strext(strHost).to_int<int>(p + 1);
					strHost = strHost.substr(0, p);
				}
				return std::make_shared<client_request>(strSchema == "https", strHost, nPort, strPath);
			}

			static inline client_request_ptr		make(const std::string & strFullURL, header && h, const std::string & strBody) {
				auto res = make(strFullURL);
				res->set_header(std::move(h)).body(strBody);
				return res;
			}
			static inline client_request_ptr		make(const std::string & strFullURL, header && h, std::string && strBody) {
				auto res = make(strFullURL);
				res->set_header(std::move(h)).body(std::move(strBody));
				return res;
			}
			static inline client_request_ptr		make(const std::string & strFullURL, header && h, client_request::body_callback_func && func) {
				auto res = make(strFullURL);
				res->set_header(std::move(h)).body(std::move(func));
				return res;
			}
		};


		class server_dispatch_pattern {
		public:
			virtual ~server_dispatch_pattern() {}
			virtual bool check_before_data(const server_request & req) = 0;
		};
		using server_dispatch_pattern_ptr = std::shared_ptr<server_dispatch_pattern>;

	}//http
}//ara

#endif//ARA_HTTPBASE_H

