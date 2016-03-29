
#ifndef ARA_ASYNC_QUEUE_H
#define ARA_ASYNC_QUEUE_H

#include "ara_def.h"
#include "dlist.h"

#include <boost/asio.hpp>
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>

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
				res->setHashSize(nBlockSize);
			return res;
		}
		~async_queue();

		void setTraceLog(bool boTrace) {
			m_boTrace = boTrace;
		}

		void clear();

		//It's not thread-safe. set the hash size without lock.
		void setHashSize(size_t nBlockSize);

		void	dump(const std::string & strPrefix, std::ostream & out);

		void	apply(boost::asio::io_service & io, const typeKey & key, const TTimerValue & tTimeout, funcCallback && func, std::string && strTodo);

	protected:
		async_queue();

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
			mission(boost::asio::io_service & io, funcCallback && func, std::string && strTodo, const node_ptr & pParent)
				: m_io(io), m_func(std::move(func)), m_strTodo(std::move(strTodo)), m_pParent(pParent) {
				m_boTrace = pParent->isTraceLog();
			}

			~mission() {
				if (m_pParent) 	{
					m_pParent->doFinish(this);
					if (m_boTrace)
						gError().log(TLog::levelDebug).format("Token Release for : %").arg(m_strTodo) << std::endl;
				}
			}

			void cancel() {
				action(boost::asio::error::operation_aborted, nullptr);
			}

			void active(const async_token & token) {
				action(boost::system::error_code(), token);
			}

			void setTimer(const TTimerValue & tTimeout) {
				boost::system::error_code ec;
				m_timer.reset(new boost::asio::deadline_timer(m_io));
				m_timer->expires_from_now(boost::posix_time::seconds(static_cast<long>(tTimeout.sec())) + boost::posix_time::microseconds(tTimeout.micro_sec()), ec);
				mission_ptr pSelf = m_pSelf;
				m_timer->async_wait([this, pSelf](boost::system::error_code const & ec) {
					if (ec != boost::asio::error::operation_aborted) {
						if (mission_base::cando()) {
							funcCallback func = std::move(m_func);
							func(boost::asio::error::timed_out, nullptr);
						}
					}
				}
				);
			}
			void    setHolder(const mission_ptr & token) {
				m_pSelf = token;
			}
			mission_ptr releaseHolder() {
				mission_ptr res;
				res.swap(m_pSelf);
				return res;
			}
			void clear() {
				m_pParent = nullptr;
				cancel();
				m_pSelf = nullptr;
			}
			boost::asio::io_service & io() { return m_io; }
			void	dump(const tstring & strPrefix, std::ostream & out) {
				tstring strExpire;
				if (m_timer) {
					strExpire = " Expires at:";
					strExpire += async::ptime_to_string(m_timer->expires_at());
				}
				out << strPrefix << "[" << m_strTodo << "]" << strExpire << std::endl;
			}
		protected:
			inline void action(boost::system::error_code const & ec, const async_token & token) {
				if (mission_base::cando()) {
					boost::system::error_code temp;
					if (m_timer)
						m_timer->cancel(temp);
					if (!ec && m_boTrace)
						gError().log(TLog::levelDebug).format("Token get for : %").arg(m_strTodo) << std::endl;
					m_func(ec, token);
				}
			}

			boost::asio::io_service & m_io;
			funcCallback			m_func;
			std::unique_ptr<boost::asio::deadline_timer> m_timer;
			tstring					m_strTodo;
			mission_ptr				m_pSelf;
			node_ptr				m_pParent;
			bool					m_boTrace = false;
		};

		class node : public std::enable_shared_from_this<node>
		{
		public:
			node(const typeKey & key, hashnode_ptr pParent) : m_key(key), m_pParent(pParent) {
				m_boTrace = pParent->isTraceLog();
				m_Root.as_root();
			}
			~node() {
				clear();
			}

			inline bool	isTraceLog() const {
				return m_boTrace;
			}

			void doApply(boost::asio::io_service & io, const TTimerValue & tTimeout, funcCallback && func, tstring && strTodo) {

				mission_ptr pMission = std::make_shared<mission>(io, std::move(func), std::move(strTodo), this->shared_from_this());

				std::unique_lock<std::mutex> _guard(m_Lock);

				if (m_Root.root_empty()) { //first time
					pMission->append_before(m_Root);
					_guard.unlock();
					io.dispatch([pMission]() { pMission->active(pMission); });
				}
				else {
					pMission->append_before(m_Root);
					pMission->setHolder(pMission);

					if (tTimeout != TTimerValue::max_time)
						pMission->setTimer(tTimeout);
				}
			}
			void doFinish(mission * pMission) {
				std::unique_lock<std::mutex> _guard(m_Lock);
				pMission->unlink();

				if (m_Root.root_empty()) {
					hashnode_ptr pParent = m_pParent.lock();
					_guard.unlock();
					if (pParent)
						pParent->destroy(m_key);
				}
				else {
					mission * mis = static_cast<mission *>(m_Root.get_next());
					mission_ptr pMission = mis->releaseHolder();
					_guard.unlock();
					pMission->io().post([pMission]() { pMission->active(pMission); });
				}
			}

			bool empty() {
				std::lock_guard<std::mutex> _guard(m_Lock);
				return m_Root.root_empty();
			}

			void clear() {
				std::lock_guard<std::mutex> _guard(m_Lock);

				mission_base * pBegin = m_Root.get_next();
				mission_base * pEnd = &m_Root;
				while (pBegin != pEnd) {
					mission * pCurMission = static_cast<mission *>(pBegin);
					pBegin = pBegin->get_next();

					pCurMission->unlink();
					pCurMission->clear();
				}
			}

			void	dump(const tstring & strPrefix, std::ostream & out) {
				std::lock_guard<std::mutex> _guard(m_Lock);
				mission_base * pBegin = m_Root.get_next();
				mission_base * pEnd = &m_Root;
				while (pBegin != pEnd) {
					mission * pCurMission = static_cast<mission *>(pBegin);
					pCurMission->dump(strPrefix + "  ", out);
					pBegin = pBegin->get_next();
				}
			}
		protected:
			std::mutex m_Lock;
			typeKey m_key;
			hashnode_weak_ptr m_pParent;
			mission_base m_Root;
			bool			m_boTrace = false;
		};

		typedef std::map<typeKey, std::shared_ptr<node>> typeNodeMap;

		class HashNode : public std::enable_shared_from_this<HashNode>
		{
		public:
			HashNode(bool boTrace) : m_boTrace(boTrace) {}

			~HashNode() {
				clear();
			}

			inline bool	isTraceLog() const {
				return m_boTrace;
			}

			std::mutex m_Lock;
			std::map<typeKey, std::shared_ptr<node>> m_mapData;
			bool		m_boTrace = false;

			std::shared_ptr<node> get(const typeKey & key) {
				std::shared_ptr<node> pNode;

				std::unique_lock<std::mutex> _guard(m_Lock);
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
				std::unique_lock<std::mutex> _guard(m_Lock);
				typename typeNodeMap::iterator it = m_mapData.find(key);
				if (it != m_mapData.end() && it->second->empty())
					m_mapData.erase(it);
			}

			void clear() {
				std::unique_lock<std::mutex> _guard(m_Lock);
				auto it = m_mapData.begin();
				auto itEnd = m_mapData.end();
				for (; it != itEnd; ++it)
					it->second->clear();
				m_mapData.clear();
			}

			bool	empty() {
				std::unique_lock<std::mutex> _guard(m_Lock);
				return m_mapData.empty();
			}
			void	dump(const tstring & strPrefix, std::ostream & out) {
				std::unique_lock<std::mutex> _guard(m_Lock);
				auto it = m_mapData.begin();
				auto itEnd = m_mapData.end();
				for (; it != itEnd; ++it)
				{
					out << strPrefix << "Key:" << it->first << std::endl;
					it->second->dump(strPrefix + "  ", out);
				}
			}
		};

		std::vector<shared_ptr<HashNode>> m_vecHash;
		typeKeyHash m_hashFunc;
		bool m_boTrace = false;
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
		for (shared_ptr<HashNode> & pNode : m_vecHash) {
			if (pNode)
				pNode->clear();
		}
		m_vecHash.clear();
	}

	template<class typeKey, class typeKeyHash>
	void async_queue<typeKey, typeKeyHash>::setHashSize(size_t nBlockSize)
	{
		if (!m_vecHash.empty())
			m_vecHash.clear();

		m_vecHash.resize(nBlockSize);
	}

	template<class typeKey, class typeKeyHash>
	void async_queue<typeKey, typeKeyHash>::apply(boost::asio::io_service & io, const typeKey & key, const TTimerValue & tTimeout, funcCallback && func, tstring && strTodo)
	{
		size_t nHash = m_hashFunc(key);
		std::shared_ptr<node> pNode;

		if (m_vecHash.empty()) {
			func(boost::asio::error::invalid_argument, nullptr);
			return;
		}
		else {
			size_t nIndex = nHash % m_vecHash.size();
			if (!m_vecHash[nIndex])
				m_vecHash[nIndex] = std::make_shared<HashNode>(m_boTrace);

			pNode = m_vecHash[nIndex]->get(key);
		}

		if (pNode)
			pNode->doApply(io, tTimeout, std::move(func), std::move(strTodo));
	}

	template<class typeKey, class typeKeyHash>
	void async_queue<typeKey, typeKeyHash>::dump(const tstring & strPrefix, std::ostream & out)
	{
		size_t	nIndex = 0;
		for (shared_ptr<HashNode> & pNode : m_vecHash) {
			if (pNode && !pNode->empty()) {
				out << strPrefix << "[" << nIndex << "] :" << std::endl;
				pNode->dump(strPrefix + "  ", out);
			}
			++nIndex;
		}
	}
}

#endif ARA_ASYNC_QUEUE_H
