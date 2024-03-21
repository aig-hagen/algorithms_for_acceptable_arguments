#include "cgss2.h"
#include "global.h"

#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <random>

using namespace std;
using namespace cgss2;



// generic functions

// resize data structures
void CGSS2::reserve_vars(int nof_vars) {
  unsigned n = 2*(nof_vars+1);
  if (_lit_from_solver.size() > n) return;

  bit_data.resize(n);
  _lit_from_solver.resize(n, -1);
  _lit_to_solver.resize(n);
  wi.resize(n);
  ws.resize(n);
  wsb.resize(n);
  cws.resize(n);
  lit_to_clause.resize(n);
  output_to_totalizer.resize(n);
  lit_in_cores.resize(n);
  lit_in_am1s.resize(n);
  connsn.resize(n);
  connsp.resize(n);
}

void CGSS2::set_nof_vars(int nof_vars) {
  vars = nof_vars;
  reserve_vars(vars);
}

// create new variable
int CGSS2::new_var() {
  int nv = ++vars;
  reserve_vars(vars);
  bit_data[posLit(nv)] |= FROM_SOLVER;
  bit_data[negLit(nv)] |= FROM_SOLVER;
  return nv;
}


// given lit from problem, map it to lit used by solver
int CGSS2::lit_to_solver(int lit, bool dimacs) {
  if (dimacs) lit = fromDimacs(lit);

  reserve_vars(litVariable(lit));

  if (bit_data[lit]&TO_SOLVER) return _lit_to_solver[lit]; // mapping exists
  if (map_all_vars || (bit_data[lit]&FROM_SOLVER) || lit<2) { // all variables should be remapped, or some other lit already mapped to this id, or this var is 0, which we don't use
    int nv = ++vars;
    reserve_vars(vars);

    int mlit = posLit(nv);
    if (keep_polarities && (lit&1)) mlit = negLit(nv);
    _lit_to_solver[lit] = mlit;
    _lit_to_solver[litNegation(lit)] = litNegation(mlit);
    _lit_from_solver[mlit] = lit;
    _lit_from_solver[litNegation(mlit)] = litNegation(lit);

    bit_data[lit] |= TO_SOLVER;
    bit_data[litNegation(lit)] |= TO_SOLVER;
    bit_data[mlit] |= FROM_SOLVER;
    bit_data[litNegation(mlit)] |= FROM_SOLVER;
    return mlit;
  } else {
    _lit_to_solver[lit] = lit;
    _lit_to_solver[litNegation(lit)] = litNegation(lit);
    _lit_from_solver[lit] = lit;
    _lit_from_solver[litNegation(lit)] = litNegation(lit);
    bit_data[lit] |= TO_SOLVER|FROM_SOLVER;
    bit_data[litNegation(lit)] |= TO_SOLVER|FROM_SOLVER;
    if (vars < litVariable(lit)) vars = litVariable(lit);

    return lit;
  }
}

int CGSS2::lit_from_solver(int lit, bool dimacs) {
  int mlit = _lit_from_solver[lit];
  if (mlit == -1 && dimacs) return 0;
  if (dimacs) mlit = toDimacs(mlit);
  return mlit;
}



void CGSS2::add_weight(int l, int64_t w) {
  if (bit_data[l]&WEIGHTED) {
    ws[l] += w;
    if (ws[l] < climit && ws[l]-w < climit) sumw += w;
    else if (ws[l]-w < climit) sumw -= ws[l]-w;
    else if (ws[l] < climit) sumw += ws[l];
  } else {
    ws[l] += w;
    if (ws[l] != HARDWEIGHT && ws[l] != 0) {
      bit_data[l] |= WEIGHTED;
      alit_order.push_back(l);
      weighted_lits.push_back(l);
    }
    if (ws[l] < climit) sumw += ws[l];
  }
}

void CGSS2::set_weight(int l, uint64_t w) {
  add_weight(l, -ws[l] + w);
}

void CGSS2::init_weight(int l, uint64_t w) {
  if ((w || wi[l]) && log.level(5)) log.l(5) << "c init_weight " << l << " " << w << "\n";
  wi[l] = w;
  add_weight(l, -ws[l] + w);
  if (w == 0) bit_data[l] &= ~(WEIGHTED);
}


inline bool CGSS2::is_base_assumption(int l) {
  return ws[l] == HARDWEIGHT;
}

inline int CGSS2::found_core(vector<int>& lits, uint64_t weight) {
  cores.emplace_back(lits, weight, get_time(0));
  return cores.size()-1;
}
inline int CGSS2::found_core(int lit, uint64_t weight) {
  cores.emplace_back(lit, weight, get_time(0));
  return cores.size()-1;
}

inline int CGSS2::found_am1(vector<int>& lits, int rl) {
  int rv = atmost_ones.size();
  for (int l : lits) lit_in_am1s[l].push_back(rv);
  atmost_ones.emplace_back();
  lits.swap(atmost_ones.back().first);
  atmost_ones.back().second = rl;
  return rv;
}

inline uint64_t CGSS2::gminw(const vector<int>& lits) {
  uint64_t r = HARDWEIGHT;
  for (int l : lits) r=min(r, ws[l]);
  return r;
}





int CGSS2::push_clock() {
  clocks.push_back(chrono::steady_clock::now());
  return clocks.size()-1;
}

double CGSS2::get_time() {
  return (chrono::steady_clock::now() - clocks.back()).count()*1e-9;
}

double CGSS2::get_time(int i) {
  return (chrono::steady_clock::now() - clocks[i]).count()*1e-9;
}

double CGSS2::pop_clock() {
  double tm = (chrono::steady_clock::now() - clocks.back()).count()*1e-9;
  clocks.pop_back();
  return tm;
}






// ensure that the soft clause associated with l is in sat solver
void CGSS2::prepare_assumption(int l) {
  if (bit_data[l]&RELAX_LIT) {
    int i = lit_to_clause[l];
    if (! (bit_data[i]&SOFT_CL_IN_SOLVER)) {
      soft_clauses[i].push_back(litNegation(l));
      satsolver->add_clause(soft_clauses[i]);
      if (upsolver) upsolver->add_clause(soft_clauses[i]);
      soft_clauses[i].pop_back();
      bit_data[i] |= SOFT_CL_IN_SOLVER;
    }
  }
}

// l was in core i, save that information
// if l is an output literal, activate the next output
void CGSS2::in_core(int l, int i) {
  if (i != -1) lit_in_cores[l].push_back(i);
  if (bit_data[l]&OUTPUT_LIT) {
    Totalizer& t = totalizers[output_to_totalizer[l]];
    if (t.outputs.back() == litNegation(l) && (int)t.outputs.size() < t.nof_inputs) {
      int toplit = vars<<1;
      int nxt = encoder->next_output(litNegation(l), toplit);
      toplit_from_encoder(toplit);
      add_encoder_clauses();

      t.outputs.push_back(nxt);
      if (t.minw < climit) sumw -= t.minw; // the weight of this lit already in sumw, remove the weight, add_weight will add it again

      nxt = litNegation(nxt);
      add_weight(nxt, t.minw);
      wsb[nxt] = ws[nxt];
      bit_data[nxt] |= OUTPUT_LIT;
      output_to_totalizer[nxt] = output_to_totalizer[l];
    }
  }
}


// auxiliary functions to sync new variables and clauses from the encoder
void CGSS2::toplit_from_encoder(int toplit) {
  reserve_vars(toplit>>1);
  while (vars < (toplit>>1)) {
    bit_data[posLit(++vars)] |= FROM_SOLVER;
    bit_data[negLit(vars)]   |= FROM_SOLVER;
  }
}

void CGSS2::add_encoder_clauses() {
  vector<vector<int> > new_clauses;
  encoder->get_new_clauses(new_clauses);
  for (auto& cl : new_clauses) {
    satsolver->add_clause(cl);
    if (upsolver) upsolver->add_clause(cl);
  }
}


void CGSS2::remove_contradictory_assumptions() {
  // remove negation pairs from softs
  for (int l : alit_order) {
    if (is_base_assumption(l)) continue;
    uint64_t w = ws[l];
    if (!w) continue;
    if (ws[litNegation(l)] && !is_base_assumption(litNegation(l))) {
      uint64_t minw = min(w, ws[litNegation(l)]);
      LB += minw;
      HLB += minw;
      add_weight(l, -minw);
      add_weight(litNegation(l), -minw);
    }
  }

  // harden softs falsified by base_assumptions
  for (int l : base_assumptions) {
    if (ws[litNegation(l)] && !is_base_assumption(litNegation(l))) {
      LB += ws[litNegation(l)];
      HLB += ws[litNegation(l)];
      add_weight(litNegation(l), -ws[litNegation(l)]);
    }
  }
  if (LB) log.l(1) << "c LB after removing contradictory assumptions: " << LB << "\n";
}

void CGSS2::reset_weights() {
  for (unsigned l=0; l<wi.size(); ++l) {
    init_weight(l, wi[l]);
  }
  for (int l : base_assumptions) {
    set_weight(l, HARDWEIGHT);
  }
  LB = 0;
  HLB = 0;
  remove_contradictory_assumptions();
}
void CGSS2::restore_weights(int core, bool restore_lb) {
  Core& c = cores[core];
  for (int l : c.lits) {
    if (is_base_assumption(l)) continue;
    add_weight(l, c.weight+cws[l]);
    cws[l]=0;
  }
  if (restore_lb) LB -= c.weight;
}


void CGSS2::restore_weights(vector<int>& core_set, bool restore_lb) {
  for (int i : core_set) {
    Core& c = cores[i];
    for (int l : c.lits) {
      if (is_base_assumption(l)) continue;
      add_weight(l, c.weight+cws[l]);
      cws[l]=0;
    }
    if (restore_lb) LB -= c.weight;
  }
}

