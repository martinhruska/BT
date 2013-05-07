/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Removing unreachable states for explicitly represented finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_UNREACH_HH_
#define _VATA_EXPLICIT_FINITE_UNREACH_HH_

#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

// Standard library headers
#include <vector>
#include <unordered_set>

namespace VATA {
  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> RemoveUnreachableStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      AutBase::ProductTranslMap* pTranslMap = nullptr);
}


template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::RemoveUnreachableStates(
  const VATA::ExplicitFiniteAut<SymbolType> &aut,
  AutBase::ProductTranslMap* pTranslMap = nullptr) {

  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;
  typedef typename ExplicitFA::StateToTransitionClusterMapPtr 
    StateToTransitionClusterMapPtr;

  // start from start states
  std::unordered_set<AutBase::StateType> reachableStates(aut.GetStartStates());
  std::vector<AutBase::StateType> newStates(reachableStates.begin(),reachableStates.end());

  // Find all reachable states
  while (!newStates.empty()) { // while there are new reacheable states 
    auto actState = newStates.back();

    newStates.pop_back();

    auto cluster = ExplicitFA::genericLookup(*aut.transitions_,actState);

     if (!cluster) {
       continue;
     }

     // Add all reachable states to reachable state set
     for (auto &symbolsToStateSet : *cluster) {
       for (auto &state : symbolsToStateSet.second) {
         if (reachableStates.insert(state).second) {
           newStates.push_back(state);
         }
        }
      }
    }

    ExplicitFA res;
    //initialize the result automaton
    res.startStates_ = aut.startStates_;
    res.startStateToSymbols_ = aut.startStateToSymbols_;
    
    res.transitions_ = StateToTransitionClusterMapPtr(
        new typename ExplicitFA::StateToTransitionClusterMap()
        );

    // Add all reachables states and its transitions to result nfa
    for (auto& state : reachableStates) {

      if (aut.IsStateFinal(state)) {
        res.SetStateFinal(state);
      }

      auto stateToClusterIterator = aut.transitions_->find(state);
      if (stateToClusterIterator == aut.transitions_->end()) {
        continue;
      }

      res.transitions_->insert(std::make_pair(state,stateToClusterIterator->second));
    }

    return res;
  }
#endif
