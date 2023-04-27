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