// loop through cores, recalculate weights
bool CGSS2::recalculate_weights() {
  push_clock();
  // select am1s in the order in which they were found
  priority_queue<pair<int, int> > ordered_am1s;
  set<int>                        in_ordered_am1s; // keep track of am1s that are queued to be handled

  // select cores in the order of their weights
  priority_queue<pair<uint64_t, int> > ordered_cores;


  uint64_t old_LB = LB;

  reset_weights();
  for (unsigned i=0; i<atmost_ones.size(); ++i) {
    if (!greedy_costs) {
      uint64_t minw = gminw(atmost_ones[i].first);
      if (minw) {
        ordered_am1s.push({-i, i});
        in_ordered_am1s.insert(i);
      }
    } else {
      int am = 0;
      for (int l : atmost_ones[i].first) {
        if (ws[l] && ++am>1) break;
      }
      if (am>1) {
        ordered_am1s.push({-i, i});
        in_ordered_am1s.insert(i);
      }
    }
  }

  auto lit_weight_updated = [&](int l, bool update_cores) {
    if (update_cores) {
      for (int i : lit_in_cores[l]) {
        Core& cc = cores[i];
        uint64_t minw = gminw(cc.lits);
        if (minw != cc.weight) {
          cc.weight = minw;
          ordered_cores.push({cc.weight, i});
        }
      }
    }
    for (int i : lit_in_am1s[l]) {
      if (in_ordered_am1s.count(i)) continue;
      if (!greedy_costs) {
        uint64_t minw = gminw(atmost_ones[i].first);
        if (minw) {
          ordered_am1s.push({-i, i});
          in_ordered_am1s.insert(i);
        }
      } else {
        int am = 0;
        for (int l2 : atmost_ones[i].first) {
          if (ws[l2] && ++am>1) break;
        }
        if (am>1) {
          ordered_am1s.push({-i, i});
          in_ordered_am1s.insert(i);
        }
      }
    }
  };
  //
  auto empty_am1s = [&](bool update_cores) {
    while (!ordered_am1s.empty()) {
      int ai = ordered_am1s.top().second;
      ordered_am1s.pop();
      in_ordered_am1s.erase(ai);
      if (log.level(5)) { log.l(5) << "c recalculate weights add am1 { "; for (int l : atmost_ones[ai].first) log.l(5) << l << ":" << ws[l] << " "; log.l(5) << "} LB: " << LB << " -> "; }
      if (!greedy_costs) {
        uint64_t minw = gminw(atmost_ones[ai].first);
        if (minw) {
          for (int l : atmost_ones[ai].first) {
            if (is_base_assumption(l)) continue;
            add_weight(l, -minw);
          }
          add_weight(atmost_ones[ai].second, minw);
          lit_weight_updated(atmost_ones[ai].second, update_cores);
          LB += minw * (atmost_ones[ai].first.size() - 1);
        }
      } else {
        uint64_t maxw = ws[atmost_ones[ai].first[0]];
        for (int l2 : atmost_ones[ai].first) {
          maxw = max(maxw, ws[l2]);
          LB += ws[l2];
        }
        LB -= maxw;
        for (int l2 : atmost_ones[ai].first) {
          if (is_base_assumption(l2)) continue;
          if (ws[l2] != maxw) {
            add_weight(litNegation(l2), maxw - ws[l2]);
            lit_weight_updated(litNegation(l2), update_cores);
          }
          add_weight(l2, -ws[l2]);
        }
        lit_weight_updated(atmost_ones[ai].second, update_cores);
        add_weight(atmost_ones[ai].second, maxw);
      }
      log.l(5) << LB << "\n";
    }
  };

  empty_am1s(0);

  for (unsigned i=0; i<cores.size(); ++i) {
    Core& c = cores[i];
    uint64_t minw = gminw(c.lits);
    ordered_cores.push({minw, i});
    c.weight = minw;
  }

  while (!ordered_cores.empty()) {
    auto& it = ordered_cores.top();
    uint64_t w = it.first;
    int i = it.second;
    Core& c = cores[i];
    ordered_cores.pop();

    if (c.weight != w) continue;
    if (c.weight == 0) continue;
    if (c.weight == HARDWEIGHT) {
      double tm = pop_clock();
      time_recalculate_weights += tm;
      log.l(1) << "c recalculate weights done, instance UNSAT under given assumptions\n";
      return 1;
    }
    uint64_t minw = gminw(c.lits);
    if (minw != c.weight) {
      c.weight = minw;
      if (minw) ordered_cores.push({minw, i});
      continue;
    }

    int nof_lits = 0;
    for (int l : c.lits) {
      if (is_base_assumption(l)) continue;
      ++nof_lits;
    }
    if (core_relax_threshold && nof_lits > (int)core_relax_threshold) continue;
    if (log.level(5)) { log.l(5) << "c recalculate_weights add core { "; for (int l : c.lits) log.l(5) << l << ":" << ws[l] << " "; log.l(5) << "} minw = " << w << " LB: " << LB << " -> " << LB+w << "\n"; }
    LB += w;
    for (int l : c.lits) {
      if (is_base_assumption(l)) continue;
      add_weight(l, -w);
    }

    if (nof_lits == 1) continue;

    // check if there is valid totalizer for the core
    bool found_totalizer = 0;
    for (int ii : c.totalizers) {
      Totalizer& t = totalizers[ii];
      int good = 1;
      for (int l : c.lits) {
        if (is_base_assumption(l)) continue;
        if (!binary_search(t.inputs.begin(), t.inputs.end(), litNegation(l))) {
          good = 0;
          break;
        }
      }
      if (good) {
        // update weights of outputs
        for (unsigned j = 1; j < t.outputs.size(); ++j) {
          int l = litNegation(t.outputs[j]);
          add_weight(l, w);
          output_to_totalizer[l] = ii;
          lit_weight_updated(l, 1);
        }
        if (w < climit) sumw += w*(t.nof_inputs-t.outputs.size()); // if core weight is below stratification level, sumw must be handled

        found_totalizer = 1;
        break;
      }
    }
    if (!found_totalizer) {
      cores_to_relax.push_back(i);
    } else {
      empty_am1s(1);
    }
  }
  double tm = pop_clock();
  time_recalculate_weights += tm;

  log.l(1) << "c recalculate weights done, old LB = " << old_LB << " new LB  = " << LB << ", time = " << tm << "\n";
  return 0;
}



uint64_t CGSS2::gsumw() {
  uint64_t sum = 0;
  for (int l : alit_order) {
    uint64_t w = ws[l];
    if (w == 0) continue;
    if (w < climit) {
      sum += w;
      if (bit_data[l]&OUTPUT_LIT) {
        Totalizer& t = totalizers[output_to_totalizer[l]];
        if (t.minw < climit && t.outputs.back() == litNegation(l)) {
          sum += t.minw * (t.nof_inputs - t.outputs.size());
        }
      }
    }
  }
  return sum;
}

void CGSS2::init_climit(uint64_t v) {
  climit = v;
  sumw = gsumw();
}


void CGSS2::attach_clause(std::vector<int>& clause, uint64_t w, bool dimacs) {
  if (satsolver == nullptr) return;
  ++nof_clauses;

  for (int& l : clause) {
    l = lit_to_solver(l, dimacs);
  }

  if (log.level(5)) { log.l(5) << "c attach_clause { "; for (int li : clause) log.l(5) << li << " "; if (w!= HARDWEIGHT) log.l(5) << "}, w=" << w  << "\n"; else log.l(5) << "}, HARD \n"; }

  if (w == HARDWEIGHT) {
    satsolver->add_clause(clause);
    if (upsolver) upsolver->add_clause(clause);
    if (optimizer) optimizer->add_clause(clause, 0);
  } else {
    if (clause.size() == 1) {
      --nof_clauses;
      unsigned sclid = soft_clauses.size();
      reserve_vars((sclid>>1));
      soft_clauses.emplace_back(clause);
      soft_clause_weight.push_back(w);
      soft_clause_relax_lit.push_back(clause[0]);
      bit_data[sclid] |= SOFT_CL_IN_SOLVER;
      init_weight(clause[0], w+ws[clause[0]]);
    } else {
      unsigned sclid = soft_clauses.size();
      reserve_vars((sclid>>1));
      soft_clauses.emplace_back(clause);
      soft_clause_weight.push_back(w);
      soft_clause_relax_lit.push_back(0);

      if (soft_clauses.size() >= bit_data.size()) bit_data.resize(soft_clauses.size());

      if (!late_soft_cl_to_lit) {
        int r = new_var();
        init_weight(posLit(r), w);
        bit_data[posLit(r)] |= RELAX_LIT;
        lit_to_clause[posLit(r)] = sclid;
        soft_clause_relax_lit[sclid] = posLit(r);

        if (!lazy_add_softs) {
          clause.push_back(negLit(r));
          satsolver->add_clause(clause);
          if (upsolver) upsolver->add_clause(clause);
          if (optimizer) optimizer->add_clause(clause,0);
          bit_data[sclid] |= SOFT_CL_IN_SOLVER;
        } else {
          soft_clauses_not_in_solver.push_back(sclid);
          if (optimizer) {
            clause.push_back(negLit(r));
            optimizer->add_clause(clause,0);
            clause.pop_back();
          }
        }
      } else {
        soft_clauses_not_in_solver.push_back(sclid);
      }
    }
  }
}


void CGSS2::consume_clause_buffer() {
  for (unsigned i=0; i<clause_buffer.clauses.size(); ++i) {
    bool dimacs = clause_buffer.is_dimacs[i];
    for (unsigned j = 0; j<clause_buffer.clauses[i].size(); ++j) {
      attach_clause(clause_buffer.clauses[i][j], clause_buffer.weights[i][j], dimacs);
    }
  }
  clause_buffer.clear();
}


// build totalizers in ss_encoder
void CGSS2::build_totalizers(vector<vector<int> >& inputs, vector<vector<int > >& outputs, vector<SSEncoder::NodeRef>& roots) {
  push_clock();

  int toplit = vars<<1;
  encoder->relax(inputs, outputs, roots, toplit);
  double tm = pop_clock();

  time_ss_relax_calls += tm;

  log.l(3) << "c totalizers built, time used " << tm << " total time used in building totalizers: " << time_ss_relax_calls << "\n";

  toplit_from_encoder(toplit);
  add_encoder_clauses();
}

// exhaust a totalizer, start exhaustion from output bgi (default 0)
void CGSS2::exhaust_totalizer(Totalizer& t, int bgi) {
  for (int i=bgi; i<(int)t.outputs.size(); ++i) {
    int l = litNegation(t.outputs[i]);
    push_clock();
    bool rv = satsolver->solve(l);
    time_satsolver_all += get_time();
    if (!rv) {
      in_core(l, found_core(l, ws[l]));
      LB += ws[l];
      add_weight(l, -ws[l]);
      wsb[l] = ws[l];
      satsolver->add_clause(litNegation(l));
      if (upsolver) upsolver->add_clause(litNegation(l));
      int toplit = vars<<1;
      encoder->forced_true(litNegation(l), toplit);
      toplit_from_encoder(toplit);
      add_encoder_clauses();
    }
    time_exhaust_core += pop_clock();
  }
}


// build totalizers in totalizer list from index tbegin to tend-1
void CGSS2::build_totalizers(int tbegin, int tend) {
  vector<vector<int> > inputs;
  vector<vector<int> > outputs;
  vector<SSEncoder::NodeRef> roots;

  inputs.resize(tend-tbegin);
  for (int i = tbegin; i<tend; ++i) {
    swap(totalizers[i].inputs, inputs[i-tbegin]);
  }

  build_totalizers(inputs, outputs, roots);

  for (int i = tbegin; i<tend; ++i) {
    Totalizer& t = totalizers[i];
    swap(t.inputs, inputs[i-tbegin]);
    swap(t.outputs, outputs[i-tbegin]);
    t.nof_inputs = t.inputs.size();
    t.root=roots[i-tbegin];
    for (int l : t.outputs) {
      bit_data[litNegation(l)] |= OUTPUT_LIT;
      output_to_totalizer[litNegation(l)] = i;
    }
  }
}

// init new totalizer in totalizer list
int CGSS2::init_totalizer(const vector<int>& lits, uint64_t minw) {
  totalizers.emplace_back(lits, minw);
  unsigned jj = 0;
  for (unsigned ii = 0; ii < totalizers.back().inputs.size(); ++ii) {
    if (!is_base_assumption(totalizers.back().inputs[ii])) totalizers.back().inputs[jj++] = litNegation(totalizers.back().inputs[ii]);
  }
  totalizers.back().inputs.resize(jj);
  totalizers.back().nof_inputs = totalizers.back().inputs.size();
  return totalizers.size()-1;
}


bool CGSS2::relax_ss(std::vector<int>& core_set) {
  vector<int> delayed;

  totalizers.reserve(totalizers.size()+core_set.size());
  unsigned tfirst = totalizers.size();

  // filter out base assumption lits from cores and filter out too large cores from cores to be relaxed now
  unsigned j = 0;
  for (unsigned i = 0; i < core_set.size(); ++i) {
    Core& c = cores[core_set[i]];
    if (!c.weight) continue;

    unsigned nof_lits = 0;
    for (int l : c.lits) {
      if (!is_base_assumption(l)) ++nof_lits;
      wsb[l] = ws[l];
    }

    if (core_relax_threshold && nof_lits > core_relax_threshold) {
      delayed.push_back(core_set[i]);
      continue;
    }
    c.totalizers.push_back(init_totalizer(c.lits, c.weight));
    core_set[j++] = core_set[i];
  }
  core_set.resize(j);

  if (tfirst == totalizers.size()) {
    swap(core_set, delayed);
    return 0;
  }

  build_totalizers(tfirst, totalizers.size());


  // handle cores
  for (unsigned ii=0; ii<core_set.size(); ++ii) {
    Core& c = cores[core_set[ii]];
    Totalizer& t = totalizers[c.totalizers.back()];
    if (late_in_core) for (int l : c.lits) in_core(l, core_set[ii]);

    uint64_t minw = c.weight;


    int toplit = vars<<1;
    encoder->forced_true(t.outputs[0], toplit);
    toplit_from_encoder(toplit);
    add_encoder_clauses();

    // set weight to output 1
    for (j=1; j<t.outputs.size(); ++j) {
      add_weight(litNegation(t.outputs[j]), minw);
      wsb[litNegation(t.outputs[j])] = ws[litNegation(t.outputs[j])];
    }
    if (minw < climit) sumw += minw*(t.nof_inputs-t.outputs.size()); // if core weight is below stratification level, sumw must be handled

    if (exhaust) exhaust_totalizer(t, 1);
  }

  swap(core_set, delayed);
  return 1;
}

