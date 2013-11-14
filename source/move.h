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

// Functions for applying moves to puzzle states or single vectors.

#ifndef MOVE_H
#define MOVE_H

// faster version of original applyMove
static void applyMove(Position& state, Position& new_state, Position& move, PieceTypes& datasets){
	Position::iterator iter;
	for (iter = move.begin(); iter != move.end(); iter++){
		int size = iter->second.size;
		int omod = datasets[iter->first].omod;
		int* orientOut = new_state[iter->first].orientation;
		int* permute1 = state[iter->first].permutation;
		int* permute2 = iter->second.permutation;
		int* permuteOut = new_state[iter->first].permutation;
		
		if (omod == 1) {
			for (int i=0; i < size; i++) {
				orientOut[i] = 0;
				permuteOut[i] = permute1[permute2[i] - 1];
			}
		} else {
			int* orient1 = state[iter->first].orientation;
			int* orient2 = iter->second.orientation;
			for (int i=0; i < size; i++) {
				int permuted = permute2[i] - 1;
				orientOut[i] = (orient1[permuted] + orient2[permuted]) % omod;
				permuteOut[i] = permute1[permuted];
			}
		}
	}
}

static std::vector<int> applySubmoveO(std::vector<int> orientation, int change_o[], int change_p[], unsigned int size, int omod){
	if (size != orientation.size()){
		std::cerr << "Vectors not matching in size in call to applySubmoveO(...)\n";
		exit(-1);
	}
	unsigned int i;
	for (i = 0; i < size; i++)
		orientation[i] = (orientation[i] + change_o[i]) % omod;
	std::vector<int> temp;
	temp.resize(size);
	for (i = 0; i < size; i++)
		temp[i] = orientation[change_p[i] - 1];          
	return temp;  
}

static std::vector<int> applySubmoveP(std::vector<int> permutation, int change_p[], unsigned int size)
{
	if (size != permutation.size()){
		std::cerr << "Vectors not matching in size in call to applySubmoveP(...)\n";
		std::cerr << "size = " << size << ", permutation.size() = " << permutation.size() << "\n";
		exit(-1);
	}
	std::vector<int> temp;
	temp.resize(size);
	for (unsigned int i = 0; i < size; i++)
		temp[i] = permutation[change_p[i] - 1];          
	return temp;  
}

static int* applySubmoveP(int permutation[], int change_p[], int size)
{
	int* temp = new int[size];
	for (int i = 0; i < size; i++)
		temp[i] = permutation[change_p[i] - 1];          
	return temp;  
}

static Position mergeMoves(Position move1, Position move2, PieceTypes& datasets){
	Position ans;
	Position::iterator iter;    
	for (iter = move1.begin(); iter != move1.end(); iter++){
		substate temp;
		temp.permutation = applySubmoveP(move1[iter->first].permutation, move2[iter->first].permutation, move2[iter->first].size);

		int* pinv = new int[move1[iter->first].size];
		for (int i = 0; i < move1[iter->first].size; i++){
			for (int j = 0; j < move1[iter->first].size; j++){
				if (move1[iter->first].permutation[j] == i + 1)
					pinv[i] = j + 1;
			}
		}
		
		temp.orientation = applySubmoveP(move2[iter->first].orientation, pinv, move2[iter->first].size);
		for (int i = 0; i < move1[iter->first].size; i++)
			temp.orientation[i] += move1[iter->first].orientation[i];
		if (datasets[iter->first].omod > 1) // fix for bandaged puzzle centers
			for (int i = 0; i < move1[iter->first].size; i++)
				temp.orientation[i] = temp.orientation[i] % datasets[iter->first].omod;
				
		ans[iter->first].permutation = temp.permutation;
		ans[iter->first].orientation = temp.orientation;
		ans[iter->first].size = move2[iter->first].size;
	}
	return ans;
}

// print the details of a position
static void printPosition(Position p) {
	Position::iterator iter;
	int i;
	for (iter = p.begin(); iter != p.end(); iter++) {
		std::cout << iter->first << "\n";
		for (i=0; i<iter->second.size; i++)
			std::cout << iter->second.permutation[i] << " ";
		std::cout << "\n";
		for (i=0; i<iter->second.size; i++)
			std::cout << iter->second.orientation[i] << " ";
		std::cout << "\n";
	}
}

// creates a new, blank substate of given size
static substate newSubstate(int size) {
	substate newState;
	newState.size = size;
	newState.permutation = new int[size];
	newState.orientation = new int[size];
	return newState;
}

// does this limit apply to this move?
static bool limitMatches(MoveLimit& limit, fullmove& move) {
	return limit.name == (limit.moveGroup ? move.parentMove : move.name);
}

static void processMoveLimits(MoveList& moves, std::vector<MoveLimit> limits) {
	unsigned int i;
	MoveList::iterator iter;
	for (i=0; i<limits.size(); i++) {
		iter = moves.begin();
		while (iter != moves.end()) {
			if (limits[i].limit <= 0 && limitMatches(limits[i], iter->second)) {
				moves.erase(iter++);
			} else {
				iter++;
			}
		}
	}
}

#endif
