#ifndef _VATA_EXPLICIT_FINITE_AUT_CANDIDATE_
#define _VATA_EXPLICIT_FINITE_AUT_CANDIDATE_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

// std library headers
#include <list>
#include <unordered_map>

namespace VATA {
  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> GetCandidateTree(
      const ExplicitFiniteAut<SymbolType> &aut);
}

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::GetCandidateTree(
    const VATA::ExplicitFiniteAut<SymbolType> &aut) {
 
  typedef ExplicitFiniteAut<SymbolType> ExplicitFA;
  typedef typename ExplicitFA::StateType StateType;

  std::unordered_set<StateType> reachableStates;
  std::list<StateType> newStates;

  auto transitions_ = aut.transitions_;

  ExplicitFA res;


  // Starts from start states
  for (StateType s : aut.GetStartStates()) {
    if (reachableStates.insert(s).second) {
      newStates.push_back(s);
    }
    res.SetExistingStateStart(s,aut.GetStartSymbols(s));
  }

  while (!newStates.empty()) {
    StateType actState = newStates.front();
    // Find transitions for state s
    auto transitionsCluster = transitions_->find(actState);
    
    newStates.pop_front();

    if (transitionsCluster == transitions_->end()) {
      continue;
    }

    for (auto symbolToState : *transitionsCluster->second) {
      // All states which are reachable from actState are added to result automaton
      for (auto stateInSet : *symbolToState.second){
       
        if (reachableStates.insert(stateInSet).second) {
          newStates.push_back(stateInSet);
        }

        res.transitions_->insert(std::make_pair(actState,transitionsCluster->second));

        if (aut.IsStateFinal(stateInSet)) { // Set final state
          res.SetStateFinal(stateInSet);
          goto found_;
        }
      }
    }
  }

found_:

  //TODO optimalizace res.trasitions_=aut.transitions pokud byly napumpovany veschny stavy

  return RemoveUselessStates(res);
}

#endif
