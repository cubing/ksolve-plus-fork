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

// Data structures for storing moves and puzzle information.

#ifndef DATA_H
#define DATA_H

// The three different types of puzzle pieces. Not used at the moment.
static const int TYPE_PERMUTE = 1;    // Data to permute
static const int TYPE_ORIENT = 2; // Data to orient
static const int TYPE_PURE = 3;    // Data to orient which does not permute

// Max size table for one set of pieces. Size in number of elements, not actual bytes.
static const int MAX_COMPLETE_PERMUTATION_TABLE_SIZE = 10000000; // >10! (perm of 10 pieces)
static const int MAX_COMPLETE_ORIENTATION_TABLE_SIZE = 10000000; // Complete tables contain one int (4 byte) per entry.
static const int MAX_PARTIAL_PERMUTATION_TABLE_SIZE = 1000000; // Max number of entries in a partial table.
static const int MAX_PARTIAL_ORIENTATION_TABLE_SIZE = 1000000; // SIZE is number of entries.

// The types of pruning tables. 
static const int TABLE_TYPE_NONE = 0;
static const int TABLE_TYPE_COMPLETE = 1;
static const int TABLE_TYPE_PARTIAL = 2;

// Some general data for a set of pieces
struct dataset{
	int type;
	int size;
	int omod; // Orientations are calculated mod this value
	int maxInSolved; // maximum value in solved perm; assumes 1-base
	int permbits, oribits ; // bits for perm and ori
	int ptabletype;
	int otabletype;
	bool uniqueperm; // Perm of unique numbers (1,2,3,...), or repeated (1,3,1,2)
	bool oparity; // Does orientation have a parity constraint? (If so, last orientation is unnecessary)
};

// part of a state, including orientation and permutation
struct substate {
	int *orientation;
	int *permutation;
	int size;
};

// part of a pruning table
struct subprune{
	std::vector<char> orientation;
	std::vector<char> permutation;
	std::map<std::vector<long long>, char> partialorientation;
	std::map<std::vector<long long>, char> partialpermutation;
	int partialpermutation_depth;
	int partialorientation_depth;
};

// some typedefs to make things easier
typedef std::string string;
typedef std::map<string, substate> Position;
typedef std::map<string, std::set<int> > Block;
typedef std::pair<int, int> MovePair;
typedef std::map<string, subprune> PruneTable;
typedef std::map<string, dataset> PieceTypes;

// all the information needed to describe a possible move
struct fullmove {
	string name;
	int id;
	int parentID;
	int qtm;
	Position state;
};

// info about a particular move limit
struct MoveLimit {
	int move; // ID of move (or parent move) to limit
	int limit; // maximum number of moves of this type
	bool moveGroup; // is this a group of moves, or just one?
	Block owned; // pieces that can only be affected by these moves
};

struct ScrambleDef {
	string name;
	Position state;
	Position ignore;
	int max_depth;
	int slack;
	int metric; // 0 = HTM, 1 = QTM
	int printState; // 0 = no, 1 = yes
	std::vector<MoveLimit> moveLimits;
};

typedef std::map<int, fullmove> MoveList;

#endif
