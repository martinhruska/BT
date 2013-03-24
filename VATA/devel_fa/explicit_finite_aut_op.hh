#ifndef _VATA_EXPLICIT_FINITE_AUT_OP_HH_
#define _VATA_EXPLICIT_FINITE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/util.hh>
#include "explicit_finite_aut.hh"
#include "explicit_finite_isect.hh"
#include "explicit_finite_unreach.hh"
#include "explicit_finite_useless.hh"
#include "explicit_finite_reverse.hh"
#include "explicit_finite_compl.hh"
#include "explicit_finite_candidate.hh"

namespace VATA {

  /*
   * Creates union of two automata. It just reindexs
   * existing states of both automata to a new one.
   */
	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> Union(const ExplicitFiniteAut<SymbolType>& lhs,
		const ExplicitFiniteAut<SymbolType>& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr) {


		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;

    /*
     * If the maps are not given
     * it creates own new maps
     */
		AutBase::StateToStateMap translMapLhs;
		AutBase::StateToStateMap translMapRhs;

		if (!pTranslMapLhs) {
			pTranslMapLhs = &translMapLhs;
		}

		if (!pTranslMapRhs) {
			pTranslMapRhs = &translMapRhs;
		}

		StateType stateCnt = 0; 
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);

		ExplicitFiniteAut<SymbolType> res;

    lhs.ReindexStates(res, stateTransLhs);
		rhs.ReindexStates(res, stateTransRhs);


		return res;
	}

 template <class SymbolType>
  ExplicitFiniteAut<SymbolType> Intersection(
      const ExplicitFiniteAut<SymbolType> &lhs,
      const ExplicitFiniteAut<SymbolType> &rhs,
      AutBase::ProductTranslMap* pTranslMap = nullptr);

  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> RemoveUnreachableStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      AutBase::ProductTranslMap* pTranslMap = nullptr);


  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> RemoveUselessStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      AutBase::ProductTranslMap* pTranslMap = nullptr);

  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> Reverse(
    const ExplicitFiniteAut<SymbolType> &aut,
    AutBase::ProductTranslMap* pTranslMap = nullptr);

   template <class SymbolType, class Dict>
   ExplicitFiniteAut<SymbolType> Complement(
      const ExplicitFiniteAut<SymbolType> &aut,
      const Dict &alphabet);

	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> GetCandidateTree(
      const ExplicitFiniteAut<SymbolType>& aut);
}
#endif
