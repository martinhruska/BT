#ifndef EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_
#define EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include "explicit_finite_aut.hh"

namespace VATA {
  template <class SymbolType> class ExplicitFAInclusionFunctor;
}

template <class SymbolType>
class VATA::ExplicitFAInclusionFunctor {

public : // data types
  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  typedef typename ExplicitFA::StateType StateType;
  typedef typename ExplicitFA::StateSet StateSet;

  typedef VATA::Util::Antichain2Cv2<StateType,StateSet> AntichainType;

private: // data memebers
  AntichainType& antichain_;
  AntichainType& next_;

public: // constructor
  ExplicitFAInclusionFunctor(AntichainType& antichain, AntichainType& next) :
    antichain_(antichain),
    next_(next)
  {}

public: // public functions
  void AddNewPairToAntichain(StateType state, StateSet &set) {
    // Comparator of macro states
    auto comparator = [](const StateSet& lhs, const StateSet& rhs) { 
      return lhs.IsSubsetOf(rhs); };

    std::vector<StateType> tempStateSet = {state};
    if (!antichain_.contains(tempStateSet,set,comparator)) {
      antichain_.refine(tempStateSet,set,comparator);
      antichain_.insert(state,set);
      AddToNext(state,set);
    }
  }

private: // private functions
  void AddToNext (StateType state, StateSet &set) {

    // Comparator of macro states
    auto comparator = [](const StateSet& lhs, const StateSet& rhs) { 
      return lhs.IsSubsetOf(rhs); };

    std::vector<StateType> tempStateSet = {state};
    if (!next_.contains(tempStateSet,set,comparator)) {
      next_.refine(tempStateSet,set,comparator);
      next_.insert(state,set);
    }
  }
};

#endif
