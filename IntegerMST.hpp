#ifndef INTEGERMST_HPP
#define INTEGERMST_HPP

#include "MSTStrategy.hpp"

class IntegerMST : public MSTStrategy {
public:
    Tree computeMST(const Graph& graph) override;

private:
    size_t find(std::vector<size_t>& parent, size_t u);
    void Union(std::vector<size_t>& parent, std::vector<size_t>& rank, size_t u, size_t v);
};

#endif
