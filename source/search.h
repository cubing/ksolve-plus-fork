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

// Functions for searching for solutions

#ifndef SEARCH_H
#define SEARCH_H

static bool treeSolve(Position state, Position& solved, MoveList& moves, PieceTypes& datasets, PruneTable& prunetables, std::set<MovePair>& forbiddenPairs, Position& ignore, std::set<Block>& blocks, int depth, int metric, std::map<string, int>& moveLimits, string sequence, string old_move){
	if (depth <= 0) {
		if (depth == 0) {
			if (isSolved(state, solved, ignore)){
				std::cout << sequence << "\n";
				return true;
			}
		}
		return false;
	}
	

	if (prune(state, depth, datasets, prunetables))
		return false;
 
	// Apply new legal moves
	Position::iterator iter2;
	bool success = false;
	Position new_state;
	for (iter2 = state.begin(); iter2 != state.end(); iter2++){
		new_state[iter2->first].permutation = new int[iter2->second.size];
		new_state[iter2->first].orientation = new int[iter2->second.size];
		new_state[iter2->first].size = iter2->second.size;
	}
	
	bool using_blocks = (blocks.size() != 0);
	bool using_limits = (moveLimits.size() != 0);

	MoveList::iterator iter;
	for (iter = moves.begin(); iter != moves.end(); iter++){
		// Test for forbidden pair
		if (forbiddenPairs.find(MovePair(old_move, iter->first)) != forbiddenPairs.end()){
			continue; // forbidden pair
		}
		// Test for block breaking
		if (using_blocks)
			if (!blocklegal(state, blocks, iter->second.state))
				continue; // move would break block
		// Test for limited move
		if (using_limits)
			if (moveLimits.find(iter->second.parentMove) != moveLimits.end())
				if (moveLimits[iter->second.parentMove] <= 0)
					continue; // no moves left

		applyMove(state, new_state, iter->second.state, datasets);
		
		int newDepth;
		if (metric == 0) { // HTM
			newDepth = depth - 1;
		} else { // QTM
			newDepth = depth - iter->second.qtm;
		}
		
		if (using_limits)
			if (moveLimits.find(iter->second.parentMove) != moveLimits.end())
				moveLimits[iter->second.parentMove]--;
		
		if (treeSolve(new_state, solved, moves, datasets, prunetables, forbiddenPairs, ignore, blocks, newDepth, metric, moveLimits, sequence + " " + iter->first, iter->first))
			success = true;
		
		if (using_limits)
			if (moveLimits.find(iter->second.parentMove) != moveLimits.end())
				moveLimits[iter->second.parentMove]++;
	}

	for (iter2 = state.begin(); iter2 != state.end(); iter2++){
		delete new_state[iter2->first].permutation;
		delete new_state[iter2->first].orientation;
	}

	return success;    
}

static bool isSolved(Position state1, Position& state2, Position& ignore){
	if (ignore.size() == 0){
		 return isEqual(state1, state2);
	}
	else{
		Position::iterator iter;
		for (iter = state1.begin(); iter != state1.end(); iter++){
			if (ignore.find(iter->first) != ignore.end()){
				for (int i = 0; i < state1[iter->first].size; i++){
					if (ignore[iter->first].permutation[i] == 0 && state1[iter->first].permutation[i] != state2[iter->first].permutation[i])
						return false;
					if (ignore[iter->first].orientation[i] == 0 && state1[iter->first].orientation[i] != state2[iter->first].orientation[i])
						return false;
				}
			} else {
				if (memcmp(iter->second.permutation, state2[iter->first].permutation, iter->second.size*sizeof(int)) != 0)
					return false;
				if (memcmp(iter->second.orientation, state2[iter->first].orientation, iter->second.size*sizeof(int)) != 0)
					return false;
			}
		}
	}
	return true;
}

static bool isEqual(Position state1, Position& state2){
	Position::iterator iter;
	for (iter = state1.begin(); iter != state1.end(); iter++){
		if (memcmp(iter->second.permutation, state2[iter->first].permutation, iter->second.size*sizeof(int)) != 0)
			return false;
		if (memcmp(iter->second.orientation, state2[iter->first].orientation, iter->second.size*sizeof(int)) != 0)
			return false;
	}
	return true;
}

#endif
