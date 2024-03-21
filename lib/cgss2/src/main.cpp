#include "interface.h"
#include "options.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace cgss2;

void usage(string cmd) {
  cout << CGSS2::version() << endl;
#ifdef ZLIB
  cout << "usage: " << cmd << " [params] instance.wcnf[.gz|.xz]" << endl;
#else
  cout << "usage: " << cmd << " [params] instance.wcnf" << endl;
#endif
}

int main(int argc, char* argv[]) {
  vector<string> args;
  for (int i=1;i<argc;++i) {
    args.push_back(argv[i]);
  }
  Options ops;
  ops.add(Options::Option::INT, "verbose", "verbose", 'v', "verbose level (0=silent, 1=something...)", "0", 1);
  ops.add(Options::Option::BOOL, "version", "version", 0, "print version info", "0", 1);
  ops.add(Options::Option::STRING, "out", "out", 'o', "write output to a file", "", 1);
  ops.add(Options::Option::BOOL, "help", "help", 'h', "print help", "0", 0);
  ops.add(Options::Option::BOOL, "help-sparse", "help-sparse", 0, "print help in less compact form", "0", 1);
  ops.add(Options::Option::BOOL, "help-all", "help-all", 'H', "print extended help", "0", 1);
  ops.add(Options::Option::BOOL, "help-all-sparse", "help-all-sparse", 0, "print extended help in less comact form", "0", 1);
  ops.add(Options::Option::BOOL, "print-stats", "print-stats", 'p', "print statistics after solving", "0", 1);
  ops.add(Options::Option::BOOL, "no-model", "no-model", 'n', "do not print model after solving", "0", 1);
  CGSS2Interface::add_options(ops);
  CGSS2::add_options(ops);

  ops.prepare(cerr);


  vector<string> extra = ops.read_options(args, cerr);
  if (ops.bools["version"]) {
    cout << CGSS2::version(3855, 0) << endl;
    return 0;
  }
  if (ops.bools["help"]) {
    usage(argv[0]);
    ops.print_help(cout);
    return 0;
  }
  if (ops.bools["help-sparse"]) {
    usage(argv[0]);
    ops.print_help(cout, 1, 0);
    return 0;
  }
  if (ops.bools["help-all"]) {
    usage(argv[0]);
    ops.print_help(cout, 2, 1);
    return 0;
  }
  if (ops.bools["help-all-sparse"]) {
    usage(argv[0]);
    ops.print_help(cout, 2, 0);
    return 0;
  }
  vector<string> instances;
  for (string& s : extra) {
    if (s[0]=='-') {
      cout << "unrecognized parameter " << s << endl;
      cout << "use -h / --help" << endl;
      return 1;
    } else {
      instances.push_back(s);
    }
  }
  if (!instances.size()) {
    usage(argv[0]);
    cout << "use -h / --help for more information" << endl;
  } else if (instances.size()>1) {
    cout << "sorry, currently no support for multiple instances" << endl;
  } else {
    int verbose_level = ops.ints["verbose"];
    bool print_stats = ops.bools["print-stats"];
    bool print_model = !ops.bools["no-model"];


    CGSS2Interface interface(ops, verbose_level, cout);
    if (interface.load_from_file(instances[0])) return 1;
    if (ops.strs.count("out") && ops.strs["out"].size()) {
      std::ofstream f(ops.strs["out"]);
      if (f.fail()) {
        cerr << "failed to open file \"" << ops.strs["out"] << "\" for writing.";
        return 1;
      }
      interface.solve(print_stats, print_model, f);
    } else {
      interface.solve(print_stats, print_model, cout);
    }

  }
}
