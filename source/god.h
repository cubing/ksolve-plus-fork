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

static bool godTable(Position& solved, MoveList& moves, PieceTypes& datasets, std::set<MovePair>& forbiddenPairs, Position& ignore, std::vector<Block>& blocks, int metric){
	// compute size of puzzle
	// this pair<string,int> holds the piece set name and the type of data:
	//		0 (orientation with parity constraint),
	//		1 (orientation without parity constraint),
	//		2 (unique permutation),
	//		3 (non-unique permutation)
	std::map<std::pair<string, int>, long long> subSizes;
	
	Position::iterator iter;
	for (iter = solved.begin(); iter != solved.end(); iter++) {
		int size = solved[iter->first].size;
		if (datasets[iter->first].oparity) {
			// Orientation, parity constraint
			long long tablesize = 1;
			for (int i = 0; i < size - 1; i++)
				tablesize *= datasets[iter->first].omod;
			subSizes.insert(std::pair<std::pair<string, int>, long long>
					(std::pair<string, int> (iter->first, 0), tablesize));
		} else {
			// Orientation, no parity constraint
			long long tablesize = 1;
			for (int i = 0; i < size; i++)
				tablesize *= datasets[iter->first].omod;
			subSizes.insert(std::pair<std::pair<string, int>, long long>
					(std::pair<string, int> (iter->first, 1), tablesize));
		}
		
		if (factorial(datasets[iter->first].size) != -1 && uniquePermutation(solved[iter->first].permutation, size)){
			// Permutation, unique pieces
			std::vector<int> temp_perm (size);
			for (int i = 0; i < size; i++)
				temp_perm[i] = solved[iter->first].permutation[i];
			long long tablesize = factorial(size);
			subSizes.insert(std::pair<std::pair<string, int>, long long>
				(std::pair<string, int> (iter->first, 2), tablesize));
		}
		else {
			// Permutation, not unique pieces
			std::vector<int> temp_perm (size);
			for (int i = 0; i < size; i++)
				temp_perm[i] = solved[iter->first].permutation[i];
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
	
	bool using_blocks;
	if (blocks.size() == 0)
		using_blocks = false;
	else
		using_blocks = true;
	
	// try to initialize an array of sufficient size and set all to -1
	int dataStructure = 0; // 0 = array, 1 = map<longlong,char>,
	                        // 2 = map<vector<longlong>,char>
	signed char* distance = NULL;
	if (logSize < 34*log(2)) { // just don't bother trying to get >2^31 bytes
		distance = new (std::nothrow) signed char[(std::size_t) totalSize];
	}
	std::map<long long, signed char> distMap1;
	std::map<std::vector<long long>, signed char> distMap2;
	long long i;
	
	if (distance == NULL) {
		std::cout << "Could not allocate array of size " << totalSize << "\n";
		if (logSize >= 63*log(2)) {
			std::cout << "Puzzle cannot fit in a long long int.\n";
			dataStructure = 2;
		} else {
			std::cout << "Puzzle can fit in a long long int.\n";
			dataStructure = 1;
		}
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
		temp1[iter3->first] = newSubstate(iter3->second.size);
		temp2[iter3->first] = newSubstate(iter3->second.size);
	}
	MoveList::iterator moveIter;
	if (dataStructure==0) {
		distance[packPosition(solved, subSizes, datasets)] = 0;
	} else if (dataStructure==1) {
		distMap1[packPosition(solved, subSizes, datasets)] = 0;
	} else if (dataStructure==2) {
		distMap2[packPosition2(solved, subSizes, datasets, 0)] = 0;
	}
	std::cout << "Moves\tPositions\n";
	std::cout << depth << "\t" << cnt[depth] << "\n"<<std::flush;
	
	// Loop through depths
	if (dataStructure==0) {
		while (1) {
			// look for positions at this depth
			for (i=0; i<totalSize; i++) {
				if (distance[i] == depth) {
					unpackPosition(temp1, i, subSizes, datasets, solved);
					// try all possible moves and see if that position hasn't been visited
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blockLegal(temp1, blocks, moveIter->second.state))
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
			std::cout << depth << "\t" << cnt[depth] << "\n" << std::flush;
		}
	} else if (dataStructure==1) {
		while (1) {
			// look for positions at this depth
			std::map<long long, signed char>::iterator mapIter;
			for (mapIter = distMap1.begin(); mapIter != distMap1.end(); mapIter++) {
				if (mapIter->second == depth) {
					unpackPosition(temp1, mapIter->first, subSizes, datasets, solved);
					// try all possible moves and see if that position hasn't been visited
					
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blockLegal(temp1, blocks, moveIter->second.state))
								continue;
					
						// apply move and pack new position
						applyMove(temp1, temp2, moveIter->second.state, datasets);
						long long packTemp = packPosition(temp2, subSizes, datasets);
						
						if (metric == 0) { // HTM
							if (distMap1.find(packTemp) == distMap1.end()) { // not visited yet
								cnt[depth+1]++;
								distMap1[packTemp] = depth+1;
							}
						} else if (metric == 1) { // QTM
							int newDepth = depth + moveIter->second.qtm;
							if (distMap1.find(packTemp) == distMap1.end()) {
								cnt[newDepth]++;
								distMap1[packTemp] = newDepth;
							} else if (distMap1[packTemp] > newDepth) {
								cnt[newDepth]++;
								distMap1[packTemp] = newDepth;
							}
						}
					}
				}
			}
			
			// increment depth and print
			depth++;
			if (cnt[depth] == 0) break;
			std::cout << depth << "\t" << cnt[depth] << "\n" << std::flush;
		}
	} else if (dataStructure==2) {
		while (1) {
			// look for positions at this depth
			std::map<std::vector<long long>, signed char>::iterator mapIter;
			for (mapIter = distMap2.begin(); mapIter != distMap2.end(); mapIter++) {
				if (mapIter->second == depth) {
					unpackPosition2(temp1, mapIter->first, subSizes, datasets, solved);
					// try all possible moves and see if that position hasn't been visited
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blockLegal(temp1, blocks, moveIter->second.state))
								continue;
					
						// apply move and pack new position
						applyMove(temp1, temp2, moveIter->second.state, datasets);
						std::vector<long long> packTemp = packPosition2(temp2, subSizes, datasets, mapIter->first.size());
						
						if (metric == 0) { // HTM
							if (distMap2.find(packTemp) == distMap2.end()) { // not visited yet
								cnt[depth+1]++;
								distMap2[packTemp] = depth+1;
							}
						} else if (metric == 1) { // QTM
							int newDepth = depth + moveIter->second.qtm;
							if (distMap2.find(packTemp) == distMap2.end()) {
								cnt[newDepth]++;
								distMap2[packTemp] = newDepth;
							} else if (distMap2[packTemp] > newDepth) {
								cnt[distMap2[packTemp]]--;
								cnt[newDepth]++;
								distMap2[packTemp] = newDepth;
							}
						}
					}
				}
			}
			
			// increment depth and print
			depth++;
			if (cnt[depth] == 0) break;
			std::cout << depth << "\t" << cnt[depth] << "\n" << std::flush;
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
	if (dataStructure==0) {
		for (i=0; i<totalSize; i++) {
			if (distance[i] == depth - 1) {
				// found an antipode!
				unpackPosition(temp1, i, subSizes, datasets, solved);
				Position curPos = temp1;
				Position nextPos;
				Position::iterator iter3;
				
				// find a solution
				std::cout << "Antipode solved by";
				int curDepth = depth - 1;
				
				while (curDepth > 0) {
					// try all moves to see which leads to the lowest depth
					int minDepth = curDepth;
					int minIndex = -1;
					for (iter3 = solved.begin(); iter3 != solved.end(); iter3++){
						nextPos[iter3->first] = newSubstate(iter3->second.size);
					}
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blockLegal(curPos, blocks, moveIter->second.state))
								continue;
						
						applyMove(curPos, nextPos, moveIter->second.state, datasets);
						int nextDepth = distance[packPosition(nextPos, subSizes, datasets)];
						if (nextDepth < minDepth) {
							minDepth = nextDepth;
							minIndex = moveIter->first;
						}
					}
					
					// apply best move
					applyMove(curPos, nextPos, moves[minIndex].state, datasets);
					curPos = nextPos;
					curDepth = minDepth;
					std::cout << " " << moves[minIndex].name;
				}
				
				std::cout << ":\n";
				printPosition(temp1);
				std::cout << "\n";
				antiCnt++;
				if (antiCnt >= antipodes) break;
			}
		}
		delete []distance;
	} else if (dataStructure==1) {
		std::map<long long, signed char>::iterator mapIter;
		for (mapIter = distMap1.begin(); mapIter != distMap1.end(); mapIter++) {
			if (mapIter->second == depth - 1) {
				// found an antipode!
				unpackPosition(temp1, mapIter->first, subSizes, datasets, solved);
				Position curPos = temp1;
				Position nextPos;
				Position::iterator iter3;
				
				// find a solution
				std::cout << "Antipode solved by";
				int curDepth = depth - 1;
				
				while (curDepth > 0) {
					// try all moves to see which leads to the lowest depth
					int minDepth = curDepth;
					int minIndex = -1;
					for (iter3 = solved.begin(); iter3 != solved.end(); iter3++){
						nextPos[iter3->first] = newSubstate(iter3->second.size);
					}
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blockLegal(curPos, blocks, moveIter->second.state))
								continue;
						
						applyMove(curPos, nextPos, moveIter->second.state, datasets);
						int nextDepth = distMap1[packPosition(nextPos, subSizes, datasets)];
						if (nextDepth < minDepth) {
							minDepth = nextDepth;
							minIndex = moveIter->first;
						}
					}
					
					// apply best move
					applyMove(curPos, nextPos, moves[minIndex].state, datasets);
					curPos = nextPos;
					curDepth = minDepth;
					std::cout << " " << moves[minIndex].name;
				}
				
				std::cout << ":\n";
				printPosition(temp1);
				std::cout << "\n";
				antiCnt++;
				if (antiCnt >= antipodes) break;
			}
		}
		
		distMap1.clear();
	} else if (dataStructure==2) {
		std::map<std::vector<long long>, signed char>::iterator mapIter;
		for (mapIter = distMap2.begin(); mapIter != distMap2.end(); mapIter++) {
			if (mapIter->second == depth - 1) {
				// found an antipode!
				unpackPosition2(temp1, mapIter->first, subSizes, datasets, solved);
				Position curPos = temp1;
				Position nextPos;
				Position::iterator iter3;
				
				// find a solution
				std::cout << "Antipode solved by";
				int curDepth = depth - 1;
				
				while (curDepth > 0) {
					// try all moves to see which leads to the lowest depth
					int minDepth = curDepth;
					int minIndex = -1;
					for (iter3 = solved.begin(); iter3 != solved.end(); iter3++){
						nextPos[iter3->first] = newSubstate(iter3->second.size);
					}
					for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++){
						if (using_blocks) // see if the blocks will prevent this move
							if (!blockLegal(curPos, blocks, moveIter->second.state))
								continue;
						
						applyMove(curPos, nextPos, moveIter->second.state, datasets);
						int nextDepth = distMap2[packPosition2(nextPos, subSizes, datasets, mapIter->first.size())];
						if (nextDepth < minDepth) {
							minDepth = nextDepth;
							minIndex = moveIter->first;
						}
					}
					
					// apply best move
					applyMove(curPos, nextPos, moves[minIndex].state, datasets);
					curPos = nextPos;
					curDepth = minDepth;
					std::cout << " " << moves[minIndex].name;
				}
				
				std::cout << ":\n";
				printPosition(temp1);
				std::cout << "\n";
				antiCnt++;
				if (antiCnt >= antipodes) break;
			}
		}
		
		distMap2.clear();
	}
	
	delete []cnt;
	return true;
}

