#include "StringSplit.h"

void StringSplitByChar(const std::string &s, std::vector<std::string> &tokens,
                       char delim) {
  static auto string_find_first_not = [s, delim](size_t pos = 0) -> size_t {
    for (size_t i = pos; i < s.size(); i++) {
      if (s[i] != delim)
        return i;
    }
    return std::string::npos;
  };

  size_t lastPos = string_find_first_not(0);
  size_t pos = s.find(delim, lastPos);
  while (lastPos != std::string::npos) {
    tokens.emplace_back(s.substr(lastPos, pos - lastPos));
    lastPos = string_find_first_not(pos);
    pos = s.find(delim, lastPos);
  }
}

// c++11
void StringSplitByStr(const std::string &s, std::vector<std::string> &tokens,
                      const std::string &delimiters) {
  std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
  std::string::size_type pos = s.find_first_of(delimiters, lastPos);
  while (std::string::npos != pos || std::string::npos != lastPos) {
    tokens.push_back(s.substr(lastPos, pos - lastPos));
    lastPos = s.find_first_not_of(delimiters, pos);
    pos = s.find_first_of(delimiters, lastPos);
  }
}

void StringSplitUseStrtok(char *str, std::vector<StringPiece> &strpieces,
                          const char *spl) {
  const char *token = strtok(str, spl);
  while (token) {
    strpieces.emplace_back(token);
    token = strtok(NULL, spl);
  }
}






