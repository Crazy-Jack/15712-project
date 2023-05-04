/**
 * @file lib.cpp
 * @author Abigale Kim
 * @brief Definitions of library functions.
 */

#include "lib.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

std::string sha256(const std::string& str) {
  std::vector<unsigned char> md_vec(SHA_DIGEST_LENGTH, 0);

  const unsigned char *s = (const unsigned char*)str.c_str();
  unsigned char* hash_value = SHA256(s, str.length(), md_vec.data());

  std::stringstream ss;

  for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md_vec[i]);
  }

  return ss.str();
}

uint8_t lsb_sha256(const std::string& str) {
  std::string sha256_str = sha256(str);
  std::string last_char = sha256_str.substr(sha256_str.length() - 1, 1);
  unsigned int x;   
  std::stringstream ss;
  ss << std::hex << last_char;
  ss >> x;
  return static_cast<uint8_t>(x) & 0x1;
}

bool str_starts_with(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}