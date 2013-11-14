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

// Functions to attempt to generate a God's Algorithm table.

#ifndef GOD_H
#define GOD_H

static bool godTable(Position& solved, MoveList& moves, PieceTypes& datasets, std::set<MovePair>& forbiddenPairs, Position& ignore, std::set<Block>& blocks, int metric){
	// compute size of puzzle
	// this pair<string,int> holds the piece set name and the type of data:
	//		0 (orientation with parity constraint),
	//		1 (orientation without parity constraint),
	//		2 (unique permutation),
	//		3 (non-unique permutation)
	std::map<std::pair<string, int>, long long> subSizes;
	
	Position::iterator iter;
	for (iter = solved.begin(); iter != solved.end(); iter++) {
		if (datasets[iter->first].oparity) {
			// Orientation, parity constraint
			long long tablesize = 1;
			for (int i = 0; i < solved[iter->first].size - 1; i++)
				tablesize *= datasets[iter->first].omod;
			subSizes.insert(std::pair<std::pair<string, int>, long long>
					(std::pair<string, int> (iter->first, 0), tablesize));
		} else {
			// Orientation, no parity constraint
			long long tablesize = 1;
			for (int i = 0; i < solved[iter->first].size; i++)
				tablesize *= datasets[iter->first].omod;
			subSizes.insert(std::pair<std::pair<string, int>, long long>
					(std::pair<string, int> (iter->first, 1), tablesize));
		}
		
		if (factorial(datasets[iter->first].size) != -1 && uniquePermutation(solved[iter->first].permutation, solved[iter->first].size)){
			// Permutation, unique pieces
			std::vector<int> temp_perm;
			for (int i = 0; i < solved[iter->first].size; i++)
				temp_perm.push_back(solved[iter->first].permutation[i]);
			long long tablesize = factorial(temp_perm.size());
			subSizes.insert(std::pair<std::pair<string, int>, long long>
				(std::pair<string, int> (iter->first, 2), tablesize));
		}
		else {
			// Permutation, not unique pieces
			std::vector<int> temp_perm;
			for (int i = 0; i < solved[iter->first].size; i++)
				temp_perm.push_back(solved[iter->first].permutation[i]);
			long long tablesize = combinations(temp_perm);
			subSizes.insert(std::pair<std::pair<string, int>, long long>
				(std::pair<string, int> (iter->first, 3), tablesize));
		}
	}
	
	long long totalSize = 1;
	double logSize = 0;
	std::map<std::pair<string, int>, long long>::iterator iter2;
	for (iter2 = subSizes.begin(); iter2 != subSizes.end(); iter2++) {
		if (iter2->second <= 0) {
			logSize += 1000; // ensure we will exit soon
		} else {
			logSize += log(iter2->second);
		}
		totalSize *= iter2->second;
	}
	
	if (logSize >= 63*log(2)) {
		std::cerr << "Puzzle is too big for a state to fit in a long long int\n";
		exit(-1);
	}
	
	bool using_blocks;
	if (blocks.size() == 0)
		using_blocks = false;
	else
		using_blocks = true;
	
	// initialize an array of sufficient size and set all to -1
	bool usingMap = false;
	signed char* distance = new (std::nothrow) signed char[(std::size_t) totalSize];
	std::map<long long, signed char> distMap;
	long long i;
	if (distance == NULL) {
		std::cout << "Could not allocate array of size " << totalSize << "\n";
		usingMap = true;
	} else {
		std::cout << "Allocated array of size " << totalSize << "\n";
		for (i=0; i<totalSize; i++) {
			distance[i] = -1;
		}
	}
	
	// Set the solved position to a depth of 0
	int depth = 0;
	int* cnt = new int[128]; // signed char only goes up to 127 anyway...
	for (i=0; i<128; i++) {
		cnt[i] = 0;
	}
	cnt[0] = 1;
	Position temp1, temp2;
	Position::iterator iter3;
	for (iter3 = solved.begin(); iter3 != solved.end(); iter3++){
		temp2[iter3->first] = newSubstate(iter3->second.size);
	}
	MoveList::iterator moveIter;
	if (usingMap) {
		distMap[packPosition(solved, subSizes, datasets)] = 0;
	} else {
		distance[packPosition(solved, subSizes, datasets)] = 0;
	}
	std::cout << "Moves\tPositions\n";
	std::cout << depth << "\t" << cnt[depth] << "\n";
	
	// Loop through depths
	if (!usingMap) {
		while (1) {
			// look for positions at this depth
			for (i=0; i<totalSize; i++) {
				if (distance[i] == depth) {
					temp1 = unpackPosition(i, subSizes, datasets, solved);
					// try all possible moves and see if that position hasn't been visited
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blocklegal(temp1, blocks, moveIter->second.state))
								continue;
					
						// apply move and pack new position
						applyMove(temp1, temp2, moveIter->second.state, datasets);
						long long packTemp = packPosition(temp2, subSizes, datasets);
						
						if (metric == 0) { // HTM
							if (distance[packTemp] == -1) { // not visited yet
								cnt[depth+1]++;
								distance[packTemp] = depth+1;
							}
						} else if (metric == 1) { // QTM
							int newDepth = depth + moveIter->second.qtm;
							if (distance[packTemp] == -1 || distance[packTemp] > newDepth) {
								cnt[newDepth]++;
								distance[packTemp] = newDepth;
							}
						}
					}
				}
			}
			
			// increment depth and print
			depth++;
			if (cnt[depth] == 0) break;
			std::cout << depth << "\t" << cnt[depth] << "\n";
		}
	} else {
		while (1) {
			// look for positions at this depth
			std::map<long long, signed char>::iterator mapIter;
			for (mapIter = distMap.begin(); mapIter != distMap.end(); mapIter++) {
				if (mapIter->second == depth) {
					temp1 = unpackPosition(mapIter->first, subSizes, datasets, solved);
					// try all possible moves and see if that position hasn't been visited
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blocklegal(temp1, blocks, moveIter->second.state))
								continue;
					
						// apply move and pack new position
						applyMove(temp1, temp2, moveIter->second.state, datasets);
						long long packTemp = packPosition(temp2, subSizes, datasets);
						
						if (metric == 0) { // HTM
							if (distMap.find(packTemp) == distMap.end()) { // not visited yet
								cnt[depth+1]++;
								distMap[packTemp] = depth+1;
							}
						} else if (metric == 1) { // QTM
							int newDepth = depth + moveIter->second.qtm;
							if (distMap.find(packTemp) == distMap.end()) {
								cnt[newDepth]++;
								distMap[packTemp] = newDepth;
							} else if (distMap[packTemp] > newDepth) {
								cnt[newDepth]++;
								distMap[packTemp] = newDepth;
							}
						}
					}
				}
			}
			
			// increment depth and print
			depth++;
			if (cnt[depth] == 0) break;
			std::cout << depth << "\t" << cnt[depth] << "\n";
		}
	}
	
	// print total number of positions
	int totalPositions = 0;
	for (i=0; i<128; i++) {
		totalPositions += cnt[i];
	}
	std::cout << "Total positions: " << totalPositions << "\n";
	
	// print a bunch of antipodes
	int antipodes = 5; // maximum number to print
	if (cnt[depth-1] < 5) antipodes = cnt[depth-1];
	std::cout << "\nPrinting " << antipodes << " antipodes:\n\n";
	int antiCnt = 0;
	if (usingMap) {
		std::map<long long, signed char>::iterator mapIter;
		for (mapIter = distMap.begin(); mapIter != distMap.end(); mapIter++) {
			if (mapIter->second == depth - 1) {
				temp1 = unpackPosition(mapIter->first, subSizes, datasets, solved);
				printPosition(temp1);
				std::cout << "\n";
				antiCnt++;
				if (antiCnt >= antipodes) break;
			}
		}
	} else {
		for (i=0; i<totalSize; i++) {
			if (distance[i] == depth - 1) {
				temp1 = unpackPosition(i, subSizes, datasets, solved);
				printPosition(temp1);
				std::cout << "\n";
				antiCnt++;
				if (antiCnt >= antipodes) break;
			}
		}
	}
	
	if (usingMap) {
		distMap.clear();
	} else {
		delete []distance;
	}
	delete []cnt;
	return true;
}

