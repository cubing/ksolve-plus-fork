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

// Functions for reading the scramble file

#ifndef SCRAMBLE_H
#define SCRAMBLE_H

class Scramble
{
public:
	Scramble(string filename, Position& solved, MoveList& moves, PieceTypes datasets, std::vector<Block>& blocks){
		sent = 0;
		int current_max = 999;
		int current_slack = 0;
		int current_metric = 0;
		Position state;
		Position ignore;
		string name;
		moveLimits.clear();
		std::ifstream fin(filename.c_str());
		if (!fin.good()){
			std::cout << "Can't open scramble file!\n";
			exit(-1);
		}
	   
		while(!fin.eof()){
			string command;
			fin >> command;

			// Scramble - read a single scramble's definition
			if (command == "Scramble"){
				getline(fin, name);
				if (name.size() >= 1) name = name.substr(1);
			 
				string setname, tmpStr;
				long i;
				fin >> setname;
				state.clear();
				ignore.clear();
				while(setname != "End"){
					if (fin.fail()){
						std::cerr << "Error reading scramble sets.\n";
						exit(-1);
					}
				
					// Check set names for consistency
					if (state.find(setname) != state.end()){
						std::cerr << "Set " << setname << " declared more than once in scramble " << name << ".\n";
						exit(-1);
					}
					if (datasets.find(setname) == datasets.end()){
						std::cerr << "Unknown set " << setname << " in scramble " << name <<".\n";
						exit(-1);
					}
				
					// initialize some info
					state[setname] = newSubstate(datasets[setname].size);
					ignore[setname] = newSubstate(datasets[setname].size);
					if (state[setname].permutation == NULL || state[setname].orientation == NULL ||
						ignore[setname].permutation == NULL || ignore[setname].orientation == NULL){
						std::cerr << "Can't allocate memory in Scramble::Scramble(...)\n";
						exit(-1);
					}

					// read permutation
					for (i = 0; i < datasets[setname].size; i++){
						fin >> tmpStr;
						if (fin.fail()){
							std::cerr << "Error reading " << setname << " permutation for scramble " << name << ".\n";
							exit(-1);
						}
						if (tmpStr.at(0) == '?') {
							ignore[setname].permutation[i] = 1;
							if (tmpStr.size() == 1) { // just a ?
								state[setname].permutation[i] = -1;
								
								// throw an error message if using blocks
								if (blocks.size() != 0) {
									std::cerr << "Cannot use unknown permutations on puzzles with blocks!\n";
									exit(-1);
								}
							} else { // ? and then a number
								string tmp2 = tmpStr.substr(1);
								state[setname].permutation[i] = atol(tmp2.c_str());
							}
						} else {
							state[setname].permutation[i] = atol(tmpStr.c_str());
							ignore[setname].permutation[i] = 0;
						}
					}
					
					// set orientation to zeros (in case user did not give it)
					for (i = 0; i < datasets[setname].size; i++){
						state[setname].orientation[i] = 0;
						ignore[setname].orientation[i] = 0;
					}
					
					// read something in. if it doesn't look like a number,
					// use it as the setname. otherwise, read in orientation
					fin >> tmpStr;
					if (tmpStr.at(0) != '?' && (tmpStr.at(0) < '0' || tmpStr.at(0) > '9')) {
						setname = tmpStr;
						continue;
					}
					
					for (i = 0; i < datasets[setname].size; i++){
						if (i>0) {
							fin >> tmpStr;
						}
						if (fin.fail()){
							std::cerr << "Error reading " << setname << " orientation for scramble " << name << ".\n";
							exit(-1);
						}
						if (tmpStr.at(0) == '?') {
							ignore[setname].orientation[i] = 1;
							if (tmpStr.size() == 1) { // just a ?
								state[setname].orientation[i] = 0;
							} else { // ? and then a number
								string tmp2 = tmpStr.substr(1);
								state[setname].orientation[i] = atol(tmp2.c_str());
							}
						} else {
							int tmp2 = atoi(tmpStr.c_str()) % datasets[setname].omod;
							if (tmp2 < 0)
								tmp2 += datasets[setname].omod;
							state[setname].orientation[i] = tmp2;
							ignore[setname].orientation[i] = 0;
						}
					}
				
					fin >> setname;
				}
				
				ScrambleDef scramble;
				scramble.name = name;
				scramble.state = state;
				scramble.ignore = ignore;
				scramble.max_depth = current_max;
				scramble.slack = current_slack;
				scramble.metric = current_metric;
				scramble.printState = 0;
				scramble.moveLimits = std::vector<MoveLimit>();
				for (unsigned int i=0; i<moveLimits.size(); i++) {
					scramble.moveLimits.push_back(moveLimits[i]);
				}
				states.push_back(scramble);
			}
			// ScrambleAlg - read a scramble as a set of moves
			else if (command == "ScrambleAlg"){
				getline(fin, name);
				if (name.size() >= 1) name = name.substr(1);
				
				// initialize state to solved, and ignore to empty
				state.clear();
				ignore.clear();
				Position new_state;
				PieceTypes::iterator iter;
				for (iter = datasets.begin(); iter != datasets.end(); iter++)
				{
					int size = iter->second.size;
					state[iter->first] = newSubstate(size);
					ignore[iter->first] = newSubstate(size);
					new_state[iter->first] = newSubstate(size);
					if (state[iter->first].permutation == NULL || state[iter->first].orientation == NULL ||
						ignore[iter->first].permutation == NULL || ignore[iter->first].orientation == NULL ||
						new_state[iter->first].permutation == NULL || new_state[iter->first].orientation == NULL){
						std::cerr << "Can't allocate memory in Scramble::Scramble(...)\n";
						exit(-1);
					}
					for (int i=0; i<size; i++) {
						state[iter->first].permutation[i] = solved[iter->first].permutation[i];
						state[iter->first].orientation[i] = solved[iter->first].orientation[i];
						ignore[iter->first].permutation[i] = 0;
						ignore[iter->first].orientation[i] = 0;
					}
				}
					
				string movename;
				fin >> movename;
				while(movename != "End"){
					if (fin.fail()) {
						std::cerr << "Error reading scramble " << name << ".\n";
						exit(-1);
					}
					
					// apply move called movename to solved, if possible
					if (!moveIn(movename, moves)) {
						std::cerr << "Move " << movename << " in scramble " << name << " is unknown.\n";
						exit(-1);
					}
					
					if (blocks.size() != 0) {
						if (!blockLegal(state, blocks, moves[getMoveID(movename, moves)].state)) {
							std::cerr << "Move " << movename << " in scramble " << name << " is blocked.\n";
							exit(-1);
						}
					}
					applyMove(state, new_state, moves[getMoveID(movename, moves)].state, datasets);
					
					// copy new_state to state
					for (iter = datasets.begin(); iter != datasets.end(); iter++) {
						int size = iter->second.size;
						for (int i=0; i<size; i++) {
							state[iter->first].permutation[i] = new_state[iter->first].permutation[i];
							state[iter->first].orientation[i] = new_state[iter->first].orientation[i];
						}
					}
				
					fin >> movename;
				}
				
				ScrambleDef scramble;
				scramble.name = name;
				scramble.state = state;
				scramble.ignore = ignore;
				scramble.max_depth = current_max;
				scramble.slack = current_slack;
				scramble.metric = current_metric;
				scramble.printState = 1;
				scramble.moveLimits = std::vector<MoveLimit>();
				for (unsigned int i=0; i<moveLimits.size(); i++) {
					scramble.moveLimits.push_back(moveLimits[i]);
				}
				states.push_back(scramble);
			}
			// RandomScramble - read a scramble as a set of moves
			else if (command == "RandomScramble"){
				getline(fin, name);
				if (name.size() >= 1) name = name.substr(1);
				
				// initialize state to solved, and ignore to empty
				state.clear();
				ignore.clear();
				Position new_state;
				PieceTypes::iterator iter;
				for (iter = datasets.begin(); iter != datasets.end(); iter++)
				{
					int size = iter->second.size;
					state[iter->first] = newSubstate(size);
					ignore[iter->first] = newSubstate(size);
					new_state[iter->first] = newSubstate(size);
					if (state[iter->first].permutation == NULL || state[iter->first].orientation == NULL ||
						ignore[iter->first].permutation == NULL || ignore[iter->first].orientation == NULL ||
						new_state[iter->first].permutation == NULL || new_state[iter->first].orientation == NULL){
						std::cerr << "Can't allocate memory in Scramble::Scramble(...)\n";
						exit(-1);
					}
					for (int i=0; i<size; i++) {
						state[iter->first].permutation[i] = solved[iter->first].permutation[i];
						state[iter->first].orientation[i] = solved[iter->first].orientation[i];
						ignore[iter->first].permutation[i] = 0;
						ignore[iter->first].orientation[i] = 0;
					}
				}
					
				string movename;
				fin >> movename;
				while(movename != "End"){
					if (fin.fail()){
						std::cerr << "Error reading scramble " << name << ".\n";
						exit(-1);
					}
				
					fin >> movename;
				}
				
				// apply a bunch of random moves
				int RANDOM_MOVES = 10000 + (rand()%2);
				int nMoves = moves.size();
				for (int i=0; i<RANDOM_MOVES; i++) {
					// get random move
					MoveList::iterator iter2 = moves.begin();
					std::advance(iter2, rand() % nMoves);
					
					if (blocks.size() != 0) {
						if (!blockLegal(state, blocks, iter2->second.state)) {
							i--;
							continue;
						}
					}
					
					applyMove(state, new_state, iter2->second.state, datasets);
					
					// copy new_state to state
					for (iter = datasets.begin(); iter != datasets.end(); iter++)
					{
						int size = iter->second.size;
						for (int i=0; i<size; i++) {
							state[iter->first].permutation[i] = new_state[iter->first].permutation[i];
							state[iter->first].orientation[i] = new_state[iter->first].orientation[i];
						}
					}
				}
				
				ScrambleDef scramble;
				scramble.name = name;
				scramble.state = state;
				scramble.ignore = ignore;
				scramble.max_depth = current_max;
				scramble.slack = current_slack;
				scramble.metric = current_metric;
				scramble.printState = 1;
				scramble.moveLimits = std::vector<MoveLimit>();
				for (unsigned int i=0; i<moveLimits.size(); i++) {
					scramble.moveLimits.push_back(moveLimits[i]);
				}
				states.push_back(scramble);
			}
			// MaxDepth - maximum depth to search to
			else if (command == "MaxDepth"){
				fin >> current_max;
				if (fin.fail()){
					std::cerr << "Error reading MaxDepth\n";
					exit(-1);
				}
			}
			// Slack - extra depth after optimal
			else if (command == "Slack"){
				fin >> current_slack;
				if (fin.fail()){
					std::cerr << "Error reading Slack\n";
					exit(-1);
				}
			}
			// QTM - use QTM
			else if (command == "QTM") {
				current_metric = 1;
			}
			// HTM - use HTM
			else if (command == "HTM") {
				current_metric = 0;
			}
			// Move Limits
			else if (command == "MoveLimits"){
				moveLimits.clear();
				string movename;
				int limit;
				fin >> movename;
				while(movename != "End") {
					if (fin.fail() || movename.size() < 1) {
						std::cerr << "Error reading move limits.\n";
						exit(-1);
					}
					
					// get basic elements of movelimit
					MoveLimit ml;
					ml.moveGroup = (movename.at(movename.size()-1) == '*');
					if (ml.moveGroup)
						movename = movename.substr(0, movename.size()-1);
					
					if (!moveIn(movename, moves)) {
						std::cerr << "Move " << movename << " used in move list is not previously declared.\n";
						exit(-1);
					}
					fin >> limit;
					if (fin.fail()){
						std::cerr << "Error reading move limits.\n";
						exit(-1);
					}
					ml.move = getMoveID(movename, moves);
					ml.limit = limit;
					
					// calculate block
					Block owned;
					PieceTypes::iterator pieceIter;
					MoveList::iterator moveIter;
					// for each type of piece...
					for (pieceIter = datasets.begin(); pieceIter != datasets.end(); pieceIter++) {
						int size = pieceIter->second.size;
						owned[pieceIter->first] = std::set<int>();
						// for each individual piece...
						for (int i=0; i<size; i++) {
							// if all moves affecting it are limited here...
							bool allLimited = true;
							for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++) {
								if (moveIter->second.state[pieceIter->first].permutation[i] != i+1 || moveIter->second.state[pieceIter->first].orientation[i] != 0) {
									if (!limitMatches(ml, moveIter->second)) {
										allLimited = false;
									}
								}
							}

							// ...add it to the block
							if (allLimited) {
								owned[pieceIter->first].insert(i);
							}
						}
					}
					ml.owned = owned;				
					
					moveLimits.push_back(ml);
				
					fin >> movename;
				}
			}
			else if (command.at(0) == '#'){ // Comment
					char buff[500];
					fin.getline(buff,500);
			}
			else if (command == "") {} // Empty line
			else {
				std::cerr << "Unknown command \"" << command << "\" in scramble file.\n";  
				exit(-1);
			}
		}               
	}
	
	ScrambleDef getScramble(){
		if (states.size() > sent){
			sent++;
			return states[sent-1];
		}
		ScrambleDef tmp;
		return tmp;
	}
	
private:
	std::vector<ScrambleDef> states;
	std::vector<MoveLimit> moveLimits;
	unsigned int sent;
};

#endif
