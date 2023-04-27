/**
 * @file lib.h
 * @author Abigale Kim (abigalek)
 * @brief Brief library of functions.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <cstring>
#include <openssl/sha.h>
#include <string>

#ifndef __LIB_H__
#define __LIB_H__

std::string sha256(const std::string& str);

#endif // __LIB_H__