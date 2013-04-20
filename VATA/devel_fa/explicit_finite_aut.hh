#ifndef _VATA_EXPLICIT_FINITE_AUT_HH_
#define _VATA_EXPLICIT_FINITE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <unordered_set>
#include <iostream>
#include <vata/util/transl_weak.hh>
#include <vata/explicit_lts.hh>

namespace VATA {
	template <class Symbol>	class ExplicitFiniteAut;
}

/*
 * There is no template needed, because
 * symbol is not ranked
 */
template <class Symbol>
class VATA::ExplicitFiniteAut : public AutBase {

  // Need access to class members, so the functions is made friend
  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> Union(
		const ExplicitFiniteAut<SymbolType>&, const ExplicitFiniteAut<SymbolType>&,
		AutBase::StateToStateMap*, AutBase::StateToStateMap*);

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> UnionDisjunctStates(
      const ExplicitFiniteAut<SymbolType>&,
      const ExplicitFiniteAut<SymbolType>&);

  template <class SymbolType>
	friend ExplicitFiniteAut<SymbolType> Intersection(
		const ExplicitFiniteAut<SymbolType> &, 
    const ExplicitFiniteAut<SymbolType> &,
		AutBase::ProductTranslMap*);

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> RemoveUnreachableStates(
      const ExplicitFiniteAut<SymbolType> &,
      AutBase::ProductTranslMap*);

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> RemoveUselessStates(
      const ExplicitFiniteAut<SymbolType> &,
      AutBase::ProductTranslMap*);

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> Reverse(
      const ExplicitFiniteAut<SymbolType> &,
      AutBase::ProductTranslMap*);

  template <class SymbolType, class Dict>
  friend ExplicitFiniteAut<SymbolType> Complement(
      const ExplicitFiniteAut<SymbolType> &,
      const Dict &);
	
  template <class SymbolType>
	friend ExplicitFiniteAut<SymbolType> GetCandidateTree(
      const ExplicitFiniteAut<SymbolType> &);

  template <class SymbolType, class Rel, class Index>
  friend ExplicitFiniteAut<SymbolType> CollapseStates(
      const ExplicitFiniteAut<SymbolType> &,
      const Rel &,
      const Index &);

  // Translator for simulation
  template <class SymbolType, class Index>
	friend ExplicitLTS Translate(const ExplicitFiniteAut<SymbolType>& aut,
    std::vector<std::vector<size_t>>& partition,
    Util::BinaryRelation& relation,
		const Index& stateIndex);

  // Checking inclusion
  template <class SymbolType>
  friend bool CheckInclusion(
       const ExplicitFiniteAut<SymbolType> smaller,
       const ExplicitFiniteAut<SymbolType> bigger);

  // Checking inclusion
  template<class SymbolType, class Rel, class Functor>
  friend bool CheckFiniteAutInclusion (
    const ExplicitFiniteAut<SymbolType>& smaller, 
    const ExplicitFiniteAut<SymbolType>& bigger, 
    const Rel& preorder); 

  template<class SymbolType, class Rel>
  friend class ExplicitFAAbstractFunctor;

  template<class SymbolType, class Rel>
  friend class ExplicitFAInclusionFunctor;

  template<class SymbolType, class Rel>
  friend class ExplicitFACongrFunctor;
  template<class SymbolType, class Rel>
  friend class ExplicitFACongrFunctorOpt;
  template<class SymbolType, class Rel>
  friend class ExplicitFACongrFunctorCache;
  template<class SymbolType, class Rel>
  friend class ExplicitFACongrFunctorCacheOpt;

  template<class SymbolType, class Rel>
  friend class ExplicitFAStateSetComparator;
  
  template<class Aut>
  friend class MacroStatePtrPair;
  template<class Aut>
  friend class MacroStateCache;

public:
	typedef Symbol SymbolType;

private: // private type definitions
	typedef AutBase::StateType StateType;
	typedef std::vector<SymbolType> AlphabetType;

