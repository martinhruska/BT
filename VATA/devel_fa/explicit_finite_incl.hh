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

  std::unordered_set<StateType> procMacroState;
  StateType procState;
  bool macroFinal=false;

  for (StateType startState : bigger.startStates_) {
    procMacroState.insert(startState);
    macroFinal |= bigger.IsStateFinal(startState);
  }
  std::cout << "macro final " << macroFinal << std::endl;

  bool inclNotHold = false;
  for (StateType smallState : smaller.startStates_) {
    inclNotHold |= smaller.IsStateFinal(smallState) && !macroFinal;
    // Tady bude pridavani do antichainu
  }

  return ~inclNotHold;
}



#endif
