
#ifndef ARA_ASYNC_RESPOOL_H
#define ARA_ASYNC_RESPOOL_H

#include "async_queue.h"
#include "threadext.h"

#if 0
	using ara::async_resourcepool<std::string>		arespool;

	auto pool = arespool::make_pool(100);
	pool->add("res1", 0);
	pool->add("res1", 1);
	pool->add("res1", 2);
	pool->add("res2", 3);
	pool->add("res2", 4);
	pool->add("res2", 5);

	pool->apply(io, "res1", ara::timer_val(10), [](const boost::system::error_code & ec, arespool::async_res_token token) {
		if (ec == boost::asio::error::timed_out) {
			//while time out
		} 
		else if (ec == boost::asio::error::interrupted) {
			//while application shutdown
		}
		else if (ec == boost::asio::error::invalid_argument) {
			//while resource key is invalid (no resource)
		}
		else if (!ec) {
			//While OK
		}
		else {
			//Other error, but why???
		}
	}, "Todo ...");
#endif

namespace ara {
	namespace internal {
		template<class typeRes = size_t>
		class  async_respool_token_base: public asyc_token_impl_base {
		public:
			virtual ~async_respool_token_base() {}
			virtual const typeRes	&	get() const = 0;
		};
	}

	template<class typeKey, class typeRes = size_t, class typeKeyHash = std::hash<typeKey>>
	class async_resourcepool : public std::enable_shared_from_this<async_resourcepool<typeKey, typeRes, typeKeyHash>>
	{
	public:
		typedef std::shared_ptr<async_resourcepool> async_resourcepool_ptr;
		typedef std::weak_ptr<async_resourcepool> async_resourcepool_weak_ptr;
		typedef std::shared_ptr<internal::async_respool_token_base<typeRes>>	async_res_token;
		typedef std::function<void(boost::system::error_code const& ec, async_res_token token)> funcCallback;

		static async_resourcepool_ptr make_pool(size_t nBlockSize = 0) {
			async_resourcepool_ptr res(new async_resourcepool);
			if (nBlockSize)
				res->set_hash_size(nBlockSize);
			return res;
		}

		~async_resourcepool();

		void		add(const typeKey & k, const typeRes & res);
		void		add(const typeKey & k, typeRes && res);

		void apply(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo);

		void set_trace_log(bool boTrace) {
			trace_ = boTrace;
		}
		void	set_hash_size(size_t n);

		void	clear();
	protected:
		async_resourcepool();
		class res_node;
		class mission_imp;
		class mission;
		class res_node_list;

		typedef std::shared_ptr<res_node_list>	res_node_list_ptr;
		typedef std::map<typeKey, res_node_list_ptr>	type_map;

		class res_node : public dlist<res_node>
		{
		public:
			res_node(const typeRes & n) : res_(n) {}
			res_node(typeRes && n) : res_(std::move(n)) {}
			res_node() {}
			typeRes		res_ = typeRes();
		};

		class mission_imp : public std::enable_shared_from_this<mission_imp>
		{
		public:
			mission_imp(bool boTrace, boost::asio::io_service & io, funcCallback &&f, std::string && s) 
				: trace_(boTrace), io_(io), func_(std::move(f)), todo_(std::move(s)) {}
			void		setTimeout(const timer_val & tTimeout)
			{
				boost::system::error_code ec;
				timer_.reset(new boost::asio::deadline_timer(io_));
				timer_->expires_from_now(boost::posix_time::seconds(static_cast<long>(tTimeout.sec())) + boost::posix_time::microseconds(tTimeout.micro_sec()), ec);
				std::shared_ptr<mission_imp> pSelf = this->shared_from_this();

				if (trace_)
					ara::glog(log::debug).printfln("mission (%v) begin wait p:%v", todo_, this);

				timer_->async_wait([this, pSelf](boost::system::error_code const & ec) {
					funcCallback func;
					{
						std::lock_guard<std::mutex>		_guard(lock_);
						if (ec != boost::asio::error::operation_aborted && !has_val_)
						{
							func = std::move(func_);
							func_ = nullptr;

							if (trace_)
								ara::glog(log::debug).printfln("mission (%v) timedout p:%v", todo_, this);
						}
					}
					if (func)
						func(boost::asio::error::timed_out, nullptr);
				});
			}
			void	dump(const std::string & strPrefix, std::ostream & out) {
				std::string strExpire;
				if (timer_) {
					strExpire = " Expires at:";
					strExpire += to_iso_string(timer_->expires_at());
				}
				out << strPrefix << "[" << todo_ << "]" << strExpire << std::endl;
			}
			void	action(async_res_token token) {
				BEGIN_CALL(todo_);
				funcCallback func;
				{
					std::lock_guard<std::mutex>		_guard(lock_);
					has_val_ = true;
					func = std::move(func_);
					func_ = nullptr;
					if (timer_) {
						boost::system::error_code ec;
						timer_->cancel(ec);
					}
				}
				if (func)
					func(boost::system::error_code(), token);
			}
			void cancel() {
				funcCallback func;
				{
					std::lock_guard<std::mutex>		_guard(lock_);
					has_val_ = true;
					func = std::move(func_);
					func_ = nullptr;
					if (timer_) {
						boost::system::error_code ec;
						timer_->cancel(ec);
					}
				}
				if (func)
					func(boost::asio::error::interrupted, nullptr);
			}

