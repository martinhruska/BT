#ifndef EXPLICIT_FINITE_AUT_CONGR_FCTOR_
#define EXPLICIT_FINITE_AUT_CONGR_FCTOR_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include "explicit_finite_aut.hh"

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFACongrFunctor;
}

template <class SymbolType, class Rel>
class VATA::ExplicitFACongrFunctor {

public : // data types
  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  typedef typename ExplicitFA::StateType StateType;
  typedef typename ExplicitFA::StateSet StateSet;

  typedef StateType SmallerElementType;
  typedef StateSet BiggerElementType;

  typedef VATA::Util::Antichain2Cv2<SmallerElementType,BiggerElementType> 
    AntichainType;
  typedef VATA::Util::Antichain1C<SmallerElementType> Antichain1Type;

  typedef AntichainType ProductStateSetType;

  typedef typename Rel::IndexType IndexType;

private: // Private data members
  AntichainType& antichain_;
  AntichainType& next_;
  Antichain1Type& singleAntichain_;

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  IndexType& index_;
  IndexType& inv_;

  Rel preorder_; // Simulation or identity

  bool inclNotHold_;

public:
  ExplicitFACongrFunctor(AntichainType& antichain, AntichainType& next,
      Antichain1Type& singleAntichain,
      const ExplicitFA& smaller, 
      const ExplicitFA& bigger,
      IndexType& index,
      IndexType& inv,
      Rel preorder) :
    antichain_(antichain),
    next_(next),
    singleAntichain_(singleAntichain),
    smaller_(smaller),
    bigger_(bigger),
    index_(index),
    inv_(inv),
    preorder_(preorder),
    inclNotHold_(false)
  {}

public: // public functions
  void Init() {
    StateSet smallerInit;
    StateSet biggerInit;

    bool smallerInitFinal = false;
    bool biggerInitFinal = false;

    for (auto state : smaller_.startStates_) {
      smallerInit.insert(state);
      smallerInitFinal |= smaller_.IsStateFinal(state);
    }

    for (auto state : bigger_.startStates_) {
      biggerInit.insert(state);
      biggerInitFinal |= bigger_.IsStateFinal(state);
    }

    inclNotHold_ |= smallerInitFinal != biggerInitFinal;
  };

  void MakePost(SmallerElementType& smaller, BiggerElementType& bigger) {};

public: // public inline functions
  bool DoesInclusionHold() {
    return !inclNotHold_;
  }
};

#endif
