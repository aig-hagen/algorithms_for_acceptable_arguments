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

#ifndef CM_SATSOLVER_H
#define CM_SATSOLVER_H

#if defined(SAT_CMSAT)

#include "SATSolver.h"
#include <cryptominisat.h>


class CryptoMiniSatSolver : public SATSolver {

private:
	CMSat::SATSolver solver;
	std::vector<CMSat::Lit> assumptions;
	int32_t decision_vars;

public:
	CryptoMiniSatSolver(int32_t n_vars, int32_t n_args);
	~CryptoMiniSatSolver() {};
	void add_clause(const std::vector<int32_t> & clause);
	void assume(int32_t lit);
	int solve();
	int solve(const std::vector<int32_t> assumptions);
	bool propagate(std::vector<int32_t> & out_lits);
	bool get_value(int32_t lit);
	std::vector<bool> model;

};
#endif
#endif