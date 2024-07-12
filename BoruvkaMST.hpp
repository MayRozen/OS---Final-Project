#ifndef BORUVKAMST_HPP
#define BORUVKAMST_HPP

#include "MSTStrategy.hpp"

class BoruvkaMST : public MSTStrategy {
public:
    Tree computeMST(const Graph& graph) override;

private:
    int find(std::vector<int>& component, int v);
    void Union(std::vector<int>& component, std::vector<int>& cheapest, int u, int v, int comp_u, int comp_v);
};

#endif // BORUVKAMST_HPP
