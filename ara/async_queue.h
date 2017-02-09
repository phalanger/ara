
#ifndef ARA_ASYNC_QUEUE_H
#define ARA_ASYNC_QUEUE_H

#include <boost/asio.hpp>

#include "ara_def.h"
#include "dlist.h"
#include "datetime.h"
#include "log.h"

#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
#include <memory>

namespace ara {
	namespace internal {
		class asyc_token_impl_base {
		public:
			virtual ~asyc_token_impl_base() {}
		};
	}

	typedef std::shared_ptr<internal::asyc_token_impl_base> async_token;

	template<class typeKey, class typeKeyHash = std::hash<typeKey>>
	class async_queue : public std::enable_shared_from_this<async_queue<typeKey,typeKeyHash>>
	{
	public:
		typedef std::function<void(boost::system::error_code const& ec, async_token token)> funcCallback;
		typedef std::shared_ptr<async_queue> async_queue_ptr;
		typedef std::weak_ptr<async_queue> async_queue_weak_ptr;

		static async_queue_ptr make_queue(size_t nBlockSize = 0) {
			async_queue_ptr res(new async_queue);
			if (nBlockSize)
				res->set_hash_size(nBlockSize);
			return res;
		}
		~async_queue();

		void set_trace_log(bool boTrace) {
			trace_ = boTrace;
		}

		void clear();

		//It's not thread-safe. set the hash size without lock.
		void	set_hash_size(size_t nBlockSize);

		void	dump(const std::string & strPrefix, std::ostream & out);

		void	apply(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo);

	protected:
		async_queue();

		class HashNode;
		class node;
		class mission;
		typedef std::shared_ptr<node>		node_ptr;
		typedef std::shared_ptr<mission>	mission_ptr;
		typedef std::shared_ptr<HashNode>	hashnode_ptr;
		typedef std::weak_ptr<HashNode>		hashnode_weak_ptr;

		struct mission_base : dlist<mission_base> {
			mission_base() : finished_(false) {}

			inline bool cando() {
				bool boWant = false;
				return finished_.compare_exchange_strong(boWant, true);
			}
			std::atomic<bool> finished_;
		};

		class mission : public mission_base, public internal::asyc_token_impl_base
		{
		public:
			mission(boost::asio::io_service & io, funcCallback && func, std::string && strTodo, const node_ptr & pParent)
				: io_(io), func_(std::move(func)), todo_(std::move(strTodo)), parent_ptr_(pParent) {
				trace_ = pParent->is_trace_log();
			}

			~mission() {
				if (parent_ptr_) 	{
					parent_ptr_->do_finish(this);
					if (trace_)
						ara::glog(log::debug).printfln("Token Release for : %v", todo_);
				}
			}

			void cancel() {
				action(boost::asio::error::operation_aborted, nullptr);
			}

			void active(const async_token & token) {
				action(boost::system::error_code(), token);
			}

			void set_timer(const timer_val & tTimeout) {
				boost::system::error_code ec;
				timer_.reset(new boost::asio::deadline_timer(io_));
				timer_->expires_from_now(boost::posix_time::seconds(static_cast<long>(tTimeout.sec())) + boost::posix_time::microseconds(tTimeout.micro_sec()), ec);
				mission_ptr pSelf = self_ptr_;
				timer_->async_wait([this, pSelf](boost::system::error_code const & ec) {
					if (ec != boost::asio::error::operation_aborted) {
						if (mission_base::cando()) {
							funcCallback func = std::move(func_);
							func(boost::asio::error::timed_out, nullptr);
						}
					}
				}
				);
			}
			void    set_holder(const mission_ptr & token) {
				self_ptr_ = token;
			}
			mission_ptr release_holder() {
				mission_ptr res;
				res.swap(self_ptr_);
				return res;
			}
			void clear() {
				parent_ptr_ = nullptr;
				cancel();
				self_ptr_ = nullptr;
			}
			boost::asio::io_service & io() { return io_; }
			void	dump(const std::string & strPrefix, std::ostream & out) {
				std::string strExpire;
				if (timer_) {
					strExpire = " Expires at:";
					strExpire += date_time(timer_->expires_at()).format();
				}
				out << strPrefix << "[" << todo_ << "]" << strExpire << std::endl;
			}
		protected:
			inline void action(boost::system::error_code const & ec, const async_token & token) {
				if (mission_base::cando()) {
					boost::system::error_code temp;
					if (timer_)
						timer_->cancel(temp);
					if (!ec && trace_)
						ara::glog(ara::log::debug).printfln("Token get for : %v", todo_);
					func_(ec, token);
				}
			}

