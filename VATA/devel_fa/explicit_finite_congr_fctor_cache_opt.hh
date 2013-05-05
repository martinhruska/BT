#ifndef EXPLICIT_FINITE_AUT_CONGR_FCTOR_CACHE_OPT_
#define EXPLICIT_FINITE_AUT_CONGR_FCTOR_CACHE_OPT_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include "util/map_to_list.hh"
#include "util/macrostate_cache.hh"

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFACongrFunctorCacheOpt;
}

template <class SymbolType, class Rel>
class VATA::ExplicitFACongrFunctorCacheOpt : 
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
  typedef std::pair<SmallerElementType*,BiggerElementType*> ProductState;

  class ProductStateSetType : public std::vector<ProductState> {
    public:
    bool get(SmallerElementType& smaller, BiggerElementType& bigger) {
      if (this->size() == 0) {
        return false;
      }

      auto& nextPair = this->back();
      smaller = *nextPair.first;
      bigger = *nextPair.second;
      this->pop_back();

      return true;
    }
  };
  typedef ProductStateSetType ProductNextType;
  
  typedef typename VATA::MacroStateCache<ExplicitFA> MacroStateCache;
  typedef typename VATA::MapToList<StateSet*,StateSet*> MacroStatePtrPair;

  typedef typename AbstractFunctor::IndexType IndexType;

private: // Private data members
  ProductStateSetType& relation_;
  ProductStateSetType&  next_;
  Antichain1Type& singleAntichain_; // just for compability with the antichain functor

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  IndexType& index_;
  IndexType& inv_;

  Rel preorder_; // Simulation or identity

  MacroStateCache cache_;
  MacroStatePtrPair visitedPairs_;
  MacroStatePtrPair usedRules_;

