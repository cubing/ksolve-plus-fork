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

// Functions to check whether permutations are unique.

#ifndef CHECKS_H
#define CHECKS_H

// Check if a vector of length n is a permutation 
// of 1,..,n
static bool uniquePermutation(std::vector<int> test) {
   for (int i = 0; i < test.size(); i++)
      if (test[i] <= 0 || test[i] > test.size())
         return false; // Number too large or small

   std::vector<bool> temp;
   for (int i = 0; i < test.size(); i++)
      temp.push_back(false);
   for (int i = 0; i < test.size(); i++)
      temp[test[i] - 1] = true;
   for (int i = 0; i < temp.size(); i++)
      if (!temp[i])
         return false; // Numbers not unique

   return true;
}

static bool uniquePermutation(int test[], int size) {
   for (int i = 0; i < size; i++)
      if (test[i] <= 0 || test[i] > size)
         return false; // Number too large or small
         
   std::vector<bool> temp;
   for (int i = 0; i < size; i++)
      temp.push_back(false);
   for (int i = 0; i < size; i++)
      temp[test[i] - 1] = true;
   for (int i = 0; i < size; i++)
      if (!temp[i])
         return false; // Numbers not unique

   return true;
}

#endif
