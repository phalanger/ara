
#ifndef ARA_ASYNC_SESSION_H
#define ARA_ASYNC_SESSION_H

#include <boost/asio.hpp>

#include "async_rwqueue.h"
#include "session.h"

/*
Usage:

		using	AsyncSession = async_session<std::string, MyData>;
		auto 	asyncSession = asyncSession::make_session(1000, 101);
		async_thread_pool	pool("For test");
		pool.init(10);

		asyncSession->async_read( pool.io(), "cyt", timer_val(10), [](async_err const& ec, const AsyncSession::async_read_data & data){
			if (ec)
				std::cout << "Error" << std::endl;
			else if (data.empty())
				std::cout << "User not found" << std::endl;
			else
				std::cout << data->getName() << std::endl;
		}, "read user data");

		asyncSession->async_write_or_create( pool.io(), "cyt", timer_val(10), [](async_err const& ec, const AsyncSession::async_write_data & data){
			if (ec)
				std::cout << "Error" << std::endl;
			else
				data->setName("Hello");
		}, "write user data");

		asyncSession->async_remove( pool.io(), "cyt", timer_val(10), [](async_err const& ec, const AsyncSession::async_write_data & data){
			if (ec)
				std::cout << "Error" << std::endl;
			else
				std::cout << data->getName() << "Will be delete" << std::endl;
		}, "write user data");

		/////////////////////////////////////////////////////////////////////////////////

		const std::string strKey = "cyt1";
		asyncSession->apply_read( pool.io(), strKey, timer_val(10), [asyncSession,strKey](async_err const &ec, async_token token){

			if (ec)
				return;

			AsyncSession::const_data_ptr pToRead = asyncSession->get_read(strKey);
			if ( pToRead == nullptr ) //not found
			{
				asyncSession->apply_write( pool.io(), strKey, timer_val(10), [asyncSession,strKey](async_err const &ec, async_token token){

					if (ec)
						return;

					AsyncSession::data_ptr pToWrite = asyncSession->create(strKey, MyData());
					pToWrite->setName("xxxxxx");

				}, "To create data");
			}
			else
				std::cout << "Got data" << std::endl;

		}, "Test1");

*/

namespace ara {

	template<typename Key, typename Data
		, class typeKeyHash = std::hash<Key>
		, class Pre = std::less<Key>
	>
		class async_session : public session<Key, Data, typeKeyHash, Pre>, public async_rwqueue<Key, typeKeyHash>
	{
	public:
		typedef	Key		key_type;
		typedef Data	data_type;

		typedef	session<Key, Data, typeKeyHash, Pre>		typeSession;
		typedef async_rwqueue<Key, typeKeyHash>				typeQueue;
		typedef typename typeSession::data_ptr 				data_ptr;
		typedef typename typeSession::const_data_ptr		const_data_ptr;

		static std::shared_ptr<async_session> make_session(size_t nSessionBlockSize, size_t nQueueBlockSize) {
			std::shared_ptr<async_session> res(new async_session);
			if (nSessionBlockSize)
				res->resize(nSessionBlockSize);
			if (nQueueBlockSize)
				res->set_hash_size(nQueueBlockSize);
			return res;
		}

		class async_read_data
		{
		public:
			async_read_data(const async_token & t, const const_data_ptr & d) : token_(t), data_(d) {}
			async_read_data(const async_read_data & r) : token_(r.token_), data_(r.data_) {}
			async_read_data() {}

			inline bool	empty() const { return data_.get() == nullptr; }
			inline const Data * get() const { return data_.get(); }
			inline const Data * operator->() const { return data_.get(); }

			void release() {
				token_ = nullptr;
				data_ = nullptr;
			}
		protected:
			async_token		token_;
			const_data_ptr	data_;
		};
		class async_write_data
		{
		public:
			async_write_data(const async_token & t, const data_ptr & d) : token_(t), data_(d) {}
			async_write_data(const async_write_data & w) : token_(w.token_), data_(w.data_) {}
			async_write_data() {}

			inline bool	empty() const { return data_.get() == nullptr; }
			inline Data * get() const { return data_.get(); }
			inline Data * operator->() const { return data_.get(); }

			void release() {
				token_ = nullptr;
				data_ = nullptr;
			}
		protected:
			async_token		token_;
			data_ptr		data_;
		};

		typedef std::function<void(boost::system::error_code const& ec, const async_read_data & data)>	funcRead;
		typedef std::function<void(boost::system::error_code const& ec, const async_write_data & data)>	funcWrite;

