#ifndef CGSS2_H
#define CGSS2_H

#include "options.h"
#include "optimizer.h"
#include "ss_encoder.h"
#include "satsolver.h"
#include "global.h"
#include "log.h"
#include "clausebuffer.h"

#ifdef MAXPRE
#include "preprocessorinterface.hpp"
#endif

#include <vector>
#include <set>
#include <string>
#include <chrono>

namespace cgss2 {

class CGSS2 {
private:
  // totalizer encoder and satsolver
  SSEncoder* encoder;
  SATsolver* satsolver;
  SATsolver* upsolver;
  Optimizer* optimizer;
#ifdef MAXPRE
  maxPreprocessor::PreprocessorInterface* preprocessori;
#endif
  // DATA
  std::vector<unsigned char> bit_data;
  // bits for lits
  const int TO_SOLVER = 1;
  const int FROM_SOLVER = 2;
  const int WEIGHTED = 4;
  const int RELAX_LIT = 8;
  const int OUTPUT_LIT = 16;
  const int SOFT_CL_IN_SOLVER = 32;

  std::vector<int>            _lit_to_solver;
  std::vector<int>            _lit_from_solver;


  // weights and strat_weights of lits
  std::vector<int>      alit_order; // to keep assumptions in order
  std::vector<int>      weighted_lits; // lits that have weights at the beginning of the _solve call
  std::vector<uint64_t> wi;         // initial weights of soft literals
  std::vector<uint64_t> ws;         // weights
  std::vector<uint64_t> wsb;        // weights at the beginning of current wce iteration
  std::vector<uint64_t> cws;        // weights used in abstcg totalizer (ws values are set to 0)

  std::vector<int>      base_assumptions;  // assumptions on incremental solving
  int                   hardening_lit;
  std::set<std::pair<int, int> > hardened_pairs;

  // calculating implications between assumptions
  std::vector<std::vector<int> > connsp;
  std::vector<std::vector<int> > connsn;

  //  save soft clauses
  std::vector<std::vector<int> > soft_clauses;
  std::vector<uint64_t>          soft_clause_weight;
  std::vector<int>               soft_clause_relax_lit;
  std::vector<int>               soft_clauses_not_in_solver; // set of soft clauses not yet in core
  std::vector<int>               lit_to_clause; // mapping of relax lits to soft clauses


  int vars; // number of variables
  int nof_clauses; // number of clauses
  long double avg_weight; // average of soft lit weights;
  long double med_weight; // median of soft lit weights;
  int nof_weights; // number of different weights of soft lits


  // OPTIONS
  Log log;

  bool incremental;

  double optimizer_time;

  bool map_all_vars;        // don't try to keep variable names
  bool keep_polarities;     // keep polarities while mapping variables
  bool late_soft_cl_to_lit; // convert
  bool lazy_add_softs;      // add soft clause only when it is on current assumption level

  bool late_in_core; // call in_core after core is relaxed

  int order_assumptions;
  int multisolve_solves; // extract a number of cores and select best of them
  bool strat;      // use stratification
  bool blo;        // use BLO
  bool distance_based_strat; // use distance based stratification
  bool exp_weight_strat; // use exp weight strat rule
  int trim;       // use core trimming
  int  minimize;   // use core minimization
  uint64_t minimize_budget_absolute;
  double  minimize_budget_relative;
  unsigned core_relax_threshold0;
  unsigned core_relax_threshold;
  int core_relax_delay_strategy;
  bool exhaust;    // ustart_close core exhaustion
  bool calc_conns_all;
  int hardening;     // use hardening
  bool greedy_costs; // calculate cost greedily on atm1s and exhausted cores
  bool am1s;       // use atmost1 technique
  bool am1s_reiter;// reiterate on am1 search
  bool am1s_only_once; // skip lit aftout << b << "coresizes[]: [";  for (unsigned i=0; i<found_cores.size(); ++i) out << (i?", ":"") << found_cores[i].size(); out << "]\n";
  bool am1s_strat; // use stratification limit on am1 search
  int  am1s_order; // ordering of lits in am1 search

