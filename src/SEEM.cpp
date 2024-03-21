#if defined(SEEM)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> seem_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> unvisited(af.args, true);
        uint64_t max_weight;

        SAT_Solver solver = SAT_Solver(af.count, af.args);
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

            int sat = solver.solve();
            if (sat == UNSAT_V) break;

            if (solver.optimal_cost >= max_weight) break;

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
        return result;
    }
}

#endif