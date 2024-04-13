#if defined(SEE)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> see_cred(const AF & af, semantics sem) {
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

        std::vector<bool> unvisited(af.args, true);
        std::vector<int32_t> unvisited_clause;
        unvisited_clause.reserve(af.args);
        while (true) {
            if (solver.solve() == UNSAT_V) break;

            unvisited_clause.clear();
            for (uint32_t i = 0; i < af.args; i++) {
                if (unvisited[i]) {
                    if (solver.model[i]) {
                        unvisited[i] = false;
                        result.push_back(af.int_to_arg[i]);
                    } else {
                        unvisited_clause.push_back(af.accepted_var[i]);
                    }
                }
            }
            solver.add_clause(unvisited_clause);
        }
        return result;
    }

    std::vector<std::string> see_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> included(af.args, true);

        if (sem != ST) {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        SAT_Solver solver = SAT_Solver(af.count, af.args);
        #ifdef SAT_EXTERNAL
        solver.set_solver(af.solver_path);
        #endif
        Encodings::stable(af, solver);
        
        std::vector<int32_t> complement_clause;
        while (true) {
            if (solver.solve() == UNSAT_V) break;

            complement_clause.clear();
            for (uint32_t i = 0; i < af.args; i++) {
                included[i] = included[i] && solver.model[i];

                if (included[i]) {
                    complement_clause.push_back(af.rejected_var[i]);
                }
            }
            solver.add_clause(complement_clause);
        }

        for (uint32_t i = 0; i < af.args; i++) {
            if (included[i]) {
                result.push_back(af.int_to_arg[i]);
            }
        }
        return result;
    }
}
#endif