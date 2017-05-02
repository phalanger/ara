
#ifndef ARA_ASYNC_RWQUEUE_H
#define ARA_ASYNC_RWQUEUE_H

#include <boost/asio.hpp>

#include "async_queue.h"
#include <memory>

namespace ara {

	template<class typeKey, class typeKeyHash = std::hash<typeKey>>
	class async_rwqueue : public std::enable_shared_from_this<async_rwqueue<typeKey, typeKeyHash>>
	{
	public:
		typedef std::shared_ptr<async_rwqueue> async_rwqueue_ptr;
		typedef std::weak_ptr<async_rwqueue> async_rwqueue_weak_ptr;
		typedef std::function<void(boost::system::error_code const& ec, async_token token)> funcCallback;

		static async_rwqueue_ptr make_rwqueue(size_t nBlockSize = 0) {
			async_rwqueue_ptr res(new async_rwqueue);
			if (nBlockSize)
				res->set_hash_size(nBlockSize);
			return res;
		}
		~async_rwqueue();

		void set_trace_log(bool boTrace) {
			trace_ = boTrace;
		}

		void clear();

		//It's not thread-safe. set the hash size without lock.
		void set_hash_size(size_t nBlockSize);

		void	dump(const std::string & strPrefix, std::ostream & out);

		void apply_read(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo);
		void apply_write(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo);

		async_result<boost::system::error_code, async_token> apply_read(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, std::string && strTodo) {
			async_result<boost::system::error_code, async_token> res;
			apply_read(io, key, tTimeout, [res](boost::system::error_code const& ec, async_token token) {
				res.set(ec, token);
			}, std::move(strTodo));
			return res;
		}
		async_result<boost::system::error_code, async_token> apply_write(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, std::string && strTodo) {
			async_result<boost::system::error_code, async_token> res;
			apply_write(io, key, tTimeout, [res](boost::system::error_code const& ec, async_token token) {
				res.set(ec, token);
			}, std::move(strTodo));
			return res;
		}

	protected:
		async_rwqueue();
		class HashNode;
		class node;
		class mission;
		typedef std::shared_ptr<node> node_ptr;
		typedef std::shared_ptr<mission> mission_ptr;
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
			mission(bool boIsRead, boost::asio::io_service & io, funcCallback && func, std::string && strTodo, const node_ptr & pParent)
				: io_(io), func_(std::move(func)), todo_(std::move(strTodo)), parent_ptr_(pParent), is_read_(boIsRead) {
				trace_ = pParent->is_trace_log();
			}

			~mission() {
				if (parent_ptr_) {
					parent_ptr_->doFinish(this, is_read_);
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
							if (func)
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
					if (func_) {
						func_(ec, token);
						func_ = nullptr;
					}
				}
			}

			boost::asio::io_service & io_;
			funcCallback			func_;
			std::unique_ptr<boost::asio::deadline_timer> timer_;
			std::string				todo_;
			mission_ptr				self_ptr_;
			node_ptr				parent_ptr_;
			bool					is_read_ = false;
			bool					trace_ = false;
		};

		class node : public std::enable_shared_from_this<node>
		{
		public:
			node(const typeKey & key, hashnode_ptr pParent) : key_(key), parent_ptr_(pParent) {
				trace_ = pParent->is_trace_log();
				root_read_running_.as_root();
				root_read_waitting_.as_root();
				root_write_running_.as_root();
				root_write_waitting_.as_root();
			}
			~node() {
				clear();
			}

			inline bool	is_trace_log() const {
				return trace_;
			}

			void doApplyRead(boost::asio::io_service & io, const timer_val & tTimeout, funcCallback && func, std::string && strTodo) {

				mission_ptr pMission = std::make_shared<mission>(true, io, std::move(func), std::move(strTodo), this->shared_from_this());

				std::unique_lock<std::mutex> _guard(lock_);

				if (root_write_running_.root_empty() && root_write_waitting_.root_empty()) { //no writer
					pMission->append_before(root_read_running_);
					_guard.unlock();
					io.dispatch([pMission]() { pMission->active(pMission); });
				}
				else {	//someone in writing.
					pMission->append_before(root_read_waitting_);
					pMission->set_holder(pMission);

					if (tTimeout != timer_val::max_time)
						pMission->set_timer(tTimeout);
				}
			}
			void doApplyWrite(boost::asio::io_service & io, const timer_val & tTimeout,
				funcCallback && func, std::string && strTodo)
			{

				mission_ptr pMission =
					std::make_shared<mission>(false, io, std::move(func), std::move(strTodo), this->shared_from_this());

				std::unique_lock<std::mutex> _guard(lock_);

				if (root_read_running_.root_empty() && root_write_running_.root_empty()
					&& root_write_waitting_.root_empty()) { //first time
					pMission->append_before(root_write_running_);
					_guard.unlock();
					io.dispatch([pMission]() { pMission->active(pMission); });
				}
				else {
					pMission->append_before(root_write_waitting_);
					pMission->set_holder(pMission);

					if (tTimeout != timer_val::max_time)
						pMission->set_timer(tTimeout);
				}
			}

