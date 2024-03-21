#include "interface.h"
#include "log.h"

using namespace std;
using namespace cgss2;

void CGSS2Interface::add_clause(vector<int>& cl, uint64_t ws, bool dimacs) {
  if (check_solution) solver.add_clause(cl, ws, dimacs);
  else                solver._add_clause(cl, ws, dimacs);
}

void CGSS2Interface::add_clauses(vector<vector<int> > & cl, vector<uint64_t>& ws, bool dimacs) {
  if (check_solution) solver.add_clauses(cl, ws, dimacs);
  else                solver._add_clauses(cl, ws, dimacs);
}


int CGSS2Interface::load_from_file(std::string filename) {
  if (reader.read_file(filename)) {
    cerr << "reading file failed" << endl;
    cerr << reader.error << endl;
    return 1;
  }
  solver.set_nof_vars(reader.vars);
  add_clauses(reader.clauses, reader.weights, true);
  file_loaded = filename;
  return 0;
}


int CGSS2Interface::solve(bool print_stats, bool print_model, std::ostream& out) {
  out << "c solve " << file_loaded << "\n";
  int v = solver.solve();
  if (print_stats) {
    solver.print_stats("c SOLVER-STATS ", out);
  }

  if (v == 1) {
    out << "s OPTIMUM FOUND\n";
    out << "o " << solver.get_optimal_cost() << endl;
    vector<int> model;
    if (print_model || check_solution) solver.get_model(model, true);
    if (check_solution) {
      bool sol_ok = 1;
      uint64_t cost = 0;
      for (int i=0; i<(int)reader.clauses.size(); ++i) {
        auto& clause = reader.clauses[i];
        uint64_t weight = reader.weights[i];
        bool sat = 0;
        for (int l : clause) {
          for (int l2 : model) {
            if (l==l2) {sat=1;break;}
          }
          if (sat) break;
        }
        if (sat) continue;
        if (weight==HARDWEIGHT) cout << "c ERROR: unsatisfied clause found!" << endl, sol_ok=0;
        else cost+=weight;
      }
      cout << "c o " << cost << endl;
      if (cost != solver.get_optimal_cost()) cout << "c ERROR: wrong cost reported!" << endl, sol_ok=0;
      if (sol_ok) {
        cout << "c sol check ok\n";
      }
    }
    if (print_model) {
      if (!compressed_model) {
        cout << "v ";
        for (int l : model) cout << l << " ";
        cout << "\n";
      } else {
        cout << "v ";
        for (int l : model) cout << (l<0?0:1);
        cout << "\n";
      }
    }
  } else if (v==0) {
    cout << "s UNSATISFIABLE\n";
  } else {
    cout << "c NOT SOLVED (" << v << ")\n";
  }

  return v;
}

void CGSS2Interface::add_options(Options& op) {
  op.add(Options::Option::BOOL, "sol-check", "sol-check", 0, "check solution", "0", 2);
  op.add(Options::Option::BOOL, "omf", "omf", 0, "old model format v-line", "0", 2);
}

CGSS2Interface::CGSS2Interface(Options& op, int verbose_level, std::ostream& vout) : check_solution(0), compressed_model(1) {
  if (op.bools["sol-check"])    check_solution=1;
  if (op.bools["omf"])    compressed_model=0;
  Log log(vout, verbose_level);
  solver.set_log(log);
  solver.init(op);
}
