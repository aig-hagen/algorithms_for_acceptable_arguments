#if defined(SEE)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> see_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;

        SAT_Solver solver = SAT_Solver(af.count, af.args);
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
            unvisited_clause.clear();
            int sat = solver.solve();
            if (sat == 20) break;

            for (uint32_t i = 1; i <= af.args; i++) {
                if (unvisited[i-1]) {
                    if (solver.model[i]) {
                        unvisited[i-1] = false;
                        result.push_back(af.int_to_arg[i-1]);
                    } else {
                        unvisited_clause.push_back(i);
                    }
                }
            }
            solver.add_clause(unvisited_clause);
        }
        


        return result;
    }

    std::vector<std::string> see_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        return result;
    }
}

#endif