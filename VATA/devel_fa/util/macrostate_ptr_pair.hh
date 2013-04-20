#ifndef UTIL_MACROSTATE_PTR_PAIR_
#define UTIL_MACROSTATE_PTR_PAIR_

#include <unordered_map>

#include <vata/vata.hh>

namespace VATA {
  template <class Aut> class MacroStatePtrPair;
}

/*
 * Class for mapping pointer to macro state
 * to list of pointers to macro states
 */
template <class Aut>
class VATA::MacroStatePtrPair {
public:
  typedef typename Aut::StateSet StateSet;
  typedef std::unordered_set<StateSet*> SetPtrSet;
  
  std::unordered_map<StateSet*,SetPtrSet> map;

  MacroStatePtrPair() : map() {}

  /*
   * Add value v to key k.
   */
  inline void add(StateSet* k, StateSet* v) {
    auto iter = map.find(k);
    //std::cerr << "pridani novych stavu do congr hold" << k << " " << v << " " << map.size() << " " << std::endl;
    if (iter == map.end()) {
      map.insert(std::make_pair(k,SetPtrSet())).first->second.insert(v);
    }
    else {
      iter->second.insert(v);
    }
    //std::cerr << "pridano"  << map.size() << std::endl;
  }


  /*
   * Checks whether for the given key k there exists
   * value v in its list of pointers.
   */
  inline bool contains(StateSet* k, StateSet* v) {
    auto iter = map.find(k);
    //std::cerr << "Contains " << k << " " << v << std::endl;
    if (iter == map.end()) {
      return false;
    }
    else {
      return iter->second.count(v);
    }
  }
};

#endif
