#ifndef PRIMMST_HPP
#define PRIMMST_HPP

#include "MSTStrategy.hpp"

class PrimMST : public MSTStrategy {
public:
    Tree computeMST(const Graph& graph) override;
};

#endif // PRIMMST_HPP
