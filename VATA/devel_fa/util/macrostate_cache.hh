/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Header file for cache of macrostates.
 *
 *****************************************************************************/


#ifndef UTIL_MACROSTATE_CACHE_
#define UTIL_MACROSTATE_CACHE_

#include <vata/vata.hh>

#include <unordered_map>

namespace VATA {
  template<class Aut> class MacroStateCache;
}

/*
 * Cache for caching macro state
 */
template<class Aut>
class VATA::MacroStateCache {
private:
  typedef typename Aut::StateSet StateSet;
  typedef std::list<StateSet> SetList;
  typedef std::unordered_map<size_t,SetList> CacheMap;

  CacheMap cacheMap;
public:
  MacroStateCache() : cacheMap(){}

  StateSet& insert(size_t key, StateSet& value) {
    auto areEqual = [] (StateSet& lss, StateSet& rss) -> bool {
     if (lss.size() != rss.size()) {
       return false;
     }
     if (!lss.size() || !rss.size()) {
       return false;
     }
     for (auto& ls : lss) {
       //std:: cout << ls << std::endl;
       if (!rss.count(ls)) {
         return false;
       }
     }

      return true;
   };

    auto iter = cacheMap.find(key);
    if (iter == cacheMap.end()) { // new value
      //std::cerr << "pridavam zbrusu novy stav" << std::endl;
      auto& list = cacheMap.insert(std::make_pair(key,SetList())).first->second;
      list.push_back(StateSet(value));
      return list.back();
    }
    else {
      for (auto& set : iter->second) { // set already cashed
        if (areEqual(set,value)) {
      //std::cerr << "set cached" << std::endl;
          return set;
        }
      }
      iter->second.push_back(StateSet(value));
      //std::cerr << "new hash" << std::endl;
      return iter->second.back();
    }
  }

};


#endif
