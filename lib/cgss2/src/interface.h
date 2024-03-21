#ifndef INTERFACE_H
#define INTERFACE_H

#include "cgss2.h"
#include "dimacs.h"
#include "options.h"

#include <vector>

namespace cgss2{

class CGSS2Interface {
    CGSS2 solver;
    DIMACS reader;
    std::string file_loaded;
    bool check_solution;
    bool compressed_model;
public:
    void add_clause(std::vector<int>& cl, uint64_t ws, bool dimacs = 1);
    void add_clauses(std::vector<std::vector<int> > & cl, std::vector<uint64_t>& ws, bool dimacs = 1);
    int solve(bool print_stats, bool print_model, std::ostream& out);

    int load_from_file(std::string filename);

    static void add_options(Options& ops);
    CGSS2Interface(Options& ops, int verbose_level, std::ostream& vout);
};

}

#endif
