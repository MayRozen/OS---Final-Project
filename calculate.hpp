#pragma once
#include "MSTFactory.hpp"

// Total weight of the MST
void calculateTotalWeight(const Tree& tree, int clientSocket);

// Longest distance between two vertices
void calculateLongestDistance(const Tree& tree, int clientSocket);

// Average distance between and two edges n the graph.
//  ○ assume distance (x,x)=0 for any X
//  ○ We are interested in avg of all distances Xi,Xj where i=1..n j≥i.
void calculateAverageDistance(const Tree& tree, int clientSocket);

// Shortest distance between two vertices Xi,Xj where i≠j and edge belongs to MST
void calculateShortestDistance(const Tree& tree, int clientSocket);