	typedef std::string string;

  // Added subset operation to unordered_set
	class StateSet : public std::unordered_set<StateType>  {
  public:
    bool IsSubsetOf(const StateSet& rhs) const {
      for (StateType state : *this) {
        if (!rhs.count(state)) { // counterexample found
         return false;
        }
       }
      return true;
    }
  };
	typedef VATA::Util::AutDescription AutDescription;
	typedef AutDescription::State State;

  typedef std::unordered_set<SymbolType> SymbolSet;
  typedef std::unordered_map<StateType,SymbolSet> StateToSymbols;

	//typedef std::shared_ptr<const StateType> RStatePtr;
  /*
  typedef StateType RStatePtr;

	typedef std::set<RStatePtr> RStatePtrSet;
	typedef std::shared_ptr<RStatePtrSet> RStatePtrSetPtr;

	class TransitionCluster : public std::unordered_map<SymbolType,RStatePtrSetPtr>{
	public:
		const RStatePtrSetPtr uniqueRStatePtrSet(const SymbolType &symbol){
			auto &rStateSet =  this->insert(
			std::make_pair(symbol,RStatePtrSetPtr(nullptr))
				).first->second;

			if (!rStateSet){
				rStateSet = RStatePtrSetPtr(new RStatePtrSet);
			}
      else if (!rStateSet.unique()){
				rStateSet = RStatePtrSetPtr(new RStatePtrSet(*rStateSet));
      }
			return rStateSet;
		}
	}; 
*/

  // The states on the right side of transitions
  typedef StateSet RStateSet;
	typedef std::shared_ptr<RStateSet> RStateSetPtr;

	class TransitionCluster : public std::unordered_map<SymbolType,RStateSet>{
	public:
		 RStateSet& uniqueRStateSet(const SymbolType &symbol){
      auto iter = this->find(symbol);
      if (iter == this->end()) {
        return this->insert(std::make_pair(symbol, RStateSet())).first->second;
      }
      else {
       return iter->second;
      }
      /*
			RStateSetPtr &rstateSetPtr =  this->insert(
			std::make_pair(symbol,RStateSetPtr(nullptr))
				).first->second;

			if (!rstateSetPtr){
				rstateSetPtr = RStateSetPtr(new RStateSet);
			}
      else if (!rstateSetPtr.unique()){
				rstateSetPtr = RStateSetPtr(new RStateSet(*rstateSetPtr));
      }
			return rstateSetPtr;
      */
		}
	};

	typedef std::shared_ptr<TransitionCluster> TransitionClusterPtr; // TODO dodelat pointer na cluster

	class StateToTransitionClusterMap : public std::unordered_map<StateType,TransitionClusterPtr>{
	public:
		const TransitionClusterPtr &uniqueCluster(const StateType &state){
			auto& clusterPtr = this->insert(
				std::make_pair(state,TransitionClusterPtr(nullptr))
					).first->second;
				if (!clusterPtr){
					clusterPtr = TransitionClusterPtr(new TransitionCluster());	
				}
			  else if (!clusterPtr.unique()) {
					clusterPtr = TransitionClusterPtr(new TransitionCluster(*clusterPtr));	
        }
				return clusterPtr;
		}							
	};
	
	typedef std::shared_ptr<StateToTransitionClusterMap> StateToTransitionClusterMapPtr;

public: //pubic type definitions
	typedef VATA::Util::TwoWayDict<string, SymbolType> StringToSymbolDict;
	typedef VATA::Util::TranslatorStrict<typename StringToSymbolDict::MapBwdType>
		SymbolBackTranslatorStrict;

protected: // private data memebers
	StateSet finalStates_;
	StateSet startStates_;
  // Transitions defining start states
  StateToSymbols startStateToSymbols_;

private: // private static data memebers
	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;
	StateToTransitionClusterMapPtr transitions_;
	
public:

