#ifndef _VATA_EXPLICIT_FINITE_AUT_OP_HH_
#define _VATA_EXPLICIT_FINITE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/util.hh>
#include <vata/util/binary_relation.hh>

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
  ExplicitFiniteAut<SymbolType> UnionDisjunctStates(
      const ExplicitFiniteAut<SymbolType> &lhs,
      const ExplicitFiniteAut<SymbolType> &rhs) {

        ExplicitFiniteAut<SymbolType> res(lhs);

        // Use uniqueCluster function, not explicitly transitions_,
        // because of the possibility of the need of the creating of the
        // new clusterMap
        res.uniqueClusterMap()->insert(rhs.transitions_->begin(),rhs.transitions_->end());
        
        res.startStates_->insert(rhs.startStates_->begin(),rhs.startStates_->end());
        res.finalStates_->insert(rhs.finalStates_->begin(),rhs.finalStates_->end());
        return res;
      }

  template <class SymbolType, 
    class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>>
  ExplicitFiniteAut<SymbolType> CollapseStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      const Rel & rel,
      const Index &index = Index()) {

      std::vector<size_t> representatives;

      // Creates vector, which contains equivalences classes of states of 
      // aut automaton
      // If relation is identity, newly created automaton will be same
      rel.buildClasses(representatives);

   		std::vector<AutBase::StateType> rebinded(representatives.size());

      // Transl will contain new numbers (indexes) for input states
  		Util::RebindMap2(rebinded, representatives, index);

	  	ExplicitFiniteAut<SymbolType> res;

		  aut.ReindexStates(res, rebinded);

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


  /*
   * Simulation functions
   */
	template <class SymbolType, class Index>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size, const Index& index) {

    AutBase::StateBinaryRelation rel;
    return rel;
		//return ExplicitTreeAut::TranslateDownward(aut, index).computeSimulation(size);

	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size) {
    AutBase::StateBinaryRelation rel;
    return rel;
		//return ExplicitTreeAut::TranslateDownward(aut).computeSimulation(size);
	}

  // Automaton has not been sanitized
  template <class SymbolType>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut) {

		return ComputeDownwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));
	}

  /*
   * Upward simulation just for compability
   */
  template <class SymbolType, class Index>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size, const Index& index) {

	  return ComputeDownwardSimulation(aut, size);
	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size) {

	 return ComputeDownwardSimulation(aut, size);
	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut) {

		return ComputeDownwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));

	}

}
#endif