		void	async_read(boost::asio::io_service & io
							, const Key & key
							, const timer_val & tTimeout
							, funcRead && func
							, std::string && strTodo
							, int nFlags = ses_clear_expire | ses_update_access) {
			typeQueue::apply_read(io, key, tTimeout, [func = std::move(func), key, nFlags, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					func(ec, async_read_data(token, nullptr));
				else
				{
					const_data_ptr	pResult = typeSession::get_read(key, nFlags);
					func(ec, async_read_data(token, pResult));
				}
			}, std::move(strTodo));
		}
		async_result<boost::system::error_code, async_read_data>	async_read(boost::asio::io_service & io
			, const Key & key
			, const timer_val & tTimeout
			, std::string && strTodo
			, int nFlags = ses_clear_expire | ses_update_access) {

			async_result<boost::system::error_code, async_read_data> result;
			typeQueue::apply_read(io, key, tTimeout, [result, key, nFlags, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					result.set(ec, async_read_data(token, nullptr));
				else
				{
					const_data_ptr	pResult = typeSession::get_read(key, nFlags);
					result.set(ec, async_read_data(token, pResult));
				}
			}, std::move(strTodo));
			return result;
		}


		void	async_write(boost::asio::io_service & io
							, const Key & key
							, const timer_val & tTimeout
							, funcWrite && func
							, std::string && strTodo
							, int nFlags = ses_clear_expire | ses_update_access | ses_update_modify
							) {
			typeQueue::apply_write(io, key, tTimeout, [func = std::move(func), key, nFlags, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					func(ec, async_write_data(token, nullptr));
				else
				{
					data_ptr	pResult = typeSession::get_write(key, nFlags);
					func(ec, async_write_data(token, pResult));
				}
			}, std::move(strTodo));
		}
		async_result<boost::system::error_code, async_write_data> async_write(boost::asio::io_service & io
			, const Key & key
			, const timer_val & tTimeout
			, std::string && strTodo
			, int nFlags = ses_clear_expire | ses_update_access | ses_update_modify) {

			async_result<boost::system::error_code, async_write_data>	result;
			typeQueue::apply_write(io, key, tTimeout, [result, key, nFlags, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					result.set(ec, async_write_data(token, nullptr));
				else
				{
					data_ptr	pResult = typeSession::get_write(key, nFlags);
					result.set(ec, async_write_data(token, pResult));
				}
			}, std::move(strTodo));
			return result;
		}

		void	async_write_or_create(boost::asio::io_service & io
							, const Key & key
							, const timer_val & tTimeout
							, funcWrite && func
							, std::string && strTodo
							, int nFlags = ses_clear_expire | ses_update_access | ses_update_modify
							, size_t nExpireSetting = 0) {
			typeQueue::apply_write(io, key, tTimeout, [func = std::move(func), key, nFlags, nExpireSetting, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					func(ec, async_write_data(token, nullptr));
				else
				{
					data_ptr	pResult = typeSession::get_write_or_create(key, nFlags, nExpireSetting);
					func(ec, async_write_data(token, pResult));
				}
			}, std::move(strTodo));
		}
		async_result<boost::system::error_code, async_write_data>	async_write_or_create(boost::asio::io_service & io
							, const Key & key
							, const timer_val & tTimeout
							, std::string && strTodo
							, int nFlags = ses_clear_expire | ses_update_access | ses_update_modify
							, size_t nExpireSetting = 0) {
			async_result<boost::system::error_code, async_write_data>	result;
			typeQueue::apply_write(io, key, tTimeout, [result, key, nFlags, nExpireSetting, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					result.set(ec, async_write_data(token, nullptr));
				else
				{
					data_ptr	pResult = typeSession::get_write_or_create(key, nFlags, nExpireSetting);
					result.set(ec, async_write_data(token, pResult));
				}
			}, std::move(strTodo));
			return result;
		}

		void	async_remove(boost::asio::io_service & io
							, const Key & key
							, const timer_val & tTimeout
							, funcWrite && func
							, std::string && strTodo
							, int nFlags = ses_clear_expire) {
			typeQueue::apply_write(io, key, tTimeout, [func = std::move(func), key, nFlags, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					func(ec, async_write_data(token, nullptr));
				else
				{
					data_ptr	pResult = typeSession::get_write(key, nFlags);
					if (!pResult)
						func(ec, async_write_data(token, pResult));
					else
					{
						typeSession::remove(key);
						func(ec, async_write_data(token, pResult));
					}
				}
			}, std::move(strTodo));
		}
		async_result<boost::system::error_code, async_write_data>	async_remove(boost::asio::io_service & io
			, const Key & key
			, const timer_val & tTimeout
			, std::string && strTodo
			, int nFlags = ses_clear_expire) {
			async_result<boost::system::error_code, async_write_data> result;
			typeQueue::apply_write(io, key, tTimeout, [result, key, nFlags, this, pSelf = typeQueue::shared_from_this()](boost::system::error_code const& ec, async_token token){
				if (ec)
					func(ec, async_write_data(token, nullptr));
				else
				{
					data_ptr	pResult = typeSession::get_write(key, nFlags);
					if (!pResult)
						result.set(ec, async_write_data(token, pResult));
					else
					{
						typeSession::remove(key);
						result.set(ec, async_write_data(token, pResult));
					}
				}
			}, std::move(strTodo));
			return result;
		}

	protected:
		async_session() : typeSession(0) {}
	};

}

#endif//ARA_ASYNC_SESSION_H

