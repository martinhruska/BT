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

  typedef StateSet SmallerElementType;
  typedef StateSet BiggerElementType;

  typedef VATA::Util::Antichain2Cv2<SmallerElementType,BiggerElementType> 
    AntichainType;
  typedef VATA::Util::Antichain1C<SmallerElementType> Antichain1Type;

  //typedef AntichainType ProductStateSetType;


  class ProductStateSetType : public std::vector<std::pair<SmallerElementType,BiggerElementType>> {
    public:
    bool get(SmallerElementType& smaller, BiggerElementType& bigger) {
      if (this->size() == 0) {
        return false;
      }

      auto& nextPair = this->back();
      smaller = nextPair.first;
      bigger = nextPair.second;
      this->pop_back();

      return true;
    }
  };

  typedef typename Rel::IndexType IndexType;

private: // Private data members
  ProductStateSetType& antichain_;
//  AntichainType& next_;
  ProductStateSetType&  next_;
  Antichain1Type& singleAntichain_;

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  IndexType& index_;
  IndexType& inv_;

  Rel preorder_; // Simulation or identity

  bool inclNotHold_;

public:
  ExplicitFACongrFunctor(ProductStateSetType& antichain, ProductStateSetType& next,
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

    next_.push_back(std::make_pair(StateSet(smallerInit),biggerInit));
    inclNotHold_ = smallerInitFinal != biggerInitFinal;
  };

  void MakePost(SmallerElementType& smaller, BiggerElementType& bigger) {
    std::unordered_set<SymbolType> usedSymbols;

    SmallerElementType newSmaller;
    SmallerElementType newBigger;

    //TODO tady bude provereni kongruencniho uzaveru

    for (auto& trans : *smaller_.transitions_) {
      for (auto& symbolToSet : *trans.second) {
        if (usedSymbols.count(symbolToSet.first)) { // symbol already explored
          continue;
        }

        bool newSmallerAccept =  
          this->CreatePostOfMacroState(newSmaller,smaller,symbolToSet.first,smaller_);
        bool newBiggerAccpet =
          this->CreatePostOfMacroState(newSmaller,bigger,symbolToSet.first,bigger_);

        if (newSmallerAccept != newBiggerAccpet) {
          inclNotHold_ = true;
          return;
        }

      }
    }
  };

private:
  /*
   * Create a new post macro state of the given 
   * macrostate for given symbol, 
   * which is stored to the given StateSet.
   * @Return True if created macrostates is final in bigger NFA
   */
  bool CreatePostOfMacroState(StateSet& newMacroState,
      StateSet& procMacroState, const SymbolType& symbol, const ExplicitFA& macroFA) {

    bool res = false;
    // Create new macro state from current macro state for given symbol
    for (const StateType& stateInMacro : procMacroState) {
      
      // Find transition for given state
      auto iteratorTransForState = macroFA.transitions_->find(stateInMacro); 
      if (iteratorTransForState == macroFA.transitions_->end()) {
        continue;
      }
      auto transForState = iteratorTransForState->second;

      // States for given symbol
      auto symbolToStateSet = transForState->find(symbol);
      if (symbolToStateSet == transForState->end()) {
        continue;
      }

      for (auto& s : symbolToStateSet->second) {
        newMacroState.insert(s);
        res |= macroFA.IsStateFinal(s);
      }
    }
    return res;
  }

public: // public inline functions
  bool DoesInclusionHold() {
    return !inclNotHold_;
  }
};

#endif