bool CGSS2::relax_fc(int core) {
  restore_weights(core, 0);
  //c.totalizers.push_back(init_totalizer(c.lits, c.weight));
  auto cmp = [this](int& a, int& b){return ws[a] < ws[b];};

  vector<int>& clits = cores[core].lits;
  sort(clits.begin(), clits.end(), cmp);
  uint64_t lw = HARDWEIGHT;
  vector<pair<vector<int>, uint64_t> > weight_groups;
  for (int l : clits) {
    if (is_base_assumption(l)) continue;
    uint64_t w = ws[l];
    if (w!=lw) {
      weight_groups.push_back({{l}, w});
      lw = w;
    } else {
      weight_groups.back().first.push_back(l);
    }
    if (late_in_core) in_core(l, core);
    add_weight(l, -ws[l]);
    wsb[l] = 0;
  }

  int lt = 0;
  for (int i = weight_groups.size()-1; i>=0; --i) {
    Core& c = cores[core];
    uint64_t minw = c.weight;
    if (i>0) minw = weight_groups[i].second - weight_groups[i-1].second;
    int toti = init_totalizer(weight_groups[i].first, minw);
    build_totalizers(toti, totalizers.size());
    Totalizer& t = totalizers[toti];

    if (i+1 < (int)weight_groups.size()) {
      vector<int> inputs;
      vector<SSEncoder::NodeRef> nodes = {totalizers[lt].root, t.root};
      vector<int> outputs;
      SSEncoder::NodeRef root;
      int toplit = vars<<1;
      encoder -> combine(inputs, nodes, outputs, root, toplit);
      toplit_from_encoder(toplit);
      add_encoder_clauses();

      swap(t.outputs, outputs);
      t.root = root;
      t.nof_inputs += totalizers[lt].nof_inputs;
    }

    for (int j=(i==0?1:0); j<(int)t.outputs.size(); ++j) {
      int nl = litNegation(t.outputs[j]);
      add_weight(nl, t.minw);
      wsb[nl] = ws[nl];
      if (t.inputs[0]!=t.outputs[j]) {
        bit_data[nl] |= OUTPUT_LIT;
        output_to_totalizer[nl] = toti;
      }
    }
    if (t.minw < climit) sumw += t.minw*(t.nof_inputs-t.outputs.size()); // if core weight is below stratification level, sumw must be handled
    lt = toti;

    if (exhaust) exhaust_totalizer(t);
  }
  return 1;
}


bool CGSS2::relax(vector<int>& core_set, bool fc_relax) {
  push_clock();
  log.l(3) << "c relax " << core_set.size() << " cores\n";
  bool rv = 0;
  stats_cores_relaxed += core_set.size();

  if (fc_relax) {
    stats_cores_relaxed_fc += core_set.size();
    for (int c : core_set) rv = relax_fc(c) || rv;
    core_set.clear();
  } else {
    rv = relax_ss(core_set);
  }

  double tm = pop_clock();
  time_relax += tm;
  log.l(3) << "c relaxing done, time used = " << tm << " total time used in relaxing: " << time_relax << ", total time used in exhausting: " << time_exhaust_core << "\n";
  return rv;
}



void CGSS2::trim_core(vector<int>& core) {
  push_clock();
  log.l(3) << "c will try to trim a core of size " << core.size() << "\n";
  stats_lits_trim_in += core.size();

  bool last_failed = 0;
  while (1) {
    unsigned sz = core.size();
    if (trim&2) {
      std::random_device rd;
      std::mt19937 g(rd());
      shuffle(core.begin(), core.end(), g);
    }
    push_clock();
    satsolver->solve(core);
    satsolver->get_core(core);
    time_satsolver_all += pop_clock();
    stats_lits_trimmed += sz - core.size();
    if (core.size() == sz) {
      if (trim&4 && !last_failed) {
        std::random_device rd;
        std::mt19937 g(rd());
        shuffle(core.begin(), core.end(), g);
        last_failed = 1;
        continue;
      }
      double tm = pop_clock();
      time_trim_core += tm;
      log.l(3) << "c trimming finished, time used = " << tm << " total trimming time = " << time_trim_core << ", total lits removed: " << stats_lits_trimmed << " / " << stats_lits_trim_in << "\n";
      return;
    } else {
      last_failed = 0;
      log.l(3) << "c trimming successful, new size " << core.size() << " \n";
    }
  }
  time_trim_core += pop_clock();
}

void CGSS2::minimize_core(vector<int>& core) {
  push_clock();

  log.l(3) << "c will try to minimize a core of size " << core.size() << "\n";
  stats_lits_min_in += core.size();

  vector<int> corelits(core);
  auto cmp = [this](int& a, int& b){return ws[a] < ws[b];};
  sort(corelits.begin(), corelits.end(), cmp);

  vector<int> assumps;

  satsolver->set_budget(1);

  if (minimize&4) {
    for (unsigned i=0; i<corelits.size(); ++i) {
      if (is_base_assumption(corelits[i])) continue;
      assumps.clear();
      int jp=-1;
      for (unsigned j=0; j<core.size(); ++j) {
        if (!core[j])               continue;
        if (corelits[i] == core[j]) jp=j;
        else                        assumps.push_back(core[j]);
      }
      if (jp==-1) continue;
      push_clock();
      int rv;
      satsolver->increase_budget(1);
      rv = satsolver->solve_limited(assumps);
      if (!rv) {
        if (minimize&2) satsolver->get_core(core);
        else            core[jp]=0;
      }
      time_satsolver_all += pop_clock();
    }
    // remove removed lits from core
    int j=0;
    for (unsigned i=0; i<core.size(); ++i) {
      if (core[i]) core[j++]=core[i];
    }
  }

  satsolver->set_budget(minimize_budget_absolute);
  satsolver->increase_budget_relative(minimize_budget_relative);
  for (unsigned i=0; i<corelits.size(); ++i) {
    if (is_base_assumption(corelits[i])) continue;
    assumps.clear();
    int jp=-1;
    for (unsigned j=0; j<core.size(); ++j) {
      if (!core[j])               continue;
      if (corelits[i] == core[j]) jp=j;
      else                        assumps.push_back(core[j]);
    }
    if (jp==-1) continue;
    push_clock();
    int rv;
    satsolver->increase_budget(minimize_budget_absolute / core.size());
    satsolver->increase_budget_relative(minimize_budget_relative / core.size());

    rv = satsolver->solve_limited(assumps);

    if (!rv) {
      if (minimize&2) satsolver->get_core(core);
      else            core[jp]=0;
    } else if (rv==1 && (minimize&8) && (bit_data[core[jp]]&OUTPUT_LIT)) {
      int l = core[jp];
      while (1) {
        in_core(l);
        Totalizer& t = totalizers[output_to_totalizer[l]];
        if (t.outputs.back() == litNegation(l)) break;
        int toplit;
        l = litNegation(encoder->next_output(litNegation(l), toplit));
        int strengthen = 1;
        while (ws[l]==0 && t.outputs.back()!=litNegation(l)) {
          l = litNegation(encoder->next_output(litNegation(l), toplit));
          ++strengthen;
        }
        if (ws[l]==0) break;

        assumps.clear();
        jp=-1;
        for (unsigned j=0; j<core.size(); ++j) {
          if (!core[j])               continue;
          if (corelits[i] == core[j]) assumps.push_back(l), jp=j;
          else                        assumps.push_back(core[j]);
        }
        if (jp==-1) break;
        satsolver->increase_budget(minimize_budget_absolute / core.size());
        satsolver->increase_budget_relative(minimize_budget_relative / core.size());

        rv = satsolver->solve_limited(assumps);
        assumps.pop_back();
        if (!rv) {
          stats_minstrengthened_outputs+=strengthen;
          if (minimize&2) satsolver->get_core(core);
          else            core[jp]=l, corelits[i]=l;
        } else break;
      }
    }
    time_satsolver_all += pop_clock();

  }


  // remove removed lits from core
  int j=0;
  for (unsigned i=0; i<core.size(); ++i) {
    if (core[i]) core[j++]=core[i];
  }
  core.resize(j);
  stats_lits_minimized += corelits.size() - core.size();

  double tm = pop_clock();
  time_minimize_core += tm;
  log.l(3) << "c minimization done, original size = " << corelits.size() << ", final size = " << core.size() << " time used = " << tm << " total minimizing time: " << time_minimize_core << " total lits removed: " << stats_lits_minimized << " / " << stats_lits_min_in << "\n";
}


int nb_tot_vars(int input_nodes, int input_vars) {
  int height = 1; while (input_nodes>>height) ++height;
  return input_vars*height + (input_nodes-(1<<(height-1)))*2;
}

bool CGSS2::push_core(std::vector<int>& core) {
  int nof_lits  = 0;
  int core_lit;
  for (int l : core) {
    if (!is_base_assumption(l)) {
      ++nof_lits;
      core_lit = l;
    }
  }

  if (nof_lits == 0) {
    log.l(3) << "c empty core after removing the assumptions\n";
    return 1;
  } else if (nof_lits==1) {
    uint64_t minw = ws[core_lit];
    harden(litNegation(core_lit), core.size()>1);
    if (core.size()>1) found_core(core, minw);
    log.l(2) << "c unit core"; log.l(4) << " {" << core_lit << "}"; log.l(2) << ", weight=" << minw << ", LB = " << LB; if (use_ub) log.l(2) << ", UB = " << UB; log.l(2) << "\n";
  } else {
    // calculate minw, handle outputs in core
    uint64_t minw = HARDWEIGHT;
    for (int l : core) {
      minw = min(minw, ws[l]);
      if (!late_in_core) in_core(l, cores.size());
    }

    LB += minw;

    if (log.level(4)) {
      log.l(4) << "c core of size " << core.size() << ": { "; for (int l : core) log.l(4) << l << ":" << ws[l] << " "; log.l(4) << "}, minw = " << minw << ", LB = " << LB; if (use_ub) log.l(4) << ", UB = " << UB; log.l(4) << "\n";
    } else {
      log.l(2) << "c core of size " << core.size() << ", minw = " << minw << ", LB = " << LB; if (use_ub) log.l(2) << ", UB = " << UB; log.l(2) << "\n";
    }

    bool fc_core = FC;
    if (AbstCG == 2) {
      vector<uint64_t> weights;
      for (int l : core) weights.push_back(AbstCG_remove_overlapping_cores ? wsb[l] : ws[l]);
      sort(weights.begin(), weights.end());
      // estimate the size of AbstCG relax vs OLL relax
      int nbw = 1;
      int ttlw = 0;
      int nbwl = 0;
      uint64_t lw = weights[0];
      uint64_t sw = 0;
      int max_nb_vars = 0;
      for (uint64_t w : weights) {
        if (w!=lw) {
          max_nb_vars += nb_tot_vars(nbwl, ttlw)-ttlw;
          //cout << "level vars: " << ttlw << ", nodes " << nbwl << ", nb vars: " <<  nb_tot_vars(nbwl, ttlw)-ttlw << endl;
          nbwl = 1;
          lw=w;
          ++nbw;
        }
        ++nbwl;
        ++ttlw;
        sw+=w;
      }
      max_nb_vars += nb_tot_vars(nbwl, ttlw);
      //cout << "top level vars: " << ttlw << ", nodes " << nbwl << ", nb vars: " <<  nb_tot_vars(nbwl, ttlw) << endl;

      int max_nb_vars_oll = nb_tot_vars(weights.size(), weights.size());
      if (nbw<=AbstCG_max_nb_weights && AbstCG_min_avg_lits_per_weight*nbw<=(int)core.size() && AbstCG_max_core_size >= (int)core.size() && AbstCG_max_nb_vars_worst_case_ratio*max_nb_vars_oll>=max_nb_vars) {
        // remove overlapping cores that are waiting to be relaxed
        log.l(4) << "c AbstCG relax triggered (" << max_nb_vars << " vs " << max_nb_vars_oll << ") \n";
        vector<int> csorted = core;
        sort(csorted.begin(), csorted.end());
        if (AbstCG_remove_overlapping_cores) {
          vector<int> cores_to_restore;
          for (int i=0; i<(int)cores_to_relax.size(); ++i) {
            Core& ccore = cores[cores_to_relax[i]];
            for (int l : ccore.lits) {
              if (binary_search(csorted.begin(), csorted.end(), l)) {
                cores_to_restore.push_back(cores_to_relax[i]);
                swap(cores_to_relax[i--], cores_to_relax.back());
                cores_to_relax.pop_back();
                break;
              }
            }
          }
          restore_weights(cores_to_restore);
        }
        // update lit weights
        LB-=minw;
        minw = HARDWEIGHT;
        for (int l : core) minw = min(minw, ws[l]);
        LB+=minw;
        fc_core = 1;
      }
    }
    for (int l : core)  {
      if (is_base_assumption(l)) continue;
      add_weight(l, -(int64_t)minw);
      if (fc_core && ws[l]) {
        cws[l]=ws[l];
        add_weight(l, -ws[l]);
      }
    }
    // save core and add it to the cores to be
    if (!fc_core) cores_to_relax.push_back(found_core(core, minw));
    else          cores_to_relax_fc.push_back(found_core(core, minw));

    if (!WCE) {
      if (!fc_core){
        log.l(2) << "c will relax a core of size " << cores[cores_to_relax.back()].lits.size() << "\n";
        relax(cores_to_relax, 0);
      } else {
        log.l(2) << "c will relax a core of size " << cores[cores_to_relax_fc.back()].lits.size() << "\n";
        relax(cores_to_relax_fc, 1);
      }
    }
  }
  return 0;
}



