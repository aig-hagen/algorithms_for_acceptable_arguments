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
            int sat = solver.solve();
            if (sat == 20) break;

            unvisited_clause.clear();
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
        std::vector<bool> included(af.args+1, true);

        SAT_Solver solver = SAT_Solver(af.count, af.args);
        if (sem == ST) {
            Encodings::stable(af, solver);
            
            std::vector<int32_t> complement_clause;
            while (true) {
                int sat = solver.solve();
                if (sat == 20) break;

                complement_clause.clear();
                for (uint32_t i = 1; i <= af.args; i++) {
                    included[i] = included[i] && solver.model[i];

                    if (included[i]) {
                        complement_clause.push_back(af.rejected_var[i-1]);
                    }
                }
                solver.add_clause(complement_clause);
            }

        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        for (uint32_t i = 1; i <= af.args; i++) {
            if (included[i]) {
                result.push_back(af.int_to_arg[i-1]);
            }
        }
        return result;
    }
}
#endif