			bool				trace_ = false;
			boost::asio::io_service & io_;
			funcCallback		func_;
			std::string			todo_;
			bool				has_val_ = false;
			std::mutex			lock_;
			std::unique_ptr<boost::asio::deadline_timer> timer_;
		};

		class mission : public dlist<mission>
		{
		public:
			mission() {}
			mission(bool boTrace, boost::asio::io_service & io, const timer_val & tTimeout, funcCallback &&f, std::string && todo)
			{
				std::shared_ptr<mission_imp>	p = std::make_shared<mission_imp>(boTrace, io, std::move(f), std::move(todo));
				if (tTimeout != timer_val::max_time)
					p->setTimeout(tTimeout);

				if (p->timer_ == nullptr)
					holder_ = p;
				mission_imp_ = p;
			}
			void	stop() {
				std::shared_ptr<mission_imp>	holder = mission_imp_.lock();
				if (!holder)
					return;
				holder->cancel();
			}

			std::shared_ptr<mission_imp>	holder_;
			std::weak_ptr<mission_imp>		mission_imp_;
		};
		class res_node_list : public std::enable_shared_from_this<res_node_list>
		{
		public:
			res_node_list(bool	* trace) : trace_(trace) {
				res_root_.as_root(); 
				mission_root_.as_root();
			}
			~res_node_list() {
				std::lock_guard<std::mutex>	_guard(lock_);
				res_node * p1 = res_root_.get_next();
				while (p1 != &res_root_) {
					res_node * pTemp = p1;
					p1 = p1->get_next();
					pTemp->unlink();
					delete pTemp;
				}
				mission * p2 = mission_root_.get_next();
				while (p2 != &mission_root_) {
					mission * pTemp = p2;
					p2 = p2->get_next();
					pTemp->unlink();
					pTemp->stop();
					delete p2;
				}
			}
			void	add_res(const typeKey& k, res_node * node, bool boFirst) {
				std::lock_guard<std::mutex>		_guard(lock_);

				mission * cur = mission_root_.get_next();
				while(cur != &mission_root_)
				{
					mission * temp = cur;
					cur = cur->get_next();

					temp->unlink();
					std::unique_ptr<mission> _guard2(temp);
					std::shared_ptr<mission_imp>	holder = temp->mission_imp_.lock();
					if (!holder)
						continue;

					if (*trace_)
						ara::glog(log::debug).printfln("Key:%v add resource (%v) and dispatch now %v, p:%v", k, node, holder->todo_, temp);

					std::shared_ptr<async_respool_token>		pToken = std::make_shared<async_respool_token>(k, this->shared_from_this(), node);
					holder->io_.post([ holder, pToken]() {
						holder->action(pToken);
					});
					return;
				}

				if (*trace_)
					ara::glog(log::debug).printfln("Key:%v add resource (%v)", k, node);

				if (boFirst)
					node->append_after(res_root_);
				else
					node->append_before(res_root_);
			}
			void clear() {
				std::lock_guard<std::mutex>		_guard(lock_);
				mission* cur = mission_root_.get_next();
				while (cur != &mission_root_)
				{
					mission* temp = cur;
					cur = cur->get_next();

					temp->unlink();
					std::unique_ptr<mission> _guard2(temp);
					temp->holder_.reset();
				}
			}

			bool	*		trace_ = nullptr;
			res_node		res_root_;
			mission			mission_root_;
			std::mutex		lock_;
		};

		class async_respool_token : public internal::async_respool_token_base<typeRes>
		{
		public:
			async_respool_token(const typeKey& k, res_node_list_ptr p, res_node * n) : k_(k), parent_(p), node_(n) {}
			virtual const typeRes	&	get() const override {
				return node_->res_;
			}
			virtual ~async_respool_token() {
				parent_->add_res(k_, node_, true);
			}
		protected:
			typeKey				k_;
			res_node_list_ptr	parent_;
			res_node *			node_;
		};