			void doFinish(mission * pMission, bool boIsRead) {
				std::unique_lock<std::mutex> _guard(lock_);
				pMission->unlink();

				if (boIsRead)
				{
					if (!root_write_running_.root_empty())	//Should not be here
						return;
					else if (root_write_waitting_.root_empty())	//no one want to write, continue reading
					{
						if (!root_read_waitting_.root_empty())
						{
							popupAll(root_read_waitting_, root_read_running_, _guard);
							return;
						}
					}
					else if (root_read_running_.root_empty())	//someone want to write, and no reading now.
					{
						popup(root_write_waitting_, root_write_running_, _guard);
						return;
					}
				}
				else
				{
					if (!root_write_running_.root_empty())	//Should not be here
						return;
					else if (!root_write_waitting_.root_empty())	//someone want to write, continue reading
					{
						if (!root_read_running_.root_empty())	//Should not be here
							return;
						popup(root_write_waitting_, root_write_running_, _guard);
						return;
					}
					else
					{
						if (!root_read_waitting_.root_empty())
						{
							popupAll(root_read_waitting_, root_read_running_, _guard);
							return;
						}
					}
				}

				if (root_write_running_.root_empty() && root_write_waitting_.root_empty()
					&& root_read_running_.root_empty() && root_read_waitting_.root_empty())
				{
					hashnode_ptr pParent = parent_ptr_.lock();
					_guard.unlock();
					if (pParent)
						pParent->destroy(key_);
				}
			}

			bool empty() {
				std::lock_guard<std::mutex> _guard(lock_);
				return root_write_running_.root_empty() && root_write_waitting_.root_empty()
					&& root_read_running_.root_empty() && root_read_waitting_.root_empty();
			}

			void clear() {
				std::lock_guard<std::mutex> _guard(lock_);
				clearImp(root_write_waitting_);
				clearImp(root_write_running_);
				clearImp(root_read_waitting_);
				clearImp(root_read_running_);
			}

			void	dump(const std::string & strPrefix, std::ostream & out) {
				std::lock_guard<std::mutex> _guard(lock_);
				dump(root_write_running_, strPrefix + "  WriteRun:", out);
				dump(root_write_waitting_, strPrefix + "  WriteWait:", out);
				dump(root_read_running_, strPrefix + "  ReadRun:", out);
				dump(root_read_waitting_, strPrefix + "  ReadWwait:", out);
			}
		protected:
			static void dump(mission_base & root, const std::string & strPrefix, std::ostream & out) {
				mission_base * pBegin = root.get_next();
				mission_base * pEnd = &root;
				while (pBegin != pEnd) {
					mission * pCurMission = static_cast<mission *>(pBegin);
					pCurMission->dump(strPrefix, out);
					pBegin = pBegin->get_next();
				}
			}
			static void	popup(mission_base & rootWaiting, mission_base & rootRunning, std::unique_lock<std::mutex> & lock) {
				mission * mis = static_cast<mission *>(rootWaiting.get_next());
				mis->unlink();
				mis->append_before(rootRunning);
				mission_ptr pMission = mis->release_holder();
				lock.unlock();
				pMission->io().post([pMission]() { pMission->active(pMission); });
			}
			static void	popupAll(mission_base & rootWaiting, mission_base & rootRunning, std::unique_lock<std::mutex> & lock) {
				std::list<mission_ptr>	listToRun;
				while (!rootWaiting.root_empty())
				{
					mission * mis = static_cast<mission *>(rootWaiting.get_next());
					mis->unlink();
					mis->append_before(rootRunning);
					mission_ptr pMission = mis->release_holder();
					listToRun.push_back(pMission);
				}
				lock.unlock();
				for (mission_ptr & p : listToRun)
					p->io().post([p]() { p->active(p); });
			}
			static void clearImp(mission_base & root) {
				mission_base * pBegin = root.get_next();
				mission_base * pEnd = &root;
				while (pBegin != pEnd) {
					mission * pCurMission = static_cast<mission *>(pBegin);
					pBegin = pBegin->get_next();

					pCurMission->unlink();
					pCurMission->clear();
				}
			}

