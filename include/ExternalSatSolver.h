#ifdef SAT_EXTERNAL

#include <string>
#include <vector>

/*
Class for all kinds of pre-compiled SAT solvers, e.g. cadical, cryptominisat5
SAT calls are answered by opening a pipe to an instance of the external solver with pstream
TODO some bug in pstream
*/
class ExternalSatSolver {
private:
    std::string solver;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assumptions;
    int32_t number_of_vars;

public:
    std::vector<bool> model;

    ExternalSatSolver(int32_t n_vars, int32_t n_args);
    ~ExternalSatSolver() {};
    void add_clause(const std::vector<int32_t> & clause);
    void assume(int32_t lit);
    int solve();
    int solve(const std::vector<int32_t> assumptions);
    
};
#endif