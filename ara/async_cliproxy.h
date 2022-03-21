#ifndef ARA_ASYNC_CLIENTPROXY_H
#define ARA_ASYNC_CLIENTPROXY_H

#include <string>
#include <memory>
#include <functional>

#include "internal/url.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/array.hpp>

namespace ara {

	class client_proxy : public std::enable_shared_from_this<client_proxy>
	{
	public:
		enum TYPE {
			PROXY_NONE,
			PROXY_HTTP,
			PROXY_SOCK4,
			PROXY_SOCK5,
		};
		typedef std::shared_ptr<client_proxy> client_proxy_ptr;
		typedef std::weak_ptr<client_proxy> client_proxy_weak_ptr;

		static client_proxy_ptr make() {
			client_proxy_ptr res(new client_proxy);
			return res;
		}

		void	init_auth(const std::string& strUser, const std::string& strPass) {
			user_ = strUser;
			pass_ = strPass;
		}
		void	init_http_proxy(const std::string& strHost, uint16_t nPort) {
			type_ = PROXY_HTTP;
			host_ = strHost;
			port_ = nPort;
		}
		void	init_sock4_proxy(const std::string& strHost, uint16_t nPort) {
			type_ = PROXY_SOCK4;
			host_ = strHost;
			port_ = nPort;
		}
		void	init_sock5_proxy(const std::string& strHost, uint16_t nPort) {
			type_ = PROXY_SOCK5;
			host_ = strHost;
			port_ = nPort;
		}

		TYPE	get_type() const { return type_; }
		const std::string& get_proxy_host() const { return host_; }
		uint16_t	get_proxt__port() const { return port_; }
		const std::string& get_auth_user() const { return user_; }
		const std::string& get_auth_pass() const { return pass_; }

		void	async_connect(
			boost::asio::io_service& io,
			boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket,
			const std::string& strHost, int16_t nPort, std::function<void(const boost::system::error_code& err)>&& func
		)
		{
			std::shared_ptr<context> pContext = std::make_shared<context>(io, socket, strHost, nPort, std::move(func), shared_from_this());

		}
	protected:
		class context {
		public:
			context(boost::asio::io_service& io,
				boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket,
				const std::string& strHost, int16_t nPort,
				std::function<void(const boost::system::error_code& err)>&& func,
				client_proxy_ptr proxy_setting
			)
				: io_(io), socket_(socket), resolver_(io_), func_(std::move(func)), target_url_(strHost)
				, target_port_(nPort), proxy_(proxy_setting)
			{}

		protected:
			boost::asio::io_service& io_;
			boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket_;
			boost::asio::ip::tcp::resolver			resolver_;
			std::function<void(const boost::system::error_code& err)> func_;
			std::string		target_url_;
			int64_t			target_port_;
			client_proxy_ptr proxy_;
		};

		client_proxy() {}
		TYPE			type_ = PROXY_NONE;
		std::string		host_;
		uint16_t		port_ = 0;
		std::string		user_, pass_;
	};
};

#endif
