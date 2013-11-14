                      ksolve+ v1.0
                     (c)  2007-2013
            by Kare Krig and Michael Gottlieb

###### Contents ######

* Contents
* What is ksolve+?
* The Definition File
  * Name
  * Set
  * Solved
  * Move
  * Ignore
  * Block
  * ForbiddenPairs and ForbiddenGroups
  * MoveLimits
  * Using Comments
  * Deprecated Commands
* The Scramble File
  * Scramble
  * ScrambleAlg
  * RandomScramble
  * MaxDepth
  * Slack
  * QTM and HTM
* God's Algorithm
* Details and Tricks
  * Pruning Tables
  * Interchangeable Pieces
  * Finding All Short Algs
  * Bandaging Pieces to Centers
* Version History

###### What is ksolve+? ######

ksolve+ is a program that can generate algorithms for twisty puzzles. It is not designed for any specific puzzle. Instead, you can define your own puzzle, define a position on that puzzle, and then find move sequences that solve that position. There are many options, allowing for many different types of algorithms to be generated. ksolve+ now also has the ability to generate God's Algorithm tables for simple enough puzzles.

Note that, because ksolve+ is so general, it may not be as fast or memory-efficient as a program specifically designed for solving a particular puzzle. The advantage is that you can use relatively complex techniques to find algorithms for an unusual puzzle, without having to spend many hours programming.

You can run ksolve+ from the command line, using a command like this:
        ksolve puzzle.def scramble.txt
I have included some sample puzzle and scramble files for you to play with. You can also compute God's Algorithm tables without a scramble file (see the section below).

###### The Definition File ######