void CGSS2::harden(int lit, bool soft_harden) {
  int nlit = litNegation(lit);
  if (log.level(5)) log.l(5) << "c harden " << lit  << " (" << soft_harden << ") " << ws[nlit] << "\n";

  LB += ws[nlit];
  HLB += ws[nlit];

  if (!soft_harden) { // "hard hardening", nlit is a core with respect to the formula
    in_core(nlit, found_core(nlit, ws[nlit]));
  } else { // "soft hardening" nlit is not a core wrt. current formula
    in_core(nlit);
  }

  add_weight(nlit, -ws[nlit]);
  add_weight(lit, -ws[lit]);
  wsb[nlit] = wsb[lit] = 0;

  if ((bit_data[lit]&RELAX_LIT) && !(bit_data[lit_to_clause[lit]]&SOFT_CL_IN_SOLVER)) {
    // soft clause is not yet relaxed, harden the original soft clause
    if (soft_harden && hardening_lit) soft_clauses[lit_to_clause[lit]].push_back(litNegation(hardening_lit));

    satsolver->add_clause(soft_clauses[lit_to_clause[lit]]);
    if (upsolver) upsolver->add_clause(soft_clauses[lit_to_clause[lit]]);


    if (soft_harden && hardening_lit) soft_clauses[lit_to_clause[lit]].pop_back();
  } else {
    if (soft_harden && hardening_lit) {
      satsolver->add_clause({litNegation(hardening_lit), lit});
      if (upsolver) upsolver->add_clause({litNegation(hardening_lit), lit});
    } else {
      satsolver->add_clause(lit);
      if (upsolver) upsolver->add_clause(lit);
    }
  }
}

void CGSS2::calc_conns() {
  push_clock();
  // statistics variables
  int hardened = 0;
  uint64_t hardened_weight = 0;
  int unit_cores = 0;
  uint64_t unit_cores_weight = 0;
  int ttl_solver_calls = 0;
  int ttl_ups = 0;
  int final_ups = 0;
  int final_conns = 0;

  // calc edges
  vector<int> up;
  for (int l : alit_order) {
    int nl = litNegation(l);
    connsp[l].clear();
    connsn[l].clear();
    connsp[nl].clear();
    connsn[nl].clear();
    if (!ws[l]) continue;
    push_clock();
    up.clear();
    push_clock();
    bool rv = (upsolver?upsolver:satsolver)->propagate(l, up);
    time_satsolver_all += pop_clock();
    if (!rv) {
      if (is_base_assumption(l)) {
        time_calc_conns_pl += pop_clock();
        continue; // formula is UNSAT since an assumption is a unit core TODO: return 1 here and stop search
      }
      ++unit_cores;
      unit_cores_weight += ws[l];
      harden(nl);
      time_calc_conns_pl += pop_clock();
      continue;
    }
    ++ttl_solver_calls;
    ttl_ups += up.size();
    // filter
    for (int tl : up) {
      if (tl != l) {
        if (ws[tl])              connsp[l].push_back(tl);
        if (ws[litNegation(tl)]) connsn[l].push_back(tl);
      }
    }
    final_ups += connsp[l].size() + connsn[l].size();

    time_calc_conns_pl += pop_clock();

    if (!calc_conns_all) continue;

    push_clock();
    up.clear();
    if (!(bit_data[l]&RELAX_LIT)) {
      push_clock();
      rv = (upsolver?upsolver:satsolver)->propagate(nl, up);
      time_satsolver_all += pop_clock();
      if (!rv) {
        if (is_base_assumption(nl)) {
          time_calc_conns_nl += pop_clock();
          continue; // formula is UNSAT since an assumption is a unit core TODO: return 1 here and stop search
        }
        ++hardened;
        hardened_weight += ws[l];
        harden(l);
        time_calc_conns_nl += pop_clock();
        continue;
      }
    } else {
      vector<int> cl(soft_clauses[lit_to_clause[l]]);
      for (int& l2 : cl) l2=litNegation(l2);
      cl.push_back(nl);
      push_clock();
      rv = (upsolver?upsolver:satsolver)->propagate(cl, up);
      time_satsolver_all += pop_clock();
      if (!rv) {
        ++hardened;
        hardened_weight += ws[l];
        harden(l);
        time_calc_conns_nl += pop_clock();
        continue;
      }
    }
    ++ttl_solver_calls;
    ttl_ups += up.size();
    for (int tl : up) {
      if (tl != nl) {
        if (ws[tl])              connsp[nl].push_back(tl);
        if (ws[litNegation(tl)]) connsn[nl].push_back(tl);
      }
    }
    final_ups += connsp[nl].size() + connsn[nl].size();

    time_calc_conns_nl += pop_clock();
  }
  // backwards edges
  for (int l : alit_order) {
    if (!ws[l]) continue;
    int nl = litNegation(l);
    for (int c : connsp[l])  connsn[litNegation(c)].push_back(nl);
    for (int c : connsn[l])  connsn[litNegation(c)].push_back(nl);
    for (int c : connsp[nl]) connsp[litNegation(c)].push_back(l);
    for (int c : connsn[nl]) connsp[litNegation(c)].push_back(l);
  }
  // remove duplicates
  for (int l : alit_order) {
    if (!ws[l]) continue;
    int nl = litNegation(l);
    sort(connsp[l].begin(), connsp[l].end());  connsp[l].erase(unique(connsp[l].begin(), connsp[l].end()), connsp[l].end());
    sort(connsn[l].begin(), connsn[l].end());  connsn[l].erase(unique(connsn[l].begin(), connsn[l].end()), connsn[l].end());
    sort(connsp[nl].begin(), connsp[nl].end());connsp[nl].erase(unique(connsp[nl].begin(), connsp[nl].end()), connsp[nl].end());
    sort(connsn[nl].begin(), connsn[nl].end());connsn[nl].erase(unique(connsn[nl].begin(), connsn[nl].end()), connsn[nl].end());
    final_conns += connsp[l].size() + connsn[l].size() + connsp[nl].size() + connsn[nl].size();
  }


  log.l(2) << "c calc_conns finished, checks: " << ttl_solver_calls << ", avgups1: " << ttl_ups/(double)ttl_solver_calls
           << " avgups2: " << final_ups/(double)ttl_solver_calls << ", avgconns: " << final_conns/(double)ttl_solver_calls
           << ", hardened: " << hardened << ", wght = " << hardened_weight << ", unit cores: " << unit_cores << ", wght = " << unit_cores_weight << "\n";

  time_calc_conns += pop_clock();
}

bool CGSS2::try_am1s() {
  push_clock();
  // statistic vars
  int found_am1s = 0;
  int am1s_sizes = 0;
  uint64_t total_weight = 0;
  uint64_t weight_increased = 0;

  vector<int> lits;
  for (int l : alit_order) {
    if ((ws[l] >= climit || (!am1s_strat && ws[l])) && connsn[l].size()) {
      lits.push_back(l);
    }
  }


  auto cmp_conns_increasing = [this](int& a, int& b){return connsn[a].size() < connsn[b].size();};
  auto cmp_conns_decreasing = [this](int& a, int& b){return connsn[a].size() > connsn[b].size();};
  auto cmp_cost_increasing = [this](int& a, int& b){return ws[a] < ws[b];};
  auto cmp_cost_decreasing = [this](int& a, int& b){return ws[a] > ws[b];};

  auto cmp_conns_increasing_n = [this](int& a, int& b){return connsn[litNegation(a)].size() < connsn[litNegation(b)].size();};
  auto cmp_conns_decreasing_n = [this](int& a, int& b){return connsn[litNegation(a)].size() > connsn[litNegation(b)].size();};
  auto cmp_cost_increasing_n = [this](int& a, int& b){return ws[litNegation(a)] < ws[litNegation(b)];};
  auto cmp_cost_decreasing_n = [this](int& a, int& b){return ws[litNegation(a)] > ws[litNegation(b)];};

  bool done = 0;
  int iters = 0;
  while (!done) {
    done = 1;
    ++iters;
    if (am1s_order == 1) sort(lits.begin(), lits.end(), cmp_conns_increasing);
    if (am1s_order == 2) sort(lits.begin(), lits.end(), cmp_conns_decreasing);
    if (am1s_order == 3) sort(lits.begin(), lits.end(), cmp_cost_increasing);
    if (am1s_order == 4) sort(lits.begin(), lits.end(), cmp_cost_decreasing);

    set<int> u;
    for (unsigned i=0; i<lits.size(); ++i) {
      int l = lits[i];
      if (!(ws[l]>=climit || (!am1s_strat && ws[l])) || u.count(l)) continue;

      vector<int> am1 = {l};
      if (am1s_only_once) u.insert(l);

      if (am1s_order == 1) sort(connsn[l].begin(), connsn[l].end(), cmp_conns_increasing_n);
      if (am1s_order == 2) sort(connsn[l].begin(), connsn[l].end(), cmp_conns_decreasing_n);
      if (am1s_order == 3) sort(connsn[l].begin(), connsn[l].end(), cmp_cost_increasing_n);
      if (am1s_order == 4) sort(connsn[l].begin(), connsn[l].end(), cmp_cost_decreasing_n);

      uint64_t minw = ws[am1[0]];

      for (int nl2 : connsn[l]) {
        int l2 = litNegation(nl2);
        if (!(ws[l2]>=climit || (!am1s_strat && ws[l2])) || u.count(l2)) continue;
        bool can_add=1;
        for (int added : am1) {
          if (!binary_search(connsn[l2].begin(), connsn[l2].end(), litNegation(added))) {
            can_add = 0;
            break;
          }
        }
        if (can_add) {
          am1.push_back(l2);
          minw = min(minw, ws[l2]);
          if (am1s_only_once) u.insert(l2);
        }
      }
      if (am1s_order) sort(connsn[l].begin(), connsn[l].end());

      if (am1.size()>1) {
        if (minw == HARDWEIGHT) {
          time_am1s += pop_clock();
          return 1;
        }
        unsigned am1sz = am1.size();
        done = 0;
        if (!greedy_costs) {
          LB += minw*(am1sz-1);
          HLB += minw*(am1sz-1);

          for (int l2 : am1) {
            if (is_base_assumption(l2)) continue;
            add_weight(l2, -minw);
            wsb[l2] = ws[l2];
            in_core(l2);
          }


          int rl = new_var();

          if (log.level(5)) { log.l(5) << "c am1: { "; for (int l2 : am1) log.l(5) << l2 << " "; log.l(5) << "} relax lit: " << negLit(rl) << "\n"; }

          am1.push_back(negLit(rl));
          add_weight(posLit(rl), minw);
          wsb[posLit(rl)] = ws[posLit(rl)];
          satsolver->add_clause(am1);
          if (upsolver) upsolver->add_clause(am1);

          // save am1 constraint
          am1.pop_back();
          found_am1(am1, posLit(rl));
        } else {
          uint64_t maxw = ws[am1[0]];
          uint64_t LB_without_greedy_cost = LB + minw*(am1sz-1);
          for (int l2 : am1) {
            maxw = max(maxw, ws[l2]);
            LB += ws[l2];
            HLB += ws[l2];
          }
          LB -= maxw;
          HLB -= maxw;
          stats_greedy_extra_cost = LB - LB_without_greedy_cost;

          for (int l2 : am1) {
            if (is_base_assumption(l2)) continue;
            if (ws[l2] != maxw) add_weight(litNegation(l2), maxw - ws[l2]);
            add_weight(l2, -ws[l2]);
            wsb[l2] = ws[l2];
            in_core(l2);
          }

          int rl = new_var();
          am1.push_back(negLit(rl));
          add_weight(posLit(rl), maxw);
          wsb[rl] = ws[rl];
          satsolver->add_clause(am1);
          if (upsolver) upsolver->add_clause(am1);

          // save found am1 constraint
          am1.pop_back();
          found_am1(am1, posLit(rl));
        }
        ++found_am1s;
        am1s_sizes += am1sz;
        total_weight += minw;
        weight_increased += minw*(am1sz-1);
      }
    }
    if (!am1s_reiter) break;
  }
  log.l(2) << "c found " << found_am1s << " am1s, iters: " << iters << " avg size: " << am1s_sizes/(double)found_am1s
           << " avg weight: " << total_weight/(double)found_am1s << " total weight increase: " << weight_increased << "\n";
  time_am1s += pop_clock();
  return 0;
}



