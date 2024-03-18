#ifdef SAT_CADICAL

#include "SATSolver.h"
#include <lib/cadical/src/cadical.hpp>


class CadicalSatSolver : public SATSolver {

private:
	CaDiCaL::Solver * solver;
	std::vector<int32_t> assumptions;
	int32_t decision_vars;

public:
	CadicalSatSolver(int32_t n_vars, int32_t n_args);
	~CadicalSatSolver() {};
	void add_clause(const std::vector<int32_t> & clause);
	void assume(int32_t lit);
	int solve();
	int solve(const std::vector<int32_t> assumptions);
	bool propagate(std::vector<int32_t> & out_lits);
	bool get_value(int32_t lit);
	std::vector<bool> model;

};
#endif