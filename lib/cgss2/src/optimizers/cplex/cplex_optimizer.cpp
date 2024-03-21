#include "cplex_optimizer.h"
using namespace std;
using namespace cgss2;

long long CPLEXOptimizer::convert_expr(std::vector<long long>& coeffs, const std::vector<int>& lits, std::vector<IloNumVar>& rvars, std::vector<IloNumVar>& lprvars) {
  long long rhs = 0;
  int n = coeffs.size();
  rvars.resize(n);
  lprvars.resize(n);
  for (int i=0; i<n; ++i) {
    int l = lits[i];
    if (l<0) {
      rhs += coeffs[i];
      coeffs[i]=-coeffs[i];
      l=-l;
    }
    while (l > (int)cplexvars.size()) {
      cplexvars.push_back(IloNumVar(env, 0, 1, ILOBOOL));
      lpcplexvars.push_back(IloNumVar(env, 0, 1, ILOFLOAT));
    }
    rvars[i] = cplexvars[l-1];
    lprvars[i] = lpcplexvars[l-1];
  }
  return rhs;
}

void CPLEXOptimizer::add_constr_(std::vector<long long>& coeffs, const std::vector<int>& lits, long long rhs) {
  vector<IloNumVar> vars;
  vector<IloNumVar> lpvars;
  rhs -= convert_expr(coeffs, lits, vars, lpvars);
  IloExpr expr(env);
  IloExpr lpexpr(env);

  for (int i=0; i<(int)coeffs.size(); ++i) {
    expr += (double)coeffs[i] * vars[i];
    lpexpr += (double)coeffs[i] * lpvars[i];
  }
  model.add(expr >= (double)rhs);
  lpmodel.add(lpexpr >= (double)rhs);
}


void CPLEXOptimizer::set_objective_(std::vector<long long>& coeffs, const std::vector<int>& lits) {
  vector<IloNumVar> vars;
  vector<IloNumVar> lpvars;
  objective_shift = convert_expr(coeffs, lits, vars, lpvars);
  IloExpr expr(env);
  IloExpr lpexpr(env);
  for (int i=0; i<(int)coeffs.size(); ++i) {
    expr += (double)coeffs[i] * vars[i];
    lpexpr += (double)coeffs[i] * lpvars[i];
  }
  model.add(IloObjective(env, expr));
  lpmodel.add(IloObjective(env, lpexpr));
}

ILOMIPINFOCALLBACK1(lim_stop, double, lim) {
  if (hasIncumbent()) {
    double c = getIncumbentObjValue();
    if (c<=lim) abort();
  }
}


int CPLEXOptimizer::optimize(std::vector<int>& ret_model, long long bound) {
  double db = (double)bound + 0.5;
  db -= objective_shift;
  cplex.use(lim_stop(env, db));
  cplex.solve();
  if (cplex.getStatus() == IloAlgorithm::Infeasible) return 0;
  bool optimal = cplex.getStatus() == IloAlgorithm::Optimal;
  ret_model.clear();

  IloNumArray vals(env);
  ret_model.reserve(cplexvars.size());
  for (int i = 0; i < (int)cplexvars.size(); ++i) {
    try {
      if (cplex.getValue(cplexvars[i])>0.5) ret_model.push_back(i+1);
      else                                  ret_model.push_back(-i-1);
    } catch (IloAlgorithm::NotExtractedException const& e) {
      ret_model.push_back(i+1);
    }
  }
  return optimal ? 2 : 1;
}


bool CPLEXOptimizer::solve_lp(std::vector<double>& ret_model, std::vector<double>& rcs) {
  lpcplex.solve();
  if (lpcplex.getStatus() == IloAlgorithm::Infeasible) return 0;
  if (lpcplex.getStatus() != IloAlgorithm::Optimal) cerr << "c warning: cplex did not return infeasible or optimal (" << lpcplex.getStatus() << ")\n";
  ret_model.clear();
  rcs.clear();
  ret_model.reserve(lpcplexvars.size());
  rcs.reserve(lpcplexvars.size());
  for (int i = 0; i < (int)lpcplexvars.size(); ++i) {
    try {
      ret_model.push_back(lpcplex.getValue(lpcplexvars[i]));
      rcs.push_back(lpcplex.getReducedCost(lpcplexvars[i]));
    } catch (IloAlgorithm::NotExtractedException const& e) {
      ret_model.push_back(0.5);
      rcs.push_back(0);
    }
  }
  return 1;
}

