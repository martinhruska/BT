#ifndef _VATA_EXPLICIT_FINITE_AUT_TRANSL_
#define _VATA_EXPLICIT_FINITE_AUT_TRANSL_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

#include <vata/explicit_lts.hh>
#include <vata/util/transl_weak.hh>

namespace VATA {
  template <class SymbolType,
  class Index = Util::IdentityTranslator<AutBase::StateType>>
	  ExplicitLTS Translate(const ExplicitFiniteAut<SymbolType>& aut,
    std::vector<std::vector<size_t>>& partition,
    Util::BinaryRelation& relation,
		const Index& stateIndex = Index());
}

template <class SymbolType, class Index>
VATA::ExplicitLTS VATA::Translate(
  const VATA::ExplicitFiniteAut<SymbolType>& aut,
  std::vector<std::vector<size_t>>& partition,
  VATA::Util::BinaryRelation& relation,
  const Index& stateIndex) {

  VATA::ExplicitLTS res;

  // Add all transitions to LTS
  for (auto stateToCluster : *aut.transitions_) { // left state of transition
    assert(stateToCluster.second);
    size_t leftStateTranslated = stateIndex[stateToCluster.first];

    for (auto symbolToSet : *stateToCluster.second) { // symbol of transition

      for (auto setState : symbolToSet.second) { // right state of transition
        assert(setState);

        res.addTransition(leftStateTranslated,symbolToSet.first,setState);
      }
    }
  }


  std::cout << "tady padam " << res.states() << std::endl;
  /*
  for (size_t i = 0; i < res.states(); ++i)
	  partition[0].push_back(i);
  */
  partition.resize(2);
  partition[0].push_back(0);
  partition[0].push_back(1);
  partition[1].push_back(0);
  partition[1].push_back(1);
  std::cout << "tady taky " << std::endl;

	relation.resize(partition.size());
	relation.reset(false);

	// 0 accepting, 1 non-accepting, 2 .. environments
	relation.set(0, 0, true);

	relation.set(1, 0, true);
	relation.set(1, 1, true);


  res.init();

  return res;
}

#endif
