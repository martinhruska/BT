#ifndef EXPLICIT_FINITE_AUT_CONGR_FCTOR_OPT_
#define EXPLICIT_FINITE_AUT_CONGR_FCTOR_OPT_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include "explicit_finite_aut.hh"
#include "explicit_finite_abstract_fctor.hh"

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFACongrFunctorOpt;
}

template <class SymbolType, class Rel>
class VATA::ExplicitFACongrFunctorOpt : 
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

  typedef std::unordered_map<size_t,StateSet> CongrMap;

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
  ExplicitFACongrFunctorOpt(ProductStateSetType& relation, ProductStateSetType& next,
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

    //std::cout << "Smallerinit state: ";
    //macroPrint(smallerInit);
    //std::cout << "Biggerinit state: ";
    //macroPrint(biggerInit);
    next_.push_back(std::make_pair(StateSet(smallerInit),biggerInit));
    this->inclNotHold_ = smallerInitFinal != biggerInitFinal;
  };

  void MakePost(SmallerElementType& smaller, BiggerElementType& bigger) {
    SymbolSet usedSymbols;

    //std::cout << "Kongruencuji to" <<  std::endl;
    //std::cout << "Novy pruchod, velikost R: " << relation_.size() << std::endl;
    //std::cout << "Novy pruchod, velikost Next: " << next_.size() << std::endl;

    auto areEqual = [] (StateSet& lss, StateSet& rss) -> bool {
      if (lss.size() != rss.size()) {
        return false;
      }
      if (!lss.size() || !rss.size()) {
        return false;
      }
      for (auto& ls : lss) {
        //std:: cout << ls << std::endl;
        if (!rss.count(ls)) {
          return false;
        }
      }

      return true;
    };

    CongrMap congrMap;
    auto insertNewPair = [&congrMap](size_t i, StateSet& set) -> bool {
      congrMap.insert(std::make_pair(i,StateSet(set)));
      return true;
    };
    StateSet congrSmaller(smaller);
    GetCongrClosure(congrSmaller,insertNewPair);

    // Comapring given set with the sets 
    // which has been computed in steps of computation of congr closure
    auto isCongrClosureSetNew = [&congrMap,&areEqual](size_t i, StateSet& set) -> 
      bool {
        return !areEqual(congrMap[i],set);
    };

    StateSet congrBigger(bigger);
    if (GetCongrClosure(congrBigger,isCongrClosureSetNew)) {
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
  bool MatchPair(const StateSet& closure, const StateSet& rule) {
    if (rule.size() > closure.size()) {
        return false;
    }
    for (auto& s : rule) {
      if (!closure.count(s)) {
        return false;
      }
    }
    return true;
  }

  void AddSubSet(StateSet& mainset, StateSet& subset) {
    mainset.insert(subset.begin(),subset.end());
  }

  template<class CongrMapManipulator>
  bool GetCongrClosure(StateSet& set, CongrMapManipulator& congrMap) {
    std::unordered_set<int> usedRulesN;
    std::unordered_set<int> usedRulesR;

    bool appliedRule = true;
    while (appliedRule) {
      appliedRule = false;
      for (unsigned int i=0; i < next_.size(); i++) {

       if (usedRulesN.count(i)) {
         continue;
       }

       if (MatchPair(set, next_[i].first) || 
             MatchPair(set, next_[i].second)) { // Rule matches
         AddSubSet(set,next_[i].first);
         AddSubSet(set,next_[i].second);
         usedRulesN.insert(i);
         appliedRule = true;
         congrMap(i,set);
       }
      }
      for (unsigned int i=0; i < relation_.size(); i++) {

       if (usedRulesR.count(i)) {
         continue;
       }
       if (MatchPair(set, relation_[i].first) || 
             MatchPair(set, relation_[i].second)) { // Rule matches
         AddSubSet(set,relation_[i].first);
         AddSubSet(set,relation_[i].second);
         usedRulesR.insert(i);
         appliedRule = true;
         if (!congrMap(next_.size()+i,set)) {
           return true;
         }
       }
      }
    }
    return false;
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
          //macroPrint(newSmaller);
          //macroPrint(newBigger);
          //std::cout << "NEPLATI" << std::endl;
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
