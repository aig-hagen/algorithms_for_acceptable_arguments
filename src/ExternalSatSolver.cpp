#ifdef SAT_EXTERNAL

#include "ExternalSatSolver.h"

#include <pstream.h>
#include <iostream>


ExternalSatSolver::ExternalSatSolver(int32_t n_vars, int32_t n_args) {
    number_of_vars = n_vars;
    model = std::vector<bool>(n_args);
    clauses = std::vector<std::vector<int>>();
    //solver = "./lib/cryptominisat-5.11.21/build/cryptominisat5";
    //solver = "./lib/cadical-1.9.5/build/cadical";
}

void ExternalSatSolver::set_solver(std::string path) {
    solver = path;
}

void ExternalSatSolver::assume(int32_t lit) {
    assumptions.push_back(lit);
}

void ExternalSatSolver::add_clause(const std::vector<int32_t> & clause) {
    clauses.push_back(clause);
    clauses[clauses.size()-1].push_back(0);
}

void ExternalSatSolver::add_clause_1(int32_t lit) {
    std::vector<int32_t> clause = { lit, 0 };
    clauses.push_back(clause);
}

void ExternalSatSolver::add_clause_2(int32_t lit1, int32_t lit2) {
    std::vector<int32_t> clause = { lit1, lit2, 0 };
    clauses.push_back(clause);
}

void ExternalSatSolver::add_clause_3(int32_t lit1, int32_t lit2, int32_t lit3) {
    std::vector<int32_t> clause = { lit1, lit2, lit3, 0 };
    clauses.push_back(clause);
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
    model.resize(number_of_vars);
    while (process.peek() != EOF && std::getline(process, line)) {
        if (line.rfind("c ", 0) == 0) {
            continue;
        }
        if (line.rfind("s ", 0) == 0) {
            if (line.rfind("UNSATISFIABLE") != std::string::npos) {
                return UNSAT_V;
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
                    model[var-1] = true;
                } else if (var < 0) {
                    model[1-var] = false;
                } else {
                    break;
                }
                line.erase(0, pos + 1);
            }
        }
    }
    return SAT_V;
}

int ExternalSatSolver::solve(const std::vector<int32_t> assumptions) {
    for(auto const& assumption: assumptions) {
        assume(assumption);
    }
    return solve();
}
#endif