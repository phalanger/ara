
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

		class options
		{
		public:
			options() {}

			void				set_timeout(const timer_val & v) { time_out_ = v; }
			const timer_val &	get_timeout() const { return time_out_; }

			void		set_verify_peer(bool b) { verify_peer_ = b; }
			bool		get_verify_peer() const { return verify_peer_; }

			void		set_cache_size(size_t n) { cache_size_ = n; }
			size_t		get_cache_size() const { return cache_size_; }
		protected:
			timer_val	time_out_ = timer_val(10);
			bool		verify_peer_ = false;
			size_t		cache_size_ = 1024 * 1024;
		};

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

		class request;
		typedef std::shared_ptr<request>		request_ptr;
		typedef std::weak_ptr<request>			request_weak_ptr;
		class request
		{
		public:
			typedef std::function<size_t(void * p, size_t nBuf)>	body_callback_func;

			static request_ptr		make(bool boHttps, const std::string & strServer, int nPort, const std::string & strURL) {
				return std::make_shared<request>(boHttps, strServer, nPort, strURL);
			}
			static request_ptr		make(const std::string & strFullURL) {
				std::string		strHost, strSchema, strPath;
				ara::internal::url::split_url(strFullURL, strSchema, strHost, strPath);
				int nPort = 0;
				std::string::size_type p = strHost.find(':');
				if (p != std::string::npos) {
					nPort = strext(strHost).to_int<int>(p + 1);
					strHost = strHost.substr(0, p);
				}
				return std::make_shared<request>(strSchema == "https", strHost, nPort, strPath);
			}

			static request_ptr		make(const std::string & strFullURL, header && h, const std::string & strBody) {
				auto res = make(strFullURL);
				res->set_header(std::move(h)).body(strBody);
				return res;
			}
			static request_ptr		make(const std::string & strFullURL, header && h, std::string && strBody) {
				auto res = make(strFullURL);
				res->set_header(std::move(h)).body(std::move(strBody));
				return res;
			}
			static request_ptr		make(const std::string & strFullURL, header && h, body_callback_func && func) {
				auto res = make(strFullURL);
				res->set_header(std::move(h)).body(std::move(func));
				return res;
			}

			request(bool boHttps, const std::string & strServer, int nPort, const std::string & strURL) : is_https_(boHttps), server_(strServer), url_(strURL) {
				if (nPort == 0)
					port_ = boHttps ? 443 : 80;
				else
					port_ = nPort;
			}

			request &	set_method(const std::string & strMethod) {
				method_ = strMethod;
				return *this;
			}

			request &		set_header(header && h) {
				header_ = std::move(h);
				return *this;
			}

			request &		add_header(header && h) {
				for (auto & it : h) {
					header_.insert(std::move(it));
				}
				return *this;
			}
			request &		add_header(const header & h) {
				for (auto & it : h) {
					header_.insert( it );
				}
				return *this;
			}

			request &		set_header(const std::string & strHeaderKey, const std::string & strHeaderItem) {
				header_[strHeaderKey] = strHeaderItem;
				return *this;
			}
			request &		set_header(const std::string & strHeaderKey, std::string && strHeaderItem) {
				header_[strHeaderKey] = std::move(strHeaderItem);
				return *this;
			}
			request &		set_http_version(const std::string & strVer) {
				version_ = strVer;
				return *this;
			}

			inline bool is_https() const { return is_https_; }
			inline const std::string & get_server() const { return server_; }
			inline const std::string & get_method() const { return method_; }
			inline const std::string & get_url() const { return url_; }
			inline int				get_port() const { return port_; }
			inline const std::string & get_http_version() const { return version_; }

			void		nobody() {}
			void		body(const std::string & strBody) {
				string_body(std::string(strBody));
			}
			void		body(std::string && strBody) {
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
	}//http
}//ara

#endif//ARA_HTTPBASE_H