		res_node_list_ptr	findNodeList(const typeKey & k, bool boCreateWhileNoExist);

		std::mutex				glock_;
		bool					trace_ = false;
		std::vector<type_map>	pool_;
		typeKeyHash				hash_func_;
	};

	//////////////////////////////////////////////////////////

	template<class typeKey, class typeRes, class typeKeyHash>
	async_resourcepool<typeKey, typeRes,typeKeyHash>::async_resourcepool()
	{
	}

	template<class typeKey, class typeRes, class typeKeyHash>
	async_resourcepool<typeKey, typeRes, typeKeyHash>::~async_resourcepool()
	{
	}

	template<class typeKey, class typeRes, class typeKeyHash>
	void async_resourcepool<typeKey, typeRes, typeKeyHash>::set_hash_size(size_t n)
	{
		std::lock_guard<std::mutex>	_guard(glock_);
		if (pool_.empty()) {
			pool_.resize(n);
			return;
		}
		std::vector<type_map>	pool2(n);
		for (auto itemMap : pool_)	{
			for (auto it : itemMap) {
				const typeKey & k = it.first;
				size_t nHash = hash_func_(k);
				pool2[nHash % n].insert(it);
			}
		}
		pool2.swap(pool_);
	}

	template<class typeKey, class typeRes, class typeKeyHash>
	typename async_resourcepool<typeKey, typeRes, typeKeyHash>::res_node_list_ptr	async_resourcepool<typeKey,typeRes,typeKeyHash>::findNodeList(const typeKey & k, bool boCreateWhileNoExist)
	{
		if (trace_)
			ara::glog(log::debug).printfln("Key:%v resource find node", k);

		size_t nHash = hash_func_(k);
		std::lock_guard<std::mutex>	_guard(glock_);
		auto & mapNode = pool_[nHash % pool_.size()];
		auto itFind = mapNode.find(k);
		if (itFind == mapNode.end())
		{
			if (!boCreateWhileNoExist)
				return nullptr;
			
			mapNode[k] = std::make_shared<res_node_list>(&trace_);
			itFind = mapNode.find(k);
		}
		return itFind->second;
	}

	template<class typeKey, class typeRes, class typeKeyHash>
	void async_resourcepool<typeKey, typeRes, typeKeyHash>::add(const typeKey & k, const typeRes & res)
	{
		res_node_list_ptr	plist = findNodeList(k, true);
		plist->add_res(k, new res_node(res), false);
	}

	template<class typeKey, class typeRes, class typeKeyHash>
	void async_resourcepool<typeKey, typeRes, typeKeyHash>::add(const typeKey & k, typeRes && res)
	{
		res_node_list_ptr	plist = findNodeList(k, true);
		plist->add_res(k, new res_node(std::move(res)), false);
	}

	template<class typeKey, class typeRes, class typeKeyHash>
	void async_resourcepool<typeKey, typeRes, typeKeyHash>::clear()
	{
		std::lock_guard<std::mutex>	_guard(glock_);
		for (type_map & mapData : pool_) {
			for (auto& it : mapData) {
				res_node_list_ptr& root = it.second;
				root->clear();
			}
		}
	}

	template<class typeKey, class typeRes, class typeKeyHash>
	void async_resourcepool<typeKey, typeRes, typeKeyHash>::apply(boost::asio::io_service & io, const typeKey & key, const timer_val & tTimeout, funcCallback && func, std::string && strTodo)
	{
		res_node_list_ptr	plist = findNodeList(key, false);
		if (plist == nullptr) {
			if (trace_)
				ara::glog(log::debug).printfln("Key:%v resource not found: %v", key, strTodo);

			func(boost::asio::error::invalid_argument, nullptr);
			return;
		}

		std::lock_guard<std::mutex>	_guard(plist->lock_);
		if (!plist->res_root_.root_empty()) {
			res_node * node = plist->res_root_.get_next();
			node->unlink();
			if (trace_)
				ara::glog(log::debug).printfln("Key:%v got resource right now:(%v) %v", key, node, strTodo);

			std::shared_ptr<async_respool_token>		pToken = std::make_shared<async_respool_token>(key, plist, node);
			io.post([f = std::move(func), pToken, strTodo = std::move(strTodo)](){
				BEGIN_CALL(strTodo);
				f(boost::system::error_code(), pToken);
			});
			return;
		}
		else {
			mission * m = new mission(trace_, io, tTimeout, std::move(func), std::move(strTodo));
			if (trace_)
				ara::glog(log::debug).printfln("Key:%v no resource, begin waitting : %v , p:%v", key, strTodo, m);
			m->append_before(plist->mission_root_);
		}
	}
}


#endif//ARA_ASYNC_RESPOOL_H
