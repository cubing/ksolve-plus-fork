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

static int oVector2Index(std::vector<int> orientations, int omod) {
	for (int i = 0; i < orientations.size(); i++)
		orientations[i] = orientations[i] % omod;
		
	int tmp = 0;
	for (int i = 0; i < orientations.size(); i++){
		tmp *= omod;
		tmp += orientations[i];
	}
	return tmp;
}

static int oVector2Index(int orientations[], int size, int omod) {
	for (int i = 0; i < size; i++)
		orientations[i] = orientations[i] % omod;
		
	int tmp = 0;
	for (int i = 0; i < size; i++){
		tmp *= omod;
		tmp += orientations[i];
	}
	return tmp;
}

// version of oVector2Index for orientation with parity constraint
static int oparVector2Index(int orientations[], int size, int omod) {
	for (int i = 0; i < size; i++)
		orientations[i] = orientations[i] % omod;
		
	int tmp = 0;
	for (int i = 0; i < size - 1; i++){
		tmp *= omod;
		tmp += orientations[i];
	}
	return tmp;
}

static std::vector<int> oIndex2Vector(int index, int size, int omod) {
	std::vector<int> orientations;
	orientations.resize(size);
	for (int i = size - 1; i >= 0; i--){
		orientations[i] = index % omod;
		index /= omod;
	}
	return orientations;         
}

// Non-vector version of oIndex2Vector
static int* oIndex2Array(int index, int size, int omod) {
	int* orientation = new int[size];
	for (int i = size - 1; i >= 0; i--){
		orientation[i] = index % omod;
		index /= omod;
	}
	return orientation;         
}

