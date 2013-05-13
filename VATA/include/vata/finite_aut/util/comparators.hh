/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Header file for comparators of macrostates using simulation
 *  or identity on finite automata.
 *
 *****************************************************************************/

#ifndef EXPLICIT_FINITE_AUT_INCL_COMPARATOR_HH__
#define EXPLICIT_FINITE_AUT_INCL_COMPARATOR_HH__

// std headers
#include <vector>

// vata headers
#include <vata/vata.hh>
#include <vata/util/antichain1c.hh>
#include "../explicit_finite_aut.hh"

namespace VATA {
  template <class SymbolType,class Rel> class ExplicitFAStateSetComparatorIdentity;
  template <class SymbolType,class Rel> class ExplicitFAStateSetComparatorSimulation;
}

/*
 * Class for comparing P <= Q during inclusion checking for
 * antichain algorithm
 */
template<class SymbolType, class Rel>
class VATA::ExplicitFAStateSetComparatorIdentity {

public:
  typedef ExplicitFiniteAut<SymbolType> ExplicitFA;
  typedef typename ExplicitFA::StateType StateType;
  typedef typename ExplicitFA::StateSet StateSet;
  typedef VATA::Util::Antichain1C<StateType> Antichain1Type;
private: // private data members
  Rel preorder_;

public:
  ExplicitFAStateSetComparatorIdentity(Rel preorder) : preorder_(preorder) {}
public: // public methods
  // lss is subset of rss
  inline bool lte(const StateSet& lss, const StateSet& rss) {
      bool res = true;

      if (lss.size() > rss.size()) {
      // komparator ktery zohledni tuhle podminku a jeste taky podminku p=<P - v nesimulaci bude true/false(?) vzddycky
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
  }

  // rss is subset of lss
  inline bool gte(const StateSet& lss, const StateSet& rss) {
    return lte(rss,lss);
  }

  /*
   * Get candidates for p <= q
   */
  inline void getCandidate(std::vector<StateType>& candidates, StateType state,
      Antichain1Type& antichain) {
    candidates.push_back(state);
  }
};

/*
 * Class for comparing macrostates during inclusion checking
 * using simulation
 */
template<class SymbolType, class Rel>
class VATA::ExplicitFAStateSetComparatorSimulation {

public:
  typedef ExplicitFiniteAut<SymbolType> ExplicitFA;
  typedef typename ExplicitFA::StateSet StateSet;
  typedef typename ExplicitFA::StateType StateType;
  typedef VATA::Util::Antichain1C<StateType> Antichain1Type;

private: // private data members
  Rel preorder_;

public:
  ExplicitFAStateSetComparatorSimulation(Rel preorder) : preorder_(preorder) {}
public: // public methods
  // lss is subset of rss
  inline bool lte(const StateSet& lss, const StateSet& rss) {
      bool res = true;

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
  }

  // rss is subset of lss
  inline bool gte(const StateSet& lss, const StateSet& rss) {
    return lte(rss,lss);
  }

  inline void getCandidate(std::vector<StateType>& candidates, StateType state,
      Antichain1Type& antichain) {
    for (StateType candidate : antichain.data()) {
      if (preorder_.get(candidate,state)) {
        candidates.push_back(candidate);
      }
    }
  }
};



#endif
