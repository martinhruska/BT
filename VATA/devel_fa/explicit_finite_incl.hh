#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

namespace VATA {

  template<class SymbolType, class Rel>
  bool CheckFiniteAutInclusion(
    const ExplicitFiniteAut<SymbolType>& smaller, 
    const ExplicitFiniteAut<SymbolType>& bigger, 
    const Rel& preorder);
}

template<class SymbolType, class Rel>
bool CheckFiniteAutInclusion(
  const ExplicitFiniteAut<SymbolType>& smaller, 
  const ExplicitFiniteAut<SymbolType>& bigger, 
  const Rel& preorder) {
 
  typedef ExplicitFiniteAut<SymbolType> ExplicitFA;
  typedef ExplicitFA::StateType StateType;


  return true;
}


#endif
