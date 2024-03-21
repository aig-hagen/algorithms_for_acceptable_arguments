#include "scip_optimizer.h"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace cgss2;

long long SCIPOptimizer::convert_expr(const std::vector<long long>& coeffs, const std::vector<int>& lits, std::vector<SCIP_Real>& rcoeffs, std::vector<SCIP_VAR*>& rvars, std::vector<SCIP_VAR*>& lprvars) {
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
    while (l > (int)scipvars.size()) {
      scipvars.push_back(nullptr);
      lpscipvars.push_back(nullptr);
      SCIP_CALL(SCIPcreateVarBasic(scip, &scipvars.back(), to_string(l).c_str(), 0, 1, 0, SCIP_VARTYPE_BINARY));
      SCIP_CALL(SCIPaddVar(scip, scipvars.back()));
      SCIP_CALL(SCIPcreateVarBasic(lpscip, &lpscipvars.back(), "", 0, 1, 0, SCIP_VARTYPE_CONTINUOUS));
      SCIP_CALL(SCIPaddVar(lpscip, lpscipvars.back()));
    }
    rvars[i] = scipvars[l-1];
    lprvars[i] = lpscipvars[l-1];
  }
  return rhs;
}

void SCIPOptimizer::add_constr_(std::vector<long long>& coeffs, const std::vector<int>& lits, long long rhs) {
  vector<SCIP_Real> dcoeffs;
  vector<SCIP_VAR*> vars;
  vector<SCIP_VAR*> lpvars;
  rhs -= convert_expr(coeffs, lits, dcoeffs, vars, lpvars);
  SCIP_CONS* cons = nullptr;
  SCIP_CONS* lpcons = nullptr;
  SCIPcreateConsBasicLinear(scip, &cons, "", vars.size(), &vars[0], &dcoeffs[0], rhs, SCIPinfinity(scip));
  SCIPcreateConsBasicLinear(lpscip, &lpcons, "", vars.size(), &lpvars[0], &dcoeffs[0], rhs, SCIPinfinity(lpscip));
  SCIPaddCons(scip, cons);
  SCIPaddCons(lpscip, lpcons);
  SCIPreleaseCons(scip, &cons);
  SCIPreleaseCons(lpscip, &lpcons);
}


void SCIPOptimizer::set_objective_(std::vector<long long>& coeffs, const std::vector<int>& lits) {
  vector<SCIP_Real> dcoeffs;
  vector<SCIP_VAR*> vars;
  vector<SCIP_VAR*> lpvars;
  objective_shift = convert_expr(coeffs, lits, dcoeffs, vars, lpvars);

  // set weights of new objective variables
  for (int i=0; i<(int)lits.size(); ++i) {
    int l=lits[i];
    if (l<0) l=-l;
    if (binary_search(obj_lits.begin(), obj_lits.end(), l)) {
      wght[l-1] = 0;
    }
    SCIPchgVarObj(scip, vars[i], dcoeffs[i]);
    SCIPchgVarObj(lpscip, lpvars[i], dcoeffs[i]);
  }
  // reset weights of old objective variables
  for (int l : obj_lits) {
    if (wght[l-1]) {
      SCIPchgVarObj(scip, scipvars[l-1], 0);
      SCIPchgVarObj(lpscip, lpscipvars[l-1], 0);
      wght[l-1] = 0;
    }
  }
  // update obj_lits
  obj_lits.clear();
  obj_lits.reserve(lits.size());
  for (int i=0; i<(int)lits.size(); ++i) {
    int l = lits[i];
    if (l<0) l=-l;
    obj_lits.push_back(l);
    if ((int)wght.size()<l) wght.resize(l);
    wght[l-1] = dcoeffs[i];
  }
  sort(obj_lits.begin(), obj_lits.end());
}


