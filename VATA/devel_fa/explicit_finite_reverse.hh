#ifndef _VATA_EXPLICIT_FINITE_AUT_REVERSE_HH_
#define _VATA_EXPLICIT_FINITE_AUT_REVERSE_HH_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

namespace VATA {
template <class SymbolType>
  ExplicitFiniteAut<SymbolType> Reverse(
    const ExplicitFiniteAut<SymbolType> &aut,
    AutBase::ProductTranslMap* pTranslMap = nullptr); 
}

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::Reverse(
  const VATA::ExplicitFiniteAut<SymbolType> &aut,
  AutBase::ProductTranslMap* pTranslMap = nullptr) {

  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  auto transitions_ = aut.transitions_;

  ExplicitFA res;

  res.finalStates_ = aut.startStates_;
  res.startStates_ = aut.finalStates_;

  // Changing the order of left and right in each transition
  for (auto stateToCluster : *transitions_) {
    for (auto symbolToSet : *stateToCluster.second) {
      for (auto stateInSet : *symbolToSet.second) {
        res.AddTransition(stateInSet,symbolToSet.first,
          stateToCluster.first);
      }
    }
  }

  return res;
}
#endif
