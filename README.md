# b-adorators
Implementation of a parallel efficient approximation algorithm for weighted B-matching 
Solution is based on paper: https://www.cs.purdue.edu/homes/apothen/Papers/bMatching-SISC-2016.pdf

## Problem description

From the abstract of Efficient approximation algorithms for weighted b-Matching (Khan, Arif, et al, 2016):

> b-Matching is a generalization of the well-known Matching problem in graphs, where the objective is to choose a subset of M edges in the graph such that at most a specified number b(v) of edges in M are incident on each vertex v. Subject to this restriction we maximize the sum of the weights of the edges in M.

## Pseudo-code

Input: A graph G = (V, E, w) and a vector b. Output: A 1/2−approximate edge weighted b-Matching M.

```
procedure Parallel b-Suitor(G, b):
 Q = V ; Q'= {}; 
 while Q is not empty:
   for all vertices u in Q in parallel:
     i = 1;
     while i <= b(u) and N(u) != exhausted:
       Let p in N(u) be an eligible partner of u;
       if p != NULL:
         Lock p; 
         if p is still eligible:
           i = i + 1; 
           Make u a Suitor of p;
           if u annuls the proposal of a vertex v:
             Add v to Q';
             Update db(v); 
         Unlock p; 
       else:
         N(u) = exhausted; 
   Update Q using Q';
   Update b using db;
```
