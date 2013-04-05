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
  typedef typename InclFunc::Antichain1Type Antchain1Type;

  
  typedef typename InclFunc::IndexType IndextType;
  typedef typename InclFunc::HeadType HeadType;

  AntichainType antichain;
  AntichainType next;
  Antichain1Type singleAntichain;


  IndexType index;
  HeadType head;

  InclFunc inclFunc(antichain,next,singleAntichain,
      smaller,bigger,index,head,rel);

  preorder.buildIndex(index,head);

  for (auto i : index) {
    std::cout << "Index " << i << std::endl;
  }

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
