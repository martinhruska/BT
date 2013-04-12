#ifndef EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_
#define EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/util/antichain1c.hh>
#include "explicit_finite_aut.hh"

#include "explicit_finite_abstract_fctor.hh"

// standard libraries
#include <vector>

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFAInclusionFunctor;
}

template <class SymbolType, class Rel>
class VATA::ExplicitFAInclusionFunctor : 
  public ExplicitFAAbstractFunctor <SymbolType,Rel> {

public : // data types
  typedef typename VATA::ExplicitFAAbstractFunctor<SymbolType,Rel> AbstractFunctor;
  typedef typename AbstractFunctor::ExplicitFA ExplicitFA;

  typedef typename AbstractFunctor::StateType StateType;
  typedef typename AbstractFunctor::StateSet StateSet;

  typedef typename AbstractFunctor::Antichain1Type Antichain1Type;

  typedef StateType SmallerElementType;
  typedef StateSet BiggerElementType;

  typedef VATA::Util::Antichain2Cv2<SmallerElementType,BiggerElementType> 
    AntichainType;

  typedef AntichainType ProductStateSetType;

  typedef bool(*Comparator)(const StateSet&, const StateSet&);
  typedef typename AbstractFunctor::IndexType IndexType;

private: // data memebers
  AntichainType& antichain_;
  AntichainType& next_;
  Antichain1Type& singleAntichain_;

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  IndexType& index_;
  IndexType& inv_;

  Rel preorder_; // Simulation or identity

  Comparator lte = &ExplicitFAInclusionFunctor<SymbolType,Rel>::comparatorLTE; 
  Comparator gte = &ExplicitFAInclusionFunctor<SymbolType,Rel>::comparatorGTE; 

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
    lte(),
    gte()
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
      this->inclNotHold_ |= smaller_.IsStateFinal(smallState) && !macroFinal;
      this->AddNewPairToAntichain(smallState,procMacroState);
    }
  }

  /*
   * Make post set of the product states
   * for given product state (r,R) and
   * add states to the antichain
   */
  void MakePost(StateType procState, StateSet& procMacroState) {
      //std::cout << "Antichainuju to" <<  std::endl;
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
            newMacroState,procMacroState,smallerSymbolToState.first,
            bigger_);

        this->inclNotHold_ |= smaller_.IsStateFinal(newSmallerState) && 
          !IsMacroAccepting; 

        if (this->inclNotHold_) {
          return;
        }

        //TODO dodelat podminku p <= p', p \in  P,
        this->AddNewPairToAntichain(newSmallerState,newMacroState);
      }
    }
    procMacroState.clear();
  }

private: // private functions
  /*
   * Add a new product state to the antichains sets
   */
  void AddNewPairToAntichain(StateType state, StateSet &set) {

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

    if (!antichain_.contains(tempStateSet,set,lte)) {
      antichain_.refine(tempStateSet,set,gte);
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
    std::vector<StateType> tempStateSet = {state};
    if (!next_.contains(tempStateSet,set,lte)) {
      next_.refine(tempStateSet,set,gte);
      next_.insert(state,set);
    }
  }

public:
  /*
   * lss is subset of rss
   */
  bool comparatorLTE (const StateSet& lss, const StateSet& rss) {
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
  
  /*
   * rss is subset of lss
   */
  bool comparatorGTE (const StateSet& lss, const StateSet& rss) {
    return comparatorLTE(rss,lss);
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