			std::mutex lock_;
			typeKey key_;
			hashnode_weak_ptr parent_ptr_;
			mission_base	root_read_running_;
			mission_base	root_read_waitting_;
			mission_base	root_write_running_;
			mission_base	root_write_waitting_;
			bool			trace_ = false;
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
			std::map<typeKey, std::shared_ptr<node>> m_mapData;
			bool		trace_ = false;

			std::shared_ptr<node> get(const typeKey & key) {
				std::shared_ptr<node> pNode;

				std::unique_lock<std::mutex> _guard(lock_);
				typename typeNodeMap::iterator it = m_mapData.find(key);
				if (it == m_mapData.end()) {
					pNode = std::make_shared<node>(key, this->shared_from_this());
					m_mapData.insert(std::make_pair(key, pNode));
				}
				else
					pNode = it->second;
				return pNode;
			}

			void destroy(const typeKey & key) {
				std::unique_lock<std::mutex> _guard(lock_);
				typename typeNodeMap::iterator it = m_mapData.find(key);
				if (it != m_mapData.end() && it->second->empty())
					m_mapData.erase(it);
			}

			void clear() {
				std::unique_lock<std::mutex> _guard(lock_);
				auto it = m_mapData.begin();
				auto itEnd = m_mapData.end();
				for (; it != itEnd; ++it)
					it->second->clear();
				m_mapData.clear();
			}

			bool	empty() {
				std::unique_lock<std::mutex> _guard(lock_);
				return m_mapData.empty();
			}
			void	dump(const std::string & strPrefix, std::ostream & out) {
				std::unique_lock<std::mutex> _guard(lock_);
				auto it = m_mapData.begin();
				auto itEnd = m_mapData.end();
				for (; it != itEnd; ++it)
				{
					out << strPrefix << "Key:" << it->first << std::endl;
					it->second->dump(strPrefix + "  ", out);
				}
			}
		};

		std::shared_ptr<node>	findNode(const typeKey & key) {
			{
				size_t nHash = hash_func_(key);
				std::shared_ptr<node> pNode;

				if (hash_ary_.empty()) {
					return pNode;
				}
				else {
					size_t nIndex = nHash % hash_ary_.size();
					if ( UNLIKELY(!hash_ary_[nIndex]) ) {
						std::lock_guard<std::mutex>		_guard(lock_);
						if (!hash_ary_[nIndex]) {
							auto p = std::make_shared<HashNode>(trace_);
							hash_ary_[nIndex] = p;
						}
					}

					pNode = hash_ary_[nIndex]->get(key);
				}

				return pNode;
			}
		}

		std::mutex	lock_;
		std::vector<std::shared_ptr<HashNode>> hash_ary_;
		typeKeyHash hash_func_;
		bool trace_ = false;
	};


	//////////////////////////////////////////////////////////////////////////

	template<class typeKey, class typeKeyHash>
	async_rwqueue<typeKey, typeKeyHash>::async_rwqueue()
	{}

	template<class typeKey, class typeKeyHash>
	async_rwqueue<typeKey, typeKeyHash>::~async_rwqueue()
	{
		clear();
	}

	template<class typeKey, class typeKeyHash>
	void async_rwqueue<typeKey, typeKeyHash>::clear()
	{
		for (std::shared_ptr<HashNode> & pNode : hash_ary_) {
			if (pNode)
				pNode->clear();
		}
		hash_ary_.clear();
	}

	template<class typeKey, class typeKeyHash>
	void async_rwqueue<typeKey, typeKeyHash>::set_hash_size(size_t nBlockSize)
	{
		if (!hash_ary_.empty())
			hash_ary_.clear();

		hash_ary_.resize(nBlockSize);
	}

	template<class typeKey, class typeKeyHash>
	void async_rwqueue<typeKey, typeKeyHash>::apply_read(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo)
	{
		std::shared_ptr<node> pNode = findNode(key);
		if (pNode)
			pNode->doApplyRead(io, tTimeout, std::move(func), std::move(strTodo));
		else
			func(boost::asio::error::invalid_argument, nullptr);
	}

	template<class typeKey, class typeKeyHash>
	void async_rwqueue<typeKey, typeKeyHash>::apply_write(boost::asio::io_service & io,
		const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo)
	{
		std::shared_ptr<node> pNode = findNode(key);
		if (pNode)
			pNode->doApplyWrite(io, tTimeout, std::move(func), std::move(strTodo));
		else
			func(boost::asio::error::invalid_argument, nullptr);
	}

	template<class typeKey, class typeKeyHash>
	void async_rwqueue<typeKey, typeKeyHash>::dump(const std::string & strPrefix, std::ostream & out)
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
}

#endif//ARA_ASYNC_RWQUEUE_H