Normally, to run ksolve+, you will need two files: a definition file and a scramble file. They can be named whatever you want (the .def extension isn't necessary, for instance), and you can create these files in any simple text editor. The program comes bundled with a few of these files for you to try out.

The definition file defines the characteristics of a puzzle, in enough detail to let ksolve+ find algorithms for it. (The scramble file, on the other hand, defines scrambles for that puzzle.) A definition file is composed of commands which each describe some aspect of the puzzle. Commands should be separated by newlines. Note that it is not necessary or suggested to use all of these commands in each definition file.

The following sections describe each command. I will include what the command should look like; when you see anything in brackets (such as [string]), that is just a stand-in for information you will provide, and you should not actually type out the brackets or the text inside them.

-- Name --

Name [string]

The Name command just says the name of the puzzle you are describing. This is not necessary for the program to run, but it's useful to write it anyway.

-- Set --

Set [set_name] [number_of_pieces] [number_of_orientations]

The Set command defines one type of piece in your puzzle; there can be as many types as you want. Pieces in a set should be able to move into each others' position. After the name of the set, you will include the number of pieces of that type in your puzzle, and the number of orientations each piece has.

-- Solved --

Solved
[set_name]
[permutation vector]
[orientation vector]
...
End

The Solved command defines the solved state of your puzzle. Of course, you will usually want the puzzle to end up with every piece in its original position and unoriented, but you have the option of solving to a different state. You need to define a permutation and orientation for each set in your puzzle.

Permutation vectors and orientation vectors simply describe where every piece in a group is and how they are oriented. A permutation vector is a list of the numbers from 1 to n in some order, such as 2 3 1 4 5 6. This describes where each piece goes - for instance, the 2 in the first spot means that piece number 1 is in spot 2. An orientation vector is a list of n numbers from 0 up to the maximum orientation, such as 0 0 0 1 1 0. A piece marked with a 0 is unoriented, and a piece with an orientation of 1, 2, etc. is oriented by that much. For example, 3x3x3 edges have two orientations each, so with that type of piece your orientation vector will only have 0s and 1s.

-- Move --

Move [move_name]
[set_name]
[permutation vector]
[orientation vector]
...
End

The Move command defines one of the possible moves and how it affects the pieces in your puzzle. Again, you need to define a permutation and orientation for each set in your puzzle.

ksolve+ will not just understand this move, but also all powers of it. For example, if your puzzle is a 3x3x3 and you define a move of the right face which you call R, ksolve+ will also create moves called R2 and R' automatically. You do not need to define those moves separately.

-- Ignore --

Ignore
[set_name]
[permutations_to_ignore]
[orientations_to_ignore]
...
End

The Ignore command defines which parts of the puzzle ksolve+ may ignore while solving. You do not need to include all of the piece types here - if there are any piece types you do not include, ksolve+ will assume you want to solve everything of that type.

The permutations to ignore and orientations to ignore are simply lists of n numbers, each 0 or 1, where n is the number of pieces of that type. A 0 means ksolve+ will solve that, and a 1 means it will ignore it. Note that, if you want, you can ignore the orientation of a piece while still solving its permutation, or the other way around.

Note that, unlike earlier versions of ksolve, an Ignore command does not necessarily mean pieces will actually be ignored in the scramble - it just describes all of the pieces scrambles are allowed to ignore. When you write scrambles, you will describe which pieces should be ignored (if any). Thus the same definition file can be used to fully solve positions and to solve positions with some pieces (or some orientations or permutations) ignored.

-- Block --

Block
[set_name]
[pieces_to_join]
...
End

The Block command defines pieces that should be joined together, so that moves must either move all of the pieces in a block at once, or none of them. This is also known as bandaging. Note that a Block command does not describe all of these blocks in a puzzle, but merely one of them - for multiple bandaged groups, you will need multiple Block commands.

The syntax of this command is a bit different from other commands. Inside the Block, you will write the name of a set, then the pieces in that set that form the block. You will then repeat that for any other sets included in this block. The pieces should be identified using the same 1, 2, ... numbering scheme that was used in permutations throughout the definition file.

-- ForbiddenPairs and ForbiddenGroups --

ForbiddenPairs
[move_name] [move_name] 
...
End

ForbiddenGroups
[move_name] ...
...
End

The ForbiddenPairs command defines pairs of moves that can not be used together. For instance, if you have U F', then ksolve+ will not produce any solutions with a U move followed by an F' move. ForbiddenGroups is similar, but each line can have several moves, and it will forbid any pair of moves from the same line.

Note that ksolve+ already forbids obvious move pairs, such as U2 U or R R', so you do not need to add those. If you want to forbid other pairs of moves, however, you can still do that.

-- MoveLimits --

MoveLimits
[move_name] [number]
...
End

The MoveLimits command puts upper limits on the number of times a given move (or any of its powers) can be included in a solution. For instance, if you have a limit of "F 2" on a 3x3x3 definition file, then a maximum of two F, F2, or F' moves can be used in any algorithm. If a move has a limit of 0, it will never be used.

-- Using Comments --

# [string]

To make a comment, simply type a # at the beginning of the line. ksolve+ will ignore the rest of the line no matter what you write there. These are useful for writing yourself notes about the puzzle or keeping track of which numbers correspond to which pieces.

-- Deprecated Commands --

There are two commands which are no longer used: Multiplicators (used to describe powers of moves, such as R2 being equal to two R moves) and ParallelMoves (used to describe moves which are parallel, such as R and L on the 3x3x3). Although you may still include them in a definition file without causing an error in the program, they are no longer necessary because ksolve+ automatically generates move powers and checks for parallel moves.

###### The Scramble File ######

Recall that ksolve+ runs on a definition file and a scramble file. (In fact, you don't always need a scramble file - see the God's Algorithm for some details there.)

The scramble file defines as many scrambles as you want; each one is given as a position of the puzzle to solve. There are also some commands to modify what kind of solutions ksolve+ will search for. As with the definition file, commands should be separated by newlines, and it is not necessary to use all of the commands.

The following sections describe each command. I will include what the command should look like; when you see anything in brackets (such as [string]), that is just a stand-in for information you will provide, and you should not actually type out the brackets or the text inside them.

-- Scramble --

Scramble [scramble_name]
[set_name]
[permutation vector]
[orientation vector]
...
End

The Scramble command defines a scramble that ksolve+ will attempt to solve when you feed it this file. You must include a permutation and orientation for each set in the puzzle. You can have any number of scrambles, and ksolve+ will solve them each separately, in order.

Scrambles can ignore pieces - permutation, orientation, or both. The simplest way to ignore something is replace that number with a ?. Remember, however, that you can only ignore permutations or orientations that you specified with the def file's Ignore command - but you don't need to ignore all of those.

If you want to ignore something, but still give ksolve+ a hint about one possible permutation or orientation, you can add the number after the ? (for instance, ?2). For something simple, like solving PLL on a 3x3x3, those hints are unnecessary, but for complex puzzles or solutions they may be very important. Not giving hints may lead to incorrect results - such as ksolve+ not finding some algorithm. This is especially important on bandaged puzzles, where they allow ksolve+ to properly determine what moves are possible.

-- ScrambleAlg --

ScrambleAlg [scramble_name]
[move1] [move2] [move3] ...
End

The Scramble command defines a scramble in terms of a move sequence. ksolve+ will apply that move sequence to the solved state, print the result, and then solve it. Only moves that are defined in the definition file are allowed (plus inverses and so on). You cannot ignore pieces with this command.

This can be useful to find alternatives to an existing algorithm (by entering in the inverse), or to solve a position that you don't know the permutation and orientation for.

-- RandomScramble --

RandomScramble [scramble_name]
End

The Scramble command generates a random scramble. ksolve+ will print the position and then solve it.

-- MaxDepth --

MaxDepth [number]

The MaxDepth command specifies the maximum move depth (i.e. algorithm length) ksolve+ will try. The maximum depth is normally set to a default of 999, which is for all practical purposes infinity. When you use this command, the maximum depth you give will apply to all scrambles until the end of the file or the next MaxDepth command. Note that this may mean ksolve+ will not find any solutions to certain scrambles.

-- Slack --

Slack [number]

Normally ksolve+ will only return optimal solutions. The Slack command specifies how many extra moves ksolve+ will try, with the default of course being 0. When you use this command, the slack you give will apply to all scrambles until the end of the file or the next Slack command.

Having a few moves of slack can be very useful for finding fast algorithms, because sometimes the optimal algorithms are somewhat awkward. However, slack will make the program take longer to run, and the time taken is generally exponential in the number of moves. Because of this, Slack and MaxDepth make a good combination - MaxDepth prevents the program from spending far too long on any individual scramble, even if it has a long optimal solution. MaxDepth has priority, so if you have a slack of 5 and a maximum depth of 15, a position with an optimal solution of 12 moves will still only search up to 15 moves.

-- QTM and HTM --

QTM

HTM

The QTM and HTM commands specify that a scramble will be solved either in QTM (Quarter Turn Metric, where turns of the smallest possible amount count as one turn) or HTM (Half Turn Metric, where turns of any amount count as one turn). The default is HTM. When you use one of these commands, that metric will be used for all scrambles until the end of the file or the next QTM or HTM command.

###### God's Algorithm ######

ksolve+ can also compute God's Algorithm tables. That is, for each N, it will compute the number of positions that can be solved in N moves but no fewer. You only need a .def file for this. To compute a God's Algorithm table in HTM (Half Turn Metric), use this command:
	ksolve puzzle.def !
To compute a God's Algorithm table in QTM (Quarter Turn Metric), use this command:
	ksolve puzzle.def !q

After finishing the computation of a God's Algorithm table, ksolve+ will print out up to 5 antipodes. These are puzzle positions that require the maximum number of moves to solve. 

Currently there is a limitation on the complexity of the puzzle. If the puzzle has more than about 9 * 10^18 possible states (counting all permutation and orientation cases whether or not they can be achieved by the puzzle), ksolve+ will not try to compute these tables. In the future I may remove this restriction, but there will be a speed and memory tradeoff since it is generally more difficult to store and manipulate numbers larger than 64 bits.

###### Details and Tricks ######

This section contains some advanced information about ksolve+. This information is not necessary for most use of the program, but it may help with defining or solving certain puzzles.

-- Pruning Tables --

Pruning tables are a technique that ksolve+ uses to save time when looking for algorithms. Essentially, for each piece type, and for permutation and orientation separately, the program will generate a table of the minimum number of moves every state can be solved in. This lets ksolve+ ignore certain groups of algorithms by determining that none of them can solve the scramble, without actually trying all of the algorithms in that group. This speeds up the search substantially.

For example, suppose we are searching for 10-move solution to a particular 3x3x3 scramble. Starting from the scramble, if we do the moves F U R2, and the pruning tables tell us that the resulting position is at least 8 moves from solved, we know that algorithms starting with F U R2 must be at least 11 moves to solve this scramble. Thus no 10-move algorithm starting with F U R2 can solve this scramble, and we can ignore all of them.

ksolve+ stores these tables in a .tables file. This file belongs with the corresponding definition file, and may be relatively large (several megabytes); if you ever want to send someone information about a puzzle, you do not need to send them the tables file. It can easily be recomputed, but if you do have the file already, ksolve+ will just load information from it.

If you change your definition file, so that it is newer than the .tables file it corresponds to, ksolve will recalculate the .tables file anyway. However, it is still a good idea to delete the .tables file when you modify the definition file, just in case - otherwise it is possible you will get incorrect results.

The restrictions on the Ignore command are a result of the pruning table setup. When you ignore pieces in the definition file, ksolve uses that information to construct partial pruning tables which also ignore those pieces. If the scramble tries to ignore pieces that were not ignored in the pruning table, ksolve+ may incorrectly conclude that a position cannot be solved in a certain number of moves, when in fact it can. This means that some solutions may not be found. So don't forget, Ignore anything you might not want to consider! You can always make more than one separate definition file for the same puzzle if necessary.

-- Interchangeable Pieces --

ksolve+ also supports making some pieces interchangeable, although it is slower. A typical example is the centers of a 4x4x4, which have 24 pieces organized into four pieces each of six different types. To do this, repeat numbers in your solved and scrambled positions - for instance, the 4x4x4 centers example would have four 1's, four 2's, and so on up to four 6's. Moves, however, must still use unique numbers (in this case 1 through 24).

-- Finding All Short Algs --

It is possible to get ksolve+ to find all short algorithms of a particular type - for instance, all short PLLs on a 3x3x3. The basic method is as follows:
- Decide what information you will have to ignore to get the type of case you are looking for. For instance, for PLLs on a 3x3x3, you would ignore only the permutation of all U-layer pieces.
- Create a definition file which ignores that information.
- Create a scramble file with a single scramble that ignores the ignored information, but is otherwise solved.
- In the scramble file, add a large amount of Slack - for PLLs, for instance, you may want something like 12 or 13 moves.

Put together, since ksolve+ will immediately find the solved state, it will then search for any algorithms of up to Slack moves which bring the cube back to a position of the given type. Since ksolve+ automatically prohibits sequences of moves which obviously cancel (such as R R2 or R L R, on the 3x3x3) it will not print thousands of algorithms which obviously do nothing.

This trick is particularly useful for complex bandaged puzzles, where you will often want to move pieces around without disturbing the location of the blocks.

-- Bandaging Pieces to Centers --

ksolve+ only supports bandaging moving pieces together, but some bandaged puzzles involve pieces bandaged to centers, and centers do not move. The trick here is to add the centers anyway, and never change their permutation, but instead define moves so that they change the orientation of the centers. When the orientation of a piece is changed it counts as being moved, so if you bandage a piece together with a center, the piece can be moved using any move that changes the orientation of that center.

It is possible to define all of the centers together as one piece group in this way, but it is also possible to put each center as a separate piece type, with one piece and one possible orientation. (You can still define moves that orient a piece with no orientation - that move will not change the center's state, but the center will still act properly for Block purposes.) This second option can be useful to decrease the number of possible states of the puzzle for God's Algorithm calculations, and you can see an example in the included Bicube.def file.

###### Version History ######

(ksolve+)
1.0  Large amount of code refactoring and various small fixes
     Allowed ? to ignore pieces in scrambles, with optional hints
     Automated the effect of Multiplicators, ForbiddenPairs, ParallelMoves
     Added QTM/HTM, Slack, MoveLimits, ScrambleAlg, RandomScramble
     Added God's Algorithm computations
     Optimized applyMove to make everything ~25% faster
(ksolve)
0.10 Added Block that fuses pieces together.
0.09 A scramble file can now contain the command "MaxDepth n"
     which causes the solver to give up if no solution is found
     in n or less moves.
0.08 Added ParallelMoves and ForbiddenGroups to make writing 
     def-files less painfull. It is now possible to ignore 
     permutation of some pieces in a big set.
0.07 Bug fix, partial tables should work for real now.
0.06 For large puzzles the program now uses partial pruning 
     tables. This does not yet work together with ignoring 
     pieces. Also multiple scrambles in one file are now 
     possible.
0.05 It is now possible to ignore part of the cube.
0.04 Added Multiplicator-command in definitions.
0.03 Added some error checking when reading def-files 
     and scrambles.
0.02 Pruning tables are now computed once and stored on disk.
0.01 First version.