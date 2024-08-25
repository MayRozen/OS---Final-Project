#ifndef MSTFACTORY_HPP
#define MSTFACTORY_HPP

#include "MSTStrategy.hpp"
#include <memory>

class MSTFactory {
public:
    enum class Algorithm {
        KRUSKAL,
        PRIM,
        Boruvka,
        Tarjan
    };

    static std::unique_ptr<MSTStrategy> createMSTStrategy(Algorithm algo);
};

#endif // MSTFACTORY_HPP