	ExplicitFiniteAut() :
		finalStates_(),
		startStates_(),
		startStateToSymbols_(),
		transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap))
	{ }

  ExplicitFiniteAut(const ExplicitFiniteAut& aut) :
    finalStates_(aut.finalStates_),
		startStates_(aut.startStates_),
		startStateToSymbols_(aut.startStateToSymbols_),
		transitions_(aut.transitions_) 
  { }


	~ExplicitFiniteAut() {}

	void devel(VATA::Parsing::AbstrParser& parser, 
		const std::string& str, StringToStateDict& stateDict)
	{
		LoadFromString(parser,str, stateDict);
    /* DEBUG
		std::cerr << "Vata finite automata development begins" << std::endl;
    */
	}

	/*
	 ** Function load automaton to the intern representation 
	 ** from the string.
	 ** It translates from string to the automaton descrtiption
	 ** data structure and the calls another function.
	 **
	 */
	void LoadFromString(VATA::Parsing::AbstrParser& parser,
	 const std::string& str, StringToStateDict& stateDict)
	{
		LoadFromAutDesc(parser.ParseString(str), stateDict);
	}

	void LoadFromAutDesc(const AutDescription& desc, StringToStateDict& stateDict)
	{
	
		
		typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
			StateTranslator; // Translatoror for states
		typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
			SymbolTranslator;// Translatoror for symbols 

		StateType stateCnt = 0;
		SymbolType symbolCnt = 0;

		LoadFromAutDesc(desc,
			StateTranslator(stateDict,
				[&stateCnt](const std::string&){return stateCnt++;}),
			SymbolTranslator(GetSymbolDict(),
				[&symbolCnt](const std::string&){return symbolCnt++;}));
	}


	/*
	** Creating internal representation of automaton from
	** AutoDescription structure.
	** @param desc AutoDescription structure
	** @param stateTranslator Translates states to internal number representation
	** @param symbolTranslator Translates symbols to internal number representation
	*/ 
	template <class StateTransFunc, class SymbolTransFunc>
	void LoadFromAutDesc(
		const AutDescription&          desc,
		StateTransFunc                 stateTranslator,
		SymbolTransFunc                symbolTranslator,
		const std::string&             /* params */ = "")
	{
		// Load symbols
		for (auto symbolRankPair : desc.symbols){ // Symbols translater
			symbolTranslator(symbolRankPair.first);
			//std::cerr << "Symbol processed " <<  symbolTranslator(symbolRankPair.first) << std::endl;
		}

		// Load final states
		for (auto s : desc.finalStates) // Finale states extraction
		{
			//std::cerr << "Final state processed " << s   << " " << stateTranslator(s) << std::endl;
			this->finalStates_.insert(stateTranslator(s));
		}

		// Load transitions
		for (auto t : desc.transitions) {

      // TODO dodelat do cli kontrolu syntaxe pro FA

			// traverse the transitions
			const State& leftState = t.first[0];

			const std::string& symbol = t.second;
			const State& rightState = t.third;

			// Check whether there are no start states
      if (t.first.empty()) {
        StateType translatedState = stateTranslator(rightState);
        SymbolType translatedSymbol = symbolTranslator(symbol);

        SetStateStart(translatedState, translatedSymbol);

        continue;
      }

      /*
      std::cerr << "Transition left state " << leftState  <<  " "  <<  stateTranslator(leftState) << std::endl;
			std::cerr << "Transition right state " << rightState << " "  <<  stateTranslator(rightState) <<  std::endl;
      */
			//std::cerr << "Transition symbol " << symbol << std::endl;
			this->AddTransition(stateTranslator(leftState),symbolTranslator(symbol),
															stateTranslator(rightState));
		}
	}

	/*
	 * Function converts the internal automaton description
	 * to a string.
	 */
	template <class StatePrintFunc, class SymbolPrintFunc>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		StatePrintFunc                            statePrinter, // States from internal to string
		SymbolPrintFunc                           symbolPrinter, // Symbols from internal to string
		const std::string&                        /* params */ = "") const

	{
			AutDescription desc;

			// Dump the final states
			for (auto& s : this->finalStates_) {
				desc.finalStates.insert(statePrinter(s));
			}

      /* DEBUG
      std::cerr  <<  "Dmuping"  <<  std::endl;
      */
      // Dump start states
      std::string x("x"); // initial input symbol
		  std::vector<std::string> leftStateAsTuple;
      for (auto &s : this->startStates_) {

        SymbolSet symset = this->GetStartSymbols(s);

        if (!symset.size()) { // No start symbols are defined, 'x' will be used
          AutDescription::Transition trans(
			  			leftStateAsTuple,
              x,
              statePrinter(s));
          desc.transitions.insert(trans);
        }
        else { // print all starts symbols
          for (auto &sym : symset) {
           AutDescription::Transition trans(
			   			leftStateAsTuple,
              symbolPrinter(sym),
              statePrinter(s));
           desc.transitions.insert(trans);
           break; // TODO rozmyslet se, jak to chci delat
         }
        }
      }

			/*
			 * Converts transitions to a string
			 */
			for (auto& ls : *(this->transitions_)) {
				for (auto& s : *ls.second){
					for (auto& rs : s.second) {
						std::vector<std::string> leftStateAsTuple;
						leftStateAsTuple.push_back(statePrinter(ls.first));
           
            /* DEBUG
            std::cerr  <<  "Printing left state: "  << ls.first  <<  " " << statePrinter(ls.first)  <<  std::endl;
            std::cerr  <<  "Printing right state: "  <<  rs   <<  " "  <<   statePrinter(rs)  <<  std::endl;
            std::cerr  <<  "Printing symbol: "  << symbolPrinter(s.first)  <<  std::endl;
            */
						AutDescription::Transition trans(
							leftStateAsTuple,
							symbolPrinter(s.first),
							statePrinter(rs));
						desc.transitions.insert(trans);
					}
				}
			}

			return serializer.Serialize(desc);
	}
    
  /*
	 * The current indexes for states are transform to the new ones,
	 * and stored to the new automaton
	 */
	template <class Index>
	void ReindexStates(ExplicitFiniteAut& dst, Index& index) const {

    // Converts the final states
		for (auto& state : this->finalStates_)
			dst.SetStateFinal(index[state]);

    // Converts the start states
    for( auto& state : this->startStates_) {
      dst.SetExistingStateStart(index[state],GetStartSymbols(state));
    }

		auto clusterMap = dst.uniqueClusterMap();

    /*
     * Conversion of all states and symbols that are in 
     * transitions.
     */
		for (auto& stateClusterPair : *this->transitions_) {

			//assert(stateClusterPair.second);

      //DEBUG
      //std::cerr  << "Original left state: "  <<   stateClusterPair.first  <<  std::endl;
			auto cluster = clusterMap->uniqueCluster(index[stateClusterPair.first]);
      //std::cerr  << "Reindex left state: "  <<  index[stateClusterPair.first]  <<  std::endl;

			for (auto& symbolRStateSetPair : *stateClusterPair.second) {

				//assert(symbolTupleSetPair.second);

				RStateSet& rstatesSet = cluster->uniqueRStateSet(symbolRStateSetPair.first);

				for (auto& rState : symbolRStateSetPair.second) {
          //std::cerr  << "Original right state: "  <<   stateClusterPair.first  <<  std::endl; DEBUG
					rstatesSet.insert(index[rState]);
          //std::cerr  << "Reindex right state: "  <<  index[stateClusterPair.first]  <<  std::endl;
				}
			}
		}
	}



