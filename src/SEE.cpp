#if defined(SEE)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> see_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> unvisited(af.args, false);

        SAT_Solver solver = SAT_Solver(af.count, af.args);
        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        std::vector<int32_t> assumptions;
        assumptions.reserve(af.args);
        while (true) {
            // TODO must be clause, not assumptions
            for (uint32_t i = 0; i < af.args; i++) {
                if (unvisited[i]) {
                    solver.assume(af.accepted_var[i]);
                }
            }

            int ret = solver.solve();
            if (ret == 20) break;

            for (uint32_t i = 1; i <= af.args; i++) {
                if (solver.model[i]) {
                    if (unvisited[i-1]) {
                        unvisited[i-1] = false;
                        result.push_back(af.int_to_arg[i-1]);
                    }
                }
            }
        }
        


        return result;
    }

    std::vector<std::string> see_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        return result;
    }
}

#endif