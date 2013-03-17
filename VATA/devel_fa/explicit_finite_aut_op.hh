#ifndef _VATA_EXPLICIT_FINITE_AUT_OP_HH_
#define _VATA_EXPLICIT_FINITE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/util.hh>
#include "explicit_finite_aut.hh"

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
      AutBase::ProductTranslMap* pTranslMap = nullptr) {

    typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

    AutBase::ProductTranslMap translMap;


    if (!pTranslMap) {
      pTranslMap = &translMap;
    }

    ExplicitFiniteAut<SymbolType> res;

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

//      std::cout << "Leftstate " << actState->first.first << " " << actState->first.second << " indexes " << actState->second <<  std::endl; //DEBUG
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
//            std::cout << "Righstates " << lstate << " " << rstate << " indexes " << istate.first->second << std::endl; //DEBUG
            
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

}
#endif
