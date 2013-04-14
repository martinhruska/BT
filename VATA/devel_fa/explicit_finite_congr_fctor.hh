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

    //TODO tady bude provereni kongruencniho uzaveru
    std::cerr << "Smaler: " ;
    macroPrint(smaller);
    StateSet congrSmaller(smaller);
    GetCongrClosure(congrSmaller);
    //std::cout << "Smaler congr: " ;
    //macroPrint(congrSmaller);

    std::cerr << "Bigger: ";
    macroPrint(bigger);
    StateSet congrBigger(bigger);
    GetCongrClosure(congrBigger);
    //std::cout << "Bigger congr: ";
    //macroPrint(congrBigger);
    
    if (areEqual(congrSmaller,congrBigger)) {
      //std::cout << "equal" << std::endl;
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
     // //std::cout << "matchin rule: ";
      if (rule.size() > closure.size()) {
        return false;
      }
      for (auto& s : rule) {
        ////std::cout << s << " " << std::endl;
        if (!closure.count(s)) {
          return false;
     // //std::cout << std::endl;
        }
      }
      return true;
    };

    auto addSubSet = [](StateSet& mainset, StateSet& subset) -> void {
      mainset.insert(subset.begin(),subset.end());
    };

    std::unordered_set<int> usedRulesN;
    std::unordered_set<int> usedRulesR;
    //macroPrint(set);

    bool appliedRule = true;
    while (appliedRule) {
      appliedRule = false;
      for (unsigned int i=0; i < next_.size(); i++) {

       if (usedRulesN.count(i)) {
         continue;
       }

    ////std::cout << "PRAVIDLO: " << rel[i].first.size() << std::endl;
       if (matchPair(set, next_[i].first) || 
             matchPair(set, next_[i].second)) { // Rule matches
     // //std::cout << "PRAVIDLO pridano, kongruenci uzaver: ";
    //   macroPrint(set);
         addSubSet(set,next_[i].first);
         addSubSet(set,next_[i].second);
         usedRulesN.insert(i);
         appliedRule = true;
  //  macroPrint(set);
       }
      // //std::cout << "PRAVIDLO preskoceno:" << std::endl;
      }
      for (unsigned int i=0; i < relation_.size(); i++) {

       if (usedRulesR.count(i)) {
         continue;
       }

    ////std::cout << "PRAVIDLO: " << rel[i].first.size() << std::endl;
       if (matchPair(set, relation_[i].first) || 
             matchPair(set, relation_[i].second)) { // Rule matches
     // //std::cout << "PRAVIDLO pridano, kongruenci uzaver: ";
    //   macroPrint(set);
         addSubSet(set,relation_[i].first);
         addSubSet(set,relation_[i].second);
         usedRulesR.insert(i);
         appliedRule = true;
  //  macroPrint(set);
       }
      // //std::cout << "PRAVIDLO preskoceno:" << std::endl;
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
