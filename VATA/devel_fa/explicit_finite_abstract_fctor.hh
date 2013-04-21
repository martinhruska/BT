#ifndef EXPLICIT_FINITE_AUT_ABSTRACT_FCTOR_HH_
#define EXPLICIT_FINITE_AUT_ABSTRACT_FCTOR_HH_

#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFAAbstractFunctor;
}


template <class SymbolType, class Rel>
class VATA::ExplicitFAAbstractFunctor {
public: // data types
  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  typedef typename ExplicitFA::StateType StateType;
  typedef typename ExplicitFA::StateSet StateSet;

  typedef VATA::Util::Antichain1C<StateType> Antichain1Type;

  typedef typename Rel::IndexType IndexType;

protected: // data memebers
  bool inclNotHold_;

public:
  ExplicitFAAbstractFunctor() : inclNotHold_(false) {}

protected:
  /*
   * Create a new post macro state of the given 
   * macrostate for given symbol. 
   * New macrostate is stored to the given StateSet.
   * @Return True if created macrostates is final in bigger NFA
   */
  bool CreatePostOfMacroState(StateSet& newMacroState,
      const StateSet& procMacroState, const SymbolType& symbol, 
      const ExplicitFA& macroFA) {

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

  bool CreatePostOfMacroStateWithSum(StateSet& newMacroState,
      const StateSet& procMacroState, const SymbolType& symbol, 
      const ExplicitFA& macroFA, size_t& sum) {

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
        sum += 31*sum + s;
        res |= macroFA.IsStateFinal(s);
      }
    }
    return res;
  }

  void macroPrint(StateSet& set) {
      for (auto& s : set) {
        std::cerr << s << " ";
      }
      std::cerr << std::endl;
  }

public: // Public inline functions
  inline bool DoesInclusionHold() {
    return !inclNotHold_;
  }


};

#endif