// version of oIndex2Array for orientation with parity constraint
static int* oparIndex2Array(int index, int size, int omod) {
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

static int pVector2Index(std::vector<int> permutation) {
	int t = 0;
	int size = permutation.size();
	for (int i = 0; i < size - 1; i++){
		t *= (size - i);
		for (int j = i+1; j<size; j++)
			if (permutation[i] > permutation[j])
				t++;
	}
	return t;
}

static int pVector2Index(int permutation[], int size) {
	int t = 0;
	for (int i = 0; i < size - 1; i++){
		t *= (size - i);
		for (int j = i+1; j<size; j++)
			if (permutation[i] > permutation[j])
				t++;
	}
	return t;
}

static std::vector<int> pIndex2Vector(int index, int size) {
	std::vector<int> permutation;
	permutation.resize(size);
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

// Non-vector version of pIndex2Vector
static int* pIndex2Array(int index, int size) {
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

static long long pVector3Index(std::vector<int> permutation) {
	if (permutation.size() == 1)
		return 0;
	long long index = 0;
	int max = 0;
	std::vector<int> temp_vec;
	std::vector<int>::iterator iter;
	for (int i = 1; i < permutation[0]; i++){
		temp_vec = permutation;
		
		iter = find(temp_vec.begin(), temp_vec.end(), i);
		if (iter != temp_vec.end()){
			temp_vec.erase(iter);
			index += combinations(temp_vec);
		}
	}
	temp_vec = permutation;
	temp_vec.erase(temp_vec.begin());
	index += pVector3Index(temp_vec);
	return index;
}

static long long pVector3Index(int permutation[], int size) {
	// Quick and ugly, but it works for now
	std::vector<int> temp_vec;
	for (int i = 0; i < size; i++)
		temp_vec.push_back(permutation[i]);
	
	return pVector3Index(temp_vec);
}

static std::vector<int> pIndex3Vector(long long index, std::vector<int> solved) {
	int vec_length = solved.size();
	std::vector<int> vec;
	std::vector<int> temp_vec1, temp_vec2;
	std::vector<int>::iterator iter;
	vec.resize(vec_length);
	int max = solved[0];
	for (int i = 0; i < solved.size(); i++)
		if (max < solved[i])
			max = solved[i];
			
	temp_vec1 = solved;
	for (int i = 0; i < vec_length; i++){
		int j;
		for (j = 0; j <= max; j++){
			temp_vec2 = temp_vec1;
			iter = find(temp_vec2.begin(), temp_vec2.end(), j);
			if (iter != temp_vec2.end()){
				temp_vec2.erase(iter);
				int num = combinations(temp_vec2);
				if (num <= index)
					index -= num;
				else
					break;
			}
		}
		vec[i] = j;
		iter = find(temp_vec1.begin(), temp_vec1.end(), j);
		temp_vec1.erase(iter);
	}
	return vec;
}

// Non-vector version of pIndex3Vector
static int* pIndex3Array(long long index, int* solved, int size) {
	int i, j;
	
	// Determine maximum value in solved array
	int max = solved[0];
	for (int i = 0; i < size; i++)
		if (max < solved[i])
			max = solved[i];
			
	int* vec = new int[size];
	std::vector<int> temp_vec1, temp_vec2;
	std::vector<int>::iterator iter;
	for (i = 0; i<size; i++) {
		temp_vec1.push_back(solved[i]);
	}
	
	for (i = 0; i < size; i++){
		for (j = 0; j <= max; j++){
			temp_vec2 = temp_vec1;
			iter = find(temp_vec2.begin(), temp_vec2.end(), j);
			if (iter != temp_vec2.end()){
				temp_vec2.erase(iter);
            int num = combinations(temp_vec2);
            if (num <= index)
				index -= num;
            else
				break;
			}
		}
		vec[i] = j;
		iter = find(temp_vec1.begin(), temp_vec1.end(), j);
		temp_vec1.erase(iter);
	}
	return vec;
}

static long long combinations(std::vector<int> vec) {
	std::map<int, int> counter;
	std::map<int, int>::iterator iter;
	for (int i = 0; i < vec.size(); i++){
		if (counter.find(vec[i]) == counter.end())
			counter[vec[i]] = 1;
		else
			counter[vec[i]]++;
	}
	
	int sum = 0;
	for (iter = counter.begin(); iter != counter.end(); iter++){
		sum += iter->second;
	}
	long long comb = factorial(sum);
	if (comb == -1){ // Too big to compute
		return -1;
	}
	for (iter = counter.begin(); iter != counter.end(); iter++)
		comb /= factorial(iter->second);
	return comb;
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
	
	int sum = 0;
	for (iter = counter.begin(); iter != counter.end(); iter++){
		sum += iter->second;
	}
	long long comb = factorial(sum);
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
	std::vector<long long> result;
	std::vector<int> temp;
	for (int i = 0; i < 8; i++)
		temp.push_back(0);
		
	for (int i = 0; i < vec.size(); i++){
		temp[i % 8] = vec[i];
		if (i % 8 == 7 || i == vec.size() - 1){
			result.push_back(packSubVector(temp));
			for (int i = 0; i < 8; i++)
				temp[i] = 0;
		}
	}
	return result;
}
            
static std::vector<long long> packVector(int vec[], int size){
	std::vector<long long> result;
	std::vector<int> temp;
	for (int i = 0; i < 8; i++)
		temp.push_back(0);
		
	for (int i = 0; i < size; i++){
		temp[i % 8] = vec[i];
		if (i % 8 == 7 || i == size - 1){
			result.push_back(packSubVector(temp));
			for (int i = 0; i < 8; i++)
				temp[i] = 0;
		}
	}
	return result;
}

static long long packSubVector(std::vector <int> vec){
	long long result = 0;
	for (int i = 0; i < vec.size(); i++){
		long long tmp = vec[i];
		result += (tmp << (8*i));
	}
	return result;         
}

static std::vector<int> unpackVector(std::vector<long long> vec){
	std::vector <int> result;
	for (int i = 0; i < vec.size(); i++){
		std::vector<int> tmp;
		tmp = unpackSubVector(vec[i]);
		for (int j = 0; j < tmp.size(); j++)
			result.push_back(tmp[j]);
		tmp.clear();
	}
	return result;
}

static std::vector<int> unpackSubVector(long long number){
	std::vector<int> temp;
	for (int i = 0; i < 8; i++)
		temp.push_back(0);
	for (int i = 0; i < 8; i++){
		temp[i] = (number & 0x00000000000000FF);
		number = (number >> 8);
	}
	while(temp[temp.size() - 1] == 0)
		temp.pop_back();
	return temp;
}

#endif
