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

// Functions for generating the pruning tables

#ifndef PRUNING_H
#define PRUNING_H

static PruneTable getCompletePruneTables(Position solved, MoveList moves, PieceTypes datasets, Position ignore, string filename)
{
	PruneTable table;
	string filename2 = filename + ".tables";
	std::ifstream fin;
	fin.open(filename2.c_str(), std::ios::in | std::ios::binary);
	
	bool tablesExist = fin.is_open(); // do tables exist?
	bool oldTables = false; // are the tables older than the def file?
	
	if (tablesExist) {
		fin.close(); // close ifstream so we can open a handle
		
		FILETIME defWrite, tableWrite;
		HANDLE defHandle, tableHandle;
		
		defHandle = CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (defHandle == INVALID_HANDLE_VALUE) {
			std::cerr << "defHandle fail\n";
			exit(-1);
		}
		if (!GetFileTime(defHandle, NULL, NULL, &defWrite)) {
			std::cerr << "GetFileTime of def fail\n";
			exit(-1);
		}
		CloseHandle(defHandle);
		
		tableHandle = CreateFile(filename2.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (tableHandle == INVALID_HANDLE_VALUE) {
			std::cerr << "tableHandle fail\n";
			exit(-1);
		}
		if (!GetFileTime(tableHandle, NULL, NULL, &tableWrite)) {
			std::cerr << "GetFileTime of table fail\n";
			exit(-1);
		}
		CloseHandle(tableHandle);

		if (CompareFileTime(&defWrite, &tableWrite) == 1) { // yes, def file is newer!
			oldTables = true;
		}
		
		// reopen ifstream
		fin.open(filename2.c_str(), std::ios::in | std::ios::binary);
	}
	
	if (tablesExist && !oldTables){
		std::cout << "Pruning tables found on file.\n";
		
		// Tables exist
		int checksum;
		fin.read((char*) (&checksum), sizeof(checksum)); // Not used yet

		Position::iterator iter;
		for (iter = solved.begin(); iter != solved.end(); iter++){

			if (factorial(datasets[iter->first].size) <= MAX_COMPLETE_PERMUTATION_TABLE_SIZE && factorial(datasets[iter->first].size) != -1 && uniquePermutation(solved[iter->first].permutation, solved[iter->first].size)){ 
				// Complete tables, unique pieces
				char buff;

				// Read permutation table
				int tab_size = factorial(datasets[iter->first].size);
				table[iter->first].permutation.reserve(tab_size);

				char *tmp_buff = new char[tab_size+1];
				fin.get(tmp_buff, tab_size+1, 'A');
				for (int i = 0; i < tab_size; i++){
					table[iter->first].permutation.push_back(tmp_buff[i]);
				}
				delete tmp_buff;
			
			}
			else if (combinations(solved[iter->first].permutation, solved[iter->first].size) <= MAX_COMPLETE_PERMUTATION_TABLE_SIZE && combinations(solved[iter->first].permutation, solved[iter->first].size) != -1 && !uniquePermutation(solved[iter->first].permutation, solved[iter->first].size)){ 
				// Complete table, non-unique pieces
				char buff;
				int tab_size = combinations(solved[iter->first].permutation, solved[iter->first].size);
				for (int i = 0; i < tab_size; i++){
					fin.read((char*) (&buff), sizeof(buff));
					table[iter->first].permutation.push_back(buff);
				}
			}
			else{
				// Partial table
				int elements, keysize;
				fin.read((char*) (&elements), sizeof(elements));
				fin.read((char*) (&keysize), sizeof(keysize));

				//std::cout << "elements " << elements << "\n";
				//std::cout << "keysize " << keysize << "\n";
				for (int i = 0; i < elements; i++){
					char depth;
					std::vector<long long> key;
					long long tmp;
					fin.read((char*) (&depth), sizeof(depth));
					key.clear();
					for (int j = 0; j < keysize; j++){
						fin.read((char*) (&tmp), sizeof(tmp));
						key.push_back(tmp);
					}
					table[iter->first].partialpermutation[key] = depth;
				}
				table[iter->first].partialpermutation_depth = maxDepth(table[iter->first].partialpermutation);
			}

			double osize = log(datasets[iter->first].omod) * datasets[iter->first].size;
			if (osize < log(MAX_COMPLETE_ORIENTATION_TABLE_SIZE)){ // Not to big tables. Using log to avoid overflow.
				char buff;
				long long num = 1;
				for (int t = 0; t < datasets[iter->first].size; t++)
					num *= datasets[iter->first].omod;

					 table[iter->first].orientation.reserve(num);
	 
					 char *tmp_buff = new char[num+1];
					 fin.get(tmp_buff, num+1, 'A');
					 for (int i = 0; i < num; i++){
						 table[iter->first].orientation.push_back(tmp_buff[i]);
					 }
					 delete tmp_buff;
				
			}    
			else{ // Partial orientation tables
				int elements, keysize;
				fin.read((char*) (&elements), sizeof(elements));
				fin.read((char*) (&keysize), sizeof(keysize));
				//std::cout << "elements " << elements << "\n";
				//std::cout << "keysize " << keysize << "\n";
				for (int i = 0; i < elements; i++){
					char depth;
					std::vector<long long> key;
					long long tmp;
					fin.read((char*) (&depth), sizeof(depth));
					key.clear();
					for (int j = 0; j < keysize; j++){
						fin.read((char*) (&tmp), sizeof(tmp));
						key.push_back(tmp);
					}
					table[iter->first].partialorientation[key] = depth;
				}
				table[iter->first].partialorientation_depth = maxDepth(table[iter->first].partialorientation);
			}
		}
		fin.close();
		
	}    
	else{
		if (tablesExist) { // tables exist, but they're old
			std::cout << "Tables older than def file, recomputing.\n";
		} else { // no tables on file
			std::cout << "Pruning tables not found on file, computing.\n";
		}
		table = buildCompletePruneTables(solved, moves, datasets, ignore);
		std::ofstream fout;
		fout.open(filename2.c_str(), std::ios::out | std::ios::binary);
		int checksum = 1; // Not used yet
		fout.write((char*) (&checksum), sizeof(checksum));
		Position::iterator iter;
		for (iter = solved.begin(); iter != solved.end(); iter++){
			if (factorial(datasets[iter->first].size) <= MAX_COMPLETE_PERMUTATION_TABLE_SIZE && factorial(datasets[iter->first].size) != -1 && uniquePermutation(solved[iter->first].permutation, solved[iter->first].size)){

				// Write permutation table
				int tab_size = factorial(datasets[iter->first].size);
				for (int i = 0; i < tab_size; i++){
					fout.write((char*) (&table[iter->first].permutation[i]), sizeof(table[iter->first].permutation[i]));
				}
			}
			else if (combinations(solved[iter->first].permutation, solved[iter->first].size) <= MAX_COMPLETE_PERMUTATION_TABLE_SIZE && combinations(solved[iter->first].permutation, solved[iter->first].size) != -1){
				// Complete permutation table, not unique pieces
				int tab_size = combinations(solved[iter->first].permutation, solved[iter->first].size);
				for (int i = 0; i < tab_size; i++)
					fout.write((char*) (&table[iter->first].permutation[i]), sizeof(table[iter->first].permutation[i]));
			}
			else{
				// Partial permutation table  
				// Table entries
				int tmp_size = table[iter->first].partialpermutation.size();
				fout.write((char*) (&tmp_size), sizeof(tmp_size));
				
				std::map<std::vector<long long>, char>::iterator tmp_iter;
				tmp_iter = table[iter->first].partialpermutation.begin();
				// Key size
				tmp_size = tmp_iter->first.size();
				fout.write((char*) (&tmp_size), sizeof(tmp_size));
				
				for (tmp_iter = table[iter->first].partialpermutation.begin(); tmp_iter != table[iter->first].partialpermutation.end(); tmp_iter++){
					// Depth
					fout.write((char*) (&tmp_iter->second), sizeof(tmp_iter->second));
					for (int i = 0; i < tmp_iter->first.size(); i++)
						fout.write((char*) (&(*tmp_iter).first[i]), sizeof(tmp_iter->first[i]));
						// Keys
				}
			}

			double osize = log(datasets[iter->first].omod) * datasets[iter->first].size;

			if (osize < log(MAX_COMPLETE_ORIENTATION_TABLE_SIZE)){ // Not too big tables. Using log to avoid overflow.
				for (int i = 0; i < table[iter->first].orientation.size(); i++){
					fout.write((char*) (&table[iter->first].orientation[i]), sizeof(table[iter->first].orientation[i]));
				}
			}
			else{ // Partial orientation table
				// Table entries
				int tmp_size = table[iter->first].partialorientation.size();
				fout.write((char*) (&tmp_size), sizeof(tmp_size));

				std::map<std::vector<long long>, char>::iterator tmp_iter;
				tmp_iter = table[iter->first].partialorientation.begin();
				// Key size
				tmp_size = tmp_iter->first.size();
				fout.write((char*) (&tmp_size), sizeof(tmp_size));
				
				for (tmp_iter = table[iter->first].partialorientation.begin() ; tmp_iter != table[iter->first].partialorientation.end(); tmp_iter++){
					// Depth
					fout.write((char*) (&tmp_iter->second), sizeof(tmp_iter->second));
					for (int i = 0; i < tmp_iter->first.size(); i++)
					// Keys
						fout.write((char*) (&(*tmp_iter).first[i]), sizeof(tmp_iter->first[i]));
				}
			}
		}
		fout.close();

	}
	return table;
}
				
static PruneTable buildCompletePruneTables(Position solved, MoveList moves, PieceTypes datasets, Position ignore)
{
	Position::iterator iter;
	PruneTable table;
	std::vector<int> tmp_ignore;
	for (iter = solved.begin(); iter != solved.end(); iter++){
		tmp_ignore.clear();
		if (ignore.find(iter->first) != ignore.end())
			for (int i = 0; i < ignore[iter->first].size; i++)
				tmp_ignore.push_back(ignore[iter->first].permutation[i]);
			
		if (factorial(datasets[iter->first].size) <= MAX_COMPLETE_PERMUTATION_TABLE_SIZE && factorial(datasets[iter->first].size) != -1 && uniquePermutation(solved[iter->first].permutation, solved[iter->first].size)){
			// Complete table, unique pieces
			std::vector<int> temp_perm;
			for (int i = 0; i < solved[iter->first].size; i++)
				temp_perm.push_back(solved[iter->first].permutation[i]);
			table[iter->first].permutation = buildCompletePermutationPruningTable(temp_perm, moves, iter->first, tmp_ignore);
		}
		else if (combinations(solved[iter->first].permutation, solved[iter->first].size) <= MAX_COMPLETE_PERMUTATION_TABLE_SIZE && combinations(solved[iter->first].permutation, solved[iter->first].size) != -1 && !uniquePermutation(solved[iter->first].permutation, solved[iter->first].size)){
			// Complete table, not unique pieces
			std::vector<int> temp_perm;
			for (int i= 0; i < solved[iter->first].size; i++)
				temp_perm.push_back(solved[iter->first].permutation[i]);
			table[iter->first].permutation = buildCompletePermutationPruningTable3(temp_perm, moves, iter->first, tmp_ignore);
		}
		else{
			// Partial permutation table 
			std::vector<int> temp_perm;
			for (int i = 0; i < solved[iter->first].size; i++)
				temp_perm.push_back(solved[iter->first].permutation[i]);
			table[iter->first].partialpermutation = buildPartialPermutationPruningTable(temp_perm, moves, iter->first, tmp_ignore);
			table[iter->first].partialpermutation_depth = maxDepth(table[iter->first].partialpermutation);
		}

		tmp_ignore.clear();
		if (ignore.find(iter->first) != ignore.end())
			for (int i = 0; i < ignore[iter->first].size; i++)
				tmp_ignore.push_back(ignore[iter->first].orientation[i]);
		double osize = log(datasets[iter->first].omod) * datasets[iter->first].size;
		if (osize < log(MAX_COMPLETE_ORIENTATION_TABLE_SIZE)){ // Not to big tables. Using log to avoid overflow.
			std::vector<int> temp_orient;
			for (int i = 0; i < solved[iter->first].size; i++)
				temp_orient.push_back(solved[iter->first].orientation[i]);
			table[iter->first].orientation = buildCompleteOrientationPruningTable(temp_orient , moves, iter->first, datasets[iter->first].omod, tmp_ignore);
		}
		else{
			std::vector<int> temp_orient;
			for (int i = 0; i < solved[iter->first].size; i++)
				temp_orient.push_back(solved[iter->first].orientation[i]);
			table[iter->first].partialorientation = buildPartialOrientationPruningTable(temp_orient, moves, iter->first, datasets[iter->first].omod, tmp_ignore);
			table[iter->first].partialorientation_depth = maxDepth(table[iter->first].partialorientation);
		}
	}
	return table;
}                    

static std::vector<char> buildCompleteOrientationPruningTable(std::vector<int> solved, MoveList moves, string setname, int omod, std::vector<int> ignore)
{
	std::cout << "Building pruning for " << setname << " orientation.\n";
	std::vector<char> table;
	int vector_size = solved.size();
	MoveList::iterator iter;
	int tablesize = 1;
	for (int i = 0; i < solved.size(); i++)
		tablesize *= omod;  // tablesize := omod ^ solved.size() 
							// checking for numbers getting to large might be smart
		
	table.resize(tablesize);
	for (int i = 0; i < tablesize; i++)
		table[i] = -1;
		
	std::cout << "tablesize " << tablesize << "\n";

	table[oVector2Index(solved, omod)] = 0; // Put solved position in table

	int len = 0;
	int c;
	do
	{
		c = 0;
		for (int p = 0; p < tablesize; p++){
			if (table[p] == len){
				for (iter = moves.begin(); iter != moves.end(); iter++){
					int q = oVector2Index(applySubmoveO(oIndex2Vector(p, vector_size, omod), iter->second.state[setname].orientation, iter->second.state[setname].permutation, iter->second.state[setname].size, omod), omod);
					if (table[q] == -1){
						table[q] = len + 1;
						c++;
					}
				}
			}      
		}
		len++;
		if (ignore.empty()) // Dont write if first pass
			std::cout << c << " positions at depth " << len << "\n"; 
	}while(c > 0);
	
	if (!ignore.empty()){ // If some pieces are to be ignored, use first pass to generate all
								// solved positions. Then generate the real table.
		c = 0;
		for (int i = 0; i < tablesize; i++){
			if (table[i] != -1){
				std::vector<int> tmp_o = oIndex2Vector(i, vector_size, omod);
				bool solved_pos = true;
				for (int j = 0; j < vector_size; j++)
					if (ignore[j] == 0 && tmp_o[j] != solved[j])
						solved_pos = false;
				if (solved_pos){
					table[i] = 0;
					c++;
				}
				else
					table[i] = -1;
			}
		}
		std::cout << c << " solved positions.\n";
		
		int len = 0;
		int c;
		do
		{
			c = 0;
			for (int p = 0; p < tablesize; p++){
				if (table[p] == len){
					for (iter = moves.begin(); iter != moves.end(); iter++){
						int q = oVector2Index(applySubmoveO(oIndex2Vector(p, vector_size, omod), iter->second.state[setname].orientation, iter->second.state[setname].permutation, iter->second.state[setname].size, omod), omod);
						if (table[q] == -1){
							table[q] = len + 1;
							c++;
						}
					}
				}
			}      
			len++;
			std::cout << c << " positions at depth " << len << "\n"; 
		}while(c > 0);
	}
	
	return table;
}

// Complete table, unique pieces
static std::vector<char> buildCompletePermutationPruningTable(std::vector<int> solved, MoveList moves, string setname, std::vector<int> ignore)
{
	std::cout << "Building pruning for " << setname << " permutation.\n";
	std::vector<char> table;
	int vector_size = solved.size();
	MoveList::iterator iter;
	int tablesize = 1;
	tablesize = factorial(solved.size());
	
	table.resize(tablesize);
	for (int i = 0; i < tablesize; i++)
		table[i] = -1;
		
	std::cout << "tablesize " << tablesize << "\n";

	table[pVector2Index(solved)] = 0; // Put solved position in table

	int len = 0;
	int c;
	do
	{
		c = 0;
		for (int p = 0; p < tablesize; p++){
			if (table[p] == len){
				for (iter = moves.begin(); iter != moves.end(); iter++){
					int q = pVector2Index(applySubmoveP(pIndex2Vector(p, vector_size), iter->second.state[setname].permutation, iter->second.state[setname].size));
					if (table[q] == -1){
						table[q] = len + 1;
						c++;
					}
				}
			}      
		}
		len++;
		if (ignore.empty())
			std::cout << c << " positions at depth " << len << "\n";
		else
			std::cout << c << " positions in phase one, depth " << len << "\n"; 
	}while(c > 0);

	if (!ignore.empty()){
		c = 0;
		for (int i = 0; i < tablesize; i++){
			if (table[i] != -1){
				std::vector<int> tmp_p = pIndex2Vector(i, vector_size);
				bool solved_pos = true;
				for (int j = 0; j < vector_size; j++)
					if (ignore[j] == 0 && tmp_p[j] != solved[j])
						solved_pos = false;
				if (solved_pos){
					table[i] = 0;
					c++;
				}
				else
					table[i] = -1;
			}
		}
		std::cout << c << " solved positions.\n";
		int len = 0;
		int c;
		do
		{
			c = 0;
			for (int p = 0; p < tablesize; p++){
				if (table[p] == len){
					for (iter = moves.begin(); iter != moves.end(); iter++){
						int q = pVector2Index(applySubmoveP(pIndex2Vector(p, vector_size), iter->second.state[setname].permutation, iter->second.state[setname].size));
						if (table[q] == -1){
							table[q] = len + 1;
							c++;
						}
					}
				}      
			}
			len++;
			std::cout << c << " positions at depth " << len << "\n"; 
		}while(c > 0);
	}

	return table;
}

// Complete table, not unique pieces
static std::vector<char> buildCompletePermutationPruningTable3(std::vector<int> solved, MoveList moves, string setname, std::vector<int> ignore)
{
	std::cout << "Building pruning for " << setname << " permutation\n";
	std::vector<char> table;
	int vector_size = solved.size();
	MoveList::iterator iter;
	int tablesize = combinations(solved);
		
	table.resize(tablesize);
	for (int i = 0; i < tablesize; i++)
		table[i] = -1;
		
	std::cout << "tablesize " << tablesize << "\n";

	table[pVector3Index(solved)] = 0; // Put solved position in table

	int len = 0;
	int c;
	do
	{
		c = 0;
		for (int p = 0; p < tablesize; p++){
			if (table[p] == len){
				for (iter = moves.begin(); iter != moves.end(); iter++){
					// FIX, assumes that inverses to all moves are also one move
					int q = pVector3Index(applySubmoveP(pIndex3Vector(p, solved), iter->second.state[setname].permutation, iter->second.state[setname].size));
					// FIX
					if (table[q] == -1){
						table[q] = len + 1;
						c++;
					}
				}
			}      
		}
		len++;
		if (ignore.empty())
			std::cout << c << " positions at depth " << len << "\n"; 
	}while(c > 0);
	
	if (!ignore.empty()){
		c = 0;
		for (int i = 0; i < tablesize; i++){
			if (table[i] != -1){
				std::vector<int> tmp_p = pIndex3Vector(i, solved);
				bool solved_pos = true;
				for (int j = 0; j < vector_size; j++)
					if (ignore[j] == 0 && tmp_p[j] != solved[j])
						solved_pos = false;
				if (solved_pos){
					table[i] = 0;
					c++;
				}
				else
					table[i] = -1;
			}
		}
		std::cout << c << " solved positions.\n";

		int len = 0;
		int c;
		do
		{
			c = 0;
			for (int p = 0; p < tablesize; p++){
				if (table[p] == len){
					for (iter = moves.begin(); iter != moves.end(); iter++){
						// FIX, assumes that inverses to all moves are also one move
						int q = pVector3Index(applySubmoveP(pIndex3Vector(p, solved), iter->second.state[setname].permutation, iter->second.state[setname].size));
						// FIX
						if (table[q] == -1){
							table[q] = len + 1;
							c++;
						}
					}
				}      
			}
			len++;
			std::cout << c << " positions at depth " << len << "\n"; 
		}while(c > 0);

	}
	return table;
}

static std::map<std::vector<long long>, char> buildPartialOrientationPruningTable(std::vector<int> solved, MoveList moves, string setname, int omod, std::vector<int> ignore)
{
	std::cout << "Building partial pruning table for " << setname << " orientation.\n";
	std::map<std::vector<long long>, char> table;
	std::map<std::vector<long long>, char> old_table;
	std::map<std::vector<long long>, char>::iterator iter2, iter3;
	MoveList::iterator iter;
	int tablesize = 1;

	table[packVector(solved)] = 0; // Put solved position in table

	int len = 0;
	int c, tot_c;
	tot_c = 0;
	bool abort = false;

	do
	{
		c = 0;
		for (iter2 = table.begin(); iter2 != table.end(); iter++){
			if (iter2->second == len && !abort){
				for (iter = moves.begin(); iter != moves.end(); iter++){
					std::vector<int> pos = unpackVector(iter2->first);
					std::vector<int> q = applySubmoveO(pos, iter->second.state[setname].orientation, iter->second.state[setname].permutation, iter->second.state[setname].size, omod);
					std::vector<long long> newpos = packVector(q);
					if (table.find(newpos) == table.end()){
						table[newpos] = len + 1;
						c++;
						tot_c++;
						if (tot_c >= MAX_PARTIAL_ORIENTATION_TABLE_SIZE){
							abort = true;
							break;
						}
					}
				}
			}
		}
		if (!abort)
			old_table = table;
		len++;
		std::cout << c << " positions at depth " << len << "\n"; 
			
	}while(c > 0 && !abort);
	if (abort){
		std::cout << "Too many positions at depth " << len << ", removing.\n";
		return old_table;
	}
	
	return table;
}


static std::map<std::vector<long long>, char> buildPartialPermutationPruningTable(std::vector<int> solved, MoveList moves, string setname, std::vector<int> ignore)
{
	std::cout << "Building partial pruning for " << setname << " permutation.\n";
	std::map<std::vector<long long>, char> table;
	std::map<std::vector<long long>, char> old_table;

	std::map<std::vector<long long>, char>::iterator iter2;
	MoveList::iterator iter;

	std::vector<long long> first_key = packVector(solved);
	table[first_key] = 0; // Put solved position in table

	if (!ignore.empty()){
		std::vector<int> repermutation;
		std::vector<int> first_perm;
		for (int i = 0; i < ignore.size(); i++)
			if (ignore[i] == 1)
				repermutation.push_back(i);
		if (repermutation.size() > 8){
			std::cout << "Can't ignore permutation of more than 8 pieces in a big set.\n";
			std::cout << "Set: " << setname << "\n";
			exit(-1);
		}
		while(next_permutation(repermutation.begin(), repermutation.end())){
			std::vector<int> tmp_perm;
			tmp_perm = solved;
			int v = 0;
			
			for (int i = 0; i < solved.size(); i++){
				if (ignore[i] == 1){
					tmp_perm[i] = solved[repermutation[v]];
					v++;
				}
			}
			table[packVector(tmp_perm)] = 0;
		}
		std::cout << table.size() << " solved positions.\n";
	}

	int len = 0;
	int c, tot_c;
	tot_c = 0;
	bool abort = false;
	do
	{
		c = 0;
		for (iter2 = table.begin(); iter2 != table.end(); iter2++){
			if (iter2->second == len && !abort){
				for (iter = moves.begin(); iter != moves.end(); iter++){
					std::vector<int> pos = unpackVector(iter2->first);
					std::vector<int> q = applySubmoveP(pos , iter->second.state[setname].permutation, iter->second.state[setname].size);
					std::vector<long long> newpos = packVector(q);
					if (table.find(newpos) == table.end()){
						table[newpos] = len + 1;
						c++;
						tot_c++;
						if (tot_c >= MAX_PARTIAL_PERMUTATION_TABLE_SIZE){
							abort = true;
							break;
						}
					}
				}
			}      
		}
		if (!abort)
			old_table = table;
		len++;
		std::cout << c << " positions at depth " << len << "\n";
	}while(c > 0 && !abort);
	
	if (abort){
		std::cout << "Too many positions at depth " << len << ", removing.\n";
		return old_table;
	}

	return table;
}

static int maxDepth(std::map<std::vector<long long>, char> table){
	int maxdepth = 0;
	std::map<std::vector<long long>, char>::iterator iter;
	for (iter = table.begin(); iter != table.end(); iter++)
		if (maxdepth < iter->second)
			maxdepth = iter->second;                     
	return maxdepth;        
}

// Function checks the tables and assign flags accordingly
static void updateDatasets(PieceTypes& datasets, PruneTable& tables)
{
	PruneTable::iterator iter;
	for (iter = tables.begin(); iter != tables.end(); iter++){
		if (iter->second.permutation.size() >= 1)
			datasets[iter->first].ptabletype = TABLE_TYPE_COMPLETE;
		else if (iter->second.partialpermutation.size() >= 1)
			datasets[iter->first].ptabletype = TABLE_TYPE_PARTIAL;
		else
			datasets[iter->first].ptabletype = TABLE_TYPE_NONE;

		double tablesize = 1.0;
		for (int i = 0; i < datasets[iter->first].size; i++)
			tablesize *= datasets[iter->first].omod;  // tablesize := omod ^ solved.size() 
									// checking for numbers getting to large might be smart
		if (iter->second.orientation.size() < 1)
			datasets[iter->first].otabletype = TABLE_TYPE_NONE;
		else if (tablesize <= MAX_COMPLETE_ORIENTATION_TABLE_SIZE)
			datasets[iter->first].otabletype = TABLE_TYPE_COMPLETE;
		else
			datasets[iter->first].otabletype = TABLE_TYPE_PARTIAL;
	}
}

static bool prune(Position& state, int depth, PieceTypes& datasets, PruneTable& prunetables){
	Position::iterator iter2;
	for (iter2 = state.begin(); iter2 != state.end(); iter2++){

		// Orientation pruning
		if (datasets[iter2->first].otabletype == TABLE_TYPE_COMPLETE){
			int index = oVector2Index(iter2->second.orientation, iter2->second.size, datasets[iter2->first].omod);
			if (prunetables[iter2->first].orientation[index]  > depth){
				return true;
			}
		}
		else if (datasets[iter2->first].otabletype == TABLE_TYPE_PARTIAL){
			std::vector<long long> index = packVector(iter2->second.orientation, iter2->second.size);
			
			if (prunetables[iter2->first].partialorientation_depth >= depth){
				if (prunetables[iter2->first].partialorientation.count(index) == 1){ // If the position exist in the table then...
					if (prunetables[iter2->first].partialorientation[index] > depth){
						return true;
					}                         
				}
				else{
					return true;
				}
			}
		}
		// Permutation pruning
		if (datasets[iter2->first].ptabletype == TABLE_TYPE_COMPLETE && datasets[iter2->first].uniqueperm){
			int index = pVector2Index(iter2->second.permutation, iter2->second.size);
			if (prunetables[iter2->first].permutation[index]  > depth){
				return true;
			}
		}
		else if (datasets[iter2->first].ptabletype == TABLE_TYPE_COMPLETE && !datasets[iter2->first].uniqueperm){
			long long index = pVector3Index(iter2->second.permutation, iter2->second.size);
			if (prunetables[iter2->first].permutation[index]  > depth){
				return true;
			}
		}
		else if (datasets[iter2->first].ptabletype == TABLE_TYPE_PARTIAL){;
			std::vector<long long> index = packVector(iter2->second.permutation, iter2->second.size);

			if (prunetables[iter2->first].partialpermutation_depth >= depth){
				if (prunetables[iter2->first].partialpermutation.find(index) != prunetables[iter2->first].partialpermutation.end()){
					if (prunetables[iter2->first].partialpermutation[index] > depth){
						return true;
					}
				}
				else{
					return true;
				}
			}
		}
	}
	return false;
}

#endif
