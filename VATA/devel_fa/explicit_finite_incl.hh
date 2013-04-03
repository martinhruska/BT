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

  typedef typename InclFunc::AntichainType AntichainType;

  // actually processed macro state
  StateSet procMacroState; 
  // actually processed state
  StateType procState;
  bool macroFinal=false;
  AntichainType antichain;
  AntichainType next;

  InclFunc inclFunc(antichain,next);

  // Create macro state of initial states
  for (StateType startState : bigger.startStates_) {
    procMacroState.insert(startState);
    macroFinal |= bigger.IsStateFinal(startState);
  }

  bool inclNotHold = false;

  auto macroAcceptChecker = [](const ExplicitFA& bigger, StateSet& macroState)->
    bool {
      for (const StateType& state : macroState){
        if (bigger.IsStateFinal(state)) {
          return true; // if one is accepting and whole macrostate is accepting
        }
      }
      return false;
    };

  // Check the initial states
  for (StateType smallState : smaller.startStates_) {
    inclNotHold |= smaller.IsStateFinal(smallState) && !macroFinal;
    inclFunc.AddNewPairToAntichain(smallState,procMacroState);
  }

  procMacroState.clear();

  while(next.get(procState,procMacroState)) {
    // Iterate through the all symbols in the transitions for the given state
    for (auto& smallerSymbolToState : *(smaller.transitions_->
          find(procState)->second)) {
      for (const StateType& smallerStateInSet : *smallerSymbolToState.second) {

        StateSet newMacroState;
        // Create new macro state from current macro state for given symbol
        for (const StateType& stateInMacro : procMacroState) {
          for (const StateType& biggerStateInSet : *(bigger.transitions_->
           find(stateInMacro)->second-> // Transition for given state
           find(smallerSymbolToState.first)->second)) { // States for given symbol
            newMacroState.insert(biggerStateInSet);
          }
        } // end creating new macro state
        inclNotHold |= smaller.IsStateFinal(smallerStateInSet) && !macroAcceptChecker(bigger,newMacroState);
        inclFunc.AddNewPairToAntichain(smallerStateInSet,newMacroState);
      }
    }

    procMacroState.clear();
  }
  return !inclNotHold;
}



#endif
