#if defined(EEE)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> eee_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        result.reserve(af.args);
        std::vector<bool> included(af.args, false);
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

        std::vector<int> complement_clause(af.args);
        while (true) {
            if (solver.solve() == UNSAT_V) break;

            complement_clause.clear();
            for (uint32_t i = 0; i < af.args; i++) {
                if (solver.model[i]) {
                    if (!included[i]) {
                        included[i] = true;
                        result.push_back(af.int_to_arg[i]);
                    }
                    complement_clause.push_back(-af.accepted_var[i]);
                } else {
                    complement_clause.push_back(af.accepted_var[i]);
                }
            }
            solver.add_clause(complement_clause);
        }
        return result;
    }

    std::vector<std::string> eee_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> included(af.args, true);
        uint32_t num_vars = af.count;
        SAT_Solver solver = SAT_Solver(num_vars, af.args);
        #ifdef SAT_EXTERNAL
        solver.set_solver(af.solver_path);
        #endif

        if (sem == PR) {
            Encodings::complete(af, solver);

            std::vector<int32_t> clause;
            std::vector<bool> extension(af.args);
            clause.reserve(af.args);
            while (true) {
                extension.clear();
                extension.resize(af.args, false);
                //if (solver.solve() == UNSAT_V) break;
                bool noFurther = true;
                while (true) {
                    clause.clear();

                    if (solver.solve() == UNSAT_V) break;
                    noFurther = false;

                    for (uint32_t i = 0; i < af.args; i++) {
                        if (solver.model[i]) {
                            extension[i] = true;
                            solver.assume(af.accepted_var[i]);
                        } else {
                            clause.push_back(af.accepted_var[i]);
                        }
                    }
                    solver.add_clause(clause);
                }
                if (noFurther) break;

                clause.clear();
                num_extensions++;
                for (uint32_t i = 0; i < af.args; i++) {
                    included[i] = included[i] && extension[i];
                    if (!extension[i]) {
                        clause.push_back(af.accepted_var[i]);
                    }
                }
                solver.add_clause(clause);
            }

        } else if (sem == ST) {
            Encodings::stable(af, solver);
            std::vector<int32_t> complement_clause;
            complement_clause.reserve(af.count);
            while (true) {
                if (solver.solve() == UNSAT_V) break;

                complement_clause.clear();
                for (uint32_t i = 0; i < af.args; i++) {
                    included[i] = included[i] && solver.model[i];
                    if (solver.model[i]) {
                        complement_clause.push_back(-af.accepted_var[i]);
                    }
                }
                solver.add_clause(complement_clause);
                
            }
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        for (uint32_t i = 0; i < af.args; i++) {
            if (included[i]) {
                result.push_back(af.int_to_arg[i]);
            }
        }
        std::cout << num_extensions << std::endl;
        return result;
    }
}

#endif