  int AbstCG;      // 0: no AbstCG (=use OLL), 1: use static AbstCG, 2: use dynamic AbstCG
  // Parameters for AbstCG = 1 (static AbstCG)
  int AbstCG_nof_weights_limit;
  int AbstCG_med_weights_limit;
  // Parameters for AbstCG = 2, (dynamic AbstCG)
  int    AbstCG_max_nb_weights;
  double  AbstCG_min_avg_lits_per_weight;
  double AbstCG_max_nb_vars_worst_case_ratio;
  int    AbstCG_max_core_size;
  bool AbstCG_remove_overlapping_cores;
  bool WCE;        // use weight aware core extraction
  bool FC;         //
  bool SS;         // use structure sharing
  bool use_ub;     // extract models to use UB in solving

  // preprocessing options
  bool do_preprocess;                 // if preprocessing is on or off
  double preprocess_time;             // time used in preprocessing
  std::string preprocess_techniques;  // techniques string for maxpre


  // current state of the solver
  uint64_t climit;
  uint64_t sumw; // sum of weights of softs that have weight<climit
  uint64_t HLB; //
  uint64_t LB;
  uint64_t UB;
  double next_restart_time;
  std::vector<int> best_model;



  struct Totalizer { // keep track of inputs and outputs
    std::vector<int> inputs;
    std::vector<int> outputs;
    SSEncoder::NodeRef root;
    int nof_inputs;
    uint64_t minw;
    Totalizer() : root(SSEncoder::undef), nof_inputs(0), minw(0) { }
    Totalizer(std::vector<int> inputs_) : inputs(inputs_), root(SSEncoder::undef), nof_inputs(inputs_.size()), minw(0) { }
    Totalizer(std::vector<int> inputs_, uint64_t minw_) : inputs(inputs_), root(SSEncoder::undef), nof_inputs(inputs_.size()), minw(minw_) { }
  };

  struct Core {
    std::vector<int> lits;
    uint64_t weight;
    std::vector<int> totalizers;
    double time;
    Core(std::vector<int>& lits_, uint64_t weight_,  double time_) : weight(weight_), time(time_) { lits_.swap(lits); }
    Core(int lit, uint64_t weight_,  double time_) : lits({lit}), weight(weight_), time(time_) { }
  };

  std::vector<Core>              cores;
  std::vector<Totalizer>         totalizers;
  std::vector<int>               output_to_totalizer; // mapping of output literals to totalizer index
  std::vector<std::vector<int> > lit_in_cores; // save cores where each lit was in

  std::vector<std::pair<std::vector<int>, int> > atmost_ones; // save atmost one constraints and relax lits
  std::vector<std::vector<int> > lit_in_am1s;                 // save atmost one constraints where each lit was in

  std::vector<int>               wce_iters; // core indices when WCE proceeded to next iteration
  std::vector<uint64_t>          strat_limits; // core indices when WCE proceeded to next iteration

  std::vector<int> cores_to_relax; // current unrelaxed cores (indices to found_cores)
  std::vector<int> cores_to_relax_fc;


  // when clauses are added, they are saved in this buffer before they are actually added to satsolver etc.
  ClauseBuffer clause_buffer;



  // measuring time
  std::vector<std::chrono::time_point<std::chrono::steady_clock> > clocks;

  // statistic vars
  uint64_t stats_iters, stats_cores,
           stats_lits_trim_in, stats_lits_trimmed, stats_lits_min_in, stats_lits_minimized,
           stats_minstrengthened_outputs,
           stats_replicated_cores, stats_replications, stats_wce_relax1, stats_wce_relax2,
           stats_cores_relaxed, stats_cores_relaxed_fc,
           stats_greedy_extra_cost;
  double time_preprocess, time_reweight, time_relax, time_ss_relax_calls,
         time_trim_core, time_minimize_core,
         time_exhaust_core, time_calc_conns, time_calc_conns_pl, time_calc_conns_nl,
         time_am1s, time_satsolver, time_satsolver_all, time_extract_model,
         time_recalculate_weights, time_solve;


  // functions to handle data structures
public:
  void reserve_vars(int nof_vars);
  void set_nof_vars(int nof_vars);
private:
  int new_var();
  int lit_to_solver(int l, bool dimacs);
  int lit_from_solver(int l, bool dimacs);

  void add_weight(int l, int64_t w);
  void set_weight(int l, uint64_t w);
  void init_weight(int l, uint64_t w);

  // functions to save cores etc.,
  inline bool is_base_assumption(int l);
  inline int found_core(std::vector<int>& lits, uint64_t weight); // empties vector lits
  inline int found_core(int lit, uint64_t weight);
  inline int found_am1(std::vector<int>& lits, int relax_lit); // empties vector lits

