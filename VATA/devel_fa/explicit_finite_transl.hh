#ifndef _VATA_EXPLICIT_FINITE_AUT_TRANSL_
#define _VATA_EXPLICIT_FINITE_AUT_TRANSL_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

#include <vata/explicit_lts.hh>
#include <vata/util/transl_weak.hh>

namespace VATA {
  template <class SymbolType, class Index = Util::IdentityTranslator<AutBase::StateType>>
	ExplicitLTS Translate(const ExplicitFiniteAut<SymbolType>& aut,
		const Index& stateIndex = Index());
}

template <class SymbolType, class Index>
VATA::ExplicitLTS VATA::Translate(
  const VATA::ExplicitFiniteAut<SymbolType>& aut,
  const Index& stateIndex) {

  VATA::ExplicitLTS res;

  // Add all transitions to LTS
  for (auto stateToCluster : *aut.transitions_) { // left state of transition
    assert(stateToCluster.second);
    size_t leftStateTranslated = stateIndex[stateToCluster.first];

    for (auto symbolToSet : *stateToCluster.second) { // symbol of transition
      assert(symbolToSet.second);

      for (auto setState : symbolToSet.second) { // right state of transition
        assert(setState);

        res.addTransition(leftStateTranslated,symbolToSet.first,setState);
      }
    }
  }

  res.init();

  return res;
}

#endif