public:
  ExplicitFACongrFunctorCacheOpt(ProductStateSetType& relation, ProductStateSetType& next,
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
    preorder_(preorder),
    cache_(),
    visitedPairs_(),
    usedRules_()
  {}

public: // public functions

  /*
   * Creates first product state from initial states of both NFA
   */
  void Init() {
    StateSet smallerInit;
    StateSet biggerInit;

    bool smallerInitFinal = false;
    bool biggerInitFinal = false;

    size_t smallerHashNum = 0;
    for (auto state : smaller_.startStates_) {
      smallerHashNum += state; 
      smallerInit.insert(state);
      smallerInitFinal |= smaller_.IsStateFinal(state);
    }

    size_t biggerHashNum = 0;
    for (auto state : bigger_.startStates_) {
      biggerHashNum += state; 
      biggerInit.insert(state);
      biggerInitFinal |= bigger_.IsStateFinal(state);
    }

    //std::cerr << "Smallerinit state: ";
    //macroPrint(smallerInit);
    //std::cerr << "Biggerinit state: ";
    //macroPrint(biggerInit);
    StateSet& insertSmaller = cache_.insert(smallerHashNum,smallerInit);
    StateSet& insertBigger = cache_.insert(biggerHashNum,biggerInit);
    next_.push_back(std::make_pair(&insertSmaller,&insertBigger));
    visitedPairs_.add(&insertSmaller,&insertBigger);
    this->inclNotHold_ = smallerInitFinal != biggerInitFinal;
  };

  void MakePost(SmallerElementType& smaller, BiggerElementType& bigger) {
    SymbolSet usedSymbols;
/*
    std::cerr << "Kongruencuji to" <<  std::endl;
    std::cerr << "Novy pruchod, velikost R: " << relation_.size() << std::endl;
    std::cerr << "Novy pruchod, velikost Next: " << next_.size() << std::endl;
    if (relation_.size() > 0) {
    ////std::cerr << "Jak vypada relation R: "; //macroPrint(*relation_[0].first);
    }
    
*/
    auto isSubSet = [] (StateSet& lss, StateSet& rss) -> bool {
      if (lss.size() > rss.size()) {
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


    auto sum = [](StateSet& set, size_t& sum) {for (auto& state : set) sum+=state;};
    size_t smallerHashNum = 0;
    sum(smaller,smallerHashNum);
    size_t biggerHashNum = 0;
    sum(bigger,biggerHashNum);
    //std::cerr << "Smaller: " ;
    //macroPrint(smaller);
    SmallerElementType& s = cache_.insert(smallerHashNum,smaller);
    //std::cerr << "Bigger: ";
    //macroPrint(bigger);
    BiggerElementType& b = cache_.insert(biggerHashNum,bigger);
    //std::cerr << "Adresa smaller " << &s << std::endl;
    //std::cerr << "Adresa bigger " << &b << std::endl;
   

    // Comapring given set with the sets 
    // which has been computed in steps of computation of congr closure
    auto isCongrClosureSet = [&s,&isSubSet](StateSet& bigger) -> 
      bool {
        return !isSubSet(s,bigger);
    };

    StateSet congrBigger(bigger);
    //std::cerr << "Smaller congr: " ;
    //macroPrint(congrSmaller);
    if (GetCongrClosure(b,congrBigger,isCongrClosureSet) || 
      isSubSet(s,congrBigger)) {
    //std::cerr << "Bigger congr: " ;
    //macroPrint(congrBigger);
      //std::cerr << "Plati!!!!!" << std::endl;
      smaller.clear();
      bigger.clear();
      return;
    }
    //std::cerr << "Bigger congr: " ;
    //macroPrint(congrBigger);
    //std::cerr << "Neplati!!!!!" << std::endl;

    MakePostForAut(smaller_,usedSymbols,smaller,bigger,smaller);
    if (this->inclNotHold_) {
      return;
    }
    MakePostForAut(bigger_,usedSymbols,smaller,bigger,bigger);
    
    relation_.push_back(std::make_pair(&s,&b));

    smaller.clear();
    bigger.clear();
  };

private:

  bool MatchPair(const StateSet& closure, const StateSet& rule) {
  //  ////std::cerr << "matchin rule: ";
    if (rule.size() > closure.size()) {
        return false;
    }
    for (auto& s : rule) {
    //    ////std::cerr << s << " " << std::endl;
      if (!closure.count(s)) {
        return false;
     //////std::cerr << std::endl;
     }
    }
   // ////std::cerr<< "Matched" << std::endl;
    return true;
  }

  void AddSubSet(StateSet& mainset, StateSet& subset) {
    mainset.insert(subset.begin(),subset.end());
  }

  template<class CongrMapManipulator>
  bool ApplyRulesForRelation(StateSet& origSet, StateSet& set, 
    ProductStateSetType& relation, CongrMapManipulator& congrMapManipulator,
    std::unordered_set<int>& usedRulesNumbers,
    bool& appliedRule) {
    bool visited = usedRules_.containsKey(&origSet);

   for (unsigned int i=0; i < relation.size(); i++) { // relation next
     if (usedRulesNumbers.count(i)) { // already used rule
       continue;
     }
/*
     std::cerr << "rel: ";macroPrint(*relation_[i].second);
     if (usedRules_.contains(&origSet,relation_[i].second)) {
      std::cerr << "going: " << std::endl;
      }
      */
     if (MatchPair(set, *relation[i].second)) { // Rule matches
       AddSubSet(set,*relation[i].first);
       AddSubSet(set,*relation[i].second);
       usedRules_.add(&origSet,relation[i].second); 
       usedRulesNumbers.insert(i);
      //std::cerr << "matched: " << std::endl;
       //std::cerr << "Relation: " << std::endl;
       ////std::cerr << "New congr get: " << std::endl;
       //macroPrint(set);
       appliedRule = true;
       if (!congrMapManipulator(set)) {
         return true;
       }
     }
    }
    return false;
  }


  template<class CongrMapManipulator>
  bool ApplyRulesForRelationVisited(StateSet& origSet, StateSet& set, 
    ProductStateSetType& relation, CongrMapManipulator& congrMapManipulator,
    std::unordered_set<int>& usedRulesNumbers,
    bool& appliedRule) {

   for (unsigned int i=0; i < relation.size(); i++) { // relation next
     if (usedRulesNumbers.count(i)) { // already used rule
       continue;
     }
/*
     std::cerr << "rel: ";macroPrint(*relation_[i].second);
     if (usedRules_.contains(&origSet,relation_[i].second)) {
      std::cerr << "going: " << std::endl;
      }
      */
     if (usedRules_.contains(&origSet,relation[i].second) ||
      MatchPair(set, *relation[i].second)) { // Rule matches
       AddSubSet(set,*relation[i].first);
       AddSubSet(set,*relation[i].second);
       usedRulesNumbers.insert(i);
      //std::cerr << "matched: " << std::endl;
       //std::cerr << "Relation: " << std::endl;
       ////std::cerr << "New congr get: " << std::endl;
       //macroPrint(set);
       appliedRule = true;
       if (!congrMapManipulator(set)) {
         return true;
       }
     }
    }
    return false;
  }

  template<class CongrMapManipulator>
  bool GetCongrClosure(StateSet& origSet,StateSet& set, CongrMapManipulator& congrMapManipulator) {
    std::unordered_set<int> usedRulesNumbersN;
    std::unordered_set<int> usedRulesNumbersR;

    bool appliedRule = true;
    bool visited = usedRules_.containsKey(&origSet);

//if (visited) std::cerr << "Visited" << std::endl;
    if (!visited) {
      while (appliedRule) { // Apply all possible rules
        appliedRule = false;
    
        if (ApplyRulesForRelation(
          origSet,set,next_,congrMapManipulator,
          usedRulesNumbersN,appliedRule)) {
          return true;
        }
        if (ApplyRulesForRelation(origSet,set,relation_,
            congrMapManipulator,usedRulesNumbersR,appliedRule)) {
          return true;
        }
      }
    }
    else {
      while (appliedRule) { // Macrostate allready visited
        appliedRule = false;
    
        if (ApplyRulesForRelationVisited(
          origSet,set,next_,congrMapManipulator,usedRulesNumbersN,appliedRule)){
          return true;
        }
        if (ApplyRulesForRelationVisited(
          origSet,set,relation_,congrMapManipulator,usedRulesNumbersR,appliedRule)) {
          return true;
        }
      }
    }
    return false;
  }


  /*
   * Create post of macro state for given automaton
   */
  void MakePostForAut(const ExplicitFA& aut, SymbolSet& usedSymbols,
      const SmallerElementType& smaller, const BiggerElementType& bigger,
      const StateSet& actStateSet) {

        ////std::cerr << "POST" << std::endl;
    for (auto& state : actStateSet) {
      auto transIter = aut.transitions_->find(state);
      if (transIter == aut.transitions_->end()) {
        continue;
      }
      
        ////std::cerr << "KOKOS" << std::endl;
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

       // std::cerr << "false or true " << newSmallerAccept << " " << newBiggerAccpet << std::endl;
        if (newSmallerAccept != newBiggerAccpet) {
          ////macroPrint(newSmaller);
          ////macroPrint(newBigger);
          //////std::cerr << "NEPLATI" << std::endl;
          this->inclNotHold_ = true;
          return;
        }

        ////std::cerr << "PRIDAVAM" << std::endl;
        if (newSmaller.size() || newBigger.size()) {
          size_t smallerHashNum = 0;
          size_t biggerHashNum = 0;
          auto sum = [](StateSet& set, size_t& sum) {for (auto& state : set) sum+=state;};
          sum(newSmaller,smallerHashNum);
          sum(newBigger,biggerHashNum);
          //macroPrint(newSmaller);
          StateSet& insertSmaller = cache_.insert(smallerHashNum,newSmaller);
          //macroPrint(newBigger);
          StateSet& insertBigger = cache_.insert(biggerHashNum,newBigger);
    //std::cerr << "pridavam do next : " << std::endl; 
    //macroPrint(insertSmaller); 
    //macroPrint(insertBigger); 
          if (!visitedPairs_.contains(&insertSmaller,&insertBigger)){ 
    //std::cerr << "nenasek jsem to " << &insertSmaller << " " << &insertBigger << " " << std::endl;
            visitedPairs_.add(&insertSmaller,&insertBigger);
            next_.push_back(std::make_pair(&insertSmaller,&insertBigger));
            // TODO: OPTIMALIZACE
            //next_.insert(next_.begin(),std::make_pair(&insertSmaller,&insertBigger));
           }
           //std::cerr << "proslo" << std::endl;
          ////std::cerr << "pocet stavu pridavanych: " << newSmaller.size() << " "  << insertSmaller.size() << std::endl;
        }

      }
    }
  }
};

#endif