bool CGSS2::next_strat_level() {
  if (climit == 1 && (cores_to_relax.empty() && cores_to_relax_fc.empty())) return 0;

  // handle delayed core relaxation
  if (climit == 1) {
    int minsz = cores_to_relax.size()?cores[cores_to_relax[0]].lits.size():cores[cores_to_relax_fc[0]].lits.size();
    for (int i : cores_to_relax) {
      Core& c = cores[i];
      minsz = min(minsz, (int)c.lits.size());
    }
    for (int i : cores_to_relax_fc) {
      Core& c = cores[i];
      minsz = min(minsz, (int)c.lits.size());
    }
    core_relax_threshold = minsz;
    if (core_relax_delay_strategy == 1) {
      init_climit(HARDWEIGHT);
    } else return 1;
  } else if (cores_to_relax.size() || cores_to_relax_fc.size()) {
    if (core_relax_delay_strategy == 2) {
      int minsz = cores_to_relax.size()?cores[cores_to_relax[0]].lits.size():cores[cores_to_relax_fc[0]].lits.size();
      for (int i : cores_to_relax) {
        Core& c = cores[i];
        minsz = min(minsz, (int)c.lits.size());
      }
      for (int i : cores_to_relax_fc) {
        Core& c = cores[i];
        minsz = min(minsz, (int)c.lits.size());
      }
      core_relax_threshold = minsz;
    } else if (core_relax_delay_strategy == 3) {
      core_relax_threshold += core_relax_threshold0;
    } else if (core_relax_delay_strategy == 4) {
      core_relax_threshold *= 2;
    } else if (core_relax_delay_strategy == 5) {
      int minsz = cores_to_relax.size()?cores[cores_to_relax[0]].lits.size():cores[cores_to_relax_fc[0]].lits.size();
      for (int i : cores_to_relax) {
        Core& c = cores[i];
        minsz = min(minsz, (int)c.lits.size());
      }
      for (int i : cores_to_relax_fc) {
        Core& c = cores[i];
        minsz = min(minsz, (int)c.lits.size());
      }
      core_relax_threshold = minsz;
      return 1;
    }
  } else if (core_relax_delay_strategy == 5) {
    core_relax_threshold = core_relax_threshold0;
  }

  if (!strat && !blo && !distance_based_strat) {
    init_climit(1);
    return 1;
  }

  map<uint64_t, int, greater<uint64_t> > nof_lits_on_weight;

  int numc = 0;
  int levels_left = 0;
  for (int l : alit_order) {
    uint64_t w = ws[l];
    if (w == 0) continue;
    if (w < climit) {
      ++numc;
      if (!nof_lits_on_weight.count(w)) ++levels_left;
      if (bit_data[l]&OUTPUT_LIT) {
        Totalizer& t = totalizers[output_to_totalizer[l]];
        uint64_t cw = t.minw;
        if (cw < climit && t.outputs.back() == litNegation(l)) {
          nof_lits_on_weight[cw] += t.nof_inputs - t.outputs.size();
        }
      }
    }
    nof_lits_on_weight[w] += 1;
  }
  int total_levels = nof_lits_on_weight.size();

  uint64_t sumw_selected = 0;
  int numc_selected = 0;

  uint64_t climit_orig = climit;

  while (1) {
    // add next soft literals
    auto a = nof_lits_on_weight.upper_bound(climit);
    if (a == nof_lits_on_weight.end()) {
      climit = 1;
      log.l(3) << "c strat weight set to 1\n";
      return 1;
    }
    uint64_t old_climit = climit;

    climit = a -> first;
    if (climit_orig == HARDWEIGHT) climit_orig = climit;

    sumw -= a->second*climit;
    numc -= a->second;
    sumw_selected += a->second*climit;
    numc_selected += a->second;

    if (--levels_left == 0) {
      climit = 1;
      log.l(3) << "c strat weight set to 1\n";
      return 1;
    }

    // distance based stratification
    if (distance_based_strat && abs((int64_t)climit*numc*numc_selected - (int64_t)sumw_selected*numc) >= abs((int64_t)climit*numc*numc_selected - (int64_t)sumw*numc_selected)) {
      sumw += a->second*climit;
      climit = old_climit;
      log.l(3) << "c distance based stratification rule set strat weight to " << climit << "\n";
      return 1;
    }

    // blo and strat
    if (blo && climit>sumw) {
      log.l(3) << "c BLO rule set strat weight to " << climit << "\n";
      return 1;
    }

    if (strat &&  2 * numc > levels_left*total_levels) {
      log.l(3) << "c strat rule set strat weight to " << climit << "\n";
      return 1;
    }

    if (exp_weight_strat && climit*2 < climit_orig) {
      log.l(3) << "c strat rule exp weight set strat weight to " << climit << "\n";
      return 1;
    }
  }
  return 1;
}

void CGSS2::extract_model(bool check_cost) {
  push_clock();
  vector<bool> model;
  satsolver->get_model(model);

  if (check_cost) {
    uint64_t cost = 0;
    set<int> used_relax_lits;
    // check soft clauses that are satisfied
    for (unsigned i = 0; i<soft_clauses.size(); ++i) {
      vector<int>& cl = soft_clauses[i];
      bool sat = 0;
      for (int l : cl) {
        unsigned v = litVariable(l);
        if (v < model.size() && model[v] == !(l&1)) {
          sat = 1;
          break;
        }
      }
      if (!sat) {
        cost += soft_clause_weight[i];
      }

      unsigned l = soft_clause_relax_lit[i];
      unsigned v = litVariable(l);
      if (v < model.size() && model[v] == (bool)(l&1)) {
        if (used_relax_lits.count(l)) continue;
        used_relax_lits.insert(l);
      }
    }

    // check weights of lits not already handled when checking soft clauses
    for (unsigned v = 1; v < model.size(); ++v) {
      int l = model[v]?negLit(v):posLit(v);
      if (used_relax_lits.count(l)) continue;
      cost += wi[l];
    }

    log.l(1) << "c model found with cost " << cost << ", current LB = " << LB << ", UB = " << UB << "\n";
    if (cost<UB) {
      UB = cost;
      log.l(2) << "c model selected\n";
    } else {
      log.l(2) << "c model ignored, since cost >= UB\n";
      time_extract_model += pop_clock();
      return;
    }
  }

  vector<int> model_lits;
  for (unsigned var = 1; var<model.size(); ++var) { // loop from 1 since var 0 is never used
    int lit;
    if (model[var]) lit = posLit(var);
    else            lit = negLit(var);
    int l = lit_from_solver(lit, 0);
    if (l != -1) { // lit was in the original formula
      model_lits.push_back(l);
    }
  }
  if (!model_lits.size()) {
    time_extract_model += pop_clock();
    return;
  }

  sort(model_lits.begin(), model_lits.end());
  best_model.clear();
  // todo: variables 0... var(model_lits[0])
  best_model.push_back(model_lits[0]);
  for (unsigned i = 1; i < model_lits.size(); ++i) {
    while (litVariable(best_model.back()) + 1 < litVariable(model_lits[i])) {
      best_model.push_back(posLit(litVariable(best_model.back())+1)); // add variables that were missing on the clauses
    }
    if (litVariable(best_model.back()) >= litVariable(model_lits[i])) log.l(0) << "c bug in model extraction\n";
    best_model.push_back(model_lits[i]);
  }
  time_extract_model += pop_clock();
}

void CGSS2::try_hardening() {
  uint64_t UB2 = sumw;

  for (int ci : cores_to_relax) {
    Core& c = cores[ci];
    UB2 += c.weight * (c.lits.size()-1);
  }
  for (int ci : cores_to_relax_fc) {
    Core& c = cores[ci];
    UB2 += c.weight * (c.lits.size()-1);
    for (int l : c.lits) UB2+=cws[l];
  }


  int hardened = 0;
  uint64_t hardened_weight = 0;
  for (int l : alit_order) {
    if (is_base_assumption(l)) continue;
    if (ws[l]>UB2 || ws[l]>UB-LB) {
      ++hardened;
      hardened_weight += ws[l];
      harden(l, true);
    }
  }
  int hardened2 = 0;
  if (hardening == 2) {
    for (int l1 : alit_order) {
      if (is_base_assumption(l1) || !ws[l1]) continue;
      for (int l2 : alit_order) {
        if (l2<=l1 || !ws[l2] || is_base_assumption(l2)) continue;
        if (ws[l1]+ws[l2] > UB2 || ws[l1]+ws[l2] > UB-LB) {
          if (hardened_pairs.count({l1, l2})) continue;
          hardened_pairs.insert({l1, l2});
          ++hardened2;
          if (hardening_lit) {
            satsolver -> add_clause({l1, l2, litNegation(hardening_lit)});
          } else {
            satsolver -> add_clause({l1, l2});
          }
        }
      }
    }
  }
  log.l(2) << "c hardened " << hardened << ", " << hardened2 << " total weight = " << hardened_weight << "\n";
}


void CGSS2::restart() {
  uint64_t old_LB = LB;
  cores_to_relax.clear();
  cores_to_relax_fc.clear();
  core_relax_threshold = core_relax_threshold0;
  recalculate_weights();
  init_climit(HARDWEIGHT);
  log.l(1) << "c restart, LB was " << old_LB << " new LB: " << LB << "\n";
}


bool CGSS2::get_next_assumptions(vector<int>& assumptions) {
  assumptions = base_assumptions;
  for (int l : alit_order) {
    if (is_base_assumption(l)) continue;
    if (ws[l] < climit) continue;
    assumptions.push_back(l);
    prepare_assumption(l);
  }
  auto cmp_cost_decreasing = [this](int& a, int& b){return ws[a] > ws[b];};
  if (order_assumptions) sort(assumptions.begin()+base_assumptions.size(), assumptions.end(), cmp_cost_decreasing);
  return 1;
}

int CGSS2::_solve() {
  push_clock();
  vector<int> core;
  vector<int> assumptions;
  vector<uint64_t> weights;
  vector<long long> weightsll;
  vector<int>      weightlits;
  // init weighted lits
  weighted_lits.clear();
  weighted_lits.reserve(alit_order.size() + base_assumptions.size());
  weights.reserve(alit_order.size());
  if (optimizer) {
    weightsll.reserve(alit_order.size());
    weightlits.reserve(alit_order.size());
  }
  for (unsigned l = 0; l < ws.size(); ++l) {
    if (ws[l]) {
      weighted_lits.push_back(l);
      if (ws[l]!=HARDWEIGHT) {
        weights.push_back(ws[l]);
        if (optimizer) {
          weightsll.push_back(ws[l]);
          weightlits.push_back(-toDimacs(l));
        }
        wsb[l] = ws[l];
      }
    }
  }
  if (optimizer) optimizer->set_objective(weightsll, weightlits);

  if (optimizer && optimizer_time) {
    // TODO: base_assumptions to optimizer
    vector<int> model;
    push_clock();
    log.l(3) << "c will run ip solver for " << optimizer_time << " seconds\n";
    int v = optimizer->optimize_limited(model, optimizer_time);
    double tm = pop_clock();
    if (v==2 || v==0) {
      log.l(3) << "c ip solver solved instance in " << tm << " seconds\n";
      if (v==0) {
        pop_clock();
        return 0;
      }
      vector<int> model_lits;
      uint64_t cost = LB;
      for (int lit : model) {
        lit = fromDimacs(-lit);
        cost += ws[lit];
        int l = lit_from_solver(lit, 0);
        if (l != -1) { // lit was in the original formula
          model_lits.push_back(l);
        }
      }
      sort(model_lits.begin(), model_lits.end());
      LB=UB=cost;
      swap(model_lits, best_model);
      pop_clock();
      return 1;
    } else {
      log.l(3) << "c ip solver did not solve the instance in " << tm << " seconds (" << v << ")\n";
    }
  }

  nof_weights = avg_weight = med_weight = 0;
  if (weights.size()) {
    sort(weights.begin(), weights.end());
    uint64_t sumwghts=0;
    uint64_t lw = 0;
    for (uint64_t w : weights) {
      sumwghts+=w;
      if (w!=lw) {
        lw=w;
        ++nof_weights;
      }
    }
    avg_weight = ((long double)(sumwghts))/(weights.size());
    med_weight = (weights.size()&1) ? weights[weights.size()/2] :((long double)weights[weights.size()/2] + weights[weights.size()/2+1])/2;
  }
  if (AbstCG==1) {
    if (nof_weights>=AbstCG_nof_weights_limit && med_weight>=AbstCG_med_weights_limit) FC=1;
  }



  while (next_strat_level()) {
    log.l(3) << "c\n"; log.l(2) << "c\n";
    log.l(1) << "c new strat level, limit = " << climit << ", current LB = " << LB << ", core_relax_threshold = " << core_relax_threshold << "\n";
    strat_limits.push_back(climit);

    calc_conns();
    if (am1s) try_am1s();

    while (1) {
      if (LB == UB) {
        log.l(1) << "c search ended since LB==UB\n";
        pop_clock();
        return 1;
      }

      if (!get_next_assumptions(assumptions)) {
        log.l(1) << "c search ended since LB==UB\n";
        pop_clock();
        return 0;
      }


      if (!assumptions.size()) {
        if (cores_to_relax.size() || cores_to_relax_fc.size()) {
          // relax cores
          ++stats_wce_relax1;
          wce_iters.push_back(cores.size());
          log.l(2) << "c will relax " << cores_to_relax.size() << " + " << cores_to_relax_fc.size() << " cores\n";
          bool relaxed = relax(cores_to_relax, 0);
          relaxed = relax(cores_to_relax_fc, 1) || relaxed;
          if (!relaxed) {
            log.l(2) << "c nothing was relaxed\n";
            break;
          }
          continue;
        }

        if (sumw == 0) {
          // search finished
          log.l(2) << "c number of assumptions zero and no softs left, climit = " << climit << "\n";
          if (satsolver->solve()) {
            extract_model(1);
            pop_clock();
            return 1;
          } else {
            pop_clock();
            return 0;
          }
        }

        // go to the next strat level
        log.l(3) << "c number of assumptions zero\n";
        break;
      }


      if (log.level(4)) {
        log.l(4) << "c\nc solve, assumptions (" << assumptions.size() << ") : { "; for (int l : assumptions) log.l(4) << l << ":" << ws[l] << " "; log.l(4) << "}, LB = " << LB << ", climit = " << climit << "\n";
      } else log.l(3) << "c\nc solve, number of assumptions = " << assumptions.size() << ", current LB = " << LB << ", climit = " << climit << "\n";

      ++stats_iters;

      bool rv = 0;
      double tms = 0;
      for (int ci = 0; ci<multisolve_solves; ++ci) {
        if (ci) {
          std::random_device rd;
          std::mt19937 g(rd());
          shuffle(assumptions.begin(), assumptions.end(), g);
        }
        push_clock();
        rv = satsolver->solve(assumptions);
        double tm = pop_clock();
        time_satsolver += tm;
        time_satsolver_all += tm;
        tms += tm;
        if (rv) break;
        if (!ci || satsolver->core_size() < (int)core.size()) {
          satsolver->get_core(core);
        }
      }

      if (rv) {
        log.l(3) << "c sat-solver returned SAT in " << tms << "s, total satsolver time: " << time_satsolver << ", " << time_satsolver_all << " \n";
        if (use_ub) extract_model(1);

        if (cores_to_relax.size() || cores_to_relax_fc.size()) {
          ++stats_wce_relax2;
          wce_iters.push_back(cores.size());
          log.l(2) << "c will relax " << cores_to_relax.size() << " + " << cores_to_relax_fc.size() << " cores\n";
          bool relaxed = relax(cores_to_relax, 0);
          relaxed = relax(cores_to_relax_fc, 1) || relaxed;
          if (!relaxed) {
            log.l(2) << "c nothing was relaxed\n";
          } else {
            continue;
          }
        }

        if (hardening && sumw) try_hardening();
        if (sumw == 0 && !use_ub) extract_model(1);
        break;
      } else {
        log.l(3) << "c sat-solver returned UNSAT, time used " << tms << "s, total satsolver time: " << time_satsolver << ", " << time_satsolver_all << " \n";

        ++stats_cores;
        if (core.empty()) {
          log.l(2) << "c sat solver returned empty core\n";
          pop_clock();
          return 0;
        }
        if (trim)                    trim_core(core);
        if (minimize)                minimize_core(core);

        if (core.empty()) {
          log.l(2) << "c core empty after minimization\n";
          pop_clock();
          return 0;
        }

        if (push_core(core)) {
          pop_clock();
          return 0;
        }
      }
    }
  }
  pop_clock();
  return 1;
}






