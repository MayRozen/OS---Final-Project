#include <memory>
#include "MSTFactory.hpp"
#include "KruskalMST.hpp"
#include "PrimMST.hpp"

std::unique_ptr<MSTStrategy> MSTFactory::createMSTStrategy(MSTFactory::Algorithm algo) {
    switch (algo) {
        case Algorithm::KRUSKAL:
            return std::make_unique<KruskalMST>();
        case Algorithm::PRIM:
            return std::make_unique<PrimMST>();
        default:
            return nullptr;
    }
}
