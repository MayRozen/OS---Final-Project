#ifndef MSTSTRATEGY_HPP
#define MSTSTRATEGY_HPP

#include <memory>
#include "Graph.hpp"
#include "Tree.hpp"

class MSTStrategy {
public:
    virtual Tree computeMST(const Graph& graph) = 0;
};

#endif // MSTSTRATEGY_HPP