void CGSS2::prepare_for_solving() {
  if (incremental) {
    sort(base_assumptions.begin(), base_assumptions.end());
    hardening_lit = posLit(new_var());
    base_assumptions.push_back(hardening_lit);

    // reset all weights
    for (unsigned l = 0; l < wi.size(); ++l) {
      init_weight(l, wi[l]);
    }

    // reset alit_order
    unsigned j = 0;
    for (unsigned i = 0; i < alit_order.size(); ++i) {
      if (wi[alit_order[i]]) {
        alit_order[j++] = alit_order[i];
      }
    }
    alit_order.resize(j);

    for (int l : base_assumptions) {
      set_weight(l, HARDWEIGHT);
    }
  }

  // assign soft literals to clauses that don't have them yet
  for (int i : soft_clauses_not_in_solver) {
    if (!soft_clause_relax_lit[i]) {
      int r = new_var();
      init_weight(posLit(r), soft_clause_weight[i]);
      bit_data[posLit(r)] |= RELAX_LIT;
      lit_to_clause[posLit(r)] = i;
      soft_clause_relax_lit[i] = posLit(r);
    }
  }
  // add soft clauses to satsolver unless lazy adding is used
  if (!lazy_add_softs) {
    for (int i : soft_clauses_not_in_solver) {
      vector<int> cl(soft_clauses[i]);
      int rl = soft_clause_relax_lit[i];
      cl.push_back(litNegation(rl));
      satsolver->add_clause(cl);
      if (upsolver) upsolver->add_clause(cl);
      if (optimizer) optimizer->add_clause(cl, 0);
      bit_data[i] |= SOFT_CL_IN_SOLVER;
    }
    soft_clauses_not_in_solver.clear();
  } else if (optimizer) {
    for (int i : soft_clauses_not_in_solver) {
      vector<int> cl(soft_clauses[i]);
      int rl = soft_clause_relax_lit[i];
      cl.push_back(litNegation(rl));
      optimizer->add_clause(cl, 0);
    }
  }

  // init stratification, LB and UB
  init_climit(HARDWEIGHT);
  HLB = 0;
  LB = 0;
  UB = HARDWEIGHT;

  cores_to_relax.clear();
  cores_to_relax_fc.clear();

  remove_contradictory_assumptions();
}



void CGSS2::preprocess() {
#ifdef MAXPRE
  push_clock();

  log.l(1) << "c initializing MaxPRE at " << get_time(0) << " s\n";

  clause_buffer.squash();
  preprocessori = new maxPreprocessor::PreprocessorInterface(clause_buffer.clauses[0], clause_buffer.weights[0], HARDWEIGHT);

  log.l(1) << "c start preprocessing\n";
  preprocessori->preprocess(preprocess_techniques, log.level(), preprocess_time);
  log.l(2) << "c preprocessing ok, getting instance from MaxPRE\n";

  clause_buffer.is_dimacs[0] = 0;
  preprocessori->_getInstance(clause_buffer.clauses[0], clause_buffer.weights[0]);

  double tm = pop_clock();
  time_preprocess += tm;
  log.l(1) << "c preprocessing done at " << get_time(0) << " s, time used in preprocessing: " << tm << " s\n";
#endif
}

void CGSS2::reconstruct() {
#ifdef MAXPRE
  best_model = preprocessori->reconstruct(best_model, 0);
#endif
}





// public interface functions




int CGSS2::solve() {
  log.l(1) << "c =================== CGSS2::solve =================== (" << (uint64_t)(this) << ")\n";
  log.l(2) << "c\n";
  if (satsolver == nullptr) return -1;
  push_clock();

  if (do_preprocess) preprocess();

  unsigned nbclauses = nof_clauses + clause_buffer.nof_clauses();
  if (optimizer && (vars >= 100000 || nbclauses >= 150000 || (long long)vars*(long long)nbclauses>=1000000000ll)) { // TODO make parameters
    log.l(3) << "c disable optimizer by heuristics\n";
    optimizer=nullptr;
    // TODO fix destructor
    //delete optimizer;
  }

  consume_clause_buffer();
  prepare_for_solving();
  int rv = _solve();
  base_assumptions.clear();
  if (rv && LB != UB) {
    log.l(0) << "c bug, UB and LB didn't match: LB = " << LB << ", UB = " << UB << "\n";
  }

  if (rv && do_preprocess) reconstruct();

  double tm = pop_clock();
  if (clocks.size()) log.l(0) << "c bug clocks.size() = " << clocks.size() << "\n";
  time_solve += tm;
  log.l(2) << "c\n"; log.l(1) << "c CGSS2::solve finished in " << tm << "s, total solve time is now: " << time_solve << " LB = " << LB << "\n"; log.l(2) << "c\n"; log.l(3) << "c\n";
  return rv;
}

int CGSS2::solve(vector<int>& assumptions) {
  swap(assumptions, base_assumptions);
  return solve();
}




uint64_t CGSS2::get_optimal_cost() {
  return UB;
//  return LB;
}

void CGSS2::get_model(std::vector<int>& model, bool to_dimacs) {
  model.clear();
  model.reserve(best_model.size());
  for (int l : best_model) {
    if (to_dimacs) model.push_back(toDimacs(l));
    else           model.push_back(l);
  }
}

void CGSS2::_get_model(std::vector<int>& model, bool to_dimacs) {
  model.swap(best_model);
  if (to_dimacs) {
    for (int& l : model) {
      l = toDimacs(l);
    }
  }
}

void CGSS2::_add_clause(vector<int>& clause, uint64_t weight, bool dimacs) {
  clause_buffer._push(clause, weight, dimacs);
}

void CGSS2::_add_clauses(vector<vector<int> >& clauses, vector<uint64_t>& weights, bool dimacs) {
  clause_buffer._push(clauses, weights, dimacs);
}


void CGSS2::add_clause(vector<int> clause, uint64_t weight, bool dimacs) {
  _add_clause(clause, weight, dimacs);
}

void CGSS2::add_clauses(vector<vector<int> > clauses, vector<uint64_t> weights, bool dimacs) {
  _add_clauses(clauses, weights, dimacs);
}


void CGSS2::set_lit_weight(int lit, uint64_t weight, bool dimacs) {
  lit = lit_to_solver(lit, dimacs);
  log.l(5) << "c set_lit_weight " << lit << ", " << weight << "\n";
  init_weight(lit, weight);
}


void CGSS2::make_incremental() {
  incremental = 1;
}


