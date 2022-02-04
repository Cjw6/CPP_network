#pragma once

#include <cstring>
#include <string>
#include <vector>

#include "StringPiece.h"

void StringSplitByChar(const std::string &s, std::vector<std::string> &tokens,
                       char delim = ' ');

void StringSplitByStr(const std::string &s, std::vector<std::string> &tokens,
                      const std::string &delimiters = " ");

void StringSplitUseStrtok(char *str, std::vector<StringPiece> &strpieces,
                          const char *spl);
