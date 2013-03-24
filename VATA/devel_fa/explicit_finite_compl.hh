#ifndef _VATA_EXPLICIT_FINITE_AUT_COMPL_HH_
#define _VATA_EXPLICIT_FINITE_AUT_COMPL_HH_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

namespace VATA {
  template <class SymbolType, class Dict>
  ExplicitFiniteAut<SymbolType> Complement(
      const ExplicitFiniteAut<SymbolType> &aut,
      const Dict &alphabet);
}

 template <class SymbolType, class Dict>
 VATA::ExplicitFiniteAut<SymbolType> VATA::Complement(
      const VATA::ExplicitFiniteAut<SymbolType> &aut,
      const Dict &alphabet) {

  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  ExplicitFA res;
  res.transitions_ = aut.transitions_;
  res.startStates_ = aut.startStates_;

  auto transitions_ = aut.transitions_;

  // All nonfinal states are marked as final states 
  // in complementary automaton
  for (auto stateToCluster : *transitions_) {
    if (!aut.IsStateFinal(stateToCluster.first)) {
      res.SetStateFinal(stateToCluster.first);
    }
    for (auto symbolToSet : *stateToCluster.second) {
      for (auto stateInSet : *symbolToSet.second) {
        if (!aut.IsStateFinal(stateInSet)) {
          res.SetStateFinal(stateInSet);
        }
      }
    }
  }

  return res;
}

#endif
