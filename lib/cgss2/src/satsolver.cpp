#include "satsolver.h"

#ifdef CADICAL
#include "solvers/cadical/cadical_solver.h"
#endif

#ifdef GLUCOSE3
#include "solvers/glucose3/glucose3_solver.h"
#endif

#ifdef GLUCOSE3X
#include "solvers/glucose3x/glucose3x_solver.h"
#endif

#define NAME_GLUCOSE3 "glucose3"
#define NAME_GLUCOSE3X "glucose3x"
#define NAME_CADICAL "cadical"

using namespace std;
using namespace cgss2;






vector<string> cgss2::satsolver_available() {
  vector<string> solvers_available;
#ifdef CADICAL
  solvers_available.push_back(NAME_CADICAL);
#endif
#ifdef GLUCOSE3
  solvers_available.push_back(NAME_GLUCOSE3);
#endif
#ifdef GLUCOSE3X
  solvers_available.push_back(NAME_GLUCOSE3X);
#endif
  return solvers_available;
}

string cgss2::satsolver_default() {
#ifdef GLUCOSE3
  return NAME_GLUCOSE3;
#elif defined(GLUCOSE3X)
  return NAME_GLUCOSE3X;
#elif defined(CADICAL)
  return NAME_CADICAL;
#else
  return "";
#endif
}

SATsolver* cgss2::satsolver_create(string& name) {

#ifdef CADICAL
  if (name==NAME_CADICAL) {
    CadicalSolver* solver = new CadicalSolver();
    return (SATsolver*)solver;
  }
#endif

#ifdef GLUCOSE3
  if (name==NAME_GLUCOSE3) {
    Glucose3Solver* solver = new Glucose3Solver();
    return (SATsolver*)solver;
  }
#endif

#ifdef GLUCOSE3X
  if (name==NAME_GLUCOSE3X) {
    Glucose3xSolver* solver = new Glucose3xSolver();
    return (SATsolver*)solver;
  }
#endif

  return nullptr;
}


string cgss2::satsolver_version(string name, int l) {

  #ifdef CADICAL
    if (name==NAME_CADICAL) {
      return CadicalSolver::version(l);
    }
  #endif

  #ifdef GLUCOSE3
    if (name==NAME_GLUCOSE3) {
      return Glucose3Solver::version(l);
    }
  #endif

  #ifdef GLUCOSE3X
    if (name==NAME_GLUCOSE3X) {
      return Glucose3xSolver::version(l);
    }
  #endif

  return "";
}
