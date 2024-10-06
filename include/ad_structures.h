#ifndef AQUA_DB_09_AD_STRUCTURES_H
#define AQUA_DB_09_AD_STRUCTURES_H

#include "includes.h"

namespace ad_structures {
  class Block {
  public:
    char prevHash[512], hash[512];
    int dataSize;
    std::shared_ptr<std::byte[]> data;

    std::shared_ptr<std::byte[]> to_bytes();

    static std::shared_ptr<class Block> from_bytes(const std::shared_ptr<std::byte[]> &);

  };
}

#endif //AQUA_DB_09_AD_STRUCTURES_H
