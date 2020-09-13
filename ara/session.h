
#ifndef ARA_SESSION_H
#define ARA_SESSION_H

#include "ara_def.h"
#include "dlist.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <time.h>

/*
	Usage:
		using	MySession = session<tstring, data>;

		std::shared_ptr<const data> pReadData = MySession.get_read("Hello");
		std::shared_ptr<const data> pReadData = MySession.get_read("Hello", ses_clear_expire);

		std::shared_ptr<data> pWriteData = MySession.get_write("Hello");

		std::shared_ptr<data> pWriteData = MySession.create("Hello", data());
		std::shared_ptr<data> pWriteData = MySession.create("Hello", std::make_shared<data>());

		MySession.navigate( [](size_t nIndex, MySession::control_data & d)->int{
			return ses_nav_continue;
		} );
*/

namespace ara {

	enum {
		ses_clear_expire = 0x01,
		ses_update_access = 0x02,
		ses_update_modify = 0x04,

		ses_nav_continue = 0x01,
		ses_nav_delete = 0x02,
	};

	template<typename Key, typename Data
		, class typeKeyHash = std::hash<Key>
		, class Pre = std::less<Key>
		, class LockType = std::mutex
	>
		class session
	{
	public:
		class control_data;
		typedef	Key		key_type;
		typedef Data	data_type;
		typedef std::function<int(size_t nIndex, control_data & data)>	funcNavigate;

		session(size_t nHashSize);
		~session();

		void	resize(size_t nHashSize);//not thread safe.
		void	register_del_expire_callback(funcNavigate && func) {
			del_expire_func_ = func;
		}
		void	set_default_expire_setting(size_t nTimeoutSec);

		typedef std::shared_ptr<Data>		data_ptr;
		typedef std::shared_ptr<const Data>	const_data_ptr;

		const_data_ptr		get_read(const Key & key, int nFlags = ses_clear_expire | ses_update_access);
		data_ptr			get_write(const Key & key, int nFlags = ses_clear_expire | ses_update_access | ses_update_modify);
		data_ptr			get_write_or_create(const Key & key, int nFlags = ses_clear_expire | ses_update_access | ses_update_modify, size_t nExpireSetting = 0);

		data_ptr			create(const Key & key, const data_ptr & pData, int nFlags = ses_clear_expire, size_t nExpireSetting = 0);
		data_ptr			create(const Key & key, Data && data, int nFlags = ses_clear_expire, size_t nExpireSetting = 0) {
			data_ptr	pData = std::make_shared<Data>(std::move(data));
			return create(key, pData, nFlags, nExpireSetting);
		}
		data_ptr			create(const Key & key, const Data & data, int nFlags = ses_clear_expire, size_t nExpireSetting = 0) {
			data_ptr	pData = std::make_shared<Data>(data);
			return create(key, pData, nFlags, nExpireSetting);
		}

		bool				remove(const Key & key, int nFlags = ses_clear_expire);
		void				update_expire_setting(const Key & key, size_t nExpireSetting, int nFlags = ses_clear_expire);

		class control_data
		{
		public:
			inline const data_ptr & get() const { return data_ptr_; }
			inline data_ptr & get() { return data_ptr_; }
			inline void	set(const data_ptr & p) { data_ptr_ = p; }

			inline time_t		get_create_time() const { return create_time_; }
			inline void		set_create_time(time_t t) { create_time_ = t; }

			inline time_t		get_access_time() const { return access_time_; }
			inline void		set_access_time(time_t t) { access_time_ = t; }

			inline time_t		get_modify_time() const { return modify_time_; }
			inline void		set_modify_time(time_t t) { modify_time_ = t; }

			inline time_t		get_expire_time() const { return expire_time_; }
			inline void		set_expire_time(time_t t) { expire_time_ = t; }

			inline size_t		get_expire_setting() const { return expire_sec_; }
			inline void		set_expire_setting(size_t t) { expire_sec_ = t; }

