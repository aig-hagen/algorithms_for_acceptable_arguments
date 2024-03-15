#ifndef SAT_CMSAT

#include "ExternalSatSolver.h"

#include <pstream.h>
#include <iostream>


ExternalSatSolver::ExternalSatSolver(int32_t n_vars, int32_t n_args) {
    number_of_vars = n_vars;
    model = std::vector<bool>(n_vars+1);
    clauses = std::vector<std::vector<int>>();
    solver = "./lib/cryptominisat-5.11.4/build/cryptominisat5";
}

void ExternalSatSolver::assume(int32_t lit) {
    assumptions.push_back(lit);
}

void ExternalSatSolver::add_clause(const std::vector<int32_t> & clause) {
    clauses.push_back(clause);
    clauses[clauses.size()-1].push_back(0);
}

int ExternalSatSolver::solve() {
    redi::pstream process(solver, redi::pstreams::pstdout | redi::pstreams::pstdin | redi::pstreams::pstderr);
    process << "p cnf " << number_of_vars << " " << (clauses.size()+assumptions.size()) << "\n";
    for(auto const& clause: clauses) {
        for(const int lit: clause){
            process << lit << " ";
        }
        process << "\n";
    }
    if (!assumptions.empty()) {
        for(const int assumption: assumptions){
            process << assumption << " 0\n";
        }
    }
    assumptions.clear();

    process << redi::peof;
    std::string line;
    model.clear();
    while (process.peek() != EOF && std::getline(process, line)) {
        if (line.rfind("c ", 0) == 0) {
            continue;
        }
        if (line.rfind("s ", 0) == 0) {
            if (line.rfind("UNSATISFIABLE") != std::string::npos) {
                return 20;
            }
        }
        if (line.rfind("v ", 0) == 0) {
            line.erase(0, 2);
            size_t pos = 0;
            while(line.length() > 0) {
                pos = line.find(" ");
                if (pos == std::string::npos) {
                    pos = line.length();
                }
                int var = stoi(line.substr(0, pos));
                if (var > 0) {
                    model[var] = true;
                } else if (var < 0) {
                    model[-var] = false;
                } else {
                    break;
                }
                line.erase(0, pos + 1);
            }
        }
    }
    return 10;
}

int ExternalSatSolver::solve(const std::vector<int32_t> assumptions) {
    for(auto const& assumption: assumptions) {
        assume(assumption);
    }
    return solve();
}
#endif