#ifndef _VATA_EXPLICIT_FINITE_ISECT_HH_
#define _VATA_EXPLICIT_FINITE_ISECT_HH_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

// Standard library headers
#include <vector>

namespace VATA {
 template <class SymbolType>
    ExplicitFiniteAut<SymbolType> Intersection(
      const ExplicitFiniteAut<SymbolType> &lhs,
      const ExplicitFiniteAut<SymbolType> &rhs,
      AutBase::ProductTranslMap* pTranslMap = nullptr);
}

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::Intersection(
    const VATA::ExplicitFiniteAut<SymbolType> &lhs,
    const VATA::ExplicitFiniteAut<SymbolType> &rhs,
    AutBase::ProductTranslMap* pTranslMap = nullptr) {

  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  AutBase::ProductTranslMap translMap;


  if (!pTranslMap) {
    pTranslMap = &translMap;
  }

  VATA::ExplicitFiniteAut<SymbolType> res;

  std::vector<const AutBase::ProductTranslMap::value_type*> stack;

  // Loads the final states to the result automaton
  // Auto is here StateType of Explicit fitnite automata
  for (auto lss : lhs.startStates_) {
    for (auto rss : rhs.startStates_) {
      auto iss = pTranslMap->insert(std::make_pair(std::make_pair(lss,rss),
            pTranslMap->size())).first; // intersection start state

      res.SetStateStart(iss->second);

      stack.push_back(&*iss);
    }
  }

  typename ExplicitFA::StateToTransitionClusterMapPtr transitions 
    = res.transitions_;

  while (!stack.empty()) {
    // Element translates states to result
    auto actState = stack.back(); 
    stack.pop_back();
  
    // Checks whether it is final state
    if (lhs.IsStateFinal(actState->first.first)
      && rhs.IsStateFinal(actState->first.second)) {
      res.SetStateFinal(actState->second);
    }

    // Get transition clusters for given state
    //ExplicitFA::TransitionClusterPtr 
    auto lcluster = ExplicitFA::genericLookup 
      (*lhs.transitions_,actState->first.first);

    std::cout << "Leftstate " << actState->first.first << " " << actState->first.second << " indexes " << actState->second <<  std::endl; //DEBUG
    if (!lcluster) {
      continue;
    }

    //ExplicitFA::TransitionClusterPtr 
    auto rcluster = ExplicitFA::genericLookup
      (*rhs.transitions_,actState->first.second);

    if (!rcluster) {
      continue;
    }

    typename ExplicitFA::TransitionClusterPtr clusterptr(nullptr);

    // Go through transitions of the given state
    for (auto lsymbolToPtrPointer : *lcluster) {
      auto lsymbol = lsymbolToPtrPointer.first;

      // Check if there is a transition for the given symbol
      auto rstateSet =  
        ExplicitFA::genericLookup(*rcluster,lsymbol);

      if (!rstateSet) {
        continue;
      }

      if (!clusterptr) { // Insert a new translated state
        clusterptr = transitions->uniqueCluster(actState->second);
      }

      // Insert a new symbol
      auto stateSet = clusterptr->uniqueRStateSet(lsymbol); 

      for (auto lstate : *lsymbolToPtrPointer.second) {
        for (auto rstate : *rstateSet) {
          // Translate to intersection state
          auto istate = pTranslMap->insert
            (std::make_pair(std::make_pair(lstate,rstate), 
                            pTranslMap->size())); 
          std::cout << "Righstates " << lstate << " " << rstate << " indexes " << istate.first->second << std::endl; //DEBUG
          
          // Insert state from right side of transition
          stateSet->insert(istate.first->second);
        
          if (istate.second) { // New states added to stack
           stack.push_back(&*istate.first);
          }
        }
      }
      //std::cout << rstateSet << std::endl;
    }
  }

  /* DEBUG
  for (auto a : *res.transitions_) {
    for (auto b : *a.second) {
      for (auto c : *b.second){
        std::cout << a.first << " " << b.first << " " << c  << std::endl;
      }
    }
  }
*/
  return res;
}
#endif