			boost::asio::io_service & io_;
			funcCallback			func_;
			std::unique_ptr<boost::asio::deadline_timer> timer_;
			std::string				todo_;
			mission_ptr				self_ptr_;
			node_ptr				parent_ptr_;
			bool					trace_ = false;
		};

		class node : public std::enable_shared_from_this<node>
		{
		public:
			node(const typeKey & key, hashnode_ptr pParent) : key_(key), parent_ptr_(pParent) {
				trace_ = pParent->is_trace_log();
				root_.as_root();
			}
			~node() {
				clear();
			}

			inline bool	is_trace_log() const {
				return trace_;
			}

			void doApply(boost::asio::io_service & io, const timer_val & tTimeout, funcCallback && func, std::string && strTodo) {

				mission_ptr pMission = std::make_shared<mission>(io, std::move(func), std::move(strTodo), this->shared_from_this());

				std::unique_lock<std::mutex> _guard(lock_);

				if (root_.root_empty()) { //first time
					pMission->append_before(root_);
					_guard.unlock();
					io.dispatch([pMission]() { pMission->active(pMission); });
				}
				else {
					pMission->append_before(root_);
					pMission->set_holder(pMission);

					if (tTimeout != timer_val::max_time)
						pMission->set_timer(tTimeout);
				}
			}
			void do_finish(mission * pMission) {
				std::unique_lock<std::mutex> _guard(lock_);
				pMission->unlink();

				if (root_.root_empty()) {
					hashnode_ptr pParent = parent_ptr_.lock();
					_guard.unlock();
					if (pParent)
						pParent->destroy(key_);
				}
				else {
					mission * mis = static_cast<mission *>(root_.get_next());
					mission_ptr pMission = mis->release_holder();
					_guard.unlock();
					pMission->io().post([pMission]() { pMission->active(pMission); });
				}
			}

			bool empty() {
				std::lock_guard<std::mutex> _guard(lock_);
				return root_.root_empty();
			}

			void clear() {
				std::lock_guard<std::mutex> _guard(lock_);

				mission_base * pBegin = root_.get_next();
				mission_base * pEnd = &root_;
				while (pBegin != pEnd) {
					mission * pCurMission = static_cast<mission *>(pBegin);
					pBegin = pBegin->get_next();

					pCurMission->unlink();
					pCurMission->clear();
				}
			}

			void	dump(const std::string & strPrefix, std::ostream & out) {
				std::lock_guard<std::mutex> _guard(lock_);
				mission_base * pBegin = root_.get_next();
				mission_base * pEnd = &root_;
				while (pBegin != pEnd) {
					mission * pCurMission = static_cast<mission *>(pBegin);
					pCurMission->dump(strPrefix + "  ", out);
					pBegin = pBegin->get_next();
				}
			}
		protected:
			std::mutex			lock_;
			typeKey				key_;
			hashnode_weak_ptr	parent_ptr_;
			mission_base		root_;
			bool				trace_ = false;
		};

		typedef std::map<typeKey, std::shared_ptr<node>> typeNodeMap;

		class HashNode : public std::enable_shared_from_this<HashNode>
		{
		public:
			HashNode(bool boTrace) : trace_(boTrace) {}

			~HashNode() {
				clear();
			}

