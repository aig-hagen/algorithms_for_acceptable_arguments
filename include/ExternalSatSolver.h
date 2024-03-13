#ifndef EXTERNAL_SAT_SOLVER_H
#define EXTERNAL_SAT_SOLVER_H

#include <vector>
#include <string>
#include <cstdint>

/*
Class for all kinds of pre-compiled SAT solvers, e.g. cadical, cryptominisat5
SAT calls are answered by opening a pipe to an instance of the external solver with pstream
TODO some bug in pstream
*/
class ExternalSatSolver {
public:
    std::vector<bool> model;
    std::vector<std::vector<int>> clauses;
    std::vector<std::vector<int>> minimization_clauses;
    std::vector<int> assumptions;
    uint32_t n_vars;
    int num_clauses;
    int num_minimization_clauses;
    bool last_clause_closed;
    std::string solver_path;

    ExternalSatSolver(uint32_t number_of_vars, std::string path_to_solver);
    void assume(int lit);
    void addClause(std::vector<int> & clause);
    void addMinimizationClause(std::vector<int> & clause);
    int solve();
    int solve(const std::vector<int> assumptions);
    void free();
    
};

#endif