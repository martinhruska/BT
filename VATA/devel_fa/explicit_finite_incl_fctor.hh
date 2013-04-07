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
      //TODO overit korektnos podminky
    auto iteratorSmallerSymbolToState = smaller_.transitions_->find(procState);
    if (iteratorSmallerSymbolToState == smaller_.transitions_->end()) {
      return;
    }
    // Iterate through the all symbols in the transitions for the given state
    for (auto& smallerSymbolToState : *(iteratorSmallerSymbolToState->second)) {
      for (const StateType& newSmallerState : smallerSymbolToState.second) {
        StateSet newMacroState;

        bool IsMacroAccepting = this->CreatePostOfMacroState(
            newMacroState,procMacroState,smallerSymbolToState.first);

        inclNotHold_ |= smaller_.IsStateFinal(newSmallerState) && 
          !IsMacroAccepting; 

        if (inclNotHold_) {
          return;
        }

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
      
      // Find transition for given state
      auto iteratorTransForState = bigger_.transitions_->find(stateInMacro); 
      if (iteratorTransForState == bigger_.transitions_->end()) {
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
        res |= bigger_.IsStateFinal(s);
      }
    }
    return res;
  }

  /*
   * Add a new product state to the antichains sets
   */
  void AddNewPairToAntichain(StateType state, StateSet &set) {
    //lss is subset of rss -> return TRUE
    auto comparator = [&preorder_](const StateSet& lss, const StateSet& rss) -> bool {
      bool res = true;

      /*
      std::cout << "Lefstates ";
      for (auto& ls : lss) {
        std::cout << ls << " ";
      }
      std::cout << std::endl;

      std::cout << "rightstates ";
      for (auto& rs : rss) {
        std::cout << rs << " ";
      }
      std::cout << std::endl;
      */
      if (lss.size() > rss.size()) {
        return false;
      }
      for (auto ls : lss) {
        bool tempres = false;
        for (auto rs : rss) {
          if (preorder_.get(ls,rs)) {
            tempres |= true;
            break;
          }
        }
        res &= tempres;
        if (!res) {
          return false;
        }
      }
      //return true;
      return res;
    };
    
    // Rss is subset of lss, returns TRUE
    auto comparator2 = [&preorder_](const StateSet& rss, const StateSet& lss) -> bool {
      bool res = true;

      /*
      std::cout << "Lefstates ";
      for (auto& ls : lss) {
        std::cout << ls << " ";
      }
      std::cout << std::endl;

      std::cout << "rightstates ";
      for (auto& rs : rss) {
        std::cout << rs << " ";
      }
      std::cout << std::endl;
      */
      if (rss.size() > lss.size()) {
        return false;
      }
      for (auto ls : lss) {
        bool tempres = false;
        for (auto rs : rss) {
          if (preorder_.get(ls,rs)) {
            tempres |= true;
            break;
          }
        }
        res &= tempres;
        if (!res) {
          return false;
        }
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
    //  std::cout << "state " << state << std::endl;
    if (!antichain_.contains(tempStateSet,set,comparator)) {
      antichain_.refine(tempStateSet,set,comparator2);
      antichain_.insert(state,set);
      AddToSingleAC(state);
      AddToNext(state,set);
    }
    /*
    else {
      std::cout << "antichains works" << std::endl;
    }
    */
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

  /*
   * Add product state to the next set
   */
  void AddToNext(StateType state, StateSet& set) {
    // Comparator of macro states
    auto comparator = [&preorder_](const StateSet& lss, const StateSet& rss) -> bool {
      bool res = true;
      if (lss.size() > rss.size()) {
        return false;
      }
      for (auto ls : lss) {
        bool tempres = false;
        for (auto rs : rss) {
          if (preorder_.get(ls,rs)) {
            tempres |= true;
            break;
          }
        }
        res &= tempres;
        if (!res) {
          return false;
        }
      }
      return res;
    };

    auto comparator2 = [&preorder_](const StateSet& rss, const StateSet& lss) -> bool {
      bool res = true;     
      
      if (lss.size() > rss.size()) {
        return false;
      }

      for (auto ls : lss) {
        bool tempres = false;
        for (auto rs : rss) {
          if (preorder_.get(ls,rs)) {
            tempres |= true;
            break;
          }
        }
        res &= tempres;
        if (!res) {
          return false;
        }
      }
      return res;
    };

    std::vector<StateType> tempStateSet = {state};
    if (!next_.contains(tempStateSet,set,comparator)) {
      next_.refine(tempStateSet,set,comparator2);
      next_.insert(state,set);
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