void CGSS2::init(Options& op) {
  Log logi = log.l(1);
  //log.l(0) << "c initialize CGSS2 solver\n";
  //log.l(0) << "c " << version() << "\n";
  string& satsolvername = op.strs["satsolver"];
  logi << "c satsolvername = \"" << satsolvername << "\"\n";
  satsolver = satsolver_create(satsolvername);
  if (satsolver == nullptr) {
    log.l(0) << "c couldn't create sat solver " << satsolvername << "!\n";
  } else {
    for (string& s : op.lists["solver-opt"]) {
      satsolver -> set_option(s);
    }
  }

  if (op.strs.count("upsolver")) {
    string& upsolvername = op.strs["upsolver"];
    if (upsolvername.size()) {
      logi << "c upsolver enabled, upsolvername = \"" << upsolvername << "\"\n";
      upsolver = satsolver_create(upsolvername);
      if (upsolver == nullptr) {
        logi << "c couldn't create up solver, up solver disabled\n";
      }
    }
  }
  if (op.strs.count("optimizer")) {
    string& optimizernm = op.strs["optimizer"];
    if (optimizernm.size()) {
      logi << "c optimizer enabled, name = \"" << optimizernm << "\"\n";
      optimizer = optimizer_create(optimizernm, op);
      if (optimizer == nullptr) logi << "c couldn't create optimizer, optimizer disabled\n";
    }
  }
  if (op.doubles.count("optimizer-time")) optimizer_time = op.doubles["optimizer-time"], logi << "c set optimizer timelimit to " << optimizer_time << "\n";
  if (op.ints.count("multisolve"))  multisolve_solves = op.ints["multisolve"], logi << "c set multisolve = " << multisolve_solves << "\n";
  if (op.bools["no-strat"])         strat=0, logi << "c disable stratification\n";
  if (op.bools["no-blo"])           blo=0, logi << "c disable BLO\n";
  if (op.bools["dis-strat"])        distance_based_strat=1, logi << "c enable distance based strat\n";
  if (op.bools["exp-weight-strat"]) exp_weight_strat=1, logi << "c enable exp weight strat rule\n";
  if (op.bools["calc-conns-all"])   calc_conns_all=1, logi << "c enable calculating all connections between soft lits\n";
  if (op.bools["no-trim"])          trim=0, logi << "c disable core trimming\n";
  if (op.bools["shuffle-trim"])     trim=2, logi << "c enable shuffle trimming\n";
  if (op.bools["shuffle-trim2"])     trim=4, logi << "c enable shuffle trimming 2\n";
  if (op.bools["fast-min"])         minimize|=4, logi << "c enable fast core preminimizing\n";
  if (op.bools["min-strengthen"])   minimize|=8, logi << "c enable core strengthening on minimization\n";
  if (op.bools["no-min"])           minimize=0, logi << "c disable core minimization\n";
  else if (op.bools["no-min-trim"]) minimize=1, logi << "c disable core minimization trimming technique\n";
  if (op.uint64s.count("minimize-budget-abs")) {minimize_budget_absolute=op.uint64s["minimize-budget-abs"]; logi << "c set minimize abs budget to " << minimize_budget_absolute << "\n";}
  if (op.doubles.count("minimize-budget-rel")) {minimize_budget_relative=op.doubles["minimize-budget-rel"]; logi << "c set minimize rel budget to " << minimize_budget_relative << "\n";}
  if (op.bools["no-exhaust"])       exhaust=0, logi << "c disable core exhaustion\n";
  if (op.ints.count("relax-threshold"))core_relax_threshold=core_relax_threshold0=op.ints["relax-threshold"], logi << "c set threshold for delaying core relaxation to " << core_relax_threshold << "\n";
  if (op.ints.count("relax-delay-strategy"))core_relax_threshold=op.ints["relax-delay-strategy"], logi << "c set relax delay strategy to " << core_relax_delay_strategy << "\n";
  if (op.bools["pair-hardening"])   hardening=2, logi << "c enable pair hardening\n";
  if (op.bools["no-hard"])          hardening=0,  logi << "c disable hardening\n";
  if (op.bools["greedy-costs"])     greedy_costs = 1, logi << "c enable greedy costs\n";
  if (op.bools["no-am1s"])          am1s=0, logi << "c disable am1s\n";
  if (op.bools["no-am1s-reiter"])   am1s_reiter=0, logi << "c disable am1s reiteration\n";
  if (op.bools["am1s-only-once"])   am1s_only_once=1, logi << "c enable am1s only once\n";
  if (op.bools["am1s-strat"])       am1s_strat=1, logi << "c enable am1s stratification\n";
  if (op.ints.count("am1s-order"))  {
    am1s_order = op.ints["am1s-order"];
    logi << "c set am1s ordering to ";
    if (am1s_order == 1)      logi << "<conns_incr>\n";
    else if (am1s_order == 2) logi << "<conns_decr>\n";
    else if (am1s_order == 3) logi << "<cost_incr>\n";
    else if (am1s_order == 4) logi << "<cost_decr>\n";
    else                      logi << "<none>\n";
  }
  if (op.bools["am1s-rc2-style"])   am1s_reiter=0, am1s_only_once=1, am1s_strat=1, am1s_order=1, logi << "c enable rc2-style am1s search\n";
  if (op.bools["no-lazy-softs"])    lazy_add_softs=0, logi << "c disable lazy soft clause adding\n";
  if (op.bools["late-soft-to-lit"]) late_soft_cl_to_lit=1, logi << "c enable postponed soft clause relaxation\n";
  if (op.bools["no-wce"])           WCE=0, logi << "c disable WCE\n";
  if (op.bools["fc"])               FC=1, logi << "c enable FC (=AbstCG on every core)\n";
  if (op.bools["abst-cg"])          AbstCG=2, logi << "c enable dymamic AbstCG\n";
  if (op.bools["abst-cg-static"])   AbstCG=1, logi << "c enable static AbstCG\n";
  if (op.bools["abst-cg-dynamic"])  AbstCG=2, logi << "c enable dynamic AbstCG\n";
  if (op.ints.count("abst-cg-nof-weights")) {AbstCG_nof_weights_limit = op.ints["abst-cg-nof-weights"]; logi << "c set static AbstCG number of weights limit to " << AbstCG_nof_weights_limit << "\n"; }
  if (op.ints.count("abst-cg-med-weights")) {AbstCG_med_weights_limit = op.ints["abst-cg-med-weights"]; logi << "c set static AbstCG median number of softs per weight limit to " << AbstCG_med_weights_limit << "\n"; }
  if (op.ints.count("abst-cg-max-nb-weights")) {AbstCG_max_nb_weights = op.ints["abst-cg-max-nb-weights"]; logi << "c set dynamic AbstCG max nb weights threshold to " << AbstCG_max_nb_weights << "\n"; }
  if (op.doubles.count("abst-cg-min-lits-per-weight")) {AbstCG_min_avg_lits_per_weight = op.doubles["abst-cg-min-lits-per-weight"]; logi << "c set dynamic AbstCG minimum average lits per weight threshold to " << AbstCG_min_avg_lits_per_weight << "\n"; }
  if (op.doubles.count("abst-cg-max-nb-vars-ratio")) {AbstCG_max_nb_vars_worst_case_ratio = op.doubles["abst-cg-max-nb-vars-ratio"]; logi << "c set dynamic AbstCG max nb vars worst case ratio threshold to " << AbstCG_max_nb_vars_worst_case_ratio << "\n";}
  if (op.ints.count("abstcg-max-core-size")) {AbstCG_max_core_size = op.ints["abst-cg-max-core-size"]; logi << "c set dynamic AbstCG max core size threshold to " << AbstCG_max_core_size << "\n";}
  if (op.bools["abst-cg-no-ocr"])   AbstCG_remove_overlapping_cores = 0, logi << "c disable overlapping core removal from AbstCG\n";
  if (op.bools["no-ss"])            SS=0, logi << "c disable SS\n";
  if (op.bools["no-ub"])            use_ub=0, logi << "c disable UB\n";
  if (op.bools["no-worder-assumps"])order_assumptions=0, logi << "c disable weight based assumption ordering\n";
  if (op.bools["no-late-in-core"])  late_in_core=0, logi << "c disable postponed in_core calls\n";
  if (op.bools["map-all"])          map_all_vars=1, logi << "c remap all variables to solver\n";

  encoder = new SSEncoder();
  if (op.bools["no-reuse"]) encoder->disable_reusing(), logi << "c disable totalizer variable reusing\n";
  if (op.bools["no-eqs"])   encoder->disable_eqs(), logi << "c disable adding equivalence clauses on totalizers\n";
  else if (op.bools["all-eqs"])   encoder->enable_full_eqs(), logi << "c enable adding all equivalence clauses on totalizers\n";
  else {
    if (op.ints.count("eqs-max-decs")) encoder ->set_add_eq_max_decs(op.ints["eqs-max-decs"]), logi << "c set eq clause heuristics thresold for the number of decisions to " << op.ints["eqs-max-decs"] << "\n";
    if (op.ints.count("eqs-max-cost")) encoder ->set_add_eq_max_cost(op.ints["eqs-max-cost"]), logi << "c set eq clause heuristics thresold for the number of additional clauses to " << op.ints["eqs-max-cost"] << "\n";
    if (op.ints.count("eqs-max-prod")) encoder ->set_add_eq_max_prod(op.ints["eqs-max-prod"]), logi << "c set eq clause heuristics thresold for the product of the number of additional clauses and decisions to " << op.ints["eqs-max-prod"] << "\n";
  }

#ifdef MAXPRE
  if (op.bools["preprocess"]) do_preprocess=1, logi << "c enable preprocessing\n";
  if (do_preprocess) {
    if (op.doubles.count("preprocess-time")) preprocess_time=op.doubles["preprocess-time"], logi << "c set preprocessing time limit to " << op.doubles["preprocess-time"] << "\n";
    if (op.strs.count("preprocess-techniques")) preprocess_techniques=op.strs["preprocess-techniques"], logi << "c set preprocess techniques to " << op.strs["preprocess-techniques"] << "\n";
  }
#endif

  logi << "c\n";
}

void CGSS2::init_default() {
  Options op;
  CGSS2::add_options(op);
  op.prepare(cerr);
  init(op);
}

void CGSS2::add_options(Options& op) {
  op.add(Options::Option::INT, "multisolve", "multisolve", 0, "multisolve solve calls", "1", 2);
  op.add(Options::Option::BOOL, "no-strat", "no-strat", 0, "don't use standard strat rule", "0", 2);
  op.add(Options::Option::BOOL, "no-blo", "no-blo", 0, "don't use BLO rule", "0", 2);
  op.add(Options::Option::BOOL, "exp-weight-strat", "exp-weight-strat", 0, "use exponential weight strat rule", "0", 2);
  op.add(Options::Option::BOOL, "dis-strat", "dis-strat", 0, "enable distance based stratification", "0", 2);
  op.add(Options::Option::BOOL, "no-trim", "no-trim", 0, "disable core trimming", "0", 1);
  op.add(Options::Option::BOOL, "shuffle-trim", "shuffle-trim", 0, "shuffle cores while trimming", "0", 1);
  op.add(Options::Option::BOOL, "shuffle-trim2", "shuffle-trim2", 0, "shuffle cores while trimming version 2", "0", 1);
  op.add(Options::Option::BOOL, "no-min", "no-min", 0, "disable core minimizing", "0", 1);
  op.add(Options::Option::BOOL, "no-min-trim", "no-min-trim", 0, "disable min-trim techinque while mimizing cores", "0", 1);
  op.add(Options::Option::BOOL, "fast-min", "fast-min", 0, "enable fast core preminimizing", "0", 1);
  op.add(Options::Option::BOOL, "min-strengthen", "min-strengthen", 0, "enable core strengthening on minimization", "0", 1);
  op.add(Options::Option::UINT64, "minimize-budget-abs", "minimize-budget-abs", 0, "set absolute budget for core minimizing sat solver calls", "1000", 1);
  op.add(Options::Option::DOUBLE, "minimize-budget-rel", "minimize-budget-rel", 0, "set relative budget for core minimizing sat solver calls", "0.01", 1);
  op.add(Options::Option::BOOL, "no-exhaust", "no-exhaust", 0, "disable core exhaustion", "0", 1);
  op.add(Options::Option::INT, "relax-threshold", "relax-threshold", 0, "delay relaxing cores if their size is larger than the given constant (0=disabled)", "0", 1);
  op.add(Options::Option::INT, "relax-delay-strategy", "relax-delay-strategy", 0, "strategy for delaying core relaxation: 0=delay until stratification is done, 1=delay until stratification done, restart stratification", "0", 1);
  op.add(Options::Option::BOOL, "calc-conns-all", "calc-conns-all", 0, "calc all conns", "0", 2);
  op.add(Options::Option::BOOL, "no-hard", "no-hard", 0, "disable hardening", "0", 1);
  op.add(Options::Option::BOOL, "pair-hardening", "pair-hardening", 0, "enable pair hardening", "0", 1);
  op.add(Options::Option::BOOL, "greedy-costs", "greedy-costs", 'g', "calculate atm1s and exhausted cores cost greedily", "0", 1);
  op.add(Options::Option::BOOL, "no-am1s", "no-am1s", 0, "disable atmst1 technique", "0", 1);
  op.add(Options::Option::BOOL, "no-am1s-reiter", "no-am1s-reiter", 0, "disable reiterating atmost1s", "0", 2);
  op.add(Options::Option::BOOL, "am1s-strat", "am1s-strat", 0, "use stratification weight limit when searching for atm1s", "0", 2);
  op.add(Options::Option::BOOL, "am1s-only-once", "am1s-only-once", 0, "allow lit only in one am1 in per iteration", "0", 2);
  op.add(Options::Option::INT, "am1s-order", "am1s-order", 0, "ordering of lits on am1 search: 0=none, 1=conns, incr, 2=conns, dec, 3=cost, incr, 4=cost, decr", "1", 2);
  op.add(Options::Option::BOOL, "am1s-rc2-style", "am1s-rc2-style", 0, "rc2-style options for finding am1s", "0", 2);
  op.add(Options::Option::BOOL, "no-lazy-softs", "no-lazy-softs", 0, "add softs immediately to solver", "0", 2);
  op.add(Options::Option::BOOL, "late-soft-to-lit", "late-softs", 0, "relax softs until every clause is added", "0", 2);
  op.add(Options::Option::BOOL, "no-wce", "no-wce", 0, "don't use weight aware core extraction", "0", 1);
  op.add(Options::Option::BOOL, "fc", "fc", 0, "use fc totalizers", "0", 2);
  op.add(Options::Option::BOOL, "abst-cg", "abst-cg", 0, "use AbstCG", "0", 1);
  op.add(Options::Option::BOOL, "abst-cg-static", "abst-cg-static", 0, "use static version of AbstCG", "0", 2);
  op.add(Options::Option::BOOL, "abst-cg-dynamic", "abst-cg-dynamic", 0, "use dynamic version of AbstCG", "0", 2);
  op.add(Options::Option::INT, "abst-cg-nof-weights", "abst-cg-nof-weights", 0, "Static AbstCG triggered when the number of weights >= value", "3", 2);
  op.add(Options::Option::INT, "abst-cg-med-weights", "abst-cg-med-weights", 0, "Static AbstCG triggered when the medium of number of softs per weight >= value", "25", 2);
  op.add(Options::Option::INT, "abst-cg-max-nb-weights", "abst-cg-max-nb-weights", 0, "Dynamic AbstCG triggered only if the number of different weights in a core<=value", "1000000000", 2);
  op.add(Options::Option::DOUBLE, "abst-cg-min-lits-per-weight", "abst-cg-min-lits-per-weight", 0, "Dynamic AbstCG triggered only when average number of lits per weight >= value", "0", 2);
  op.add(Options::Option::DOUBLE, "abst-cg-max-nb-vars-ratio", "abst-cg-max-nb-vars-ratio", 0, "Dynamic AbstCG triggered only when the worst-case increase ratio of the number of variables <= value", "1.0", 2);
  op.add(Options::Option::INT, "abst-cg-max-core-size", "abst-cg-max-core-size", 0, "Dynamic AbstCG triggered only when the size of the core <= value", "1000000000", 2);
  op.add(Options::Option::BOOL, "abst-cg-no-ocr", "abst-cg-no-ocr", 0, "Disable overlapping core removal on AbstCG", "0", 2);
  op.add(Options::Option::BOOL, "no-eqs", "no-eqs", 0, "don't add equivalence clauses on totalizers", "0", 1);
  op.add(Options::Option::BOOL, "all-eqs", "all-eqs", 0, "add all equivalence clauses on totalizers", "0", 2);
  op.add(Options::Option::INT, "eqs-max-decs", "eqs-max-decs", 0, "set eqs thresold for the number of needed decisions", "50", 1);
  op.add(Options::Option::INT, "eqs-max-cost", "eqs-max-cost", 0, "set eqs thresold for the estimated number of added clauses", "50", 1);
  op.add(Options::Option::INT, "eqs-max-prod", "eqs-max-prod", 0, "set eqs thresold for the product of needed decisions and added clauses", "2500", 1);
  op.add(Options::Option::BOOL, "no-ss", "no-ss", 0, "disable structure sharing", "0", 1);
  op.add(Options::Option::BOOL, "no-reuse", "no-reuse", 0, "disable reusing in totalizers", "0", 2);
  op.add(Options::Option::BOOL, "no-ub", "no-ub", 0, "do not extract models to use upper bounds during search", "0", 1);
  op.add(Options::Option::BOOL, "no-worder-assumps", "no-worder-assumps", 0, "don't order assumptions on weight", "0", 2);
  op.add(Options::Option::BOOL, "no-late-in-core", "no-late-in-core", 0, "call in_core immediately when lit in core", "0", 2);
  op.add(Options::Option::BOOL, "map-all", "map-all", 0, "remap all variables to solver", "0", 2);
  string satsolvers = "{ ";
  for (string solver : satsolver_available()) {
    satsolvers += "\""+solver+"\" ";
  }
  satsolvers += "}";
  op.add(Options::Option::STRING, "satsolver", "satsolver", 's', std::string("SAT-solver used. Possible values: ")+satsolvers, satsolver_default(), 0);
  op.add(Options::Option::LIST, "solver-opt", "solver-opt", 0, "set option(s) for the sat solver.", "", 1);
  op.add(Options::Option::STRING, "upsolver", "upsolver", 0, std::string("UP-solver used. Possible values: ")+satsolvers, "", 0);

  string optimizers = "{ ";
  for (string optimizer : optimizer_available()) {
    optimizers += "\""+optimizer+"\" ";
  }
  optimizers += "}";
  op.add(Options::Option::STRING, "optimizer", "optimizer", 0, std::string("Optimizer used. Possible values: ")+optimizers+" (not supported on incremental mode)", "", 1);
  op.add(Options::Option::DOUBLE, "optimizer-time", "optimizer-time", 0, "timelimit for optimizer", "400", 2);
#ifdef MAXPRE
  op.add(Options::Option::BOOL, "preprocess", "preprocess", 0, "use preprocessing", "0", 1);
  op.add(Options::Option::STRING, "preprocess-techniques", "preprocess-techniques", 0, "set maxpre technique string", "[bu]#[buvsrgc]", 1);
  op.add(Options::Option::DOUBLE, "preprocess-time", "preprocess-time", 0, "set preprocessing time limit", "120", 1);
  op.add(Options::Option::LIST, "preprocess-opt", "preprocess-opt", 0, "set option(s) for the preproessor.", "", 1);
#endif
}

