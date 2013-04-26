/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file of computation of downward inclusion on bottom-up
 *    represented tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>
#include <vata/mtbdd/apply3func.hh>

// Standard library headers
#include <stack>

using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;

typedef VATA::AutBase::StateBinaryRelation StateBinaryRelation;
typedef VATA::AutBase::StateType StateType;
typedef VATA::AutBase::StateToStateMap StateToStateMap;
typedef VATA::AutBase::StateToStateTranslator StateToStateTranslator;

typedef BDDTopDownTreeAut::StateTuple StateTuple;
typedef BDDTopDownTreeAut::StateTupleSet StateTupleSet;

typedef BDDBottomUpTreeAut::StateSet StateSet;

typedef std::vector<size_t> CounterElementMap;
typedef VATA::MTBDDPkg::OndriksMTBDD<CounterElementMap> CounterMTBDD;
typedef std::unordered_map<StateTuple, CounterMTBDD, boost::hash<StateTuple>>
	CounterHT;

typedef BDDTopDownTreeAut::TransMTBDD TopDownMTBDD;

typedef std::pair<StateTuple, StateTuple> RemoveElement;
typedef std::unordered_set<RemoveElement, boost::hash<RemoveElement>> RemoveSet;

typedef BDDBottomUpTreeAut::TransTable BUTransTable;

namespace
{
	template <class ArbitraryFunction>
	void forAllTuplesWithMatchingStatesDo(
		const BUTransTable& tuples, const StateType& lhsState,
		const StateType& rhsState, ArbitraryFunction func)
	{
		for (auto lhsTupleBddPair : tuples)
		{
			const StateTuple& lhsTuple = lhsTupleBddPair.first;

			std::vector<size_t> matchedPositions;
			for (size_t i = 0; i < lhsTuple.size(); ++i)
			{
				if (lhsTuple[i] == lhsState)
				{
					matchedPositions.push_back(i);
				}
			}

			if (matchedPositions.empty())
			{
				continue;
			}

			for (auto rhsTupleBddPair : tuples)
			{
				const StateTuple& rhsTuple = rhsTupleBddPair.first;

				if (rhsTuple.size() != lhsTuple.size())
				{
					continue;
				}

				size_t i;
				for (i = 0; i < matchedPositions.size(); ++i)
				{
					if (rhsTuple[matchedPositions[i]] == rhsState)
					{
						break;
					}
				}

				if (i == matchedPositions.size())
				{
					continue;
				}

				func(lhsTuple, rhsTuple);
			}
		}
	}

	inline bool componentWiseSim(const StateBinaryRelation& sim,
		const StateTuple& lhsTuple, const StateTuple& rhsTuple)
	{
		for (size_t i = 0; i < lhsTuple.size(); ++i)
		{
			if (!sim.get(lhsTuple[i], rhsTuple[i]))
			{
				return false;
			}
		}

		return true;
	}

	GCC_DIAG_OFF(effc++)
	class InitCntApplyFctor :
		public VATA::MTBDDPkg::Apply2Functor<InitCntApplyFctor, StateTupleSet,
		CounterElementMap, CounterElementMap>
	{
	GCC_DIAG_ON(effc++)

	private:  // data members

		const StateType& state_;

	public:   // methods

		InitCntApplyFctor(const StateType& state) :
			state_(state)
		{ }

		CounterElementMap ApplyOperation(const StateTupleSet& lhs,
			const CounterElementMap& rhs)
		{
			// Assertions
			assert(state_ < rhs.size());

			CounterElementMap result = rhs;
			result[state_] = lhs.size();

			return result;
		}
	};


	GCC_DIAG_OFF(effc++)
	class InitRefineApplyFctor :
		public VATA::MTBDDPkg::VoidApply2Functor<InitRefineApplyFctor, StateTupleSet,
		StateTupleSet>
	{
	GCC_DIAG_ON(effc++)

	private:  // data members

		bool& isSim_;

	public:   // methods

		InitRefineApplyFctor(bool& isSim) :
			isSim_(isSim)
		{ }

		void ApplyOperation(const StateTupleSet& lhs, const StateTupleSet& rhs)
		{
			if (!lhs.empty() && rhs.empty())
			{
				isSim_ = false;
				stopProcessing();
			}
		}
	};

