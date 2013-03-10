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

  template <class SymbolType>
  friend ExplicitFiniteAut<SymbolType> Union(
		const ExplicitFiniteAut<SymbolType>&, const ExplicitFiniteAut<SymbolType>&,
		AutBase::StateToStateMap*, AutBase::StateToStateMap*);

public:
	typedef Symbol SymbolType;

private: // private type definitions
	typedef AutBase::StateType StateType;

	typedef std::string string;
	typedef std::unordered_set<StateType> StateSet;
	typedef VATA::Util::AutDescription AutDescription;
	typedef AutDescription::State State;

	//typedef std::shared_ptr<const StateType> RStatePtr;
	typedef StateType RStatePtr;

	typedef std::set<RStatePtr> RStatePtrSet;
	typedef std::shared_ptr<RStatePtrSet> RStatePtrSetPtr;

	//TODO: DODODELAT unqiue
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

	typedef std::shared_ptr<TransitionCluster> TransitionClusterPtr; // TODO dodelat pointer na cluster

	//TODO: Dodelat unique
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


private: // private data memebers
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

			std::cout << "Transition left state " << leftState << std::endl;
			std::cout << "Transition right state " << rightState << std::endl;
			std::cout << "Transition symbol " << symbol << std::endl;
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
            std::cout  <<  "Printing left state: "  << statePrinter(ls.first)  <<  std::endl;
            std::cout  <<  "Printing right state: "  << statePrinter(rs)  <<  std::endl;
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

  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> Intersection(
      ExplicitFiniteAut<SymbolType> &lhs,
      ExplicitFiniteAut<SymbolType> &rhs,
      AutBase::ProductTranslMap* pTranslMap = nullptr) {
    
    AutBase::ProductTranslMap translMap;

    if (!pTranslMap){
      pTranslMap = translMap;
    }

    ExplicitFiniteAut<SymbolType> res;

    std::vector<const AutBase::ProductTranslMap::value_type*> stack;

    for (auto lfs : lhs.finalStates_){
      for(auto rfs : rhs.finalStates_){
        auto ifs = pTranslMap->insert(std::make_pair(std::make_pair(lfs,rfs),
              pTranslMap->size())).first;

        res.SetStateFinal(ifs->second);

        res.push_back(ifs);
      }
    }
    

    return res;
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

				auto rStatePtrSet = cluster->uniqueRStatePtrSet(symbolRStateSetPair.first);

				for (auto& rState : *symbolRStateSetPair.second) {
          std::cout  << "Original right state: "  <<   stateClusterPair.first  <<  std::endl;
					rStatePtrSet->insert(index[rState]);
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
		this->uniqueClusterMap()->uniqueCluster(lstate)->uniqueRStatePtrSet(symbol)->insert(rstate);
		return;
	}

};

template <class Symbol>
typename VATA::ExplicitFiniteAut<Symbol>::StringToSymbolDict*
	VATA::ExplicitFiniteAut<Symbol>::pSymbolDict_ = nullptr;

template <class Symbol>
typename VATA::ExplicitFiniteAut<Symbol>::SymbolType*
	VATA::ExplicitFiniteAut<Symbol>::pNextSymbol_ = nullptr;
#endif
