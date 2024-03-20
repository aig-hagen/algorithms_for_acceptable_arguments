#if defined(SEEM)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> seem_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> unvisited(af.args, true);

        SAT_Solver solver = SAT_Solver(af.count, af.args);
        std::vector<bool> model_old(af.count);
        bool changed = false;
        while(true) {
            for (uint32_t i = 0; i < af.args; i++) {
                if (unvisited[i]) {
                    std::vector<int32_t> clause = { af.accepted_var[i] };
                        solver.add_soft_clause(clause);
                }
            }
            if (sem == CO || sem == PR) {
                Encodings::complete(af, solver);
            } else if (sem == ST) {
                Encodings::stable(af, solver);
            } else {
                std::cerr << sem << ": Unsupported semantics\n";
                exit(1);
            }
            int sat = solver.solve();
            if (sat == 20) break;

            changed = false;
            for (uint32_t i = 0; i < af.args; i++) {
                if (model_old[i] != solver.model[i]) {
                    changed = true;
                }
                model_old[i] = solver.model[i];
                if (unvisited[i]) {
                    if (solver.model[i]) {
                        unvisited[i] = false;
                        result.push_back(af.int_to_arg[i]);
                    } else {
                        std::vector<int32_t> clause = { af.accepted_var[i] };
                        solver.add_soft_clause(clause);
                    }
                }
            }
            if (!changed) break;
        }
        return result;
    }

    std::vector<std::string> seem_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        return result;
    }
}

#endif