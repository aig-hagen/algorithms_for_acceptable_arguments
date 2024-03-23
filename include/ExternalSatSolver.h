#ifndef EXTERNALSAT_H
#define EXTERNALSAT_H

#ifdef SAT_EXTERNAL

#include "SATSolver.h"
#include <string>

const int SAT_V = 10;
const int UNSAT_V = 20;


class ExternalSatSolver : public SATSolver {
private:
    std::string solver;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assumptions;
    int32_t number_of_vars;

public:
    ExternalSatSolver(int32_t n_vars, int32_t n_args);
    ~ExternalSatSolver() {};
    void add_clause(const std::vector<int32_t> & clause);
    void assume(int32_t lit);
    int solve();
    int solve(const std::vector<int32_t> assumptions);
    std::vector<bool> model;
    void set_solver(std::string path);
};
#endif
#endif