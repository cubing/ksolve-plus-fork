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

// Functions for reading the definition file

#ifndef RULES_H
#define RULES_H

class Rules {
public:
	Rules(string filename){
		std::ifstream fin(filename.c_str());
		if (!fin.good()){
			std::cout << "Can't open puzzle definition!\n";
			exit(-1);
		}
			
		while(!fin.eof()){
				string command;
				fin >> command;
				
				if (command == "Name"){
					getline(fin, name);
				}
				else if (command == "Set"){
					string setname;
					fin >> setname;
					if (datasets.find(setname) != datasets.end()) {
						std::cerr << "Set " << setname << " declared more than once.\n";
						exit(-1);
					}                
					fin >> datasets[setname].size;
					if (fin.fail() || datasets[setname].size < 1){
						std::cerr << "Set " << setname << " does not have positive size.\n";
						exit(-1);
					}
					fin >> datasets[setname].omod;
					if (fin.fail() || datasets[setname].omod < 0){
						std::cerr << "Pieces in " << setname << " does not have a positive (or zero) number of possible orientations.\n";
						exit(-1);
					}
					datasets[setname].ptabletype = TABLE_TYPE_NONE;
					datasets[setname].otabletype = TABLE_TYPE_NONE;
					datasets[setname].oparity = true; // adjust later if necessary
				}
				else if (command == "Move"){
					string movename, setname;
					fin >> movename;
					if (moves.find(movename) != moves.end()) {
						std::cerr << "Move " << movename << " declared more than once.\n";
						exit(-1);
					}
					
					fullmove newMove;
					newMove.name = movename;
					newMove.parentMove = movename;
					newMove.qtm = 1;
					newMove.state = readPosition(fin, true, false, "move "+movename);
					
					parentMoves.push_back(movename);
					addPowers(newMove, datasets);
					adjustOParity(datasets, newMove.state);
					moves[movename] = newMove;
				}
				else if (command == "Solved"){
					solved = readPosition(fin, false, true, "solved state");
				}
				else if (command == "ForbiddenPairs"){
					string movename1, movename2;  
					fin >> movename1;
					while(movename1 != "End") {
						if (fin.fail()){
							std::cerr << "Error reading forbidden pairs.\n";
							exit(-1);
						}
						if (moves.find(movename1) == moves.end()){
							std::cerr << "Move " << movename1 << " used in forbidden pairs is not previously declared.\n";
							exit(-1);
						}
						
						fin >> movename2;
						if (fin.fail()){
							std::cerr << "Error reading forbidden pairs.\n";
							exit(-1);
						}
						if (moves.find(movename2) == moves.end()){
							std::cerr << "Move " << movename2 << " used in forbidden pairs is not previously declared.\n";
							exit(-1);
						}
						
						forbidden.insert(MovePair(movename1, movename2));
						fin >> movename1;
					}
				}
				else if (command == "ParallelMoves"){
					std::cout << "ParallelMoves command is deprecated!\n";
					string newmove;
					fin >> newmove;
					while(newmove != "End") {
						fin >> newmove;
					}
				}
				else if (command == "ForbiddenGroups"){
					string line, move1;
					std::vector<string> group;
					getline(fin, line);
					getline(fin, line);
					std::istringstream input(line);
					input >> move1;
					while(move1 != "End"){
						if (moves.find(move1) == moves.end()){
							std::cerr << "Move " << move1 << " used in ForbiddeGroups is not previously declared.\n";
							exit(-1);
						}
							 
						group.clear();
						group.push_back(move1);
						while(!input.eof()){
							input >> move1;
							if (!input.fail()){
								if (moves.find(move1) == moves.end()){
									std::cerr << "Move " << move1 << " used in ForbiddenGroups is not previously declared.\n";
									exit(-1);
								}
								group.push_back(move1);
							}
						}
						
						for (int i = 0; i < group.size(); i++){
							for (int j = 0; j < group.size(); j++){
								forbidden.insert(MovePair(group[i], group[j]));
							}
						}

						getline(fin, line);
						input.str(line);
						input.clear();     
						input >> move1;         
					}
				}
				else if (command == "Multiplicators"){
					std::cout << "Multiplicators command is deprecated!\n";
					string newmove;
					fin >> newmove;
					while(newmove != "End") {
						fin >> newmove;
					}
				}
				else if (command == "Ignore"){
					ignore = readPosition(fin, false, false, "Ignore command");      
				}
				else if (command == "Block"){
					Block tmp_block;
					string setname, line;
					fin >> setname;
					while(setname != "End"){
						std::set<int> tmp;
						if (datasets.find(setname) == datasets.end()) {
							std::cerr << "Set " << setname << " used in Block is not previously declared.\n";
							exit(-1);
						}
						getline(fin, line); // To get past the linefeed
						getline(fin, line);
						std::istringstream input(line);
						int piece;
						while(!input.eof()){
							input >> piece;
							if (piece > datasets[setname].size || piece <= 0) {
								std::cerr << "Piece " << piece << " in Block, should not be in set " << setname << "\n";
								exit(-1);
							}
							if (!input.fail()){
								tmp.insert(piece);
							}
						}
						tmp_block[setname] = tmp;
						fin >> setname;
					}
					blocks.insert(tmp_block);
				}
				else if (command == "MoveLimits"){
					std::cout << "MoveLimits command has been moved to scramble file!\n";
					string newmove;
					fin >> newmove;
					while(newmove != "End") {
						fin >> newmove;
					}
				}
				else if (command == "#"){ // Comment
					char buff[500];
					fin.getline(buff,500);
				}
				else if (command == "") {} // Empty line
				else
					std::cerr << "Unknown command " << command << "\n";
		}
		
		processParallelMoves();
	}