// "Pack" a full-puzzle position - convert it from a position into a number
static long long packPosition(Position& position, std::map<std::pair<string, int>, long long> subSizes, PieceTypes& datasets) {
	std::map<std::pair<string, int>, long long>::iterator iter;
	long long packed = 0;
	for (iter = subSizes.begin(); iter != subSizes.end(); iter++) {
		// multiply by the size of this part
		packed *= iter->second;
		
		// then, add a number corresponding to that subSize's part
		if (iter->first.second == 0) {
			packed += oparVector2Index(position[iter->first.first].orientation, position[iter->first.first].size, datasets[iter->first.first].omod);
		} else if (iter->first.second == 1) {
			packed += oVector2Index(position[iter->first.first].orientation, position[iter->first.first].size, datasets[iter->first.first].omod);
		} else if (iter->first.second == 2) {
			packed += pVector2Index(position[iter->first.first].permutation, position[iter->first.first].size);
		} else if (iter->first.second == 3) {
			packed += pVector3Index(position[iter->first.first].permutation, position[iter->first.first].size);
		} else {
			std::cerr << "Something wrong with these subSizes!\n";
			exit(-1);
		}	
	}
	
	return packed;
}

// "Unpack" a full-puzzle position - convert it from a number into a position
static Position unpackPosition(long long position, std::map<std::pair<string, int>, long long> subSizes, PieceTypes& datasets, Position& solved) {
	// construct a new Position with blank versions of everything
	Position unpacked;
	PieceTypes::iterator iter2;
	for (iter2 = datasets.begin(); iter2 != datasets.end(); iter2++) {
		substate blankState;
		blankState.size = iter2->second.size;
		unpacked.insert(std::pair<string, substate> (iter2->first, blankState));
	}
	
	std::map<std::pair<string, int>, long long>::reverse_iterator iter;
	for (iter = subSizes.rbegin(); iter != subSizes.rend(); iter++) {
		// get the current index
		long long curIndex = position % iter->second;
		position /= iter->second;
		
		// now convert it into a permutation or orientation
		int size = unpacked[iter->first.first].size;
		if (iter->first.second == 0) {
			unpacked[iter->first.first].orientation = oparIndex2Array(curIndex, size, datasets[iter->first.first].omod);
		} else if (iter->first.second == 1) {
			unpacked[iter->first.first].orientation = oIndex2Array(curIndex, size, datasets[iter->first.first].omod);
		} else if (iter->first.second == 2) {
			unpacked[iter->first.first].permutation = pIndex2Array(curIndex, size);
		} else if (iter->first.second == 3) {
			unpacked[iter->first.first].permutation = pIndex3Array(curIndex, solved[iter->first.first].permutation, solved[iter->first.first].size);
		} else {
			std::cerr << "Something wrong with these subSizes!\n";
			exit(-1);
		}	
	}
	
	return unpacked;
}

#endif
