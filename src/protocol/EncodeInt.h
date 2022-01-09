#pragma once

#include <cstdint>

void EncodeFixed32(char *dst, uint32_t value);
void EncodeFixed64(char *dst, uint64_t value);
uint32_t DecodeFixed32(const char *ptr);
uint64_t DecodeFixed64(const char *ptr);