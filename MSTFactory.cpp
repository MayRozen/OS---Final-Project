#include <memory>
#include "MSTFactory.hpp"
#include "KruskalMST.hpp"
#include "PrimMST.hpp"
#include "BoruvkaMST.hpp"
#include "TarjanMST.hpp"

std::unique_ptr<MSTStrategy> MSTFactory::createMSTStrategy(MSTFactory::Algorithm algo) {
    switch (algo) {
        case Algorithm::KRUSKAL:
            return std::make_unique<KruskalMST>();
        case Algorithm::PRIM:
            return std::make_unique<PrimMST>();
        case Algorithm::Boruvka:
            return std::make_unique<BoruvkaMST>();
        case Algorithm::Tarjan:
            return std::make_unique<TarjanMST>();
        default:
            return nullptr;
    }
}
