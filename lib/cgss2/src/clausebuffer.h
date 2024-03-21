#ifndef CLAUSEBUFFER_H
#define CLAUSEBUFFER_H

#include <vector>
#include <stdint.h>


namespace cgss2 {

// auxiliary structure to handle adding clauses to

class ClauseBuffer {
private:
  std::vector<std::vector<std::vector<int> > > clauses;
  std::vector<bool>                            is_dimacs;
  std::vector<std::vector<uint64_t> >          weights;
  unsigned nbclauses;
public:
  void squash();
  void _push(std::vector<int>& clause, uint64_t weight, bool is_dimacs);
  void _push(std::vector<std::vector<int> >& clauses, std::vector<uint64_t>& weights, bool dimacs);
  void clear();
  unsigned nof_clauses() {return nbclauses;}
  friend class CGSS2;
  ClauseBuffer() : nbclauses(0) {}
};


}

#endif
