/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The command-line interface to the VATA library.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/bdd_td_tree_aut_op.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/explicit_tree_aut_op.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/convert.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/util.hh>
#include "../devel_fa/explicit_finite_aut.hh"
#include "../devel_fa/explicit_finite_aut_op.hh"


// standard library headers
#include <cstdlib>
#include <fstream>

// local headers
#include "parse_args.hh"
#include "operations.hh"


using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::AbstrSerializer;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::Convert;


typedef VATA::ExplicitTreeAut<size_t> ExplicitTreeAut;
typedef VATA::ExplicitFiniteAut<size_t> ExplicitFiniteAut;

typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
	StateTranslatorWeak;
typedef VATA::Util::TranslatorStrict<AutBase::StringToStateDict::MapBwdType>
	StateBackTranslatorStrict;

typedef VATA::Util::TranslatorWeak<BDDTopDownTreeAut::StringToSymbolDict>
	SymbolTranslatorWeak;

const char VATA_USAGE_STRING[] =
	"VATA: VATA Tree Automata library interface\n"
	"usage: vata [-r <representation>] [(-I|-O|-F) <format>] [-h|--help] [-t] [-n]\n"
	"            [(-p|-s)] [-o <options>] <command> [<args>]\n"
	;

const char VATA_USAGE_COMMANDS[] =
	"\nThe following commands are supported:\n"
	"    help                    Display this message\n"
	"    load    <file>          Load automaton from <file>\n"
	"    witness <file>          Get a witness for automaton in <file>\n"
	"    cmpl    <file>          Complement automaton from <file> [experimental]\n"
	"    union <file1> <file2>   Compute union of automata from <file1> and <file2>\n"
	"    isect <file1> <file2>   Compute intersection of automata from <file1> and\n"
	"                            <file2>\n"
	"    sim <file>              Computes a simulation relation for the automaton in\n"
	"                            <file>. Options:\n"
	"\n"
	"          'dir=down' : downward simulation (default)\n"
	"          'dir=up'   : upward simulation\n"
	"\n"
	"    red <file>              Reduces the automaton in <file> using simulation\n"
	"                            relation. Options:\n"
	"\n"
	"          'dir=down' : downward simulation (default)\n"
	"          'dir=up'   : upward simulation\n"
	"\n"
	"    incl <file1> <file2>    Checks language inclusion of automata from <file1>\n"
	"                            and <file2>, i.e., whether L(<file1>) is a subset\n"
	"                            of L(<file2>). Options:\n"
	"\n"
	"          'dir=down' : downward inclusion checking\n"
	"          'dir=up'   : upward inclusion checking (default)\n"
	"          'sim=yes'  : use corresponding simulation\n"
	"          'sim=no'   : do not use simulation (default)\n"
	"          'optC=yes' : use optimised cache for downward direction\n"
	"          'optC=no'  : without optimised cache (default)\n"
	"          'rec=yes'  : recursive version of downward direction (default)\n"
	"          'rec=no'   : non-recursive version (only for '-r expl' and 'optC=no')\n"
	"          'timeS=yes': include time of simulation computation (default)\n"
	"          'timeS=no' : do not include time of simulation computation\n"
	;

const char VATA_USAGE_FLAGS[] =
	"\nOptions:\n"
	"    -h, --help              Display this message\n"
	"    -r <representation>     Use <representation> for internal storage of\n"
	"                            automata. The following representations are\n"
	"                            supported:\n"
	"                               'bdd-td'   : binary decision diagrams,\n"
	"                                            top-down\n"
	"                               'bdd-bu'   : binary decision diagrams,\n"
	"                                            bottom-up\n"
	"                               'expl'     : explicit (default)\n"
	"\n"
	"    (-I|-O|-F) <format>     Specify format for input (-I), output (-O), or\n"
	"                            both (-F). The following formats are supported:\n"
	"                               'timbuk'  : Timbuk format (default)\n"
	"\n"
	"    -t                      Print the time the operation took to error output\n"
	"                            stream\n"
	"    -v                      Be verbose\n"
	"    -n                      Do not output the result automaton\n"
	"    -p                      Prune unreachable states first\n"
	"    -s                      Prune useless states first (note that this is\n"
	"                            stronger than -p)\n"
	"    -o <opt>=<v>,<opt>=<v>  Options in the form of a comma-separated\n"
	"                            <option>=<value> list\n"
	;

const size_t BDD_SIZE = 16;

timespec startTime;

void printHelp(bool full = false)
{
	std::cout << VATA_USAGE_STRING;

	if (full)
	{	// in case full help is wanted
		std::cout << VATA_USAGE_COMMANDS;
		std::cout << VATA_USAGE_FLAGS;
		std::cout << "\n\n";
	}
}


