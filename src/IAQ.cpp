#if defined(IAQ)

#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> iaq_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        uint32_t num_vars = af.count;
        SAT_Solver solver = SAT_Solver(num_vars, af.args);
        #ifdef SAT_EXTERNAL
        solver.set_solver(af.solver_path);
        #endif

        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }
        

        for (uint32_t i = 0; i < af.args; i++) {
            solver.assume(af.accepted_var[i]);
            if (solver.solve() == SAT_V) {
                result.push_back(af.int_to_arg[i]);
            }
        }
        return result;
    }

    std::vector<std::string> iaq_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        
        if (sem == PR) {
            for (uint32_t i = 0; i < af.args; i++) {
                if (is_skeptically_preferred(af, i)) {
                    result.push_back(af.int_to_arg[i]);
                }
            }
        } else if (sem == ST) {
            SAT_Solver solver = SAT_Solver(af.count, af.args);
            #ifdef SAT_EXTERNAL
            solver.set_solver(af.solver_path);
            #endif
            Encodings::stable(af, solver);
            for (uint32_t i = 0; i < af.args; i++) {
                solver.assume(-af.accepted_var[i]);
                if (solver.solve() == UNSAT_V) {
                    result.push_back(af.int_to_arg[i]);
                }
            }
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }
        return result;
    }

    bool is_skeptically_preferred(const AF & af, uint32_t arg) {
        SAT_Solver solver = SAT_Solver(af.count, af.args);
        #ifdef SAT_EXTERNAL
        solver.set_solver(af.solver_path);
        #endif
        Encodings::complete(af, solver);

        std::vector<int32_t> assumptions = { -af.accepted_var[arg] };

        while (true) {
            int sat = solver.solve(assumptions);
            if (sat == UNSAT_V) break;

            std::vector<int32_t> complement_clause;
            complement_clause.reserve(af.args);
            std::vector<uint8_t> visited(af.args);
            std::vector<int32_t> new_assumptions = assumptions;
            new_assumptions.reserve(af.args);

            while (true) {
                complement_clause.clear();
                for (int32_t i = 0; i < af.args; i++) {
                    if (solver.model[i]) {
                        if (!visited[i]) {
                            new_assumptions.push_back(af.accepted_var[i]);
                            visited[i] = 1;
                        }
                    } else {
                        complement_clause.push_back(af.accepted_var[i]);
                    }
                }
                solver.add_clause(complement_clause);
                int superset_exists = solver.solve(new_assumptions);
                if (superset_exists == UNSAT_V) break;
            }

            new_assumptions[0] = -new_assumptions[0];

            if (solver.solve(new_assumptions) == UNSAT_V) {
                return false;
            }
        }
        return true;
    }
}

#endif