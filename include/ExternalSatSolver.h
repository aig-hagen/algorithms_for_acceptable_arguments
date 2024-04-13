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
    void add_clause_1(int32_t lit);
	void add_clause_2(int32_t lit1, int32_t lit2);
    void add_clause_3(int32_t lit1, int32_t lit2, int32_t lit3);
    void assume(int32_t lit);
    int solve();
    int solve(const std::vector<int32_t> assumptions);
    std::vector<bool> model;
    void set_solver(std::string path);
};
#endif
#endif