	void print(void) // for debugging
	{
		std::cout << "Puzzle name: " << name << "\n";
		PieceTypes::iterator iter;
		for (iter = datasets.begin(); iter != datasets.end(); iter++)
		{
			std::cout << iter->first << " has type " << iter->second.type << ", " << iter->second.size << " elements and is counted mod " << iter->second.omod << " (parity = " << iter->second.oparity << ")\n";     
		}
		std::cout << "\n";
		
		MoveList::iterator iter2;
		for (iter2 = moves.begin(); iter2 != moves.end(); iter2++)
		{
			std::cout << "Move " << iter2->first << " moves the following sets:\n";
			printPosition(iter2->second.state);
			std::cout << "\n";
		}
		std::cout << "Solved state:\n";
		printPosition(solved);
	}
	
	void adjustOParity(PieceTypes& datasets, Position move) {
		Position::iterator iter;
		for (iter = move.begin(); iter != move.end(); iter++) {
			int omod = datasets[iter->first].omod;
			
			// compute sum of orientations in this move
			int osum = 0;
			for (int i=0; i<iter->second.size; i++) {
				osum += iter->second.orientation[i];
			}
			
			// if this move changes the sum of orientations, no parity constraint
			if (osum % omod != 0) {
				datasets[iter->first].oparity = false;
			}
		}
	}

	PieceTypes getDatasets(){
		return datasets;
	}

	Position getSolved(){
		return solved;
	}

	MoveList getMoves(){
		return moves;
	}

	std::set<MovePair> getForbiddenPairs(){
		return forbidden;
	}

	Position getIgnore(){
		return ignore;
	}

	std::set<Block> getBlocks(){
		return blocks;
	}
	
	std::map<string, int> getMoveLimits() {
		return moveLimits;
	}
	
private:
	string name; // The name of the puzzle
	PieceTypes datasets; // Size and properties of the state-data
	Position solved;
	Position ignore; // 0 = solve piece, 1 = don't solve piece
	MoveList moves; // Possible moves of the puzzle
	std::vector<string> parentMoves; // Names of parent moves
	std::set<MovePair> forbidden;
	std::set<Block> blocks;
	std::map<string, std::vector<string> > moveGroups;
	std::map<string, int> moveLimits; // limits on # of moves
	
	// Add all powers of this move, and keep track of them in moveGroups
	void addPowers(fullmove move, PieceTypes& datasets) {
		std::vector<string> moveGroup;
		moveGroup.push_back(move.name);
		
		// Find order of move
		Position fixedState; // fix state to remove weird orientations
		Position::iterator iter;
		for (iter = move.state.begin(); iter != move.state.end(); iter++){
			fixedState[iter->first] = newSubstate(iter->second.size);
			for (int i=0; i<iter->second.size; i++) {
				fixedState[iter->first].orientation[i] = move.state[iter->first].orientation[i] % datasets[iter->first].omod;
				fixedState[iter->first].permutation[i] = move.state[iter->first].permutation[i];
			}
		}
		
		fullmove move2;
		move2.state.insert(fixedState.begin(), fixedState.end());
		int order = 0;
		do {
			move2.state = mergeMoves(move2.state, fixedState, datasets);
			order++;
		} while (!isEqual(move2.state, fixedState));
		
		// Add derived moves
		int i, j;
		move2 = move;
		for (i=1; i<=order-2; i++) {
			move2.state = mergeMoves(move2.state, move.state, datasets);
			
			// determine name of move
			std::stringstream ss;
			int qtm;
			if (i < order/2) {
				ss << move.name << (i+1);
				qtm = i+1;
			} else {
				ss << move.name;
				qtm = order-i-1;
				if (i < order-2) {
					ss << (order-i-1);
				}
				ss << "'";
			}
			string newName = ss.str();
			
			// add updated move to moveGroup and list of moves
			moveGroup.push_back(newName);
			fullmove newMove;
			newMove.name = newName;
			newMove.parentMove = move.name;
			newMove.qtm = qtm;
			newMove.state.insert(move2.state.begin(), move2.state.end());
			moves[newName] = newMove;
		}
		
		// Forbid all pairs
		for (i=0; i<=order-2; i++) {
			for (j=0; j<=order-2; j++) {
				forbidden.insert(MovePair(moveGroup[i], moveGroup[j]));
			}
		}

		moveGroups[move.name] = moveGroup;
	}
	
