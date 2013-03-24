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
	friend ExplicitFiniteAut<SymbolType> Intersection(
		const ExplicitFiniteAut<SymbolType>&, const ExplicitFiniteAut<SymbolType>&,
		AutBase::ProductTranslMap*);

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> RemoveUnreachableStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      AutBase::ProductTranslMap* pTranslMap = nullptr);

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> RemoveUselessStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      AutBase::ProductTranslMap* pTranslMap = nullptr);

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> Reverse(
      const ExplicitFiniteAut<SymbolType> &aut,
      AutBase::ProductTranslMap* pTranslMap = nullptr);

  template <class SymbolType, class Dict>
  friend ExplicitFiniteAut<SymbolType> Complement(
      const ExplicitFiniteAut<SymbolType> &aut,
      const Dict &alphabet);
	
  template <class SymbolType>
	friend ExplicitFiniteAut<SymbolType> GetCandidateTree(const ExplicitFiniteAut<SymbolType>& aut);

public:
	typedef Symbol SymbolType;

private: // private type definitions
	typedef AutBase::StateType StateType;
	typedef std::vector<SymbolType> AlphabetType;

	typedef std::string string;
	typedef std::unordered_set<StateType> StateSet;
	typedef VATA::Util::AutDescription AutDescription;
	typedef AutDescription::State State;

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
  typedef std::set<StateType> RStateSet;
	typedef std::shared_ptr<RStateSet> RStateSetPtr;

	class TransitionCluster : public std::unordered_map<SymbolType,RStateSetPtr>{
	public:
		const RStateSetPtr& uniqueRStateSet(const SymbolType &symbol){
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

private: // private static data memebers
	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;
	StateToTransitionClusterMapPtr transitions_;
	
public:

	ExplicitFiniteAut() :
		finalStates_(),
		startStates_(),
		transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap))
	{ }

	~ExplicitFiniteAut() {}

	void devel(VATA::Parsing::AbstrParser& parser, 
		const std::string& str, StringToStateDict& stateDict)
	{
		LoadFromString(parser,str, stateDict);
		std::cout << "Vata finite automata development begins" << std::endl;
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
			//std::cout << "Symbol processed " <<  symbolTranslator(symbolRankPair.first) << std::endl;
		}

		// Load final states
		for (auto s : desc.finalStates) // Finale states extraction
		{
			//std::cout << "Final state processed " << s   << " " << stateTranslator(s) << std::endl;
			this->finalStates_.insert(stateTranslator(s));
		}

		// Load transitions
		for (auto t : desc.transitions) {

      // TODO dodelat do cli kontrolu syntaxe pro FA
      assert(!t.empty()); // Not a tree automata

			// traverse the transitions
			const State& leftState = t.first[0];

			const std::string& symbol = t.second;
			const State& rightState = t.third;

			// Check whether there are no start states
			if (rightState[0] == 's'){
				this->startStates_.insert(stateTranslator(rightState));
				//std::cout  <<  "Start states: "  <<  leftState  <<  std::endl;
			}
			if (leftState[0] == 's'){
				this->startStates_.insert(stateTranslator(leftState));
				//std::cout  <<  "Start states: "  <<  rightState  <<  std::endl;
			}

			std::cout << "Transition left state " << leftState  <<  " "  <<  stateTranslator(leftState) << std::endl;
			std::cout << "Transition right state " << rightState << " "  <<  stateTranslator(rightState) <<  std::endl;
			//std::cout << "Transition symbol " << symbol << std::endl;
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
			for (auto& s : this->finalStates_){
				desc.finalStates.insert(statePrinter(s));
			}


			/*
			 * Converts transitions to a string
			 */
			for (auto& ls : *(this->transitions_)){
				for (auto& s : *ls.second){
					for (auto& rs : *s.second){
						std::vector<std::string> leftStateAsTuple;
						leftStateAsTuple.push_back(statePrinter(ls.first));
            
            std::cout  <<  "Printing left state: "  << ls.first  <<  " " << statePrinter(ls.first)  <<  std::endl;
            std::cout  <<  "Printing right state: "  <<  rs   <<  " "  <<   statePrinter(rs)  <<  std::endl;
            std::cout  <<  "Printing symbol: "  << symbolPrinter(s.first)  <<  std::endl;
            
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
      dst.SetStateStart(index[state]);
    }

		auto clusterMap = dst.uniqueClusterMap();

    /*
     * Conversion of all states and symbols that are in 
     * transitions.
     */
		for (auto& stateClusterPair : *this->transitions_) {

			//assert(stateClusterPair.second);

      std::cout  << "Original left state: "  <<   stateClusterPair.first  <<  std::endl;
			auto cluster = clusterMap->uniqueCluster(index[stateClusterPair.first]);
      std::cout  << "Reindex left state: "  <<  index[stateClusterPair.first]  <<  std::endl;

			for (auto& symbolRStateSetPair : *stateClusterPair.second) {

				//assert(symbolTupleSetPair.second);

				RStateSetPtr rstatesSet = cluster->uniqueRStateSet(symbolRStateSetPair.first);

				for (auto& rState : *symbolRStateSetPair.second) {
          std::cout  << "Original right state: "  <<   stateClusterPair.first  <<  std::endl;
					rstatesSet->insert(index[rState]);
          std::cout  << "Reindex right state: "  <<  index[stateClusterPair.first]  <<  std::endl;
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

  inline void SetStateStart(const StateType& state) {
		this->startStates_.insert(state);
	}
	inline void AddTransition(const StateType& lstate, const SymbolType& symbol,
									const StateType& rstate){
		this->internalAddTransition(lstate, symbol, rstate);
	}

  inline bool IsStateFinal(const StateType &state) const{
    return (this->finalStates_.find(state) != this->finalStates_.end());
  }

protected:

	const StateToTransitionClusterMapPtr uniqueClusterMap(){
		if (!this->transitions_.unique()){
			this->transitions_ = StateToTransitionClusterMapPtr(
				new StateToTransitionClusterMap());
		}
		return this->transitions_;
	}
	/*
	 * Add internal transition to the automaton
	 */
	void internalAddTransition(const StateType& lstate, const SymbolType& symbol,
									const StateType& rstate){
		this->uniqueClusterMap()->uniqueCluster(lstate)->uniqueRStateSet(symbol)->insert(rstate);
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
			return nullptr;

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