// "Pack" a full-puzzle position - convert it from a position into a number
static long long packPosition(Position& position, std::map<std::pair<string, int>, long long> &subSizes, PieceTypes& datasets) {
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

// "Pack" a full-puzzle position - convert it from a position into a *vector*
static std::vector<long long> packPosition2(Position& position, std::map<std::pair<string, int>, long long> &ignored_subSizes, PieceTypes& datasets, int siz) {
	std::vector<long long> packed ;
	packed.reserve(siz) ;
	unsigned long long accum = 0 ;
	int bitAt = 0 ;
	PieceTypes::iterator iter2;
	for (iter2 = datasets.begin(); iter2 != datasets.end(); iter2++) {
		int n = iter2->second.size ;
		substate &s = position[iter2->first] ;
		int *perm = s.permutation ;
		int permBits = iter2->second.permbits ;
		int permMask = (1<<permBits)-1 ;
		for (int i=0; i<n; i++) {
                        if (bitAt + permBits > 64) {
				packed.push_back(accum) ;
				accum = 0 ;
				bitAt = 0 ;
			}
			accum |= ((unsigned long long)perm[i]-1) << bitAt ;
                        bitAt += permBits ;
                }
		int oriBits = iter2->second.oribits ;
		if (oriBits) {
			int *ori = s.orientation ;
			int oriMask = (1<<oriBits)-1 ;
			for (int i=0; i<n; i++) {
                        	if (bitAt + oriBits > 64) {
					packed.push_back(accum) ;
					accum = 0 ;
					bitAt = 0 ;
				}
				accum |= ((unsigned long long)ori[i]) << bitAt ;
                        	bitAt += oriBits ;
			}
                }
	}
	if (bitAt > 0)
		packed.push_back(accum) ;
	return packed;
}

// "Unpack" a full-puzzle position - convert it from a number into a position
static void unpackPosition(Position &unpacked, long long position, std::map<std::pair<string, int>, long long> &subSizes, PieceTypes& datasets, Position& solved) {
	// construct a new Position with blank versions of everything
	PieceTypes::iterator iter2;
	std::map<std::pair<string, int>, long long>::reverse_iterator iter;
	for (iter = subSizes.rbegin(); iter != subSizes.rend(); iter++) {
		// get the current index
		long long curIndex = position % iter->second;
		position /= iter->second;
		
		// now convert it into a permutation or orientation
		int size = unpacked[iter->first.first].size;
		if (iter->first.second == 0) {
			oparIndex2Array(curIndex, size, datasets[iter->first.first].omod, unpacked[iter->first.first].orientation);
		} else if (iter->first.second == 1) {
			oIndex2Array(curIndex, size, datasets[iter->first.first].omod,unpacked[iter->first.first].orientation);
		} else if (iter->first.second == 2) {
			pIndex2Array(curIndex, size, unpacked[iter->first.first].permutation);
		} else if (iter->first.second == 3) {
			pIndex3Array(curIndex, solved[iter->first.first].permutation, solved[iter->first.first].size, unpacked[iter->first.first].permutation);
		} else {
			std::cerr << "Something wrong with these subSizes!\n";
			exit(-1);
		}	
	}
}

// "Unpack" a full-puzzle position - convert it from a number into a *vector*
static void unpackPosition2(Position &unpacked, const std::vector<long long> &position, std::map<std::pair<string, int>, long long> &ignored_subSizes, PieceTypes& datasets, Position& ignored_solved) {
	// construct a new Position with blank versions of everything
	PieceTypes::iterator iter2;
	int positionAt = 0 ;
	int bitAt = 0 ;
	for (iter2 = datasets.begin(); iter2 != datasets.end(); iter2++) {
		substate &blankState = unpacked[iter2->first] ;
		int n = iter2->second.size ;
		blankState.size = n ;
		int *perm = blankState.permutation ;
		int permBits = iter2->second.permbits ;
		int permMask = (1<<permBits)-1 ;
		for (int i=0; i<n; i++) {
                        if (bitAt + permBits > 64) {
				bitAt = 0 ;
				positionAt++ ;
			}
			perm[i] = 1 + (permMask &
                      (((unsigned long long)position[positionAt]) >> bitAt)) ;
                        bitAt += permBits ;
                }
		int *ori = blankState.orientation ;
		int oriBits = iter2->second.oribits ;
		if (oriBits) {
			int oriMask = (1<<oriBits)-1 ;
			for (int i=0; i<n; i++) {
                        	if (bitAt + oriBits > 64) {
					bitAt = 0 ;
					positionAt++ ;
				}
				ori[i] = oriMask &
                      	(((unsigned long long)position[positionAt]) >> bitAt) ;
                        	bitAt += oriBits ;
			}
                }
	}
}

#endif