public: // Public static functions

	inline static void SetNextSymbolPtr(SymbolType* pNextSymbol)
	{
		// Assertions
		assert(pNextSymbol != nullptr);

		pNextSymbol_ = pNextSymbol;
	}

	inline static void SetSymbolDictPtr(StringToSymbolDict* pSymbolDict)
	{
		assert(pSymbolDict != nullptr);

		pSymbolDict_ = pSymbolDict;
	}


	inline static StringToSymbolDict& GetSymbolDict()
	{
		assert(pSymbolDict_ != nullptr);

		return *pSymbolDict_;
	}

  inline const StateSet& GetStartStates() const {
    return this->startStates_;
  }

  // TODO compability with tree automata part - used by complement function
	static AlphabetType GetAlphabet()
	{
		AlphabetType alphabet;
		for (auto symbol : GetSymbolDict())
		{
			alphabet.push_back(symbol.second);
		}

		return alphabet;
	}

public: // Public inline functions
  inline void SetStateFinal(const StateType& state) {
		this->finalStates_.insert(state);
	}

	inline void AddTransition(const StateType& lstate, const SymbolType& symbol,
									const StateType& rstate){
		this->internalAddTransition(lstate, symbol, rstate);
	}

  inline bool IsStateFinal(const StateType &state) const {
    return (this->finalStates_.find(state) != this->finalStates_.end());
  }

  inline bool IsStateStart(const StateType &state) const  {
    return (this->startStates_.find(state) != this->startStates_.end());
  }