void CPLEXOptimizer::print_stats(std::string bg, std::ostream& out) {
  bg+="cplex.";
  out << bg << "nbvars: " << cplexvars.size() << "\n";
}

std::string CPLEXOptimizer::version(int l) {
  static IloEnv version_env;
  static IloCplex version_cplex(version_env);
  if (l&1) return "CPLEX " + std::string(version_cplex.getVersion());
  return  std::string(version_cplex.getVersion());
}

void CPLEXOptimizer::add_options(Options& op) {
  op.add(Options::Option::DOUBLE, "cplex-EpGap", "cplex-ep-gap", 0, "EpGap parameter used when initializing CPLEX", "0", 2);
  op.add(Options::Option::DOUBLE, "cplex-EpAGap", "cplex-ep-a-gap", 0, "EpAGap parameter used when initializing CPLEX", "0.75", 1);
  op.add(Options::Option::DOUBLE, "cplex-EpInt", "cplex-ep-int", 0, "EpInt parameter used when initializing CPLEX", "0", 2);
  op.add(Options::Option::DOUBLE, "cplex-EpLin", "cplex-ep-lin", 0, "EpLin parameter used when initializing CPLEX", "1e-4", 2);
  op.add(Options::Option::DOUBLE, "cplex-EpOpt", "cplex-ep-opt", 0, "EpOpt parameter used when initializing CPLEX", "1e-7", 2);
  op.add(Options::Option::DOUBLE, "cplex-EpMrk", "cplex-ep-mrk", 0, "EpMrk parameter used when initializing CPLEX", "0.01", 2);
  op.add(Options::Option::DOUBLE, "cplex-EpPer", "cplex-ep-per", 0, "EpPer parameter used when initializing CPLEX", "1e-6", 2);
}

CPLEXOptimizer::CPLEXOptimizer(Options& op) : model(env), lpmodel(env), cplex(model), lpcplex(lpmodel), objective_shift(0) {
 cplex.setOut(env.getNullStream());
 if (op.doubles.count("cplex-EpGap"))  cplex.setParam(IloCplex::EpGap, op.doubles["cplex-EpGap"]);
 else                                  cplex.setParam(IloCplex::EpGap, 0.0);
 if (op.doubles.count("cplex-EpAGap")) cplex.setParam(IloCplex::EpAGap, op.doubles["cplex-EpAGap"]);
 else                                  cplex.setParam(IloCplex::EpAGap, 0.75);
 if (op.doubles.count("cplex-EpInt"))  cplex.setParam(IloCplex::EpInt, op.doubles["cplex-EpInt"]);
 else                                  cplex.setParam(IloCplex::EpInt, 0);
 if (op.doubles.count("cplex-EpLin"))  cplex.setParam(IloCplex::EpLin, op.doubles["cplex-EpLin"]);
 else                                  cplex.setParam(IloCplex::EpLin, 1e-4);
 if (op.doubles.count("cplex-EpOpt"))  cplex.setParam(IloCplex::EpOpt, op.doubles["cplex-EpOpt"]);
 else                                  cplex.setParam(IloCplex::EpOpt, 1e-7);
 if (op.doubles.count("cplex-EpMrk"))  cplex.setParam(IloCplex::EpMrk, op.doubles["cplex-EpMrk"]);
 else                                  cplex.setParam(IloCplex::EpMrk, 0.01);
 if (op.doubles.count("cplex-EpPer"))  cplex.setParam(IloCplex::EpPer, op.doubles["cplex-EpPer"]);
 else                                  cplex.setParam(IloCplex::EpPer, 1e-6);
 // TODO: EpRHS feasibility tolerance
 cplex.setParam(IloCplex::Threads, 1);
 cplex.setParam(IloCplex::MIPDisplay, 0);
 lpcplex.setOut(env.getNullStream());;
 lpcplex.setParam(IloCplex::MIPDisplay, 0);
}


CPLEXOptimizer::~CPLEXOptimizer() {

}
