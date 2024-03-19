#ifdef SAT_CADICAL

#include <cadical/src/cadical.hpp>


class CadicalSatSolver {

private:
	CaDiCaL::Solver * solver;
	std::vector<int32_t> assumptions;
	int32_t decision_vars;

public:
	CadicalSatSolver(int32_t n_vars, int32_t n_args);
	~CadicalSatSolver() { delete solver; };
	void add_clause(const std::vector<int32_t> & clause);
	void assume(int32_t lit);
	int solve();
	int solve(const std::vector<int32_t> assumptions);
	std::vector<bool> model;
};
#endif