#ifndef EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_
#define EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include "explicit_finite_aut.hh"

namespace VATA {
  template <class SymbolType> class ExplicitFAInclusionFunctor;
}

template <class SymbolType>
class VATA::ExplicitFAInclusionFunctor {

public : // data types
  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  typedef typename ExplicitFA::StateType StateType;
  typedef typename ExplicitFA::StateSet StateSet;

  typedef VATA::Util::Antichain2Cv2<StateType,StateSet> AntichainType;

private: // data memebers
  AntichainType& antichain_;
  AntichainType& next_;

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  bool inclNotHold_;

public: // constructor
  ExplicitFAInclusionFunctor(AntichainType& antichain, AntichainType& next,
      const ExplicitFA& smaller, 
      const ExplicitFA& bigger) :
    antichain_(antichain),
    next_(next),
    smaller_(smaller),
    bigger_(bigger),
    inclNotHold_(false)
  {}

public: // public functions

  // Initialize the antichain from init states of given automata
  void Init() {
    bool macroFinal=false;
    StateSet procMacroState; 

    // Create macro state of initial states
    for (StateType startState : bigger_.startStates_) {
      procMacroState.insert(startState);
      macroFinal |= bigger_.IsStateFinal(startState);
    }

    // Check the initial states
    for (StateType smallState : smaller_.startStates_) {
      inclNotHold_ |= smaller_.IsStateFinal(smallState) && !macroFinal;
      this->AddNewPairToAntichain(smallState,procMacroState);
    }
  }

  /*
   * Make post set of the product states
   * for given product state (r,R) and
   * add states to the antichain
   */
  void MakePost(StateType procState, StateSet& procMacroState) {

    // TODO mozna by to slo delat v kodu efektivneji - bez nutnost druheho pruchodu
    auto macroAcceptChecker = [](const ExplicitFA& bigger, StateSet& macroState)->
      bool {
        for (const StateType& state : macroState){
          if (bigger.IsStateFinal(state)) {
            return true; // if one is accepting and whole macrostate is accepting
          }
        }
        return false;
      };

    // Iterate through the all symbols in the transitions for the given state
    for (auto& smallerSymbolToState : *(smaller_.transitions_->
          find(procState)->second)) {
      for (const StateType& newSmallerState : *smallerSymbolToState.second) {
        StateSet newMacroState;

        this->CreatePostOfMacroState(
            newMacroState,procMacroState,smallerSymbolToState.first);
        inclNotHold_ |= smaller_.IsStateFinal(newSmallerState) && 
        !macroAcceptChecker(bigger_,newMacroState);

        this->AddNewPairToAntichain(newSmallerState,newMacroState);
      }
    }

  }

  inline bool DoesInclusionHold() {
    return !inclNotHold_;
  }

private: // private functions

  /*
   * Create a new post macro state of the given 
   * macrostate for given symbol, 
   * which is stored to the given StateSet
   */
  void CreatePostOfMacroState(StateSet& newMacroState,
      StateSet& procMacroState,
      const SymbolType& symbol) {
    // Create new macro state from current macro state for given symbol
    for (const StateType& stateInMacro : procMacroState) {
      this->CopySet(*(bigger_.transitions_->
          find(stateInMacro)->second-> // Transition for given state
          find(symbol)->second), // States for given symbol
          newMacroState); 
    } // end creating new macro state
 }

  /*
   * Copy one set to another 
   */
  inline void CopySet(StateSet& fullset, StateSet& emptyset) {
    emptyset.insert(fullset.begin(),fullset.end());
  }

  /*
   * Add product state to the next set
   */
  void AddToNext (StateType state, StateSet &set) {

    // Comparator of macro states
    auto comparator = [](const StateSet& lhs, const StateSet& rhs) { 
      return lhs.IsSubsetOf(rhs); };

    std::vector<StateType> tempStateSet = {state};
    if (!next_.contains(tempStateSet,set,comparator)) {
      next_.refine(tempStateSet,set,comparator);
      next_.insert(state,set);
    }
  }

  void AddNewPairToAntichain(StateType state, StateSet &set) {
    // Comparator of macro states
    auto comparator = [](const StateSet& lhs, const StateSet& rhs) { 
      return lhs.IsSubsetOf(rhs); };

    std::vector<StateType> tempStateSet = {state};
    if (!antichain_.contains(tempStateSet,set,comparator)) {
      antichain_.refine(tempStateSet,set,comparator);
      antichain_.insert(state,set);
      AddToNext(state,set);
    }
  }

};

#endif
