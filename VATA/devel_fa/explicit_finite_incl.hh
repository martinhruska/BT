#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include "explicit_finite_aut.hh"
#include "explicit_finite_incl_fctor.hh"

namespace VATA {

  template<class SymbolType, class Rel, class Functor>
  bool CheckFiniteAutInclusion(
    const ExplicitFiniteAut<SymbolType>& smaller, 
    const ExplicitFiniteAut<SymbolType>& bigger, 
    const Rel& preorder);
}

template<class SymbolType, class Rel, class Functor>
bool VATA::CheckFiniteAutInclusion(
  const VATA::ExplicitFiniteAut<SymbolType>& smaller, 
  const VATA::ExplicitFiniteAut<SymbolType>& bigger, 
  const Rel& preorder) {
 
  typedef Functor InclFunc;
  typedef typename InclFunc::ExplicitFA ExplicitFA;

  typedef typename InclFunc::SmallerElementType SmallerElementType;
  typedef typename InclFunc::BiggerElementType BiggerElementType;

  typedef typename InclFunc::ProductStateSetType ProductStateSetType;
  typedef typename InclFunc::Antichain1Type Antichain1Type; 

  typedef typename InclFunc::IndexType IndexType;

  ProductStateSetType antichain;
  ProductStateSetType next;
  Antichain1Type singleAntichain;

  IndexType index;
  IndexType inv;

  preorder.buildIndex(index,inv);

      //std::cout << "Zacinam" <<  std::endl;
  InclFunc inclFunc(antichain,next,singleAntichain,
      smaller,bigger,index,inv,preorder);

  // Initialization of antichain sets from initial states of automata
  inclFunc.Init();
    //std::cout << "Ac size " << antichain.size() << std::endl;

  if (!inclFunc.DoesInclusionHold()) {
    return false;
  }

  // actually processed macro state
  BiggerElementType procMacroState; 
  SmallerElementType procState;

  //std::cout << "Antichainuju to" <<  std::endl;
  while(inclFunc.DoesInclusionHold() && next.get(procState,procMacroState)) {
    static int i = 0;
    std::cerr << "Processed states: " << i++ << std::endl;
    /*
    //std::cout << "New state: " << procState << " Macro: "; for (StateType s : procMacroState) { std::cout << s << " ";}; std::cout << std::endl;
    //std::cout << "Ac size " << antichain.size() << std::endl;
    //std::cout << "Next size " << next.size() << std::endl;
    */
      //std::cout << "Antichainuju to" <<  std::endl;
    inclFunc.MakePost(procState,procMacroState);
  }
    //std::cout << "Ac size " << antichain.size() << std::endl;
  return inclFunc.DoesInclusionHold();
}



#endif
