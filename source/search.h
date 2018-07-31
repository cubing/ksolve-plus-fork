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

static bool treeSolve(Position state, Position& solved, MoveList& moves, PieceTypes& datasets, PruneTable& prunetables, std::set<MovePair>& forbiddenPairs, Position& ignore, std::vector<Block>& blocks, int depth, int metric, std::vector<MoveLimit>& moveLimits, string sequence, int old_move, bool splitThreads){
	// if we ran out of depth, it's either solved or not
	if (depth <= 0) {
		if (isSolved(state, solved, ignore, datasets)){
            #pragma omp critical
            {
                std::cout << sequence << "\n";
            }
			return true;
		} else {
			return false;
		}
	}

	// use pruning tables to see if we don't have enough depth left
	if (prune(state, depth, datasets, prunetables))
		return false;

	// define variables; initialize room for a new state
	bool success = false;
	bool using_blocks = (blocks.size() != 0);
	bool using_limits = (moveLimits.size() != 0);

    if(splitThreads) {
        #pragma omp parallel //num_threads(1)
        {
        //for (iter = moves.begin(); iter != moves.end(); iter++){
            Position new_state(state.size());
            for (int iter2 = 0; iter2 < state.size(); iter2++) {
                new_state[iter2] = newSubstate(state[iter2].size);
            }

            std::vector<MoveLimit> localMoveLimits = moveLimits;

            #pragma omp for
            for (int i = 0; i < moves.size(); i++){
                MoveList::iterator iter = moves.begin();
                for(int j = 0; j < i; j++)
                    iter++;
                // if we have a forbidden pair, try the next move
                if (forbiddenPairs.find(MovePair(old_move, iter->first)) != forbiddenPairs.end())
                    continue;
                // if this move breaks the blocks, try the next move
                if (using_blocks)
                    if (!blockLegal(state, blocks, iter->second.state))
                        continue;
                // if movelimits make this move impossible, try the next move
                if (using_limits) {
                    bool forbidden = false;
                    for (unsigned int i=0; i<localMoveLimits.size(); i++) {
                        if (localMoveLimits[i].limit <= 0 && limitMatches(localMoveLimits[i], iter->second)) {
                            forbidden = true;
                            break;
                        }
                    }
                    if (forbidden) continue;
                }

                // compute depth of new position using HTM or QTM
                int newDepth;
                if (metric == 0) { // HTM
                    newDepth = depth - 1;
                } else { // QTM
                    newDepth = depth - iter->second.qtm;
                }
                if (newDepth < 0) continue; // not enough depth for this move? try the next one

                // compute new position
                applyMove(state, new_state, iter->second.state, datasets);

                // decrement applicable move limits, and check if we got into an unsolvable state
                if (using_limits) {
                    bool isSolvable = true; // see if we have stumbled into a situation that requires more of the limited moves
                    for (unsigned int i=0; i<localMoveLimits.size(); i++) {
                        if (limitMatches(localMoveLimits[i], iter->second)) {
                            localMoveLimits[i].limit--;
                            if (localMoveLimits[i].limit == 0) {
                                isSolvable = isSolvable && stillSolvable(new_state, solved, ignore, localMoveLimits[i].owned);
                            }
                        }
                    }
                    if (!isSolvable) {
                        for (unsigned int i=0; i<moveLimits.size(); i++)
                            if (limitMatches(localMoveLimits[i], iter->second))
                                localMoveLimits[i].limit++;
                        continue;
                    }
                }

                // recurse!
                if (treeSolve(new_state, solved, moves, datasets, prunetables, forbiddenPairs, ignore, blocks, newDepth, metric, localMoveLimits, sequence + " " + iter->second.name, iter->first, false))
                    success = true;

                // clean up modified move limits
                if (using_limits)
                    for (unsigned int i=0; i<localMoveLimits.size(); i++)
                        if (limitMatches(localMoveLimits[i], iter->second))
                            localMoveLimits[i].limit++;
            }
            // free new_state memory
            for (int iter2=0; iter2<state.size(); iter2++) {
                delete new_state[iter2].permutation;
                delete new_state[iter2].orientation;
            }
        }
	}
    else {
        Position new_state(state.size());
        for (int iter2 = 0; iter2<state.size(); iter2++) {
            new_state[iter2] = newSubstate(state[iter2].size);
        }
        MoveList::iterator iter = moves.begin();
        for (iter = moves.begin(); iter != moves.end(); iter++){
            // if we have a forbidden pair, try the next move
            if (forbiddenPairs.find(MovePair(old_move, iter->first)) != forbiddenPairs.end())
                continue;
            // if this move breaks the blocks, try the next move
            if (using_blocks)
                if (!blockLegal(state, blocks, iter->second.state))
                    continue;
            // if movelimits make this move impossible, try the next move
            if (using_limits) {
                bool forbidden = false;
                for (unsigned int i=0; i<moveLimits.size(); i++) {
                    if (moveLimits[i].limit <= 0 && limitMatches(moveLimits[i], iter->second)) {
                        forbidden = true;
                        break;
                    }
                }
                if (forbidden) continue;
            }
            // compute depth of new position using HTM or QTM
            int newDepth;
            if (metric == 0) { // HTM
                newDepth = depth - 1;
            } else { // QTM
                newDepth = depth - iter->second.qtm;
            }
            if (newDepth < 0) continue; // not enough depth for this move? try the next one

            // compute new position
            applyMove(state, new_state, iter->second.state, datasets);

            // decrement applicable move limits, and check if we got into an unsolvable state
            if (using_limits) {
                bool isSolvable = true; // see if we have stumbled into a situation that requires more of the limited moves
                for (unsigned int i=0; i<moveLimits.size(); i++) {
                    if (limitMatches(moveLimits[i], iter->second)) {
                        moveLimits[i].limit--;
                        if (moveLimits[i].limit == 0) {
                            isSolvable = isSolvable && stillSolvable(new_state, solved, ignore, moveLimits[i].owned);
                        }
                    }
                }
                if (!isSolvable) {
                    for (unsigned int i=0; i<moveLimits.size(); i++)
                        if (limitMatches(moveLimits[i], iter->second))
                            moveLimits[i].limit++;
                    continue;
                }
            }

            // recurse!
            if (treeSolve(new_state, solved, moves, datasets, prunetables, forbiddenPairs, ignore, blocks, newDepth, metric, moveLimits, sequence + " " + iter->second.name, iter->first, false))
                success = true;

            // clean up modified move limits
            if (using_limits)
                for (unsigned int i=0; i<moveLimits.size(); i++)
                    if (limitMatches(moveLimits[i], iter->second))
                        moveLimits[i].limit++;
        }
        // free new_state memory
        for (int iter2 = 0; iter2<state.size(); iter2++) {
            delete new_state[iter2].permutation;
            delete new_state[iter2].orientation;
        }
    }
	return success;
}

