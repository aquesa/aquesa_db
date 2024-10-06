#include "aquesa_db.h"

// Sequence

aqua_db::Sequence::Sequence(string const &db_path,
    string const &seq_name, bool is_new) {
  try {
    _metadata_dbname = ad_crypto_handler::CryptoHandler::generateSHA256(seq_name + "_metadata");
    _metadata_dbpath = db_path + _metadata_dbname;

    if(!is_new) {
      struct stat metadata_db_stat{};
      if(stat(_metadata_dbpath.c_str(), &metadata_db_stat) == 0) {
        if(!(metadata_db_stat.st_mode & S_IFDIR))
          throw std::runtime_error(
            "Exception, metadb corrupted."
          );
      } else throw std::runtime_error(
          "Exception, metadb corrupted."
        );
    }

    rocksdb::Status status;

    rocksdb::Options options;
    options.create_if_missing = true;

    status = rocksdb::DB::Open(options, _metadata_dbpath,&_db);
    if(!status.ok())
      throw std::runtime_error(
        "Exception, unable to initialize/get sequence at this moment."
      );

    _this_path = db_path + ad_crypto_handler::CryptoHandler::generateSHA256(seq_name);

    struct stat seq_file_stat{};

    if(stat(_this_path.c_str(), &seq_file_stat) == 0) {
      if (is_new) {
        throw std::runtime_error(
          "Exception, sequence already exists (provided 'is_new' as true)"
        );
      }
    } else {
      if(!is_new) {
        throw std::runtime_error(
          "Exception, sequence dones not exists (provided 'is_new' as false)"
        );
      }
    }

    fstream seq_file;
    seq_file.open(_this_path, fstream::app | fstream::binary);
    if(!seq_file.is_open())
      throw std::runtime_error(
        "Exception, unable to initialize sequence at this moment."
      );
    seq_file.close();

    cout << "Sequence (" + seq_name + ") initialized successfully." << endl;

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;

    if(_db != nullptr)
      _db->Close();
    delete _db;

    exit(-1);
  }

}

void aqua_db::Sequence::insertNewBlock(const std::string &id,
  std::shared_ptr<class ad_structures::Block> &block) {

  try {

    if (!_db)
      throw std::runtime_error(
        "Exception, unable to insert the block at this moment."
      );

    rocksdb::Status status;

    string insert_error = "Exception, unable to insert the block at this moment.", value, prevHash;

    ofstream seq_file;
    seq_file.open(_this_path, ofstream::app | ofstream::binary);
    if(!seq_file.is_open())
      throw std::runtime_error(insert_error);

    struct stat seq_file_stat{};
    stat(_this_path.c_str(), &seq_file_stat);

    auto block_id_hash = ad_crypto_handler::CryptoHandler::generateSHA256(id);

    // Checking if block with the same key exists or not.
    status = _db->Get(rocksdb::ReadOptions(), block_id_hash, &value);
    if(status.ok()) {
      seq_file.close();
      throw std::runtime_error(
        "Exception, block (" + id +") already exists."
      );
    }

    // Getting previous hash.
    status = _db->Get(rocksdb::ReadOptions(), "prevHash", &prevHash);
    if(!status.ok())
      prevHash = ad_crypto_handler::CryptoHandler::base64_encode(string("<null>"));
    strcpy(block->prevHash, prevHash.c_str());

    // Inserting the block id into metadb.
    status = _db->Put(rocksdb::WriteOptions(), block_id_hash, std::to_string(seq_file_stat.st_size));
    if (!status.ok()) {
      seq_file.close();
      throw std::runtime_error(insert_error);
    }

    status = _db->Put(rocksdb::WriteOptions(), "prevHash", block->hash);
    if (!status.ok()) {
      seq_file.close();
      throw std::runtime_error(insert_error);
    }

    // Inserting the block into sequence data page.
    auto block_size = sizeof(block->prevHash) + sizeof(block->hash)
      + sizeof(block->dataSize) + block->dataSize;

    seq_file.write(reinterpret_cast<char*>(block->to_bytes().get()),
    static_cast<std::streamsize>(block_size));
    if(!seq_file) {
      _db->Delete(rocksdb::WriteOptions(), block_id_hash);
      seq_file.close();
      throw std::runtime_error(insert_error);
    }
    seq_file.close();

    cout << "Block (" + id + ") inserted successfully." << endl;

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    if(_db != nullptr)
      _db->Close();
    delete _db;
    exit(-1);
  }
}

std::shared_ptr<ad_structures::Block> aqua_db::Sequence::getBlock(const std::string &id) {
  try {

    string block_id_hash = ad_crypto_handler::CryptoHandler::generateSHA256(id),
      blockError = "Exception, unable to get the block at this moment.", startPoint;

    if(!_db)
      throw std::runtime_error(blockError);

    rocksdb::Status status;

    status = _db->Get(rocksdb::ReadOptions(), block_id_hash, &startPoint);
    if (status.ok()) {

      fstream seq_page;
      seq_page.open(_this_path, fstream::in | fstream::binary);
      if (!seq_page.is_open())
        throw std::runtime_error(blockError);

      seq_page.seekg(std::stoi(startPoint));

      struct stat seq_page_stat{};
      stat(_this_path.c_str(), &seq_page_stat);

      auto blockBytes = std::make_shared<std::byte[]>(seq_page_stat.st_size - std::stoi(startPoint));
      seq_page.read(reinterpret_cast<char*>(blockBytes.get()), seq_page_stat.st_size - std::stoi(startPoint));
      if(seq_page) {
        seq_page.close();
        return ad_structures::Block::from_bytes(blockBytes);
      } else {
        seq_page.close();
        throw std::runtime_error(blockError);
      }

    } else throw std::runtime_error("Block not found");

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    if(_db != nullptr)
      _db->Close();
    delete _db;

    exit(-1);
  }
}

aqua_db::Sequence::~Sequence() {
  if(_db != nullptr)
    _db->Close();
  delete _db;
}

// Database code

aqua_db::Database::Database(string const& name) {
  struct stat sb{}, db_stat{};

  string root_path = "./aqua_db/";

  if(stat(root_path.c_str(), &sb) != 0) {
    if (mkdir(root_path.c_str(), 0777) == -1) {
      cerr << "Exception, unable to initialize the database at this moment." << endl;
      exit(-1);
    }
  }

  string db_path = root_path + name + "/";
  if(stat(db_path.c_str(), &db_stat) == 0) {
    if(!(db_stat.st_mode & S_IFDIR)) {
      cerr << "Exception, database corrupted." << endl;
      exit(-1);
    } else {
      this->_db_name = name;
      this->_db_path = db_path;
    }
  } else {
    if(mkdir(db_path.c_str(), 0777) == -1) {
      cerr << "Exception, unable to initialize the database at this moment." << endl;
      exit(-1);
    } else {
      this->_db_name = name;
      this->_db_path = db_path;
      cout << "Database (" + name + ") initialization successful." << endl;
    }
  }
}

std::shared_ptr<aqua_db::ad_seq> aqua_db::Database::sequence(
    const std::string &name, bool is_new) {

  return std::make_shared<aqua_db::ad_seq>(
  this->_db_path, name, is_new
  );
}