template <class Aut>
int performOperation(const Arguments& args, AbstrParser& parser,
	AbstrSerializer& serializer)
{
	typedef typename Aut::SymbolBackTranslatorStrict SymbolBackTranslatorStrict;

	Aut autInput1;
	Aut autInput2;
	Aut autResult;
  /*
	bool boolResult = false;
  */
	VATA::AutBase::StateBinaryRelation relResult;

	VATA::AutBase::StringToStateDict stateDict1;
	VATA::AutBase::StringToStateDict stateDict2;

	VATA::AutBase::StateToStateMap translMap1;
	VATA::AutBase::StateToStateMap translMap2;

	if (args.operands >= 1)
	{
		autInput1.LoadFromString(parser, VATA::Util::ReadFile(args.fileName1),
			stateDict1);
	}

	if (args.operands >= 2)
	{
		autInput2.LoadFromString(parser, VATA::Util::ReadFile(args.fileName2),
			stateDict2);
	}

/*
	if ((args.command == COMMAND_LOAD) ||
		(args.command == COMMAND_UNION) ||
		(args.command == COMMAND_COMPLEMENT) ||
		(args.command == COMMAND_INTERSECTION) ||
		(args.command == COMMAND_RED))
	{
		if (args.pruneUseless)
		{
			if (args.operands >= 1)
			{
				autInput1 = RemoveUselessStates(autInput1);
			}

			if (args.operands >= 2)
			{
				autInput2 = RemoveUselessStates(autInput2);
			}
		}
		else if (args.pruneUnreachable)
		{
			if (args.operands >= 1)
			{
				autInput1 = RemoveUnreachableStates(autInput1);
			}

			if (args.operands >= 2)
			{
				autInput2 = RemoveUnreachableStates(autInput2);
			}
		}
	}
*/

	AutBase::StateToStateMap opTranslMap1;
	AutBase::StateToStateMap opTranslMap2;
	AutBase::ProductTranslMap prodTranslMap;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);     // set the timer

	timespec finishTime;

	// process command
	if (args.command == COMMAND_LOAD)
	{
		autResult = autInput1;
	}
  /*
	else if (args.command == COMMAND_WITNESS)
	{
		autResult = GetCandidateTree(autInput1);
	}
	else if (args.command == COMMAND_COMPLEMENT)
	{
		autResult = Complement(autInput1, autInput1.GetAlphabet());
	}
  */
	else if (args.command == COMMAND_UNION)
	{
		autResult = Union(autInput1, autInput2, &opTranslMap1, &opTranslMap2);
	}
  /*
	else if (args.command == COMMAND_INTERSECTION)
	{
		autResult = Intersection(autInput1, autInput2, &prodTranslMap);
	}
  */
  /*
	else if (args.command == COMMAND_INCLUSION)
	{
		boolResult = CheckInclusion(autInput1, autInput2, args);
	}
	else if (args.command == COMMAND_SIM)
	{
		relResult = ComputeSimulation(autInput1, args);
	}
	else if (args.command == COMMAND_RED)
	{
		autResult = ComputeReduction(autInput1, args);
	}
  */
	else
	{
		throw std::runtime_error("Internal error: invalid command");
	}

	// get the finish time
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &finishTime))
	{
		throw std::runtime_error("Could not get the finish time");
	}
	double opTime = (finishTime.tv_sec - startTime.tv_sec)
		+ 1e-9 * (finishTime.tv_nsec - startTime.tv_nsec);

	if (args.showTime)
	{
		std::cerr << opTime << "\n";
	}

	if (!args.dontOutputResult)
	{	// in case output is not forbidden
		if ((args.command == COMMAND_LOAD))// ||
	//		(args.command == COMMAND_WITNESS) ||
		//	(args.command == COMMAND_RED))
		{
			std::cout << autResult.DumpToString(serializer,
				StateBackTranslatorStrict(stateDict1.GetReverseMap()),
				SymbolBackTranslatorStrict(autResult.GetSymbolDict().GetReverseMap()));
		}
  /*

		if (args.command == COMMAND_COMPLEMENT)
		{
			std::cout << autResult.DumpToString(serializer,
				[](const AutBase::StateType& state){ return "q" + Convert::ToString(state); },
				SymbolBackTranslatorStrict(autResult.GetSymbolDict().GetReverseMap()));
		}
*/
		if (args.command == COMMAND_UNION)
		{
			stateDict1 = VATA::Util::CreateUnionStringToStateMap(
				stateDict1, stateDict2, &opTranslMap1, &opTranslMap2);
		}

		if (args.command == COMMAND_INTERSECTION)
		{
			stateDict1 = VATA::Util::CreateProductStringToStateMap(
				stateDict1, stateDict2, prodTranslMap);
		}

		if ((args.command == COMMAND_UNION) ||
			(args.command == COMMAND_INTERSECTION))
		{
			std::cout << autResult.DumpToString(serializer,
				StateBackTranslatorStrict(stateDict1.GetReverseMap()),
				SymbolBackTranslatorStrict(autResult.GetSymbolDict().GetReverseMap()));
		}
/*
		if ((args.command == COMMAND_INCLUSION))
		{
			std::cout << boolResult << "\n";
		}

		if (args.command == COMMAND_SIM)
		{
			std::cout << relResult << "\n";
		}
    */
	}

	return EXIT_SUCCESS;
}


