#ifndef INTEGERMST_HPP
#define INTEGERMST_HPP

#include "MSTStrategy.hpp"
#include "Graph.hpp"
#include "Tree.hpp"
#include <vector>

class IntegerMST : public MSTStrategy {
public:
    IntegerMST() = default;

    Tree computeMST(const Graph& graph) override;

private:
    size_t find(std::vector<int>& parent, size_t u);
    void Union(std::vector<int>& parent, std::vector<int>& rank, size_t u, size_t v);
};

#endif
