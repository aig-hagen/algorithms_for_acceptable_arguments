#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include <map>
#include <string>

namespace cgss2{

class Options {
public:
  struct Option {
    enum TYPE{INT, DOUBLE, STRING, UINT64, BOOL, LIST};
    static std::string type_name(TYPE t) {
      if (t==INT)    return "int";
      if (t==DOUBLE) return "double";
      if (t==STRING) return "string";
      if (t==UINT64) return "uint64";
      if (t==BOOL)   return "bool";
      if (t==LIST)   return "list";
      return "undef";
    }
    std::string type_name() {
      return type_name(type);
    }

    TYPE type;
    std::string name; // name in map
    std::string cmd_long; // cmd parameter --cmd_long
    char cmd_short; // cmd parameter -c
    std::string description; //
    std::string defaultv;
    int level; // if shown in help: level 0 always visible, level 1 visible on extended help,

    Option(const Option& o) :
      type(o.type),name(o.name),cmd_long(o.cmd_long),
      cmd_short(o.cmd_short),description(o.description),
      defaultv(o.defaultv), level(o.level)
      {}

    Option(TYPE _type, std::string _name, std::string _cmd_long, char _cmd_short, std::string _description, std::string _defaultv, int _level) :
      type(_type),name(_name),cmd_long(_cmd_long),
      cmd_short(_cmd_short),description(_description),
      defaultv(_defaultv), level(_level)
      {}

   Option& operator=(const Option& o) {
      type=o.type;
      name=o.name;
      cmd_long=o.cmd_long;
      cmd_short=o.cmd_short;
      description=o.description;
      defaultv=o.defaultv;
      level=o.level;
      return *this;
    }
  };
  std::map<std::string, int> ints;
  std::map<std::string, double> doubles;
  std::map<std::string, std::string> strs;
  std::map<std::string, uint64_t> uint64s;
  std::map<std::string, bool> bools;
  std::map<std::string, std::vector<std::string> > lists;


  bool set_value(std::string name, Option::TYPE type, std::string value); // returns if value existed before

private:
  std::vector<Option> options;
  std::map<std::string, int> cmd_longs; // maps cmd string to index in options vector
  std::map<char, int> cmd_shorts; // maps cmd char to index in options vector

public:
  void prepare(std::ostream& err); // builds cmd maps and sets default values to ints, doubles, strs, etc...

  std::vector<std::string> read_options(std::vector<std::string>& paramsndl, std::ostream& err); // returns unused parameters in vector

  void print_help(std::ostream& o, int level = 1, bool compact = 1);

  void add (Option::TYPE type, std::string name, std::string cmd_long, char cmd_short, std::string description, std::string defaultv, int level) {
    options.push_back({type, name, cmd_long, cmd_short, description, defaultv, level});
  }

};

}

#endif