public: // Public setter
  void SetStateStart(const StateType& state, const SymbolType& symbol) {
    this->startStates_.insert(state);
    
    // Add start transition
    if (!this->startStateToSymbols_.count(state)) {
      //std::cerr << "Adding a new start states "  <<  state << std::endl; DEBUG
      this->startStateToSymbols_.insert(
         std::make_pair(state,std::unordered_set<SymbolType>())).
         first->second.insert(symbol);
      //std::cerr << "Start symbol size: "  <<  this->startStateToSymbols_.size() << std::endl;
    }
    else { // Just add new symbol
      //std::cerr << "Symbol already added" << std::endl;
      this->startStateToSymbols_.find(state)->second.insert(symbol);
    }
	}

  // Set start state with set of symbols in start transitions
  void SetExistingStateStart(const StateType& state, const SymbolSet& symbolSet) {
    this->startStates_.insert(state);

    assert(!this->startStateToSymbols_.count(state));

    this->startStateToSymbols_.insert(std::make_pair(state,symbolSet));
  }

public: // Getters
  // Return a set of the symbols which are in start transitions 
  // for given state
  const SymbolSet& GetStartSymbols(StateType state) const {
    assert(this->startStateToSymbols_.find(state) != this->startStateToSymbols_.end());
    return this->startStateToSymbols_.find(state)->second; 
  }

protected:

	const StateToTransitionClusterMapPtr& uniqueClusterMap(){
		if (!this->transitions_.unique()){
			this->transitions_ = StateToTransitionClusterMapPtr(
				new StateToTransitionClusterMap(*transitions_));
		}
		return this->transitions_;
	}
	/*
	 * Add internal transition to the automaton
	 */
	void internalAddTransition(const StateType& lstate, const SymbolType& symbol,
									const StateType& rstate){
		this->uniqueClusterMap()->uniqueCluster(lstate)->uniqueRStateSet(symbol).insert(rstate);
		return;
	}

  /*
   * Get from tree automata part of library
   */
	template <class T>
	static const typename T::mapped_type::element_type* genericLookup(const T& cont,
		const typename T::key_type& key) {

		auto iter = cont.find(key);
		if (iter == cont.end())
			return NULL;

		return iter->second.get();

	}
};

template <class Symbol>
typename VATA::ExplicitFiniteAut<Symbol>::StringToSymbolDict*
	VATA::ExplicitFiniteAut<Symbol>::pSymbolDict_ = nullptr;

template <class Symbol>
typename VATA::ExplicitFiniteAut<Symbol>::SymbolType*
	VATA::ExplicitFiniteAut<Symbol>::pNextSymbol_ = nullptr;
#endif
