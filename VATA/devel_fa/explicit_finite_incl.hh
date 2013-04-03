#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include "explicit_finite_aut.hh"
#include "explicit_finite_incl_fctor.hh"

#include <vector> 

namespace VATA {

  template<class SymbolType, class Rel>
  bool CheckFiniteAutInclusion(
    const ExplicitFiniteAut<SymbolType>& smaller, 
    const ExplicitFiniteAut<SymbolType>& bigger, 
    const Rel& preorder);
}

template<class SymbolType, class Rel>
bool VATA::CheckFiniteAutInclusion(
  const VATA::ExplicitFiniteAut<SymbolType>& smaller, 
  const VATA::ExplicitFiniteAut<SymbolType>& bigger, 
  const Rel& preorder) {
 
  typedef VATA::ExplicitFAInclusionFunctor<SymbolType> InclFunc;
  typedef typename InclFunc::ExplicitFA ExplicitFA;

  typedef typename InclFunc::StateType StateType;
  typedef typename InclFunc::StateSet StateSet;

  typedef typename InclFunc::Antichain Antichain;

  // actually processed macro state
  std::unordered_set<StateType> procMacroState; 
  // actually processed state
  StateType procState;
  bool macroFinal=false;
  Antichain antichain;


  // Comparator of macro states
  auto comparator = 
    [](const StateSet& lhs, const StateSet& rhs) -> bool {
      bool res=true;
      for (auto& lstate : lhs) {
        // search for state, which is not in rhs
        res = rhs.count(lstate);
        std::cout << res << std::endl;
      }
      return res;
  };

  // Create macro state of initial states
  for (StateType startState : bigger.startStates_) {
    procMacroState.insert(startState);
    macroFinal |= bigger.IsStateFinal(startState);
  }

  bool inclNotHold = false;
  for (StateType smallState : smaller.startStates_) {
    inclNotHold |= smaller.IsStateFinal(smallState) && !macroFinal;
    // Add to antichain
    std::vector<StateType> tempStateSet = {smallState};
    antichain.refine(tempStateSet,procMacroState,comparator);
    antichain.insert(smallState,procMacroState);
  }

  return !inclNotHold;
}



#endif