void CGSS2::print_stats(std::string b, ostream& out) {
  out << fixed;
  out << b << "iters: " << stats_iters << "\n";
  out << b << "cores: " << stats_cores << "\n";
  out << b << "cores_relaxed: " << stats_cores_relaxed << "\n";
  out << b << "cores_relaxed_fc: " << stats_cores_relaxed_fc << "\n";
  out << b << "wce_iters: " <<  wce_iters.size() << "\n";
  out << b << "wce_empty_assumps: " << stats_wce_relax1 << "\n";
  out << b << "wce_sat: " << stats_wce_relax2 << "\n";
  out << b << "greedy_extra_cost: " << stats_greedy_extra_cost << "\n";
  out << b << "time_preprocess: " << time_preprocess << "\n";
  out << b << "time_reweight: " << time_reweight << "\n";
  out << b << "time_relax: " << time_relax << "\n";
  out << b << "time_ss_relax_calls: " << time_ss_relax_calls << "\n";
  out << b << "time_trim_core: " << time_trim_core << "\n";
  out << b << "time_minimize_core: " << time_minimize_core << "\n";
  out << b << "time_exhaust_core: " << time_exhaust_core << "\n";
  out << b << "time_calc_conns: " << time_calc_conns << "\n";
  out << b << "time_calc_conns_pl: " << time_calc_conns_pl << "\n";
  out << b << "time_calc_conns_nl: " << time_calc_conns_nl << "\n";
  out << b << "time_am1s: " << time_am1s << "\n";
  out << b << "time_satsolver: " << time_satsolver << "\n";
  out << b << "time_satsolver_all: " << time_satsolver_all << "\n";
  out << b << "time_recalculate_weights: " << time_recalculate_weights << "\n";
  out << b << "time_solve: " << time_solve << "\n";
  out << b << "lits_trimmed: " << stats_lits_trimmed << "\n";
  out << b << "lits_trim_in: " << stats_lits_trim_in << "\n";
  out << b << "lits_minimized: " << stats_lits_minimized << "\n";
  out << b << "lits_min_in: " << stats_lits_min_in << "\n";
  out << b << "outputs_strengthened: " << stats_minstrengthened_outputs << "\n";
  out << b << "coresizes[]: [";  for (unsigned i=0; i<cores.size(); ++i) out << (i?", ":"") << cores[i].lits.size(); out << "]\n";
  out << b << "corecosts[]: [";  for (unsigned i=0; i<cores.size(); ++i) out << (i?", ":"") << cores[i].weight; out << "]\n";
  out << b << "coretimes[]: [";  for (unsigned i=0; i<cores.size(); ++i) out << (i?", ":"") << cores[i].time; out << "]\n";
  out << b << "wce_iter_points[]: [";  for (unsigned i=0; i<wce_iters.size(); ++i) out << (i?", ":"") << wce_iters[i]; out << "]\n";
  out << b << "strat_limits[]: [";  for (unsigned i=0; i<strat_limits.size(); ++i) out << (i?", ":"") << strat_limits[i]; out << "]\n";

  satsolver->stats(b, out);
  encoder->stats(b, out);

#ifdef MAXPRE
  if (do_preprocess) preprocessori->printPreprocessorStats(out);
#endif

}
#include "compilerversion.h"

std::string CGSS2::version(int l, bool oneliner) {
  std::stringstream vs;
  if (l&1) vs << "CGSS ";
  vs << "2.2.5";
  if (l&2) vs << " (" << GIT_IDENTIFIER << ", " << __DATE__ << " " << __TIME__ << ")";
  if (l&4) {
#ifdef MAXPRE
    if (!oneliner) vs << "\n";
    else           vs << ", ";
    vs << "with " << maxPreprocessor::PreprocessorInterface::version(l>>8);
#endif
    vector<string> satsolvers = satsolver_available();
    for (string solver : satsolvers) {
        if (!oneliner) vs << "\n";
        else           vs << ", ";
        vs << "with " << satsolver_version(solver, l>>8);
    }
  }

  if (l&8) {
    if (!oneliner) vs << "\n";
    else           vs << ", ";
    vs << "Compiler: " << COMPILER;
  }
  return vs.str();
}

CGSS2::CGSS2() :
  encoder(nullptr), satsolver(nullptr), upsolver(nullptr), optimizer(nullptr),
#ifdef MAXPRE
  preprocessori(nullptr),
#endif
  hardening_lit(0),
  vars(0), nof_clauses(0),
  incremental(0), optimizer_time(0), map_all_vars(0), keep_polarities(1), late_soft_cl_to_lit(0), lazy_add_softs(1), late_in_core(1), order_assumptions(1),
  multisolve_solves(1), strat(1), blo(1), distance_based_strat(0), exp_weight_strat(0), trim(1), minimize(2),
  minimize_budget_absolute(1000), minimize_budget_relative(0.01), core_relax_threshold0(0), core_relax_threshold(0),
  core_relax_delay_strategy(0), exhaust(1), calc_conns_all(0), hardening(1), greedy_costs(0), am1s(1), am1s_reiter(1),
  am1s_only_once(0), am1s_strat(0), am1s_order(1), AbstCG(0), AbstCG_nof_weights_limit(3), AbstCG_med_weights_limit(25),
  AbstCG_max_nb_weights(10), AbstCG_min_avg_lits_per_weight(3), AbstCG_max_nb_vars_worst_case_ratio(1), AbstCG_max_core_size(30),
  AbstCG_remove_overlapping_cores(1), WCE(1), FC(0), SS(1), use_ub(1),
  do_preprocess(0), preprocess_time(120), preprocess_techniques("[bu]#[buvsrgc]"),
  climit(HARDWEIGHT), sumw(0), HLB(0), LB(0), UB(HARDWEIGHT), next_restart_time(1),
  stats_iters(0), stats_cores(0), stats_lits_trim_in(0), stats_lits_trimmed(0),
  stats_lits_min_in(0), stats_lits_minimized(0),  stats_minstrengthened_outputs(0),
  stats_wce_relax1(0), stats_wce_relax2(0), stats_cores_relaxed(0),
  stats_cores_relaxed_fc(0), stats_greedy_extra_cost(0),
  time_preprocess(0), time_reweight(0), time_relax(0), time_ss_relax_calls(0), time_trim_core(0),
  time_minimize_core(0), time_exhaust_core(0), time_calc_conns(0), time_calc_conns_pl(0), time_calc_conns_nl(0),  time_am1s(0),
  time_satsolver(0), time_satsolver_all(0), time_extract_model(0), time_recalculate_weights(0), time_solve(0)
   {
}

CGSS2::~CGSS2() {
  if (encoder) delete encoder;
  if (satsolver) delete satsolver;
#ifdef MAXPRE
  if (preprocessori) delete preprocessori;
#endif
}





// ipamir interface

const char* CGSS2::ipamir_signature() {
  static std::string ipamir_signature_string = CGSS2::version(1,1);
  return ipamir_signature_string.c_str();
}

void* CGSS2::ipamir_init() {
  CGSS2* solver = new CGSS2();
  Log log = Log(cout, -1);
  solver->set_log(log);
  solver->init_default();
  solver->make_incremental();
  return solver;
}

void CGSS2::ipamir_add_hard(int32_t l) {
  if (l) ipamir_clause.push_back(l);
  else {
    log.l(4) << "c ipamir_add_hard { "; for (int li : ipamir_clause) log.l(4) << li << " "; log.l(4) << "}\n";
    _add_clause(ipamir_clause, HARDWEIGHT, 1);
  }
}

void CGSS2::ipamir_add_soft_lit(int32_t l, uint64_t w) {
  log.l(4) << "c ipamir_add_soft " << l << ", " << w << "\n";
  init_weight(lit_to_solver(-l, 1), w);
}

void CGSS2::ipamir_assume(int32_t l) {
  log.l(4) << "c ipamir_assume " << l << "\n";
  base_assumptions.push_back(lit_to_solver(l, 1));
}

int32_t CGSS2::ipamir_solve(){
  log.l(4) << "c ipamir_solve\nc\nc\n";
  push_clock();

  consume_clause_buffer();
  prepare_for_solving();

  if (recalculate_weights()) {
    base_assumptions.clear();
    time_solve += pop_clock();
    return 20;
  }

  int s = _solve();

  base_assumptions.clear();
  time_solve += pop_clock();
  if (s == 0) return 20;
  if (s == 1) return 30;
  return 40;
}
uint64_t CGSS2::ipamir_val_obj () {
  return LB;
}

int32_t CGSS2::ipamir_val_lit (int32_t l) {
  if (binary_search(best_model.begin(), best_model.end(), fromDimacs(l)))  return l;
  return -l;
}

void CGSS2::ipamir_set_terminate(void*, int (*)(void*)) {

}
