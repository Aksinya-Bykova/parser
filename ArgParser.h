#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ArgumentParser {
template<typename T>
class ArgType {
  char short_name{};
  std::string long_name;
  std::string description;

  T value;
  std::vector<T> values;

 public:
  ArgType(char short_name, std::string long_name, std::string description) :
      short_name(short_name), long_name(std::move(long_name)), description(std::move(description)) {};

  int min_size = 0;
  T* store = nullptr;
  std::vector<T>* vec_store = nullptr;

  ArgType<T> &MultiValue(int count = 0);
  ArgType<T> &StoreValue(T &value);
  ArgType<T> &StoreValues(std::vector<T> &values);
  ArgType<T> &Default(const T &default_val);
  ArgType<T>& AddValue(const T& val);

  bool positional = false;
  ArgType<T> &Positional();

  [[nodiscard]] const T &GetValue() const;
  bool has_value = false;
};

class ArgParser {
 private:
  bool FlagToArg(const std::string &long_name);
  bool StrIntToArg(const std::string &long_name, const std::string &value);
 public:
  std::string name;
  explicit ArgParser(const std::string &name);

  // Parsing
  std::map<char, std::string> short_name_map;
  bool Parse(int argc, char **argv);
  bool Parse(const std::vector<std::string> &args);

  // Help
  bool help_check = false; // uses like if AddHelp() called -- programm is OK
  void AddHelp(char short_name, const std::string &long_name, const std::string &description = "");
  void AddHelp(const std::string &long_name, const std::string &description = "");
  static std::string HelpDescription();
  [[nodiscard]] bool Help() const;

  // string
  std::map<std::string, ArgType<std::string> *> map_str;

  ArgType<std::string> &AddStringArgument(const std::string &long_name, const std::string &description = "");
  ArgType<std::string> &AddStringArgument(char short_name, const std::string &long_name,
                                          const std::string &description = "");
  std::string GetStringValue(const std::string &long_name);
  std::string GetStringValue(const std::string &long_name, int index);

  // int
  std::map<std::string, ArgType<int> *> map_int;

  ArgType<int> &AddIntArgument(const std::string &long_name, const std::string &description = "");
  ArgType<int> &AddIntArgument(char short_name, const std::string &long_name, const std::string &description = "");
  int GetIntValue(const std::string &long_name);
  int GetIntValue(const std::string &long_name, int index);

  // flag
  std::map<std::string, ArgType<bool> *> map_flag;

  ArgType<bool> &AddFlag(const std::string &long_name, const std::string &description = "");
  ArgType<bool> &AddFlag(char short_name, const std::string &long_name, const std::string &description = "");
  bool GetFlag(const std::string &long_name);
  bool GetFlag(const std::string &long_name, int index);
};
}