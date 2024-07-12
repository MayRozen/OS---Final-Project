#ifndef KRUSKALMST_HPP
#define KRUSKALMST_HPP

#include "MSTStrategy.hpp"

class KruskalMST : public MSTStrategy {
public:
    Tree computeMST(const Graph& graph) override;
};

#endif // KRUSKALMST_HPP
