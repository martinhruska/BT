#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include "explicit_finite_aut.hh"
#include "explicit_finite_incl_fctor.hh"

#include <vector> 

namespace VATA {

  template<class SymbolType, class Rel>
  bool CheckFiniteAutInclusion(
    const ExplicitFiniteAut<SymbolType>& smaller, 
    const ExplicitFiniteAut<SymbolType>& bigger, 
    const Rel& preorder);
}

//TODO zpristupneni i pro simulaci

template<class SymbolType, class Rel>
bool VATA::CheckFiniteAutInclusion(
  const VATA::ExplicitFiniteAut<SymbolType>& smaller, 
  const VATA::ExplicitFiniteAut<SymbolType>& bigger, 
  const Rel& preorder) {
 
  typedef VATA::ExplicitFAInclusionFunctor<SymbolType> InclFunc;
  typedef typename InclFunc::ExplicitFA ExplicitFA;

  typedef typename InclFunc::StateType StateType;
  typedef typename InclFunc::StateSet StateSet;

  typedef typename InclFunc::AntichainType AntichainType;

  // actually processed state
  AntichainType antichain;
  AntichainType next;

  InclFunc inclFunc(antichain,next,smaller,bigger);

  // Initialization of antichain sets from initial states of automata
  inclFunc.Init();

  if (!inclFunc.DoesInclusionHold()) {
    return false;
  }

  bool inclNotHold = false;
  // actually processed macro state
  StateSet procMacroState; 
  StateType procState;

  StateType s;
  for (auto state : smaller.finalStates_) s = state;
  auto printStateSet = [&preorder,&s](StateSet set)-> void {
    for (auto& state : set) {
      std::cout << "Simulation: " << preorder.get(state,s) << " " << s << " " <<  state << std::endl;
      s = state;
    }
      std::cout << std::endl;
  };

  /*
  printStateSet(smaller.startStates_);
  printStateSet(smaller.finalStates_);
  printStateSet(bigger.startStates_);
  printStateSet(bigger.finalStates_);
  */
  
  while(next.get(procState,procMacroState) && inclFunc.DoesInclusionHold()) {
    inclFunc.MakePost(procState,procMacroState);
    procMacroState.clear();
  }
  return inclFunc.DoesInclusionHold();
}



#endif
