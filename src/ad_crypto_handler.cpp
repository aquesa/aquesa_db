#include "ad_crypto_handler.h"

string ad_crypto_handler::CryptoHandler::generateSHA256(
  const std::string &input) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  const auto* data = (const unsigned char*) input.c_str();
  SHA256(data, input.size(), hash);
  stringstream ss;
  for (unsigned char i : hash)
    ss << hex << setw(2) << setfill('0') << (int)i;
  return ss.str();
}

std::string ad_crypto_handler::CryptoHandler::base64_encode(
  const std::string &input) {
  BIO* bio, * b64;
  BUF_MEM* bufferPtr;
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new(BIO_s_mem());
  bio = BIO_push(b64, bio);

  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // Ignore newlines
  BIO_write(bio, input.c_str(), static_cast<int>(input.length()));
  BIO_flush(bio);
  BIO_get_mem_ptr(bio, &bufferPtr);

  std::string encodedData(bufferPtr->data, bufferPtr->length);
  BIO_free_all(bio);

  return encodedData;
}

std::string ad_crypto_handler::CryptoHandler::base64_decode(
  const std::string &encoded) {
  BIO* bio, * b64;
  char* buffer = (char*)malloc(encoded.size());
  memset(buffer, 0, encoded.size());

  bio = BIO_new_mem_buf(encoded.c_str(), -1);
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_push(b64, bio);

  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Ignore newlines
  int decodedLength = BIO_read(bio, buffer, static_cast<int>(encoded.size()));
  std::string decodedData(buffer, decodedLength);

  free(buffer);
  BIO_free_all(bio);

  return decodedData;
}