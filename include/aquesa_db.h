#ifndef AQUA_DB_09_AQUESA_DB_H
#define AQUA_DB_09_AQUESA_DB_H

#include "ad_structures.h"
#include "ad_crypto_handler.h"

namespace aqua_db {

  typedef class Sequence {
  protected:
    rocksdb::DB* _db = nullptr;
    string _metadata_dbname, _metadata_dbpath, _this_path;
  public:
    Sequence(string const&, string const&, bool);
    void insertNewBlock(string const &,
      std::shared_ptr<class ad_structures::Block> &);
    std::shared_ptr<ad_structures::Block> getBlock(string const &);

    ~Sequence();

  } ad_seq;

  typedef class Database {
  protected:
    std::shared_ptr<ad_seq> _seq = nullptr;
    string _db_name, _db_path;

  public:
    Database(string const&);
    std::shared_ptr<ad_seq> sequence(string const&, bool);
  } ad_db;
}

#endif //AQUA_DB_09_AQUESA_DB_H
