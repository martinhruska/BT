#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include "explicit_finite_aut.hh"
#include "explicit_finite_incl_fctor.hh"

namespace VATA {

  template<class SymbolType, class Rel>
  bool CheckFiniteAutInclusion(
    const ExplicitFiniteAut<SymbolType>& smaller, 
    const ExplicitFiniteAut<SymbolType>& bigger, 
    const Rel& preorder);
}

template<class SymbolType, class Rel>
bool VATA::CheckFiniteAutInclusion(
  const VATA::ExplicitFiniteAut<SymbolType>& smaller, 
  const VATA::ExplicitFiniteAut<SymbolType>& bigger, 
  const Rel& preorder) {
 
  typedef VATA::ExplicitFAInclusionFunctor<SymbolType,Rel> InclFunc;
  typedef typename InclFunc::ExplicitFA ExplicitFA;

  typedef typename InclFunc::StateType StateType;
  typedef typename InclFunc::StateSet StateSet;

  typedef typename InclFunc::AntichainType AntichainType;
  typedef typename InclFunc::Antichain1Type Antichain1Type;

  
  typedef typename InclFunc::IndexType IndexType;

  AntichainType antichain;
  AntichainType next;
  Antichain1Type singleAntichain;


  IndexType index;
  IndexType inv;

  InclFunc inclFunc(antichain,next,singleAntichain,
      smaller,bigger,index,inv,preorder);

  preorder.buildIndex(index,inv);

  // Initialization of antichain sets from initial states of automata
  inclFunc.Init();

  if (!inclFunc.DoesInclusionHold()) {
    return false;
  }

  bool inclNotHold = false;
  // actually processed macro state
  StateSet procMacroState; 
  StateType procState;

  while(next.get(procState,procMacroState) && inclFunc.DoesInclusionHold()) {
    inclFunc.MakePost(procState,procMacroState);
    procMacroState.clear();
  }
  return inclFunc.DoesInclusionHold();
}



#endif
