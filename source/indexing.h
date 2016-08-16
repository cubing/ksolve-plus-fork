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

// Functions for translating permutations and orientations to index (and inverses)

#ifndef INDEXING_H
#define INDEXING_H

// Convert vector of orientations into an index
static long long oVector2Index(std::vector<int> orientations, int omod) {
	return oVector2Index(orientations.data(), orientations.size(), omod);
}

// Convert array of orientations into an index
static long long oVector2Index(int orientations[], int size, int omod) {
	long long tmp = 0;
	for (int i = 0; i < size; i++){
		tmp = tmp*omod + orientations[i];
	}
	return tmp;
}

// Convert array of orientations (with parity constraint) into an index
static long long oparVector2Index(int orientations[], int size, int omod) {
	long long tmp = 0;
	for (int i = 0; i < size - 1; i++){
		tmp = tmp*omod + orientations[i];
	}
	return tmp;
}

// Convert orientation index into a vector
static std::vector<int> oIndex2Vector(long long index, int size, int omod) {
	std::vector<int> orientations;
	orientations.resize(size);
	for (int i = size - 1; i >= 0; i--){
		orientations[i] = index % omod;
		index /= omod;
	}
	return orientations;         
}

// Convert orientation index into an array
static int* oIndex2Array(long long index, int size, int omod) {
	int* orientation = new int[size];
	for (int i = size - 1; i >= 0; i--){
		orientation[i] = index % omod;
		index /= omod;
	}
	return orientation;         
}

// Convert orientation index (with parity constraint) into an array
static int* oparIndex2Array(long long index, int size, int omod) {
	int* orientation = new int[size];
	orientation[size - 1] = 0;
	for (int i = size - 2; i >= 0; i--){
		orientation[i] = index % omod;
		orientation[size - 1] += omod - (index % omod);
		index /= omod;
	}
	orientation[size - 1] = orientation[size - 1] % omod;
	return orientation;         
}

// Convert permutation vector (unique) into an index
static long long pVector2Index(std::vector<int> permutation) {
	return pVector2Index(permutation.data(), permutation.size());
}

// Convert permutation array (unique) into an index
static long long pVector2Index(int permutation[], int size) {
	long long t = 0;
	for (int i = 0; i < size - 1; i++){
		t *= (size - i);
		for (int j = i+1; j<size; j++)
			if (permutation[i] > permutation[j])
				t++;
	}
	return t;
}

// Convert index into a permutation array (unique)
static int* pIndex2Array(long long index, int size) {
	int* permutation = new int[size];
	permutation[size-1] = 1;
	for (int i = size - 2; i >= 0; i--){
		permutation[i] = 1 + (index % (size-i));
		index /= (size - i);
		for (int j = i+1; j < size; j++)
			if (permutation[j] >= permutation[i])
				permutation[j]++;
	}
	return permutation;
}

// Convert permutation vector (non-unique) into an index
static long long pVector3Index(std::vector<int> permutation) {
	return pVector3Index(permutation.data(), permutation.size());
}

// Convert permutation array (non-unique) into an index
static long long pVector3Index(int permutation[], unsigned int size) {
	if (size < 2) return 0;
	int index = 0;
	
	// compute number of times each element appears
	std::map<int, int> counts;
	for (unsigned int i = 0; i < size; i++){
		if (counts.find(permutation[i]) == counts.end())
			counts[permutation[i]] = 1;
		else
			counts[permutation[i]]++;
	}
	
	// compute combinations
	long long comb = factorial(size);
	if (comb == -1){ // Too big :(
		return -1;
	}
	std::map<int, int>::iterator iter;
	for (iter = counts.begin(); iter != counts.end(); iter++)
		comb /= factorial(iter->second);
	
	unsigned int vecsize = size;
	for (unsigned int ptr = 0; ptr < size; ptr++) {
		for (int i=1; i < permutation[ptr]; i++) {
			if (counts[i] > 0) { // i still in permutation
				// add the number of combinations of our permutation without one i
				index += (comb * counts[i])/vecsize;
			}
		}
		// "remove" the first element of the permutation
		comb = (comb * counts[permutation[ptr]])/vecsize;
		vecsize--;
		counts[permutation[ptr]]--;
	}
	
	return index;
}

