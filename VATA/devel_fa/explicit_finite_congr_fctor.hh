/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Functor for checking inclusion using congruence algorithm for explicitly 
 *  represented finite automata.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_CONGR_FCTOR_
#define EXPLICIT_FINITE_AUT_CONGR_FCTOR_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include "explicit_finite_aut.hh"
#include "explicit_finite_abstract_fctor.hh"

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFACongrFunctor;
}

template <class SymbolType, class Rel>
class VATA::ExplicitFACongrFunctor : 
  public ExplicitFAAbstractFunctor <SymbolType,Rel> {

public : // data types
  typedef typename VATA::ExplicitFAAbstractFunctor<SymbolType,Rel> 
    AbstractFunctor;
  typedef typename AbstractFunctor::ExplicitFA ExplicitFA;

  typedef typename AbstractFunctor::StateType StateType;
  typedef typename AbstractFunctor::StateSet StateSet;

  typedef typename AbstractFunctor::Antichain1Type Antichain1Type;

  typedef std::unordered_set<SymbolType> SymbolSet;

  typedef StateSet SmallerElementType;
  typedef StateSet BiggerElementType;

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
  typedef ProductStateSetType ProductNextType;

  typedef typename AbstractFunctor::IndexType IndexType;

private: // Private data members
  ProductStateSetType& relation_;
  ProductStateSetType&  next_;
  Antichain1Type& singleAntichain_;

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  IndexType& index_;
  IndexType& inv_;

  Rel preorder_; // Simulation or identity

public:
  ExplicitFACongrFunctor(ProductStateSetType& relation, ProductStateSetType& next,
      Antichain1Type& singleAntichain,
      const ExplicitFA& smaller, 
      const ExplicitFA& bigger,
      IndexType& index,
      IndexType& inv,
      Rel preorder) :
    relation_(relation),
    next_(next),
    singleAntichain_(singleAntichain),
    smaller_(smaller),
    bigger_(bigger),
    index_(index),
    inv_(inv),
    preorder_(preorder)
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
    this->inclNotHold_ = smallerInitFinal != biggerInitFinal;
  };

  void MakePost(SmallerElementType& smaller, BiggerElementType& bigger) {
    SymbolSet usedSymbols;

      auto areEqual = [] (StateSet& lss, StateSet& rss) -> bool {
      if (lss.size() != rss.size()) {
        return false;
      }
      if (!lss.size() || !rss.size()) {
        return false;
      }
      for (auto& ls : lss) {
        if (!rss.count(ls)) {
          return false;
        }
      }

      return true;
    };

    StateSet congrSmaller(smaller);
    GetCongrClosure(congrSmaller);

    StateSet congrBigger(bigger);
    GetCongrClosure(congrBigger);
    
    if (areEqual(congrSmaller,congrBigger)) {
      smaller.clear();
      bigger.clear();
      return;
    }
    MakePostForAut(smaller_,usedSymbols,smaller,bigger,smaller);
    if (this->inclNotHold_) {
      return;
    }
    MakePostForAut(bigger_,usedSymbols,smaller,bigger,bigger);

    relation_.push_back(std::make_pair(smaller,StateSet(bigger)));
    smaller.clear();
    bigger.clear();
  };

private:


  void GetCongrClosure(StateSet& set) {
    auto matchPair = [](const StateSet& closure, const StateSet& rule) -> bool {
      if (rule.size() > closure.size()) {
        return false;
      }
      for (auto& s : rule) {
        if (!closure.count(s)) {
          return false;
        }
      }
      return true;
    };

    auto addSubSet = [](StateSet& mainset, StateSet& subset) -> void {
      mainset.insert(subset.begin(),subset.end());
    };

    std::unordered_set<int> usedRulesN;
    std::unordered_set<int> usedRulesR;

    bool appliedRule = true;
    while (appliedRule) {
      appliedRule = false;
      for (unsigned int i=0; i < next_.size(); i++) {

       if (usedRulesN.count(i)) {
         continue;
       }

       if (matchPair(set, next_[i].first) || 
             matchPair(set, next_[i].second)) { // Rule matches
         addSubSet(set,next_[i].first);
         addSubSet(set,next_[i].second);
         usedRulesN.insert(i);
         appliedRule = true;
       }
      }
      for (unsigned int i=0; i < relation_.size(); i++) {

       if (usedRulesR.count(i)) {
         continue;
       }

       if (matchPair(set, relation_[i].first) || 
             matchPair(set, relation_[i].second)) { // Rule matches
         addSubSet(set,relation_[i].first);
         addSubSet(set,relation_[i].second);
         usedRulesR.insert(i);
         appliedRule = true;
       }
      }

    }
  }

  void MakePostForAut(const ExplicitFA& aut, SymbolSet& usedSymbols,
      const SmallerElementType& smaller, const BiggerElementType& bigger,
      const StateSet& actStateSet) {

    for (auto& state : actStateSet) {
      auto transIter = aut.transitions_->find(state);
      if (transIter == aut.transitions_->end()) {
        continue;
      }
      
      for (auto& symbolToSet : *transIter->second) {
        if (usedSymbols.count(symbolToSet.first)) { // symbol already explored
          continue;
        }

        usedSymbols.insert(symbolToSet.first);
        SmallerElementType newSmaller;
        BiggerElementType newBigger;
        bool newSmallerAccept =  
          this->CreatePostOfMacroState(
              newSmaller,smaller,symbolToSet.first,smaller_);
        bool newBiggerAccpet =
          this->CreatePostOfMacroState(
              newBigger,bigger,symbolToSet.first,bigger_);

        if (newSmallerAccept != newBiggerAccpet) {
          this->inclNotHold_ = true;
          return;
        }

        if (newSmaller.size() || newBigger.size()) {
          next_.push_back(std::make_pair(newSmaller,newBigger));
        }
      }
    }
  }
};

#endif
