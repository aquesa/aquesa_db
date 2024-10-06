#include "includes.h"
#include "ad_crypto_handler.h"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

int main() {

  std::string data = "Hello";

  // Encode the string
  std::string encodedData = ad_crypto_handler::CryptoHandler::base64_encode(data);
  std::cout << "Encoded Data: " << encodedData << std::endl;

  // Decode the string
  std::string decodedData = ad_crypto_handler::CryptoHandler::base64_decode(encodedData);
  std::cout << "Decoded Data: " << decodedData << std::endl;

  return 0;
}