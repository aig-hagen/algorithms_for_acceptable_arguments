#include "options.h"

#include <sstream>

using namespace std;
using namespace cgss2;


bool Options::set_value(string name, Option::TYPE type, string value) {
  bool rv = 0;
  if (type==Option::INT) {
    rv = ints.count(name);
    ints[name]=stoi(value);
  } else if (type==Option::DOUBLE) {
    rv = doubles.count(name);
    doubles[name]=stod(value);
  } else if (type==Option::STRING) {
    rv = strs.count(name);
    strs[name]=value;
  } else if (type==Option::UINT64) {
    rv = uint64s.count(name);
    uint64s[name]=stoull(value);
  } else if (type==Option::BOOL) {
    rv = bools.count(name);
    bools[name]=stoi(value);
  } else if (type==Option::LIST) {
    lists[name].push_back(value);
    rv = 0;
  }
  return rv;
}

void Options::prepare(ostream& err) {
  for (uint i=0; i<options.size(); ++i) {
    Option& o = options[i];
    if (o.cmd_long.size()) {
      if (cmd_longs.count(o.cmd_long))   err << "Warning: option --" << o.cmd_long << " defined more than once\n";
      cmd_longs[o.cmd_long] = i;
    }
    if (o.cmd_short) {
      if (cmd_shorts.count(o.cmd_short)) err << "Warning: option -" << o.cmd_short << " defined more than once\n";
      cmd_shorts[o.cmd_short] = i;
    }
    if (o.type != Option::LIST && set_value(o.name, o.type, o.defaultv)) err << "Warning: option named \"" << o.name << "\" of type " << o.type_name() << " defined more than once\n";
  }
}

vector<string> Options::read_options(vector<string>& params, ostream& err) {
  vector<string> rv;
  int cmdindex=-1;
  for (string& s : params) {
    if (cmdindex!=-1) {
      set_value(options[cmdindex].name, options[cmdindex].type, s);
      cmdindex = -1;
      continue;
    }
    if (s.size()<2 || s[0] != '-') {
      rv.push_back(s);
      continue;
    }
    if (s[1] == '-') {
      string k=s.substr(2);
      if (!cmd_longs.count(k)) {
        rv.push_back(s);
        continue;
      }
      int j = cmd_longs[k];
      if (options[j].type == Option::BOOL) {
        set_value(options[j].name, options[j].type, "1");
      } else {
        cmdindex = j;
      }
    } else {
      for (uint i=1; i<s.size(); ++i) {
        if (!cmd_shorts.count(s[i])) {
          rv.push_back("-"+string(1,s[i]));
          continue;
        }
        int j = cmd_shorts[s[i]];
        if (options[j].type == Option::BOOL) {
          set_value(options[j].name, options[j].type, "1");
        } else {
          cmdindex = j;
        }
      }
    }
  }
  if (cmdindex != -1) {
    err << "Warning: value expected for parameter " << params.back() << "\n";
  }
  return rv;
}

void Options::print_help(ostream& o, int level, bool compact) {
  int maxl = 0;
  if (compact) {
    for (Option& op : options) {
      if (op.level>level) continue;
      int l = 0;
      if (op.cmd_short) l += 2;
      if (op.cmd_long.size()) {
        if (op.cmd_short) l += 2;
        l += 2 + op.cmd_long.size();
      }
      if (op.type != Option::BOOL) l+=10;
      maxl = max(maxl, l);
    }
  }

  for (Option& op : options) {
    if (op.level>level) continue;
    o << (compact?"\t":"\n");
    int l=0;
    if (op.cmd_short) o << "-" << op.cmd_short, l+=2;
    if (op.cmd_long.size()) {
      if (op.cmd_short) o << ", ", l+=2;
      o << "--" << op.cmd_long;
      l += 2 + op.cmd_long.size();
    }

    if (op.type == Option::INT)         o << " <int>    ", l+=10;
    else if (op.type == Option::DOUBLE) o << " <double> ", l+=10;
    else if (op.type == Option::STRING) o << " <string> ", l+=10;
    else if (op.type == Option::UINT64) o << " <uint64> ", l+=10;
    else if (op.type == Option::LIST)   o << " <value>  ", l+=10;
    while (l<maxl) o << " ", ++l;

    o << (compact?"  ":"\n\t") << op.description;
    if (op.type != Option::BOOL) o << (compact?"; ":"\n\t") << "default value: " << op.defaultv;

    o << "\n";
  }
}