  inline uint64_t gminw(const std::vector<int>& lits);
  // functions for measuring time
  int push_clock();
  double get_time();
  double get_time(int i);
  double pop_clock();



  // solving
  void prepare_assumption(int l);   // called before lit will be usedd as assumption (may soft clauses to satsolver if they are added lazily)
  void in_core(int l, int i = -1);   // called when lit l was in core i (updates output lits of totalizer)

  void toplit_from_encoder(int toplit);  // update number of variables
  void add_encoder_clauses();            // adds new clauses from encoder to satsolver

  void remove_contradictory_assumptions();
  void reset_weights();
  void restore_weights(int core, bool restore_lb = 1);
  void restore_weights(std::vector<int>& cores, bool restore_lb = 1); //
  bool recalculate_weights();       // init weights and recalculate core weights. Return true if instance is proved UNSAT under assumptions

  uint64_t gsumw();
  void init_climit(uint64_t v); // updates sumw etc.

  void attach_clause(std::vector<int>& clause, uint64_t ws, bool dimacs = 0);
  void consume_clause_buffer(); // add all clauses form buffer to sat solver (this empties the buffer)
  void prepare_for_solving(); // called before _solve


  void build_totalizers(std::vector<std::vector<int> >& inputs, std::vector<std::vector<int > >& outputs, std::vector<SSEncoder::NodeRef>& roots);
  void exhaust_totalizer(Totalizer& t, int bgi = 0);
  void build_totalizers(int tbegin, int tend);

  int init_totalizer(const std::vector<int>& lits, uint64_t minw = 0);

  bool relax_ss(std::vector<int>& cores);
  bool relax_fc(int core);

  bool relax(std::vector<int>& cores, bool use_fc); // relaxes a set of cores. returns false if nothing was relaxed

  void trim_core(std::vector<int>& core);           // core trimming
  void minimize_core(std::vector<int>& core);       // core minimization
  bool push_core(std::vector<int>& core);           // core handling function, returns true core proves instance UNSAT

  void harden(int lit, bool soft_harden = false);
  void calc_conns();  // updating the implications between soft lits
  bool try_am1s(); // find and remove atmost1 constraints, returns true if instance is UNSAT

  bool next_strat_level();      // next stratification level
  void try_hardening();

  void extract_model(bool check_cost);   //

  void restart();

  bool get_next_assumptions(std::vector<int>& assumptions);
  int  _solve();               // solving function


  void preprocess();
  void reconstruct();

public:
  // public interface

  // returns 0 if solved and UNSAT
  // returns 1 if solved and SAT
  // returns -1 if not solved
  int solve();
  int solve(std::vector<int>& assumptions);


  uint64_t get_optimal_cost();
  void get_model(std::vector<int>& vars, bool dimacs = 0);
  void _get_model(std::vector<int>& vars, bool dimacs = 0); // this function swaps solvers internal model with the vector given as a parameter, after calling this solver no more has valid internal model


  // THESE FUNCTIONS EMPTY THE PARAMETER VECTORS
  void _add_clause(std::vector<int>& clause, uint64_t ws, bool dimacs = 0);
  void _add_clauses(std::vector<std::vector<int> >& clauses, std::vector<uint64_t>& weights, bool dimacs = 0);

  void add_clause(std::vector<int> clause, uint64_t ws, bool dimacs = 0);
  void add_clauses(std::vector<std::vector<int> > clauses, std::vector<uint64_t> weights, bool dimacs = 0);

  void set_lit_weight(int lit, uint64_t w, bool dimacs = 0);

  static void add_options(Options& op);

  void make_incremental();

  void init(Options& op);
  void init_default();
  void set_log(Log& nlog){log = nlog;}

  void print_stats(std::string b, std::ostream& out);

  static std::string version(int l = 263, bool oneliner = 1);


  CGSS2();
  ~CGSS2();




  // ipamir interface
  std::vector<int> ipamir_clause;

  static const char * ipamir_signature();
  static void * ipamir_init();
  void ipamir_add_hard(int32_t l);
  void ipamir_add_soft_lit(int32_t l, uint64_t w);
  void ipamir_assume(int32_t l);
  int32_t ipamir_solve();
  uint64_t ipamir_val_obj();
  int32_t ipamir_val_lit(int32_t l);
  void ipamir_set_terminate(void * state, int (*terminate)(void * state));
};

}

#endif