// does this position count as solved?
static bool isSolved(Position& state1, Position& state2, Position& ignore, PieceTypes& datasets){
	if (ignore.size() == 0){
		 return isEqual(state1, state2, datasets);
	}
	else{
		Position::iterator iter;
		for (int iter=0; iter<state1.size(); iter++) {
			if (ignore[iter].size > 0) {
				for (int i = 0; i < state1[iter].size; i++){
					if (ignore[iter].permutation[i] == 0 && state1[iter].permutation[i] != state2[iter].permutation[i])
						return false;
				}
				if (datasets[iter].omod != 1)
					for (int i = 0; i < state1[iter].size; i++)
						if (ignore[iter].orientation[i] == 0 && state1[iter].orientation[i] != state2[iter].orientation[i])
							return false;
			}
		}
	}
	return true;
}

// are these two positions exactly equal?
static bool isEqual(Position& state1, Position& state2, PieceTypes& datasets){
	for (int iter=0; iter<state1.size(); iter++) {
		if (memcmp(state1[iter].permutation, state2[iter].permutation, state1[iter].size*sizeof(int)) != 0)
			return false;
		if (datasets[iter].omod != 1)
			if (memcmp(state1[iter].orientation, state2[iter].orientation, state1[iter].size*sizeof(int)) != 0)
				return false;
	}
	return true;
}

// is this position still solvable? i.e. any unsolved, unignored pieces in the block?
static bool stillSolvable(Position& state, Position& solved, Position& ignore, Block& owned){
	Block::iterator iter;
	for (iter = owned.begin(); iter != owned.end(); iter++) {
		std::set<int>::iterator iter2;
		int type = iter->first;
		for (iter2 = owned[type].begin(); iter2 != owned[type].end(); iter2++) {
			// if not solved and not ignored, return false!
			if ((ignore[type].permutation[*iter2] == 0 && state[type].permutation[*iter2] != solved[type].permutation[*iter2]) ||
				(ignore[type].orientation[*iter2] == 0 && solved[type].orientation[*iter2] != solved[type].orientation[*iter2])) {
				return false;
			}
		}
	}
	return true;
}

#endif
