#include "ArgParser.h"

ArgumentParser::ArgParser::ArgParser(const std::string &name) {
  this->name = name;
}

/* Parsing command line
 * This method get full string, which need to be parsed following rules:
 * Short flags look like -f
 * Two or more short flags can be splited app -a -b == app -ab
 * Long flags look like --flag
 * '=' means assignment to flag
 * All parameters have to have assignment with some value
 *
 *
 * 1. Reading command line
 * Flag or positional value?
 * Just flag or flag with assignment?
 * Logic with flags. Short flags can be splited.
 * If positional value, save it.
 *
 *
 * 2. Save positional values
 * Choose argument type
 * Save values
 *
 *
 * 3. Check all parameters have value or parse error
 * */

bool ArgumentParser::ArgParser::FlagToArg(const std::string &long_name) {
  auto iter = this->map_flag.find(long_name);

  if (iter != this->map_flag.end()) {
    auto *pos = iter->second;
    pos->AddValue(true);
  } else {
    return false;
  }

  return true;
}

bool ArgumentParser::ArgParser::StrIntToArg(const std::string &long_name, const std::string &value) {
  auto iter_str = this->map_str.find(long_name);
  if (iter_str != this->map_str.end()) {
    auto *pos = iter_str->second;
    pos->AddValue(value);
  } else {
    auto iter_int = this->map_int.find(long_name);
    if (iter_int != this->map_int.end()) {
      auto *pos = iter_int->second;
      pos->AddValue(std::stoi(value));
    } else {
      return false;
    }
  }

  return true;
}

template<typename T>
bool CheckArg(std::map<std::string, ArgumentParser::ArgType<T> *> map_type) {
  for (const auto &pair : map_type) {
    auto arg = pair.second;
    if (!arg->has_value) {
      return false;
    }
    if (arg->min_size != 0) {
      if (arg->vec_store->size() < arg->min_size) {
        return false;
      }
    }
  }

  return true;
}

bool ArgumentParser::ArgParser::Parse(const std::vector<std::string> &args) {
  std::vector<std::string> positional;

  for (int i = 1; i < args.size(); i++) {
    std::string argument(args[i]);

    if (argument.length() >= 1 && argument[0] == '-') {
      size_t eq_pos = argument.find('=');

      if (eq_pos == std::string::npos) {  // no '=', flag
        if (argument.length() >= 2 && argument[1] != '-') {  // short flags
          for (char short_flag_arg : argument.substr(1)) {
            auto p = this->short_name_map.find(short_flag_arg);
            if (p != this->short_name_map.end()) {
              if (!FlagToArg(p->second)) {
                return false;
              }
            } else {
              return false;
            }
          }
        } else {  // long flag
          if (!FlagToArg(argument.substr(2))) {
            return false;
          }
        }

      } else { // argument
        if (argument.length() >= 2 && argument[1] != '-') {  // short argument
          auto iter = this->short_name_map.find(argument[1]);
          if (iter != this->short_name_map.end()) {
            if (!StrIntToArg(iter->second, argument.substr(eq_pos + 1))) {
              return false;
            }
          } else {
            return false;
          }
        } else {  // long argument
          if (!StrIntToArg(argument.substr(2, eq_pos - 2), argument.substr(eq_pos + 1))) {
            return false;
          }
        }
      }

    } else { // positional
      positional.push_back(argument);
    }
  }

  // look at the last test
  if (help_check) {
    return true;
  }

  // positional
  for (auto iter : map_str) {
    if (iter.second->positional) {
      for (const auto &x : positional) {
        iter.second->AddValue(x);
      }
    }
  }

  for (auto iter : map_int) {
    if (iter.second->positional) {
      for (const auto &x : positional) {
        iter.second->AddValue(std::stoi(x));
      }
    }
  }

  // check not fail
  if (!CheckArg(map_flag)) {
    return false;
  }

  if (!CheckArg(map_str)) {
    return false;
  }

  if (!CheckArg(map_int)) {
    return false;
  }

  return true;
}

// used in main
bool ArgumentParser::ArgParser::Parse(int argc, char **argv) {
  std::vector<std::string> args(argv, argv + argc);
  return Parse(args);
}

// helping methods

bool ArgumentParser::ArgParser::Help() const {
  return help_check;
}

std::string ArgumentParser::ArgParser::HelpDescription() {
  return "Stupid leather bags...";
}

void ArgumentParser::ArgParser::AddHelp(const char short_param,
                                        const std::string &long_param,
                                        const std::string &description) {
  this->AddFlag(short_param, long_param, description).StoreValue(help_check);
}

void ArgumentParser::ArgParser::AddHelp(const std::string &long_param, const std::string &description) {
  this->AddFlag(long_param, description).StoreValue(help_check);
}

// ArgumentParser methods

// string