template <class Aut>
int executeCommand(const Arguments& args)
{
	std::unique_ptr<AbstrParser> parser(nullptr);
	std::unique_ptr<AbstrSerializer> serializer(nullptr);

	// create the input parser
	if (args.inputFormat == FORMAT_TIMBUK)
	{
		parser.reset(new TimbukParser());
	}
	else
	{
		throw std::runtime_error("Internal error: invalid input format");
	}

	// create the output serializer
	if (args.outputFormat == FORMAT_TIMBUK)
	{
		serializer.reset(new TimbukSerializer());
	}
	else
	{
		throw std::runtime_error("Internal error: invalid output format");
	}

	return performOperation<Aut>(args, *(parser.get()), *(serializer.get()));
}


int main(int argc, char* argv[])
{
	// Assertions
	assert(argc > 0);
	assert(argv != nullptr);

	if (argc == 1)
	{	// in case no arguments were given
		printHelp(true);
		return EXIT_SUCCESS;
	}

	--argc;
	++argv;
	Arguments args;

	try
	{
		args = parseArguments(argc, argv);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An error occured while parsing arguments: "
			<< ex.what() << "\n";
		printHelp(false);

		return EXIT_FAILURE;
	}

	if (args.command == COMMAND_HELP)
	{
		printHelp(true);
		return EXIT_SUCCESS;
	}

	// create the symbol directory for the BDD-based automata
	BDDTopDownTreeAut::StringToSymbolDict bddSymbolDict;
	BDDTopDownTreeAut::SetSymbolDictPtr(&bddSymbolDict);
	BDDBottomUpTreeAut::SetSymbolDictPtr(&bddSymbolDict);

	// create the ``next symbol'' variable for the BDD-based automata
	BDDTopDownTreeAut::SymbolType bddNextSymbol(BDD_SIZE, 0);
	BDDTopDownTreeAut::SetNextSymbolPtr(&bddNextSymbol);
	BDDBottomUpTreeAut::SetNextSymbolPtr(&bddNextSymbol);

	// create the symbol directory for explicit automata
	ExplicitTreeAut::StringToSymbolDict explSymbolDict;
	ExplicitTreeAut::SetSymbolDictPtr(&explSymbolDict);

	// create the ``next symbol'' variable for the explicit automaton
	ExplicitTreeAut::SymbolType explNextSymbol(0);
	ExplicitTreeAut::SetNextSymbolPtr(&explNextSymbol);

	// create the ``next state'' variable
	AutBase::StateType nextState(0);
	ExplicitTreeAut::SetNextStatePtr(&nextState);

	// create the symbol directory for finite automata
	ExplicitFiniteAut::StringToSymbolDict explFASymbolDict;
	ExplicitFiniteAut::SetSymbolDictPtr(&explFASymbolDict);
	
	// create the ``next symbol`` variable for the explicit finite automaton
	ExplicitFiniteAut::SymbolType explFANextSymbol(0);
	ExplicitFiniteAut::SetNextSymbolPtr(&explFANextSymbol);

	// create the ``next state`` variable
	AutBase::StateType explFANextState(0);
	ExplicitFiniteAut::SetNextStatePtr(&explFANextState);

	VATA::AutBase::StringToStateDict stateDict;
	
	typedef typename ExplicitFiniteAut::SymbolBackTranslatorStrict SymbolBackTranslatorStrict;

	try
	{
		if (args.representation == REPRESENTATION_BDD_TD)
		{
			return executeCommand<BDDTopDownTreeAut>(args);
		}
		else if (args.representation == REPRESENTATION_BDD_BU)
		{
			return executeCommand<BDDBottomUpTreeAut>(args);
		}
		else if (args.representation == REPRESENTATION_EXPLICIT)
		{
			return executeCommand<ExplicitTreeAut>(args);
		}
		else if (args.representation == REPRESENTATION_EXPLICIT_FA)
		{
			return executeCommand<ExplicitFiniteAut>(args);
		}
/*
			ExplicitFiniteAut aut1;

			aut1.devel(*(new TimbukParser()),VATA::Util::ReadFile(args.fileName1), stateDict);

			std::unique_ptr<AbstrSerializer> serializer(nullptr);
			serializer.reset(new TimbukSerializer());

			std::cout << aut1.DumpToString(*(serializer.get()),
					StateBackTranslatorStrict(stateDict.GetReverseMap()),
					SymbolBackTranslatorStrict(aut1.GetSymbolDict().GetReverseMap()));

			return EXIT_FAILURE;
		}
		else
		{
			std::cerr << "Internal error: invalid representation\n";
			return EXIT_FAILURE;
		}
		*/
	}
	catch (std::exception& ex)
	{
		std::cerr << "An error occured: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}
}