// Convert index into a permutation array (non-unique)
static int* pIndex3Array(long long index, std::vector<int> solved) {
	return pIndex3Array(index, solved.data(), solved.size());
}

// Convert index into a permutation array (non-unique)
static int* pIndex3Array(long long index, int* solved, int size) {
	int* vec = new int[size];
	
	// compute number of times each element appears
	std::map<int, int> counts;
	std::map<int, int>::iterator iter;
	for (int i = 0; i < size; i++){
		if (counts.find(solved[i]) == counts.end())
			counts[solved[i]] = 1;
		else
			counts[solved[i]]++;
	}
	
	// compute combinations
	long long comb = factorial(size);
	int combsize = size;
	if (comb == -1){ // Too big, WTF?
		return solved;
	}
	for (iter = counts.begin(); iter != counts.end(); iter++)
		comb /= factorial(iter->second);
	
	// now build vec
	for (int i=0; i < size; i++) {
		// loop over each thing in solved
		for (iter = counts.begin(); iter != counts.end(); iter++) {
			// if this thing is still in our permutation
			if (iter->second > 0) {
				// get the number of combinations of the permutation without one thing
				long long num = (comb * iter->second)/combsize;
				// if we can subtract it from index, do so; otherwise we found the ith thing
				if (num <= index)
					index -= num;
				else
					break;
			}
		}
		// store this thing, and "remove" the first element of solved
		vec[i] = iter->first;
		comb = (comb * iter->second)/combsize;
		combsize--;
		counts[iter->first]--;
	}
	return vec;
}

static long long combinations(std::vector<int> vec) {
	return combinations(vec.data(), vec.size());
}

static long long combinations(int vec[], int size) {
	std::map<int, int> counter;
	std::map<int, int>::iterator iter;
	for (int i = 0; i < size; i++){
		if (counter.find(vec[i]) == counter.end())
			counter[vec[i]] = 1;
		else
			counter[vec[i]]++;
	}
	
	long long comb = factorial(size);
	if (comb == -1){ // Too big to compute
		return -1;
	}
	for (iter = counter.begin(); iter != counter.end(); iter++)
		comb /= factorial(iter->second);
	return comb;
}

static long long factorial(long long x) {
	if (x <= 1)
		return 1;
	else if (x > 20)
		return -1;  // Would overflow a long long
		
	static const long long fac[21] = {1LL, 1LL, 2LL, 6LL, 24LL, 120LL,
		720LL, 5040LL, 40320LL, 362880LL, 3628800LL, 39916800LL, 479001600LL,
		6227020800LL,  87178291200LL, 1307674368000LL, 20922789888000LL, 355687428096000LL, 6402373705728000LL, 121645100408832000LL, 2432902008176640000LL};
	return fac[x];
}

static std::vector<long long> packVector(std::vector<int> vec){
	return packVector(vec.data(), vec.size());
}
       
static std::vector<long long> packVector(int vec[], int size){
	std::vector<long long> result (1 + size/8);
	for (int i = 0; i < size; i += 8) {
		long long element = 0;
		for (int j = 0; j < 8; j++)
			if (i+j < size) element += (1LL+vec[i+j]) << (8*j);
		result[i/8] = element;
	}
	return result;
}

static std::vector<int> unpackVector(std::vector<long long> vec){
	unsigned int size = vec.size();
	std::vector<int> result (8*size);
	
	for (unsigned int i = 0; i < size; i++){
		long long number = vec[i];
		for (int j = 0; j < 8; j++){
			result[i*8+j] = (number & 0xFF);
			number = (number >> 8);
		}
	}
	while(result[result.size() - 1] == 0)
			result.pop_back();
	for (int i=0; i<result.size(); i++)
           result[i]-- ;
	return result;
}

// find out if a permutation is even
/*
static bool isEven(int[] vec, int size) {
	// silly O(n^2) alg
	int transpositions = 0;
	for (int i=0; i<size; i++) {
		for (int j=i+1; j<size; j++) {
			if (vec[i]>vec[j]) transpositions++;
		}
	}
	return (transpositions%2==0);
}*/

#endif