			inline const Key &	get_key() const { return key_; }
		protected:
			Key			key_;
			data_ptr	data_ptr_;
			time_t		create_time_ = 0;
			time_t		access_time_ = 0;
			time_t		modify_time_ = 0;
			time_t		expire_time_ = 0;
			size_t		expire_sec_ = 0;
		};

		inline size_t		hash(const Key & key) {
			return hash_func_(key);
		}
		inline size_t		hash_index(const Key & key) {
			return hash(key) % hash_vec_.size();
		}

		void	navigate(funcNavigate && func);
		void	clear_all();
	protected:
		void	clear();

		struct Node : public control_data, public dlist<Node>
		{
			Node() {}
			Node(Node && n) {
				this->data_ptr_ = n.data_ptr_;
				this->create_time_ = n.create_time_;
				this->access_time_ = n.access_time_;
				this->modify_time_ = n.modify_time_;
				this->expire_time_ = n.expire_time_;
				this->expire_sec_ = n.expire_sec_;
				this->key_ = std::move(n.key_);
			}
			void	set_key(const Key & k) {
				this->key_ = k;
			}
		};

		using typeMap = std::map<Key, Node, Pre>;

		struct HashNode
		{
			HashNode() {
				root_.as_root();
			}
			LockType	lock_;
			typeMap		map_data_;
			Node		root_;
		};

		bool	navigate_without_lock(size_t & nIndex, HashNode & hashNode, funcNavigate & func);
		void	del_expire(HashNode & hashNode);
		static void	update_access_imp(Node & controlData, HashNode & hashNode, time_t tNow);

		typeKeyHash					hash_func_;
		funcNavigate				del_expire_func_;
		std::vector<HashNode *>		hash_vec_;
		size_t						default_expire_timeout_sec_ = 0;
	};

