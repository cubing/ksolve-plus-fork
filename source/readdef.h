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
	Rules(std::istream &fin){
		moveid = 0;
			
		while(!fin.eof()){
				string command;
				fin >> command;
				if (command.size() == 0) {
					break;
				}
				if (command == "Name"){
					getline(fin, name);
				}
				else if (command == "Set"){
					string setname;
					fin >> setname;
					int setindex = setnameIndex(setname) ;
					if (datasets.find(setindex) != datasets.end()) {
						std::cerr << "Set " << setname << " declared more than once.\n";
						exit(-1);
					}
					if (moves.size() > 0 || solved.size() > 0 || ignore.size() > 0) {
						std::cerr << "You must define all sets first!\n";
						exit(-1);
					}
					fin >> datasets[setindex].size;
					if (fin.fail() || datasets[setindex].size < 1){
						std::cerr << "Set " << setname << " does not have positive size.\n";
						exit(-1);
					}
					fin >> datasets[setindex].omod;
					if (fin.fail() || datasets[setindex].omod < 0){
						std::cerr << "Pieces in " << setname << " does not have a positive (or zero) number of possible orientations.\n";
						exit(-1);
					}
					datasets[setindex].ptabletype = TABLE_TYPE_NONE;
					datasets[setindex].otabletype = TABLE_TYPE_NONE;
					datasets[setindex].oparity = true; // adjust later if necessary
				}
				else if (command == "Move"){
					string movename, setname;
					fin >> movename;
					if (moveIn(movename, moves)) {
						std::cerr << "Move " << movename << " declared more than once.\n";
						exit(-1);
					}
					
					fullmove newMove;
					newMove.name = movename;
					newMove.id = moveid;
					newMove.parentID = moveid;
					newMove.qtm = 1;
					newMove.state = readPosition(fin, true, false, "move "+movename);
					parentMoves.push_back(moveid);
					moves[moveid] = newMove;
					moveid++;
					addPowers(newMove, moveid-1, datasets);
					adjustOParity(datasets, newMove.state);
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
						if (moveIn(movename1, moves)){
							std::cerr << "Move " << movename1 << " used in forbidden pairs is not previously declared.\n";
							exit(-1);
						}
						
						fin >> movename2;
						if (fin.fail()){
							std::cerr << "Error reading forbidden pairs.\n";
							exit(-1);
						}
						if (moveIn(movename2, moves)){
							std::cerr << "Move " << movename2 << " used in forbidden pairs is not previously declared.\n";
							exit(-1);
						}
						
						forbidden.insert(MovePair(getMoveID(movename1, moves), 
												getMoveID(movename2, moves)));
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
					std::vector<int> group;
					getline(fin, line);
					getline(fin, line);
					std::istringstream input(line);
					input >> move1;
					while(move1 != "End"){
						if (!moveIn(move1, moves)){
							std::cerr << "Move " << move1 << " used in ForbiddeGroups is not previously declared.\n";
							exit(-1);
						}
							 
						group.clear();
						group.push_back(getMoveID(move1, moves));
						while(!input.eof()){
							input >> move1;
							if (!input.fail()){
								if (!moveIn(move1, moves)){
									std::cerr << "Move " << move1 << " used in ForbiddenGroups is not previously declared.\n";
									exit(-1);
								}
								group.push_back(getMoveID(move1, moves));
							}
						}
						
						for (unsigned int i = 0; i < group.size(); i++){
							for (unsigned int j = 0; j < group.size(); j++){
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
					        int setindex = setnameIndex(setname) ;
						std::set<int> tmp;
						if (datasets.find(setindex) == datasets.end()) {
							std::cerr << "Set " << setname << " used in Block is not previously declared.\n";
							exit(-1);
						}
						getline(fin, line); // To get past the linefeed
						getline(fin, line);
						std::istringstream input(line);
						int piece;
						while(!input.eof()){
							input >> piece;
							if (piece > datasets[setindex].size || piece <= 0) {
								std::cerr << "Piece " << piece << " in Block, should not be in set " << setname << "\n";
								exit(-1);
							}
							if (!input.fail()){
								tmp.insert(piece);
							}
						}
						tmp_block[setindex] = tmp;
						fin >> setname;
					}
					blocks.push_back(tmp_block);
				}
				else if (command == "MoveLimits"){
					std::cout << "MoveLimits command has been moved to scramble file!\n";
					string newmove;
					fin >> newmove;
					while(newmove != "End") {
						fin >> newmove;
					}
				}
				else if (command.at(0) == '#'){ // Comment
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

	std::vector<Block> getBlocks(){
		return blocks;
	}
	
	std::map<string, int> getMoveLimits() {
		return moveLimits;
	}
	
private:
	string name; // The name of the puzzle
	int moveid; // move ID (serial)
	PieceTypes datasets; // Size and properties of the state-data
	Position solved;
	Position ignore; // 0 = solve piece, 1 = don't solve piece
	MoveList moves; // Possible moves of the puzzle
	std::vector<int> parentMoves; // IDs of parent moves
	std::set<MovePair> forbidden;
	std::vector<Block> blocks;
	std::map<string, int> moveLimits; // limits on # of moves
	
	// Add all powers of this move
	void addPowers(fullmove move, int parentid, PieceTypes& datasets) {
		std::vector<int> moveGroup;
		moveGroup.push_back(parentid);
		
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
		} while (!isEqual(move2.state, fixedState, datasets));
		
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
			moveGroup.push_back(moveid);
			fullmove newMove;
			newMove.name = newName;
			newMove.parentID = parentid;
			newMove.id = moveid;
			newMove.qtm = qtm;
			newMove.state.insert(move2.state.begin(), move2.state.end());
			moves[moveid] = newMove;
			moveid++;
		}
		
		// Forbid all pairs
		for (i=0; i<=order-2; i++) {
			for (j=0; j<=order-2; j++) {
				forbidden.insert(MovePair(moveGroup[i], moveGroup[j]));
			}
		}
	}
	
	// determine which moves are parallel, and process them
	void processParallelMoves() {
		unsigned int i, j;
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
				if (isEqual(ij, ji, datasets)) {
					
					// if so, forbid any move with parent i followed by any move with parent j
					for (iter1 = moves.begin(); iter1 != moves.end(); iter1++) {
						if (iter1->second.parentID == parentMoves[i]) {
							for (iter2 = moves.begin(); iter2 != moves.end(); iter2++) {
								if (iter2->second.parentID == parentMoves[j]) {
									// if we have not already forbidden the [j,i] move pair, forbid the [i,j] one
									if (forbidden.find(MovePair(iter2->first, iter1->first)) == forbidden.end()) {
										forbidden.insert(MovePair(iter1->first, iter2->first));
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
	Position readPosition(std::istream& fin, bool checkUnique, bool setUnique, string title) {
		Position newPosition;
		string setname, tmpStr;
		long i, tmpInt;
		fin >> setname;
		while (setname != "End") {
			int setindex = setnameIndex(setname) ;
			// check that this set is defined, but not used in this position yet
			if (datasets.find(setindex) == datasets.end()) {
				std::cerr << "Set " << setname << " used in " << title << " is not previously declared.\n";
				exit(-1);
			} 
			if (newPosition.find(setindex) != newPosition.end()) {
				std::cerr << "Set " << setname << " defined more than once in " << title << ".\n";
				exit(-1);
			}
			
			// create new substate
			newPosition[setindex] = newSubstate(datasets[setindex].size);
			if (newPosition[setindex].permutation == NULL || newPosition[setindex].orientation == NULL){
				std::cerr << "Could not allocate memory in " << title << ".\n";
				exit(-1);
			}
						
			// read in permutation
			for (i = 0; i < datasets[setindex].size; i++){
				fin >> tmpInt;
				if (fin.fail()){
					std::cerr << "Error reading " << setname << " permutation in " << title << ".\n";
					exit(-1);
				}
				newPosition[setindex].permutation[i] = tmpInt;
			}
			
			// do unique permutation stuff
			if (checkUnique) {
				if (!uniquePermutation(newPosition[setindex].permutation, newPosition[setindex].size)){
					std::cerr << "Permutation for set " << setname << " in " << title << " has repeated numbers.\n";
					exit(-1);
				}
			}
			if (setUnique) {
				datasets[setindex].uniqueperm = uniquePermutation(newPosition[setindex].permutation, newPosition[setindex].size);
				calcOtherValues(datasets[setindex], newPosition[setindex].permutation) ;
			}
			
			// set orientation to zeros (in case user did not give it)
			for (i = 0; i < datasets[setindex].size; i++){
				newPosition[setindex].orientation[i] = 0;
			}
			
			// read something in. if it doesn't look like a number,
			// use it as the setname. otherwise, read in orientation
			fin >> tmpStr;
			if (tmpStr.at(0) < '0' || tmpStr.at(0) > '9') {
				setname = tmpStr;
				continue;
			}
			for (i = 0; i < datasets[setindex].size; i++){
				if (i==0) {
					tmpInt = atol(tmpStr.c_str());
				} else {
					fin >> tmpInt;
				}
				if (fin.fail()){
					std::cerr << "Error reading " << setname << " orientation in " << title << ".\n";
					exit(-1);
				}
				newPosition[setindex].orientation[i] = tmpInt;
			}
			
			// get next setname
			fin >> setname;
		}
		
		// add "solved" permutations for all undeclared positions!
		PieceTypes::iterator pieceIter;
		for (pieceIter = datasets.begin(); pieceIter != datasets.end(); pieceIter++) {
			int setname = pieceIter->first;
			if (newPosition.find(setname) == newPosition.end()) { // piece not included
				newPosition[setname] = newSubstate(datasets[setname].size);
				if (newPosition[setname].permutation == NULL || newPosition[setname].orientation == NULL){
					std::cerr << "Could not allocate memory in " << title << ".\n";
					exit(-1);
				}
				for (i = 0; i < datasets[setname].size; i++){
					newPosition[setname].orientation[i] = 0;
				}
				if (title == "Ignore command") { // ignore-type, permutation should be all 0's
					for (i = 0; i < datasets[setname].size; i++){
						newPosition[setname].permutation[i] = 0;
					}
				} else { // permutation-type, permutation should be 1 2 3 ...
					for (i = 0; i < datasets[setname].size; i++){
						newPosition[setname].permutation[i] = i+1;
					}
					if (setUnique) {
						datasets[setname].uniqueperm = true;
					}
				}
			}
		}
		
		return newPosition;
	}
	int ceillog2(int v) {
		int r = 0 ;
		while ((1LL << r) < v)
			r++ ;
		return r ;
	}
	void calcOtherValues(dataset &ds, int *perm) {
		ds.maxInSolved = 1 ;
		for (int i=0; i<ds.size; i++)
			if (perm[i] <= 0) {
				std::cerr << "Saw a nonpositive in perm" << std::endl ;
				exit(-1) ;
			} else if (perm[i] > ds.maxInSolved) {
				ds.maxInSolved = perm[i] ;
			}
		ds.permbits = ceillog2(ds.maxInSolved-1) ;
		ds.oribits = ceillog2(ds.omod) ;
	}
};

#endif
