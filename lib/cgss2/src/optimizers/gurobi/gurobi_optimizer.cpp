#include "gurobi_optimizer.h"
using namespace std;
using namespace cgss2;

long long GurobiOptimizer::convert_expr(const std::vector<long long>& coeffs, const std::vector<int>& lits, std::vector<double>& rcoeffs, std::vector<GRBVar>& rvars, std::vector<GRBVar>& lprvars) {
  long long rhs = 0;
  int n = coeffs.size();
  rcoeffs.resize(n);
  rvars.resize(n);
  lprvars.resize(n);
  for (int i=0; i<n; ++i) {
    int l = lits[i];
    if (l<0) {
      rhs += coeffs[i];
      rcoeffs[i]=-coeffs[i];
      l=-l;
    } else {
      rcoeffs[i]=coeffs[i];
    }
    while (l > (int)grbvars.size()) {
      grbvars.push_back(model->addVar(0, 1, 0, GRB_BINARY));
      lpgrbvars.push_back(lpmodel->addVar(0, 1, 0, GRB_CONTINUOUS));
    }
    rvars[i] = grbvars[l-1];
    lprvars[i] = lpgrbvars[l-1];
  }
  return rhs;
}

void GurobiOptimizer::add_constr_(std::vector<long long>& coeffs, const std::vector<int>& lits, long long rhs) {
  vector<double> dcoeffs;
  vector<GRBVar> vars;
  vector<GRBVar> lpvars;
  rhs -= convert_expr(coeffs, lits, dcoeffs, vars, lpvars);
  GRBLinExpr lex = GRBLinExpr();
  GRBLinExpr lplex = GRBLinExpr();

  lex.addTerms(&dcoeffs[0], &vars[0], coeffs.size());
  lplex.addTerms(&dcoeffs[0], &lpvars[0], coeffs.size());
  model->addConstr(lex >= rhs);
  lpmodel->addConstr(lplex >= rhs);
}


void GurobiOptimizer::set_objective_(std::vector<long long>& coeffs, const std::vector<int>& lits) {
  vector<double> dcoeffs;
  vector<GRBVar> vars;
  vector<GRBVar> lpvars;
  objective_shift = convert_expr(coeffs, lits, dcoeffs, vars, lpvars);
  GRBLinExpr lex = GRBLinExpr();
  GRBLinExpr lplex = GRBLinExpr();
  lex.addTerms(&dcoeffs[0], &vars[0], coeffs.size());
  lplex.addTerms(&dcoeffs[0], &lpvars[0], coeffs.size());

  model->setObjective(lex, GRB_MINIMIZE);
  lpmodel->setObjective(lplex, GRB_MINIMIZE);
}


int GurobiOptimizer::optimize(std::vector<int>& ret_model, long long bound) {
  double db = (double)bound + 0.5;
  db -= objective_shift;
  model->set(GRB_DoubleParam_BestObjStop, db);
  model->optimize();
  if (model->get(GRB_IntAttr_Status) == GRB_INFEASIBLE) return 0;
  bool optimal = model->get(GRB_IntAttr_Status) == GRB_OPTIMAL;
  ret_model.clear();
  ret_model.reserve(grbvars.size());
  for (int i=0; i<(int)grbvars.size(); ++i) {
    if (grbvars[i].get(GRB_DoubleAttr_X)>0.5) ret_model.push_back(i+1);
    else                                      ret_model.push_back(-i-1);
  }
  return optimal ? 2 : 1;
}

bool GurobiOptimizer::solve_lp(std::vector<double>& ret_model, std::vector<double>& rcs) {
  lpmodel->optimize();
  if (lpmodel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE) return 0;
  if (lpmodel->get(GRB_IntAttr_Status) != GRB_OPTIMAL) cerr << "c warning: gurobi optimizer did not return infeasible or optimal (" << lpmodel->get(GRB_IntAttr_Status) << ")\n";
  ret_model.clear();
  rcs.clear();
  ret_model.reserve(lpgrbvars.size());
  rcs.reserve(lpgrbvars.size());
  for (int i=0; i<(int)lpgrbvars.size(); ++i) {
    ret_model.push_back(lpgrbvars[i].get(GRB_DoubleAttr_X));
    rcs.push_back(lpgrbvars[i].get(GRB_DoubleAttr_RC));
  }
  return 1;
}


void GurobiOptimizer::print_stats(std::string bg, std::ostream& out) {
  bg+="gurobi.";
  out << bg << "nbvars: " << grbvars.size() << "\n";
}

std::string GurobiOptimizer::version(int l) {
  if (l&1) return "Gurobi " + std::to_string(GRB_VERSION_MAJOR) + "." + std::to_string(GRB_VERSION_MINOR) + "." +std::to_string(GRB_VERSION_TECHNICAL);
  return  std::to_string(GRB_VERSION_MAJOR) + "." + std::to_string(GRB_VERSION_MINOR) + "." +std::to_string(GRB_VERSION_TECHNICAL);
}

void GurobiOptimizer::add_options(Options& op) {
  op.add(Options::Option::STRING, "gurobi-MIPGap", "gurobi-mip-gap", 0, "MIPGap parameter used when initializing gurobi", "0", 2);
  op.add(Options::Option::STRING, "gurobi-MIPGapAbs", "gurobi-mip-gap-abs", 0, "MIPGapAbs parameter used when initializing gurobi", "0.75", 1);
}

GurobiOptimizer::GurobiOptimizer(Options& op) : env(nullptr), model(nullptr), lpmodel(nullptr), objective_shift(0) {
 try {
   env = new GRBEnv(true);
   env->set("OutputFlag", "0");
   if (op.strs.count("gurobi-MIPGap")) env->set("MIPGap", op.strs["gurobi-MIPGap"]);
   else                                env->set("MIPGap", "0");
   if (op.strs.count("gurobi-MIPGapAbs")) env->set("MIPGapAbs", op.strs["gurobi-MIPGapAbs"]);
   else                                   env->set("MIPGapAbs", "0.75");
   env->set("Threads", "1");
   env->set("IntFeasTol", "1e-9");
   env->start();

   model = new GRBModel(*env);
   lpmodel = new GRBModel(*env);
 } catch(GRBException e) {
   std::cout << "Error code = " << e.getErrorCode() << std::endl;
   std::cout << e.getMessage() << std::endl;
 }
}

GurobiOptimizer::~GurobiOptimizer() {
  if (env) delete env;
  if (model) delete model;
  if (lpmodel) delete lpmodel;
}
