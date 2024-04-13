#if defined(SEEM)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> seem_cred(const AF & af, semantics sem) { // TODO some bug: ex/B-2  arguments a26 and a6 not found as EC-ST and EC-PR
        std::vector<std::string> result;
        std::vector<bool> unvisited(af.args, true);
        uint64_t max_weight;
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

        while(true) {            
            max_weight = 0;
            for (uint32_t i = 0; i < af.args; i++) {
                if (unvisited[i]) {
                    solver.add_soft_constraint(-af.accepted_var[i]);
                    max_weight++;
                } else {
                    solver.disable_soft_constraint(-af.accepted_var[i]);
                }
            }

            if (solver.solve() == UNSAT_V || solver.get_optimal_cost() >= max_weight) break;

            for (uint32_t i = 0; i < af.args; i++) {
                if (unvisited[i]) {
                    if (solver.model[i]) {
                        unvisited[i] = false;
                        result.push_back(af.int_to_arg[i]);
                    }
                }
            }
        }
        return result;
    }

    std::vector<std::string> seem_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> included(af.args, true);
        uint64_t max_weight;

        if (sem != ST) {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        SAT_Solver solver = SAT_Solver(af.count, af.args);
        #ifdef SAT_EXTERNAL
        solver.set_solver(af.solver_path);
        #endif
        Encodings::stable(af, solver);

        while (true) {
            max_weight = 0;
            for (uint32_t i = 0; i < af.args; i++) {
                if (included[i]) {
                    solver.add_soft_constraint(-af.rejected_var[i]);
                    max_weight++;
                } else {
                    solver.disable_soft_constraint(-af.rejected_var[i]);
                }
            }

            if (solver.solve() == UNSAT_V || solver.get_optimal_cost() >= max_weight) break;

            for (uint32_t i = 0; i < af.args; i++) {
                included[i] = included[i] && solver.model[i];
            }
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