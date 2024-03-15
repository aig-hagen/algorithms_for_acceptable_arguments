/*!
 * Copyright (c) <2023> <Andreas Niskanen, University of Helsinki>
 * 
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * 
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#if defined(SAT_CMSAT)

#include "CryptoMiniSatSolver.h"

using namespace std;
using namespace CMSat;


CryptoMiniSatSolver::CryptoMiniSatSolver(int32_t n_vars, int32_t n_args)
{
	solver.set_num_threads(1);
	solver.new_vars(n_vars);
	decision_vars = n_args;
	//solver.set_no_simplify();
	//solver.set_no_equivalent_lit_replacement();
	//solver.set_no_bva();
	//solver.set_no_bve();
	model = std::vector<bool>(n_vars+1);
}

void CryptoMiniSatSolver::add_clause(const vector<int32_t> & clause)
{
	vector<Lit> lits(clause.size());
	for (uint32_t i = 0; i < clause.size(); i++) {
		int32_t var = abs(clause[i])-1;
		while ((uint32_t)var >= solver.nVars())
			solver.new_var();
		lits[i] = Lit(var, clause[i] < 0);
	}
	solver.add_clause(lits);
}

void CryptoMiniSatSolver::assume(int32_t lit)
{
	int32_t var = abs(lit)-1;
	while ((uint32_t)var >= solver.nVars())
		solver.new_var();
	assumptions.push_back(Lit(var, lit < 0));
}

int CryptoMiniSatSolver::solve()
{
	int sat = solver.solve(&assumptions) == l_True ? 10 : 20;
	if (sat == 10) {
		model.clear();
		for (int32_t i = 1; i <= decision_vars; i++)
			model[i] = (solver.get_model()[i-1] == l_True ? 1 : 0);
	}
	assumptions.clear();
	return sat;
}

int CryptoMiniSatSolver::solve(const std::vector<int32_t> assumptions) {
	for (auto const& ass: assumptions) {
		assume(ass);
	}
	return solve();
}

bool CryptoMiniSatSolver::propagate(vector<int32_t> & out_lits)
{
	solver.set_no_bve();
	vector<Lit> zero_lits = solver.get_zero_assigned_lits();
	vector<Lit> implied_lits;
	bool sat = solver.implied_by(assumptions, implied_lits);
	assert(sat);
	for (const Lit & lit : zero_lits) {
		int32_t var = lit.var()+1;
		if (var > decision_vars) continue;
		out_lits.push_back(lit.sign() ? -var : var);
	}
	for (const Lit & lit : implied_lits) {
		int32_t var = lit.var()+1;
		if (var > decision_vars) continue;
		out_lits.push_back(lit.sign() ? -var : var);
	}
	return sat;
}

bool CryptoMiniSatSolver::get_value(int32_t lit)
{
	int32_t var = abs(lit)-1;
	lbool val = solver.get_model()[var];
	return (lit > 0) ? val == l_True : val == l_False;
}
#endif