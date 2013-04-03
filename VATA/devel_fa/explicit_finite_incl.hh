#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

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
 
  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;
  typedef typename ExplicitFA::StateType StateType;

  typedef typename ExplicitFA::StateType StateType;
  typedef std::unordered_set<StateType> StateSet;
  typedef std::unordered_map<StateType,StateSet&> Antichain;

  // actually processed macro state
  std::unordered_set<StateType> procMacroState; 
  // actually processed state
  StateType procState;
  bool macroFinal=false;
  Antichain antichain;

  // Create macro state of initial states
  for (StateType startState : bigger.startStates_) {
    procMacroState.insert(startState);
    macroFinal |= bigger.IsStateFinal(startState);
  }

  bool inclNotHold = false;
  for (StateType smallState : smaller.startStates_) {
    inclNotHold |= smaller.IsStateFinal(smallState) && !macroFinal;
    // Add to antichain
    antichain.insert(std::make_pair(smallState,procMacroState));
  }

  return !inclNotHold;
}



#endif