int SCIPOptimizer::optimize(std::vector<int>& ret_model, long long bound) {
  double db = (double)bound + 0.5;
  db -= objective_shift;
  SCIP_CALL(SCIPsolve(scip));
  // TODO bound...
  int status = SCIPgetStatus(scip);
  if (status ==  SCIP_STATUS_INFEASIBLE) {
    SCIPfreeTransform(scip);
    return 0;
  }
  bool optimal = (status == SCIP_STATUS_OPTIMAL);
  SCIP_SOL* sol = SCIPgetBestSol(scip);
  ret_model.clear();
  ret_model.reserve(scipvars.size());
  for (int i=0; i<(int)scipvars.size(); ++i) {
    if (SCIPgetSolVal(scip, sol, scipvars[i])>0.5) ret_model.push_back(i+1);
    else                                           ret_model.push_back(-i-1);
  }
  SCIP_CALL(SCIPfreeTransform(scip));
  return optimal ? 2 : 1;
}
int SCIPOptimizer::optimize_limited(std::vector<int>& ret_model, double limit, long long bound) {
  SCIP_CALL(SCIPsetRealParam(scip, "limits/time", limit));
  double db = (double)bound + 0.5;
  db -= objective_shift;
  SCIP_CALL(SCIPsolve(scip));
  // TODO bound...
  int status = SCIPgetStatus(scip);
  if (status ==  SCIP_STATUS_INFEASIBLE) {
    SCIPfreeTransform(scip);
    return 0;
  }
  bool optimal = (status == SCIP_STATUS_OPTIMAL);
  if (!optimal) return -1;

  SCIP_SOL* sol = SCIPgetBestSol(scip);
  ret_model.clear();
  ret_model.reserve(scipvars.size());
  for (int i=0; i<(int)scipvars.size(); ++i) {
    if (SCIPgetSolVal(scip, sol, scipvars[i])>0.5) ret_model.push_back(i+1);
    else                                           ret_model.push_back(-i-1);
  }
  SCIP_CALL(SCIPfreeTransform(scip));
  return optimal ? 2 : 1;
}


bool SCIPOptimizer::solve_lp(std::vector<double>& ret_model, std::vector<double>& rcs) {
  SCIP_CALL(SCIPsolve(lpscip));
  int status = SCIPgetStatus(lpscip);
  if (status ==  SCIP_STATUS_INFEASIBLE) {
    SCIPfreeTransform(lpscip);
    return 0;
  }
  if (status != SCIP_STATUS_OPTIMAL) cerr << "c warning: SCIP optimizer did not return infeasible or optimal (" << status << ")\n";
  SCIP_SOL *sol = SCIPgetBestSol(lpscip);
  ret_model.clear();
  rcs.clear();
  ret_model.reserve(scipvars.size());
  rcs.reserve(scipvars.size());
  for (int i=0; i<(int)lpscipvars.size(); ++i) {
    ret_model.push_back(SCIPgetSolVal(lpscip, sol, lpscipvars[i]));
    rcs.push_back(SCIPvarGetBestRootRedcost(lpscipvars[i]));
  }
  SCIPfreeTransform(lpscip);
  return 1;
}

std::string SCIPOptimizer::version(int l) {
  if (l&1) return "SCIP ";
  return  "";
}


void SCIPOptimizer::print_stats(std::string bg, std::ostream& out) {
  bg+="scip.";
  out << bg << "nbvars: " << scipvars.size() << "\n";
}

void SCIPOptimizer::add_options(Options&) {

}

int SCIPOptimizer::init() {
  SCIP_CALL(SCIPcreate(&scip));
  SCIP_CALL(SCIPincludeDefaultPlugins(scip));
  SCIP_CALL(SCIPcreateProbBasic(scip, ""));
  SCIP_CALL(SCIPsetIntParam(scip, "display/verblevel", 0));
  SCIP_CALL(SCIPsetRealParam(scip, "limits/gap", 0));
  SCIP_CALL(SCIPsetRealParam(scip, "limits/absgap", 0.75));
  SCIP_CALL(SCIPcreate(&lpscip));
  SCIP_CALL(SCIPincludeDefaultPlugins(lpscip));
  SCIP_CALL(SCIPcreateProbBasic(lpscip, ""));
  SCIP_CALL(SCIPsetIntParam(lpscip, "display/verblevel", 0));
  return 0;
}

SCIPOptimizer::SCIPOptimizer(Options&) : scip(nullptr), lpscip(nullptr), objective_shift(0) {
  init();
}

SCIPOptimizer::~SCIPOptimizer() {
  for (int i=0; i<(int)scipvars.size(); ++i) SCIPreleaseVar(scip, &scipvars[i]);
  for (int i=0; i<(int)lpscipvars.size(); ++i) SCIPreleaseVar(lpscip, &lpscipvars[i]);
  SCIPfree(&scip);
  SCIPfree(&lpscip);
}
