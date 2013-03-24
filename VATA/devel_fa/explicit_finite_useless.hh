#ifndef _VATA_EXPLICIT_FINITE_AUT_USELESS_HH_
#define _VATA_EXPLICIT_FINITE_AUT_USELESS_HH_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"
#include "explicit_finite_reverse.hh"
#include "explicit_finite_unreach.hh"

namespace VATA {
  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> RemoveUselessStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      AutBase::ProductTranslMap* pTranslMap = nullptr);
}

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::RemoveUselessStates(
    const VATA::ExplicitFiniteAut<SymbolType> &aut,
    AutBase::ProductTranslMap* pTranslMap = nullptr) {

  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  return Reverse(
     RemoveUnreachableStates(
      Reverse(
        RemoveUnreachableStates(aut)
      )
     )
    );
}

#endif