	GCC_DIAG_OFF(effc++)
	class RefineApplyFctor :
		public VATA::MTBDDPkg::Apply3Functor<RefineApplyFctor, StateSet, StateSet,
		CounterElementMap, CounterElementMap>
	{
	GCC_DIAG_ON(effc++)

	private:  // data members

		const BUTransTable& transTable_;
		StateBinaryRelation& sim_;
		RemoveSet& remove_;

	public:   // methods

		RefineApplyFctor(const BUTransTable& transTable, StateBinaryRelation& sim,
			RemoveSet& remove) :
			transTable_(transTable),
			sim_(sim),
			remove_(remove)
		{ }

		CounterElementMap ApplyOperation(const StateSet& upR, const StateSet& upQ,
			const CounterElementMap& cntQ)
		{
			if (upR.empty())
			{
				return cntQ;
			}

			CounterElementMap result = cntQ;

			for (const StateType& s : upR)
			{
				// Assertions
				assert(s < cntQ.size());
				assert(result[s] > 0);

				if (--(result[s]) == 0)
				{
					for (const StateType& p : upQ)
					{
						if (sim_.get(p, s))
						{	// if 's' simulates 'p'
							forAllTuplesWithMatchingStatesDo(
								transTable_, p, s,
								[this](const StateTuple& pTuple, const StateTuple& sTuple){
									if (componentWiseSim(sim_, pTuple, sTuple))
									{
										remove_.insert(std::make_pair(pTuple, sTuple));
									}}
								);

							// 's' no longer simulates 'p'
							sim_.set(p, s, false);
						}
					}
				}
			}

			return result;
		}

	};
}


StateBinaryRelation VATA::ComputeDownwardSimulation(
	const BDDBottomUpTreeAut& aut)
{
	BDDBottomUpTreeAut newAut = aut;
	StateType states = AutBase::SanitizeAutForSimulation(newAut);

	return ComputeDownwardSimulation(newAut, states);
}

StateBinaryRelation VATA::ComputeDownwardSimulation(
	const BDDBottomUpTreeAut& aut, const size_t& size)
{
	StateBinaryRelation sim(size);

	BDDTopDownTreeAut topDownAut = aut.GetTopDownAut();

	CounterMTBDD initCnt((CounterElementMap(size)));

	StateType firstState;
	InitCntApplyFctor initCntFctor(firstState);

	bool isSim;
	InitRefineApplyFctor initRefFctor(isSim);

	RemoveSet remove;

	for (auto firstStateBddPair : topDownAut.GetStates())
	{
		firstState = firstStateBddPair.first;
		const TopDownMTBDD& firstBdd = firstStateBddPair.second;

		initCnt = initCntFctor(firstBdd, initCnt);

		for (auto secondStateBddPair : topDownAut.GetStates())
		{
			const StateType& secondState = secondStateBddPair.first;
			const TopDownMTBDD& secondBdd = secondStateBddPair.second;
			isSim = true;
			initRefFctor(firstBdd, secondBdd);
			if (isSim)
			{
				sim.set(firstState, secondState, true);
			}
			else
			{	// prune
				forAllTuplesWithMatchingStatesDo(
					aut.GetTransTable(), firstState, secondState,
					[&remove](const StateTuple& firstTuple, const StateTuple& secondTuple){
						remove.insert(std::make_pair(firstTuple, secondTuple));}
					);
			}
		}
	}

	CounterHT cnt;
	for (auto tupleBddPair : aut.GetTransTable())
	{
		cnt.insert(std::make_pair(tupleBddPair.first, initCnt));
	}

	RefineApplyFctor refineFctor(aut.GetTransTable(), sim, remove);

	while (!remove.empty())
	{
		RemoveSet::const_iterator itRem = remove.begin();
		assert(itRem != remove.end());
		RemoveElement elem = *itRem;
		remove.erase(itRem);

		// Assertions
		assert(elem.first.size() == elem.second.size());

		CounterHT::iterator itCnt;
		if ((itCnt = cnt.find(elem.first)) == cnt.end())
		{
			assert(false);       // fail gracefully
		}

		itCnt->second = refineFctor(aut.GetMtbdd(elem.second),
			aut.GetMtbdd(elem.first),
			BDDTopDownTreeAut::GetMtbddForArity(itCnt->second, elem.first.size()));
	}

	return sim;
}

StateBinaryRelation VATA::ComputeUpwardSimulation(const BDDBottomUpTreeAut& aut)
{
	if (&aut == nullptr) { }

	throw std::runtime_error("Unimplemented");
}

StateBinaryRelation VATA::ComputeUpwardSimulation(const BDDBottomUpTreeAut& aut,
	const size_t& size)
{
	if ((&aut == nullptr) || (&size == nullptr)) { }

	throw std::runtime_error("Unimplemented");
}
