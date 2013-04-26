/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning useless states of BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>
#include <vata/util/graph.hh>

// Standard library headers
#include <stack>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::Util::Convert;
using VATA::Util::Graph;

typedef VATA::AutBase::StateType StateType;
typedef VATA::BDDBottomUpTreeAut::StateSet StateSet;
typedef VATA::BDDBottomUpTreeAut::StateHT StateHT;
typedef VATA::BDDBottomUpTreeAut::StateTuple StateTuple;
typedef VATA::BDDBottomUpTreeAut::TransMTBDD TransMTBDD;

typedef std::unordered_map<StateTuple, TransMTBDD, boost::hash<StateTuple>>
	TupleHT;

typedef Graph::NodeType NodeType;

typedef VATA::Util::TwoWayDict<NodeType, StateType,
	std::unordered_map<NodeType, StateType>,
	std::unordered_map<StateType, NodeType>> NodeToStateDict;

typedef std::stack<NodeType, std::list<NodeType>> NodeWorkSet;

namespace
{
	GCC_DIAG_OFF(effc++)
	class ReachableCollectorFctor :
		public VATA::MTBDDPkg::VoidApply1Functor<ReachableCollectorFctor,
		StateSet>
	{
	GCC_DIAG_ON(effc++)

	private:  // data members

		StateHT& reachable_;
		StateHT& workset_;
		const StateTuple& tuple_;
		NodeToStateDict& nodes_;
		Graph& graph_;

	public:   // methods

		ReachableCollectorFctor(StateHT& reachable, StateHT& workset,
			const StateTuple& tuple, NodeToStateDict& nodes, Graph& graph) :
			reachable_(reachable),
			workset_(workset),
			tuple_(tuple),
			nodes_(nodes),
			graph_(graph)
		{ }

		inline void ApplyOperation(const StateSet& value)
		{
			for (const StateType& state : value)
			{
				if (reachable_.insert(state).second)
				{	// if the value was inserted
					if (!workset_.insert(state).second)
					{	// if it is already in the workset
						assert(false);     // fail gracefully
					}
				}

				NodeType node;
				NodeToStateDict::ConstIteratorBwd itNode;
				if ((itNode = nodes_.FindBwd(state)) != nodes_.EndBwd())
				{
					node = itNode->second;
				}
				else
				{
					node = graph_.AddNode();
					nodes_.insert(std::make_pair(node, state));
				}

				for (const StateType& tupState : tuple_)
				{
					NodeToStateDict::ConstIteratorBwd itOtherNode;
					if ((itOtherNode = nodes_.FindBwd(tupState)) == nodes_.end())
					{
						assert(false);      // fail gracefully
					}

					graph_.AddEdge(node, itOtherNode->second);
				}
			}
		}
	};
}


GCC_DIAG_OFF(effc++)
class UsefulCheckerFctor :
	public VATA::MTBDDPkg::Apply1Functor<UsefulCheckerFctor,
	StateSet, StateSet>
{
GCC_DIAG_ON(effc++)

private:  // data members

	const StateHT& useful_;

public:   // methods

	UsefulCheckerFctor(const StateHT& useful) :
		useful_(useful)
	{ }

	inline StateSet ApplyOperation(const StateSet& value)
	{
		StateSet result;

		for (const StateType& state : value)
		{
			if (useful_.find(state) != useful_.end())
			{
				result.insert(state);
			}
		}

		return result;
	}

};


BDDBottomUpTreeAut VATA::RemoveUselessStates(const BDDBottomUpTreeAut& aut)
{
	BDDBottomUpTreeAut result;

	StateHT reachable;
	StateHT workset;
	Graph graph;
	NodeToStateDict nodes;

	TupleHT tuples;

	for (auto tupleBddPair : aut.GetTransTable())
	{
		tuples.insert(tupleBddPair);
	}

	StateTuple tuple;
	tuples.erase(tuple);

	ReachableCollectorFctor reachFunc(reachable, workset, tuple, nodes, graph);

	const TransMTBDD& nullaryBdd = aut.GetMtbdd(StateTuple());
	reachFunc(nullaryBdd);

	while (!workset.empty())
	{
		StateType state = *(workset.begin());
		workset.erase(workset.begin());

		TupleHT::const_iterator itTup = tuples.begin();
		while (itTup != tuples.end())
		{
			tuple = itTup->first;

			if (std::find(tuple.begin(), tuple.end(), state) != tuple.end())
			{	// if the state is there
				size_t i;
				for (i = 0; i < tuple.size(); ++i)
				{
					if (reachable.find(tuple[i]) == reachable.end())
					{
						break;
					}
				}

				if (i == tuple.size())
				{	// in case all states are reachable
					// collect reachable states
					reachFunc(itTup->second);

					// remove the tuple from the set of tuples
					decltype(itTup) tmpIt = itTup;
					++itTup;
					tuples.erase(tmpIt);
					continue;
				}
			}

			++itTup;
		}
	}

	NodeWorkSet nodeWorkset;
	StateHT useful;

	for (const StateType& fst : aut.GetFinalStates())
	{
		NodeToStateDict::ConstIteratorBwd itNodes;
		if ((itNodes = nodes.FindBwd(fst)) != nodes.EndBwd())
		{
			result.SetStateFinal(fst);
			useful.insert(fst);

			nodeWorkset.push(itNodes->second);
		}
	}

	while (!nodeWorkset.empty())
	{
		NodeType node = nodeWorkset.top();
		nodeWorkset.pop();

		for (const NodeType& inNode : Graph::GetIngress(node))
		{	// for each input edge of the node, remove the edge
			if (Graph::GetEgress(inNode).erase(node) != 1)
			{	// in case of an internal error
				assert(false);     // fail gracefully
			}
		}

		for (const NodeType& outNode : Graph::GetEgress(node))
		{	// for each input edge of the node, remove the edge
			NodeToStateDict::const_iterator itNode;
			if ((itNode = nodes.FindFwd(outNode)) == nodes.EndFwd())
			{
				assert(false);      // fail gracefully
			}

			if (useful.insert(itNode->second).second)
			{
				nodeWorkset.push(itNode->first);
			}
		}
	}

	UsefulCheckerFctor usefulFunc(useful);

	for (auto tupleBddPair : aut.GetTransTable())
	{
		const StateTuple& tuple = tupleBddPair.first;
		const TransMTBDD& bdd = tupleBddPair.second;
		size_t i;
		for (i = 0; i < tuple.size(); ++i)
		{
			if (useful.find(tuple[i]) == useful.end())
			{
				break;
			}
		}

		if (i != tuple.size())
		{
			continue;
		}

		result.SetMtbdd(tuple, usefulFunc(bdd));
	}

	return result;
}
