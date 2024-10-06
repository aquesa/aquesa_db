#ifndef AQUESA_DB_09_AD_CRYPTO_HANDLER_H
#define AQUESA_DB_09_AD_CRYPTO_HANDLER_H

#include "includes.h"

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/buffer.h>

#define AES_KEY_LENGTH 256
#define AES_BLOCK_SIZE 16

namespace ad_crypto_handler {
  class CryptoHandler {
  public:
    static string generateSHA256(string const& input);
    static std::string base64_encode(const std::string& input);
    static std::string base64_decode(const std::string& encoded);
  };
}

#endif //AQUESA_DB_09_AD_CRYPTO_HANDLER_H