			inline bool	is_trace_log() const {
				return trace_;
			}

			std::mutex lock_;
			std::map<typeKey, std::shared_ptr<node>> map_data_;
			bool		trace_ = false;

			std::shared_ptr<node> get(const typeKey & key) {
				std::shared_ptr<node> pNode;

				std::unique_lock<std::mutex> _guard(lock_);
				typename typeNodeMap::iterator it = map_data_.find(key);
				if (it == map_data_.end()) {
					pNode = std::make_shared<node>(key, this->shared_from_this());
					map_data_.insert(std::make_pair(key, pNode));
				}
				else
					pNode = it->second;
				return pNode;
			}

			void destroy(const typeKey & key) {
				std::unique_lock<std::mutex> _guard(lock_);
				typename typeNodeMap::iterator it = map_data_.find(key);
				if (it != map_data_.end() && it->second->empty())
					map_data_.erase(it);
			}

			void clear() {
				std::unique_lock<std::mutex> _guard(lock_);
				auto it = map_data_.begin();
				auto itEnd = map_data_.end();
				for (; it != itEnd; ++it)
					it->second->clear();
				map_data_.clear();
			}

			bool	empty() {
				std::unique_lock<std::mutex> _guard(lock_);
				return map_data_.empty();
			}
			void	dump(const std::string & strPrefix, std::ostream & out) {
				std::unique_lock<std::mutex> _guard(lock_);
				auto it = map_data_.begin();
				auto itEnd = map_data_.end();
				for (; it != itEnd; ++it)
				{
					out << strPrefix << "Key:" << it->first << std::endl;
					it->second->dump(strPrefix + "  ", out);
				}
			}
		};

		std::vector<std::shared_ptr<HashNode>> hash_ary_;
		typeKeyHash hash_func_;
		bool trace_ = false;
	};


	//////////////////////////////////////////////////////////////////////////

	template<class typeKey, class typeKeyHash>
	async_queue<typeKey, typeKeyHash>::async_queue()
	{}

	template<class typeKey, class typeKeyHash>
	async_queue<typeKey, typeKeyHash>::~async_queue()
	{
		clear();
	}

	template<class typeKey, class typeKeyHash>
	void async_queue<typeKey, typeKeyHash>::clear()
	{
		for (std::shared_ptr<HashNode> & pNode : hash_ary_) {
			if (pNode)
				pNode->clear();
		}
		hash_ary_.clear();
	}

	template<class typeKey, class typeKeyHash>
	void async_queue<typeKey, typeKeyHash>::set_hash_size(size_t nBlockSize)
	{
		if (!hash_ary_.empty())
			hash_ary_.clear();

		hash_ary_.resize(nBlockSize);
	}

	template<class typeKey, class typeKeyHash>
	void async_queue<typeKey, typeKeyHash>::apply(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo)
	{
		size_t nHash = hash_func_(key);
		std::shared_ptr<node> pNode;

		if (hash_ary_.empty()) {
			func(boost::asio::error::invalid_argument, nullptr);
			return;
		}
		else {
			size_t nIndex = nHash % hash_ary_.size();
			if (!hash_ary_[nIndex])
				hash_ary_[nIndex] = std::make_shared<HashNode>(trace_);

			pNode = hash_ary_[nIndex]->get(key);
		}

		if (pNode)
			pNode->doApply(io, tTimeout, std::move(func), std::move(strTodo));
	}

	template<class typeKey, class typeKeyHash>
	void async_queue<typeKey, typeKeyHash>::dump(const std::string & strPrefix, std::ostream & out)
	{
		size_t	nIndex = 0;
		for (std::shared_ptr<HashNode> & pNode : hash_ary_) {
			if (pNode && !pNode->empty()) {
				out << strPrefix << "[" << nIndex << "] :" << std::endl;
				pNode->dump(strPrefix + "  ", out);
			}
			++nIndex;
		}
	}
}//ara

#endif//ARA_ASYNC_QUEUE_H
