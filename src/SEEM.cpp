#if defined(SEEM)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> seem_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> unvisited(af.args, true);

        EvalMaxSatSolver solver = EvalMaxSatSolver(af.count, af.args);
        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        while(true) {
            int sat = solver.solve();
            if (sat == 20) break;

            for (uint32_t i = 0; i < af.args; i++) {
                if (unvisited[i]) {
                    if (solver.model[i]) {
                        unvisited[i] = false;
                        result.push_back(af.int_to_arg[i]);
                    } else {
                        solver.add_soft_clause({af.accepted_var[i]});
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