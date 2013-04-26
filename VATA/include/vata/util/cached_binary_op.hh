/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    CachedBinaryOp template header file.
 *
 *****************************************************************************/

#ifndef _VATA_CACHED_BINARY_OP_HH
#define _VATA_CACHED_BINARY_OP_HH

#include <unordered_map>
#include <set>

// insert class to proper namespace
namespace VATA {
	namespace Util {
		template <class T1, class T2, class V> class CachedBinaryOp;
	}
}

template <class T1, class T2, class V>
class VATA::Util::CachedBinaryOp {

public:

	typedef std::pair<T1, T2> Key;
	typedef std::unordered_map<Key, V, boost::hash<Key>> KeyToVMap;
	typedef std::set<typename KeyToVMap::value_type*> KeyToVMapValueTypeSet;
	typedef std::unordered_map<T1, KeyToVMapValueTypeSet, boost::hash<T1>> T1ToKeyToVMapValueTypeSetMap;
	typedef std::unordered_map<T2, KeyToVMapValueTypeSet, boost::hash<T2>> T2ToKeyToVMapValueTypeSetMap;

protected:

	KeyToVMap store_;
	T1ToKeyToVMapValueTypeSetMap storeMap1_;
	T2ToKeyToVMapValueTypeSetMap storeMap2_;

public:

	CachedBinaryOp() : store_(), storeMap1_(), storeMap2_() {}

	void clear() {

		this->storeMap1_.clear();
		this->storeMap2_.clear();
		this->store_.clear();

	}

	void invalidateFirst(const T1& x) {

		auto i = this->storeMap1_.find(x);

		if (i == this->storeMap1_.end())
			return;

		for (auto& item : i->second) {

			auto j = this->storeMap2_.find(item->first.second);

			assert(j != this->storeMap2_.end());

			j->second.erase(item);

			this->store_.erase(item->first);

		}

		this->storeMap1_.erase(i);

	}

	void invalidateSecond(const T2& x) {

		auto i = this->storeMap2_.find(x);

		if (i == this->storeMap2_.end())
			return;

		for (auto& item : i->second) {

			auto j = this->storeMap1_.find(item->first.first);

			assert(j != this->storeMap1_.end());

			j->second.erase(item);

			this->store_.erase(item->first);

		}

		this->storeMap2_.erase(i);

	}

	template <class F>
	V lookup(const T1& x, const T2& y, F f) {

		auto p = this->store_.insert(std::make_pair(std::make_pair(x, y), V()));

		if (p.second) {

			p.first->second = f(x, y);

			this->storeMap1_.insert(
				std::make_pair(x, KeyToVMapValueTypeSet())
			).first->second.insert(&*p.first);
			
			this->storeMap2_.insert(
				std::make_pair(y, KeyToVMapValueTypeSet())
			).first->second.insert(&*p.first);

		}

		return p.first->second;

	}

};

#endif
