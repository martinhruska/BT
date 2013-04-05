#ifndef EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_
#define EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/util/antichain1c.hh>
#include "explicit_finite_aut.hh"

// standard libraries
#include <vector>

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFAInclusionFunctor;
}

template <class SymbolType, class Rel>
class VATA::ExplicitFAInclusionFunctor {

public : // data types
  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  typedef typename ExplicitFA::StateType StateType;
  typedef typename ExplicitFA::StateSet StateSet;

  typedef VATA::Util::Antichain2Cv2<StateType,StateSet> AntichainType;
  typedef VATA::Util::Antichain1C<StateType> Antichain1Type;

  typedef typename Rel::IndexType IndexType;

private: // data memebers
  AntichainType& antichain_;
  AntichainType& next_;
  Antichain1Type& singleAntichain_;

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  IndexType& index_;
  IndexType& inv_;

  Rel preorder_; // Simulation or identity

  bool inclNotHold_;

public: // constructor
  ExplicitFAInclusionFunctor(AntichainType& antichain, AntichainType& next,
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

    /*
    auto macroAcceptChecker = [](const ExplicitFA& bigger, StateSet& macroState)->
      bool {
//        bool res=false;
        for (const StateType& state : macroState){
//          res |= bigger.IsStateFinal(state);
          if (bigger.IsStateFinal(state)) {
            return true; // if one is accepting and whole macrostate is accepting
          }
        }
        return false;
//OPT        return res;
      };
*/

    // Iterate through the all symbols in the transitions for the given state
    for (auto& smallerSymbolToState : *(smaller_.transitions_->
          find(procState)->second)) {
      for (const StateType& newSmallerState : smallerSymbolToState.second) {
        StateSet newMacroState;

        bool IsMacroAccepting = this->CreatePostOfMacroState(
            newMacroState,procMacroState,smallerSymbolToState.first);

        inclNotHold_ |= smaller_.IsStateFinal(newSmallerState) && 
          !IsMacroAccepting; 
//          !macroAcceptChecker(bigger_,newMacroState); 

        this->AddNewPairToAntichain(newSmallerState,newMacroState);
      }
    }

  }

public: // Public inline functions
  inline bool DoesInclusionHold() {
    return !inclNotHold_;
  }

private: // private functions

  /*
   * Create a new post macro state of the given 
   * macrostate for given symbol, 
   * which is stored to the given StateSet.
   * @Return True if created macrostates is final in bigger NFA
   */
  bool CreatePostOfMacroState(StateSet& newMacroState,
      StateSet& procMacroState, const SymbolType& symbol) {

    bool res = false;
    // Create new macro state from current macro state for given symbol
    for (const StateType& stateInMacro : procMacroState) {
      
      auto transForState = bigger_.transitions_->
          find(stateInMacro)->second; // Transition for given state
      assert(transForState != bigger_.transitions_->end());

      // States for given symbol
      auto symbolToStateSet = transForState->find(symbol);
      if (symbolToStateSet == transForState->end()) {
        continue;
      }

      for (auto& s : symbolToStateSet->second) {
        newMacroState.insert(s);
        res |= bigger_.IsStateFinal(s);
      }
      /* OPT
      this->CopySet(*(symbolToStateSet->second), 
          newMacroState); 
    } // end creating new macro state
          */
    }
    return res;
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

  /*
   * Add a new product state to the antichains sets
   */
  void AddNewPairToAntichain(StateType state, StateSet &set) {
    // Comparator of macro states
    auto comparator = [&preorder_](const StateSet& lss, const StateSet& rss) -> bool {
      bool res = true;
      for (auto ls : lss) {
        bool tempres = false;
        for (auto rs : rss) {
          //tempres |= preorder_.get(ls,rs);
          if (preorder_.get(ls,rs)) {
            tempres |= true;
            break;
          }
        }
        res &= tempres;
        /*
        if (!res) {
          return false;
        }
        */
      }
      //return true;
      return res;
    };
    

   
    // Get candidates for given state
    std::vector<StateType> candidates;
    for (StateType candidate : singleAntichain_.data()) {
      if (preorder_.get(candidate,state)) {
        candidates.push_back(candidate);
      }
    }

    // Check whether the antichain does not already 
    // contains given product state
    std::vector<StateType> tempStateSet = {state};
    if (!antichain_.contains(tempStateSet,set,comparator)) {
      antichain_.refine(tempStateSet,set,comparator);
      antichain_.insert(state,set);
      AddToSingleAC(state);
      AddToNext(state,set);
    }
  }

  /*
   * Add state to the small antichain
   */
  void AddToSingleAC(StateType state) {
    std::vector<StateType> tempStateSet = {state};
    if (!singleAntichain_.contains(tempStateSet)) {
      singleAntichain_.insert(state);
    }
  }

private: // Private inline functions
  /*
   * Copy one set to another 
   */
  inline void CopySet(StateSet& fullset, StateSet& emptyset) {
    emptyset.insert(fullset.begin(),fullset.end());
  }

};

#endif
