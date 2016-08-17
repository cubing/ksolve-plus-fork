/*
 KSolve+ - Puzzle solving program.
 Copyright (C) 2007-2013 Kåre Krig and Michael Gottlieb

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Main struct and control flow of program, with all includes used in it

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

struct ksolve {
	#include "data.h"
	#include "move.h"
	#include "blocks.h"
	#include "checks.h"
	#include "indexing.h"
	#include "pruning.h"
	#include "search.h"
	#include "readdef.h"
	#include "readscramble.h"
	#include "god.h"

	static int ksolveMain(int argc, char *argv[]) {

		srand(time(NULL)); // initialize RNG in case we need it

		if (argc != 3){
			std::cerr << "ksolve+ v1.3a - Linux Port by Matt Stiefel\n";
			std::cerr << "(c) 2007-2013 by Kare Krig and Michael Gottlieb\n";
			std::cerr << "Usage: ksolve [def-file] [scramble-file]\n";
			std::cerr << "See readme for additional help.\n";
			return EXIT_FAILURE;
		}


		std::ifstream definitionStream(argv[1]);
		if (!definitionStream.good()){
			std::cout << "Can't open definition file!\n";
			exit(-1);
		}
		std::ifstream scrambleStream(argv[2]);
		if (strcmp(argv[2], "!") != 0 && !scrambleStream.good()){
			std::cout << "Can't open scramble file!\n";
			exit(-1);
		}

		string defFileName(argv[1]);
		string scrambleFileName(argv[2]);
		return ksolveWrapped(definitionStream, scrambleStream, defFileName, scrambleFileName, true);

	}

	static int ksolveWrapped(std::istream &definitionStream,
													 std::istream &scrambleStream,
													 string defFileName,
													 string scrambleFileName,
													 bool usePruneTable)
	{

		clock_t start;
		start = clock();

		// Load the puzzle rules
		Rules ruleset(definitionStream);
		PieceTypes datasets = ruleset.getDatasets();
		Position solved = ruleset.getSolved();
		MoveList moves = ruleset.getMoves();
		std::set<MovePair> forbidden = ruleset.getForbiddenPairs();
		Position ignore = ruleset.getIgnore();
		std::vector<Block> blocks = ruleset.getBlocks();
		std::cout << "Ruleset loaded.\n";

		// Print all generated moves
		std::cout << "Generated moves: ";
		int i = 0;
		MoveList::iterator moveIter;
		for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++) {
			if (moveIter->first != moveIter->second.parentID) {
				if (i>0) std::cout << ", ";
				i++;
				std::cout << moveIter->second.name;
			}
		}
		std::cout << ".\n";

		// Compute or load the pruning tables
		PruneTable tables;
		tables = getCompletePruneTables(solved, moves, datasets, ignore, defFileName, usePruneTable);
		std::cout << "Pruning tables loaded.\n";

		//datasets = updateDatasets(datasets, tables);
		updateDatasets(datasets, tables);

		// God's Algorithm tables
		std::string godHTM = "!";
		std::string godQTM = "!q";
		if (0==godHTM.compare(scrambleFileName)) {
			std::cout << "Computing God's Algorithm tables (HTM)\n";
			godTable(solved, moves, datasets, forbidden, ignore, blocks, 0);
			std::cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s\n";
			return EXIT_SUCCESS;
		} else if (0==godQTM.compare(scrambleFileName)) {
			std::cout << "Computing God's Algorithm tables (QTM)\n";
			godTable(solved, moves, datasets, forbidden, ignore, blocks, 1);
			std::cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s\n";
			return EXIT_SUCCESS;
		}

		// Load the scramble to be solved
		Scramble states(scrambleStream, solved, moves, datasets, blocks);
		std::cout << "Scrambles loaded.\n";

		ScrambleDef scramble = states.getScramble();

		while(scramble.state.size() != 0){
			int depth = 0;
			string temp_a, temp_b;
			temp_a = " ";

			std::cout << "\nSolving " << scramble.name.c_str() << "\n";

			if (scramble.printState == 1) {
				std::cout << "Scramble position:\n";
				printPosition(scramble.state);
			}

			// give out a warning if we have some undefined permutations on a bandaged puzzle
			if (blocks.size() != 0) {
				bool hasUndefined = false;
				Position::iterator iter;
				for (iter = scramble.state.begin(); iter != scramble.state.end(); iter++) {
					int setsize = iter->second.size;
					for (int i = 0; i < setsize; i++) {
						if (iter->second.permutation[i] == -1) {
							hasUndefined = true;
						}
					}
				}
				if (hasUndefined) {
					std::cout << "Warning: using blocks, but scramble has unknown (?) permutations!\n";
				}
			}

			// get rid of any moves that are zeroed out in moveLimits
			// and set .limited for each move
			MoveList moves2;
			MoveList::iterator iter2;
			for (iter2 = moves.begin(); iter2 != moves.end(); iter2++){
				moves2[iter2->first] = iter2->second;
			}
			processMoveLimits(moves2, scramble.moveLimits);

			std::cout << "Depth 0\n";

			// The tree-search for the solution(s)
			int usedSlack = 0;
			while(1) {
				bool foundSolution = treeSolve(scramble.state, solved, moves, datasets, tables, forbidden, scramble.ignore, blocks, depth, scramble.metric, scramble.moveLimits, temp_a, -1, true);
				if (foundSolution || usedSlack > 0) {
					usedSlack++;
					if (usedSlack > scramble.slack) break;
				}
				depth++;
				if (depth > scramble.max_depth){
					std::cout << "\nMax depth reached, aborting.\n";
					break;
				}
				std::cout << "Depth " << depth << "\n";
			}
			std::cout << "\n";

			scramble = states.getScramble();
		}

		std::cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s\n";

		return EXIT_SUCCESS;
	}
};

int main(int argc, char *argv[]) {
	ksolve::ksolveMain(argc, argv);
}

extern "C" void solve(char* definition, char* state) {
	std::istringstream definitionStream(definition);
	std::istringstream scrambleStream(state);
	ksolve::ksolveWrapped(definitionStream, scrambleStream, "dummy", "dummy", false);
}