	//////////////////////////////////////////////////////////////////////////

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	session<Key, Data, typeKeyHash, Pre, LockType>::session(size_t nHashSize)
	{
		if (nHashSize)
			resize(nHashSize);
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	session<Key, Data, typeKeyHash, Pre, LockType>::~session()
	{
		clear();
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	void session<Key, Data, typeKeyHash, Pre, LockType>::clear()
	{
		for (HashNode * pNode : hash_vec_)
			delete pNode;
		hash_vec_.clear();
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	void session<Key, Data, typeKeyHash, Pre, LockType>::resize(size_t nHashSize)
	{
		if (nHashSize == 0) {
			clear();
			return;
		}

		std::vector<HashNode *>	newHash(nHashSize);
		for (size_t i = 0; i < nHashSize; ++i)
			newHash[i] = new HashNode;

		for (HashNode * pHashNode : hash_vec_) {

			HashNode & hashNode = *pHashNode;
			std::lock_guard<LockType> 	_guard(hashNode.lock_);
			time_t tNow = time(NULL);

			auto it = hashNode.map_data_.begin();
			auto itEnd = hashNode.map_data_.end();
			for (; it != itEnd; ++it) {
				const Key & key = it->first;
				Node & data = it->second;
				data.unlink();

				auto tExpire = data.get_expire_time();
				if (tExpire <= tNow)
					continue;

				HashNode & newHashNode = *(newHash[hash(key) % nHashSize]);
				auto itNew = newHashNode.map_data_.emplace(key, std::move(data)).first;
				Node & newData = itNew->second;

				if (newHashNode.root_.root_empty())
					newData.append_before(newHashNode.root_);
				else
				{
					Node * p = newHashNode.root_.get_pre();
					Node * pEnd = &newHashNode.root_;
					for (; p != pEnd; p = p->get_pre())
						if (p->get_expire_time() <= tExpire)
							break;
					newData.append_after(*p);
				}
			}
		}
		hash_vec_.swap(newHash);
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	typename session<Key, Data, typeKeyHash, Pre, LockType>::const_data_ptr session<Key, Data, typeKeyHash, Pre, LockType>::get_read(const Key & key, int nFlags)
	{
		data_ptr res = get_write(key, nFlags);
		return res;
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	inline void session<Key, Data, typeKeyHash, Pre, LockType>::update_access_imp(Node & controlData, HashNode & hashNode, time_t tNow)
	{
		time_t tExpire = tNow + controlData.get_expire_setting();
		controlData.set_access_time(tNow);
		controlData.set_expire_time(tExpire);
		controlData.unlink();

		if (hashNode.root_.root_empty())
			controlData.append_before(hashNode.root_);
		else
		{
			Node * p = hashNode.root_.get_pre();
			Node * pEnd = &hashNode.root_;
			for (; p != pEnd; p = p->get_pre())
				if (p->get_expire_time() <= tExpire)
					break;
			controlData.append_after(*p);
		}

	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	typename session<Key, Data, typeKeyHash, Pre, LockType>::data_ptr	session<Key, Data, typeKeyHash, Pre, LockType>::get_write(const Key & key, int nFlags)
	{
		size_t nHashIndex = hash_index(key);
		HashNode & hashNode = *(hash_vec_[nHashIndex]);

		std::lock_guard<LockType> 	_guard(hashNode.lock_);

		if (nFlags & ses_clear_expire)
			del_expire(hashNode);

		typeMap & map = hashNode.map_data_;
		typename typeMap::iterator it = map.find(key);
		if (it == map.end())
			return nullptr;
		Node & controlData = it->second;

		if (nFlags & (ses_update_access | ses_update_modify))
		{
			time_t tNow = time(NULL);
			if (nFlags & ses_update_access)
				update_access_imp(controlData, hashNode, tNow);
			if (nFlags & ses_update_modify)
				controlData.set_modify_time(tNow);
		}
		return controlData.get();
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	void session<Key, Data, typeKeyHash, Pre, LockType>::update_expire_setting(const Key & key, size_t nExpireSetting, int nFlags)
	{
		size_t nHashIndex = hash_index(key);
		HashNode & hashNode = *(hash_vec_[nHashIndex]);

		std::lock_guard<LockType> 	_guard(hashNode.lock_);

		if (nFlags & ses_clear_expire)
			del_expire(hashNode);

		typeMap & map = hashNode.map_data_;
		typename typeMap::iterator it = map.find(key);
		if (it == map.end())
			return;
		Node & controlData = it->second;

		time_t tNow = time(NULL);
		controlData.set_expire_setting(nExpireSetting);
		update_access_imp(controlData, hashNode, tNow);
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	typename session<Key, Data, typeKeyHash, Pre, LockType>::data_ptr session<Key, Data, typeKeyHash, Pre, LockType>::get_write_or_create(const Key & key, int nFlags, size_t nNewExpireTime)
	{
		size_t nHashIndex = hash_index(key);
		HashNode & hashNode = *(hash_vec_[nHashIndex]);

		std::lock_guard<LockType> 	_guard(hashNode.lock_);

		if (nFlags & ses_clear_expire)
			del_expire(hashNode);

		typeMap & map = hashNode.map_data_;
		typename typeMap::iterator it = map.find(key);
		bool	boIsNew = true;
		if (it == map.end())
			it = map.emplace(key, Node()).first;
		else
			boIsNew = false;

		Node & controlData = it->second;
		time_t tNow = time(NULL);
		if (boIsNew)
		{
			controlData.set(std::make_shared<Data>());
			controlData.set_key(key);
			controlData.set_create_time(tNow);
			controlData.set_expire_setting(nNewExpireTime ? nNewExpireTime : default_expire_timeout_sec_);
		}
		if (boIsNew || (nFlags & ses_update_modify))
			controlData.set_modify_time(tNow);
		if (boIsNew || (nFlags & ses_update_access))
			update_access_imp(controlData, hashNode, tNow);

		return controlData.get();
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	typename session<Key, Data, typeKeyHash, Pre, LockType>::data_ptr session<Key, Data, typeKeyHash, Pre, LockType>::create(const Key & key, const data_ptr & pData, int nFlags, size_t nNewExpireTime)
	{
		size_t nHashIndex = hash_index(key);
		HashNode & hashNode = *(hash_vec_[nHashIndex]);

		std::lock_guard<LockType> 	_guard(hashNode.lock_);

		if (nFlags & ses_clear_expire)
			del_expire(hashNode);

		typeMap & map = hashNode.map_data_;
		typename typeMap::iterator it = map.find(key);
		if (it == map.end())
			it = map.emplace(key, Node()).first;

		Node & controlData = it->second;
		time_t tNow = time(NULL);
		controlData.set_key(key);
		controlData.set(pData);
		controlData.set_expire_setting(nNewExpireTime ? nNewExpireTime : default_expire_timeout_sec_);
		controlData.set_create_time(tNow);
		controlData.set_modify_time(tNow);
		update_access_imp(controlData, hashNode, tNow);

		return controlData.get();
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	bool session<Key, Data, typeKeyHash, Pre, LockType>::remove(const Key & key, int nFlags)
	{
		size_t nHashIndex = hash_index(key);
		HashNode & hashNode = *(hash_vec_[nHashIndex]);

		std::lock_guard<LockType> 	_guard(hashNode.lock_);

		if (nFlags & ses_clear_expire)
			del_expire(hashNode);

		typeMap & map = hashNode.map_data_;
		typename typeMap::iterator it = map.find(key);
		if (it == map.end())
			return false;
		Node & controlData = it->second;
		controlData.unlink();
		map.erase(it);
		return true;
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	void session<Key, Data, typeKeyHash, Pre, LockType>::navigate(funcNavigate && func)
	{
		size_t nIndex = 0;
		for (HashNode * pHashNode : hash_vec_)
		{
			HashNode & hashNode = *pHashNode;
			std::lock_guard<LockType> 	_guard(hashNode.lock_);
			if (!navigate_without_lock(nIndex, hashNode, func))
				break;
		}
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	bool	session<Key, Data, typeKeyHash, Pre, LockType>::navigate_without_lock(size_t & nIndex, HashNode & hashNode, funcNavigate & func)
	{
		auto it = hashNode.map_data_.begin();
		auto itEnd = hashNode.map_data_.end();
		while (it != itEnd)
		{
			int res = func(nIndex, it->second);
			if (res & ses_nav_delete)
			{
				it->second.unlink();
				it = hashNode.map_data_.erase(it);
			}
			else
				++it;
			if ((res & ses_nav_continue) == 0)
				return false;
			++nIndex;
		}
		return true;
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	void session<Key, Data, typeKeyHash, Pre, LockType>::set_default_expire_setting(size_t nTimeoutSec)
	{
		default_expire_timeout_sec_ = nTimeoutSec;
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	void session<Key, Data, typeKeyHash, Pre, LockType>::del_expire(HashNode & hashNode)
	{
		funcNavigate funcDelExpire = del_expire_func_;
		if (!funcDelExpire)
		{
			time_t tExpireTime = time(NULL);
			funcDelExpire = [tExpireTime](size_t nIndex, control_data & data) mutable ->int {
				if (data.get_expire_setting() && data.get_expire_time() <= tExpireTime)
					return (ses_nav_delete | ses_nav_continue);
				return 0;
			};
		}

		Node * pBegin = hashNode.root_.get_next();
		Node * pEnd = &hashNode.root_;
		size_t nIndex = 0;
		while (pBegin != pEnd)
		{
			Node * pCur = pBegin;
			pBegin = pBegin->get_next();

			int res = funcDelExpire(nIndex, *pCur);
			if (res & ses_nav_delete)
			{
				pCur->unlink();
				hashNode.map_data_.erase(pCur->get_key());
			}
			if ((res & ses_nav_continue) == 0)
				break;
			++nIndex;
		}
	}

	template<typename Key, typename Data, class typeKeyHash, class Pre, class LockType>
	void session<Key, Data, typeKeyHash, Pre, LockType>::clear_all()
	{
		for (HashNode * pHashNode : hash_vec_)
		{
			HashNode & hashNode = *pHashNode;
			std::lock_guard<LockType> 	_guard(hashNode.lock_);

			hashNode.map_data_.clear();
			hashNode.root_.as_root();
		}
	}
}

#endif//ARA_SESSION_H