ArgumentParser::ArgType<std::string> &ArgumentParser::ArgParser::AddStringArgument(char short_param,
                                                                                   const std::string &long_param,
                                                                                   const std::string &description) {
  auto *arg = new ArgType<std::string>(short_param, long_param, description);
  this->map_str.insert(std::make_pair(long_param, arg));

  // if short_name == 0, then there is no short_name
  if (short_param != 0) {
    this->short_name_map.insert(std::make_pair(short_param, long_param));
  }

  return *arg;
}

ArgumentParser::ArgType<std::string> &ArgumentParser::ArgParser::AddStringArgument(const std::string &long_param,
                                                                                   const std::string &description) {
  return AddStringArgument(0, long_param, description);
}

std::string ArgumentParser::ArgParser::GetStringValue(const std::string &long_param) {
  auto iter = map_str.find(long_param);

  if (iter != this->map_str.end()) {
    ArgType<std::string> arg = *iter->second;
    return arg.GetValue();
  } else {
    return "Ohh...";
  }
}

std::string ArgumentParser::ArgParser::GetStringValue(const std::string &long_param, int index) {
  auto pos = map_str.find(long_param);
  ArgType<std::string> arg = *pos->second;
  return arg.vec_store->at(index);
}

// int

ArgumentParser::ArgType<int> &ArgumentParser::ArgParser::AddIntArgument(char short_param,
                                                                        const std::string &long_param,
                                                                        const std::string &description) {
  auto *arg = new ArgType<int>(short_param, long_param, description);
  this->map_int.insert(std::make_pair(long_param, arg));

  // if short_name == 0, then there is no short_name
  if (short_param != 0) {
    this->short_name_map.insert(std::make_pair(short_param, long_param));
  }

  return *arg;
}

ArgumentParser::ArgType<int> &ArgumentParser::ArgParser::AddIntArgument(const std::string &long_param,
                                                                        const std::string &description) {
  return AddIntArgument(0, long_param, description);
}

int ArgumentParser::ArgParser::GetIntValue(const std::string &long_param) {
  auto iter = map_int.find(long_param);

  if (iter != this->map_int.end()) {
    ArgType<int> arg = *iter->second;
    return arg.GetValue();
  } else {
    return 0;
  }
}

int ArgumentParser::ArgParser::GetIntValue(const std::string &long_param, int index) {
  auto pos = map_int.find(long_param);
  ArgType<int> arg = *pos->second;
  return arg.vec_store->at(index);
}

// flag

ArgumentParser::ArgType<bool> &ArgumentParser::ArgParser::AddFlag(char short_param,
                                                                  const std::string &long_param,
                                                                  const std::string &description) {
  auto *arg = new ArgType<bool>(short_param, long_param, description);
  this->map_flag.insert(std::make_pair(long_param, arg));

  // if short_name == 0, then there is no short_name
  if (short_param != 0) {
    this->short_name_map.insert(std::make_pair(short_param, long_param));
  }

  return *arg;
}

ArgumentParser::ArgType<bool> &ArgumentParser::ArgParser::AddFlag(const std::string &long_param,
                                                                  const std::string &description) {
  return AddFlag(0, long_param, description);
}

bool ArgumentParser::ArgParser::GetFlag(const std::string &long_param) {
  auto iter = map_flag.find(long_param);

  if (iter != this->map_flag.end()) {
    ArgType<bool> arg = *iter->second;
    return arg.GetValue();
  } else {
    return false;
  }
}

bool ArgumentParser::ArgParser::GetFlag(const std::string &long_param, int index) {
  auto pos = map_flag.find(long_param);
  ArgType<bool> arg = *pos->second;
  return arg.vec_store->at(index);
}

// Argument methods

template<typename T>
ArgumentParser::ArgType<T> &ArgumentParser::ArgType<T>::MultiValue(int count) {
  this->min_size = count;
  return *this;
}

template<typename T>
ArgumentParser::ArgType<T> &ArgumentParser::ArgType<T>::StoreValue(T &value_ptr) {
  this->store = &value_ptr;
  return *this;
}

template<typename T>
ArgumentParser::ArgType<T> &ArgumentParser::ArgType<T>::StoreValues(std::vector<T> &values_ptr) {
  this->vec_store = &values_ptr;
  return *this;
}

template<typename T>
const T &ArgumentParser::ArgType<T>::GetValue() const {
  return value;
}

template<typename T>
ArgumentParser::ArgType<T> &ArgumentParser::ArgType<T>::AddValue(const T &val) {
  has_value = true;
  value = val;

  if (store != nullptr) {
    *store = val;
  }

  if (vec_store != nullptr) {
    vec_store->push_back(val);
  }

  return *this;
}

template<typename T>
ArgumentParser::ArgType<T> &ArgumentParser::ArgType<T>::Default(const T &default_val) {
  return AddValue(default_val);
}

template<typename T>
ArgumentParser::ArgType<T> &ArgumentParser::ArgType<T>::Positional() {
  positional = true;

  return *this;
}

template struct ArgumentParser::ArgType<std::string>;
template struct ArgumentParser::ArgType<int>;
template struct ArgumentParser::ArgType<bool>;

