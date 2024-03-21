#include "optimizer.h"

#ifdef WITH_GUROBI
#include "optimizers/gurobi/gurobi_optimizer.h"
#endif

#ifdef WITH_CPLEX
#include "optimizers/cplex/cplex_optimizer.h"
#endif

#ifdef WITH_SCIP
#include "optimizers/scip/scip_optimizer.h"
#endif

#define NAME_GUROBI "gurobi"
#define NAME_CPLEX "cplex"
#define NAME_SCIP "scip"

using namespace std;


namespace cgss2 {

vector<string> optimizer_available() {
  vector<string> optimizers_available;
#ifdef WITH_GUROBI
  optimizers_available.push_back(NAME_GUROBI);
#endif
#ifdef WITH_CPLEX
  optimizers_available.push_back(NAME_CPLEX);
#endif
#ifdef WITH_SCIP
  optimizers_available.push_back(NAME_SCIP);
#endif
  return optimizers_available;
}

string optimizer_default() {
#ifdef WITH_GUROBI
  return NAME_GUROBI;
#elif defined(WITH_CPLEX)
  return NAME_CPLEX;
#elif defined(WITH_SCIP)
  return NAME_SCIP;
#else
  return "";
#endif
}

Optimizer* optimizer_create(string name, Options& op) {

#ifdef WITH_GUROBI
  if (name==NAME_GUROBI) {
    GurobiOptimizer* optimizer = new GurobiOptimizer(op);
    return (Optimizer*)optimizer;
  }
#endif

#ifdef WITH_CPLEX
  if (name==NAME_CPLEX) {
    CPLEXOptimizer* optimizer = new CPLEXOptimizer(op);
    return (Optimizer*)optimizer;
  }
#endif

#ifdef WITH_SCIP
  if (name==NAME_SCIP) {
    SCIPOptimizer* optimizer = new SCIPOptimizer(op);
    return (Optimizer*)optimizer;
  }
#endif

  return nullptr;
}

void optimizer_add_options(Options& op) {

#ifdef WITH_GUROBI
  GurobiOptimizer::add_options(op);
#endif

#ifdef WITH_CPLEX
  CPLEXOptimizer::add_options(op);
#endif

#ifdef WITH_SCIP
  SCIPOptimizer::add_options(op);
#endif
}


string optimizer_version(string name, int l) {

#ifdef WITH_GUROBI
  if (name==NAME_GUROBI) {
    return GurobiOptimizer::version(l);
  }
#endif

#ifdef WITH_CPLEX
  if (name==NAME_CPLEX) {
    return CPLEXOptimizer::version(l);
  }
#endif

#ifdef WITH_SCIP
  if (name==NAME_SCIP) {
    return SCIPOptimizer::version(l);
  }
#endif

  return "";
}

}