	// determine which moves are parallel, and process them
	void processParallelMoves() {
		int i, j;
		MoveList::iterator iter1, iter2;
		Position ij, ji;
		
		// loop through pairs of moves
		for (i=0; i<parentMoves.size(); i++) {
			for (j=i+1; j<parentMoves.size(); j++) {
			
				// check if this pair is parallel (i*j = j*i)
				ij.clear();
				ji.clear();
				ij = mergeMoves(moves[parentMoves[i]].state, moves[parentMoves[j]].state, datasets);
				ji = mergeMoves(moves[parentMoves[j]].state, moves[parentMoves[i]].state, datasets);
				if (isEqual(ij, ji)) {
					
					// if so, forbid any move with parent i followed by any move with parent j
					for (iter1 = moves.begin(); iter1 != moves.end(); iter1++) {
						if (0 == iter1->second.parentMove.compare(parentMoves[i])) {
							for (iter2 = moves.begin(); iter2 != moves.end(); iter2++) {
								if (0 == iter2->second.parentMove.compare(parentMoves[j])) {
									// if we have not already forbidden the [j,i] move pair, forbid the [i,j] one
									if (forbidden.find(MovePair(iter2->second.name, iter1->second.name)) == forbidden.end()) {
										forbidden.insert(MovePair(iter1->second.name, iter2->second.name));
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	// read in a position from fin
	Position readPosition(std::ifstream& fin, bool checkUnique, bool setUnique, string title) {
		Position newPosition;
		string setname, tmpStr;
		long i, tmpInt;
		fin >> setname;
		while (setname != "End") {
			// check that this set is defined, but not used in this position yet
			if (datasets.find(setname) == datasets.end()) {
				std::cerr << "Set " << setname << " used in " << title << " is not previously declared.\n";
				exit(-1);
			} 
			if (newPosition.find(setname) != newPosition.end()) {
				std::cerr << "Set " << setname << " defined more than once in " << title << ".\n";
				exit(-1);
			}
			
			// create new substate
			newPosition[setname] = newSubstate(datasets[setname].size);
			if (newPosition[setname].permutation == NULL || newPosition[setname].orientation == NULL){
				std::cerr << "Could not allocate memory in " << title << ".\n";
				exit(-1);
			}
						
			// read in permutation
			for (i = 0; i < datasets[setname].size; i++){
				fin >> tmpInt;
				if (fin.fail()){
					std::cerr << "Error reading " << setname << " permutation in " << title << ".\n";
					exit(-1);
				}
				newPosition[setname].permutation[i] = tmpInt;
			}
			
			// do unique permutation stuff
			if (checkUnique) {
				if (!uniquePermutation(newPosition[setname].permutation, newPosition[setname].size)){
					std::cerr << "Permutation for set " << setname << " in " << title << " has repeated numbers.\n";
					exit(-1);
				}
			}
			if (setUnique) {
				datasets[setname].uniqueperm = uniquePermutation(newPosition[setname].permutation, newPosition[setname].size);
			}
			
			// set orientation to zeros (in case user did not give it)
			for (i = 0; i < datasets[setname].size; i++){
				newPosition[setname].orientation[i] = 0;
			}
			
			// read something in. if it doesn't look like a number,
			// use it as the setname. otherwise, read in orientation
			fin >> tmpStr;
			if (tmpStr.at(0) < '0' || tmpStr.at(0) > '9') {
				setname = tmpStr;
				continue;
			}
			for (i = 0; i < datasets[setname].size; i++){
				if (i==0) {
					tmpInt = atol(tmpStr.c_str());
				} else {
					fin >> tmpInt;
				}
				if (fin.fail()){
					std::cerr << "Error reading " << setname << " orientation in " << title << ".\n";
					exit(-1);
				}
				newPosition[setname].orientation[i] = tmpInt;
			}
			
			// get next setname
			fin >> setname;
		}
		return newPosition;
	}
};

#endif
