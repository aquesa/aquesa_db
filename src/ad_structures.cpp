#include "ad_structures.h"

std::shared_ptr<std::byte[]> ad_structures::Block::to_bytes() {
  auto blockBytesSize = sizeof(prevHash) + sizeof(hash)
      + sizeof(dataSize) + dataSize;

  auto blockBytes = std::make_shared<std::byte[]>(blockBytesSize);

  auto loc0 = blockBytes.get(); // prevHash
  auto loc1 = loc0 + sizeof(prevHash); // hash
  auto loc2 = loc1 + sizeof(hash); // dataSize
  auto loc3 = loc2 + sizeof(dataSize); // data

  std::memcpy(loc0, prevHash, sizeof(prevHash));
  if(std::memcmp(loc0, prevHash, sizeof(prevHash)) != 0)
    goto returnErr;

  std::memcpy(loc1, hash, sizeof(hash));
  if(std::memcmp(loc1, hash, sizeof(hash)) != 0)
    goto returnErr;

  std::memcpy(loc2, &dataSize, sizeof(dataSize));
  if(std::memcmp(loc2, &dataSize, sizeof(dataSize)) != 0)
    goto returnErr;

  std::memcpy(loc3, data.get(), dataSize);
  if(std::memcmp(loc3, data.get(), dataSize) != 0)
    goto returnErr;

  return blockBytes;

  returnErr: {
    cerr << "Unable to perform storing at this moment." << endl;
    exit(-1);
  }
}

std::shared_ptr<class ad_structures::Block> ad_structures::Block::from_bytes(const std::shared_ptr<std::byte[]>& bytes) {
  auto block = std::make_shared<class Block>();

  auto loc0 = bytes.get(); // prevHash
  auto loc1 = loc0 + sizeof(Block::prevHash); // hash
  auto loc2 = loc1 + sizeof(Block::hash); //dataSize
  auto loc3 = loc2 + sizeof(Block::dataSize); // data

  std::memcpy(block->prevHash, loc0, sizeof(Block::prevHash));
  if(std::memcmp(block->prevHash, loc0, sizeof(Block::prevHash)) != 0)
    goto returnErr;

  std::memcpy(block->hash, loc1, sizeof(Block::hash));
  if(std::memcmp(block->hash, loc1, sizeof(Block::hash)) != 0)
    goto returnErr;

  std::memcpy(&block->dataSize, loc2, sizeof(Block::dataSize));
  if(std::memcmp(&block->dataSize, loc2, sizeof(Block::dataSize)) != 0)
    goto returnErr;

  block->data = std::make_shared<std::byte[]>(block->dataSize);
  std::memcpy(block->data.get(), loc3, block->dataSize);
  if(std::memcmp(block->data.get(), loc3, block->dataSize) != 0)
    goto returnErr;

  return block;

  returnErr: {
    cerr << "internal error occurred" << endl;
    exit(-1);
  }
}