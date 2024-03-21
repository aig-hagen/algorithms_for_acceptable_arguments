#include "dimacs.h"

#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>

#ifdef ZLIB
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#endif

using namespace std;
using namespace cgss2;

void DIMACS::read_clause(istream& in, uint64_t w) {
  if (in.eof()) return;
  vector<int> clause;
  int l;
  while (in >> l) {
    if (!l) break;
    vars = max(vars, abs(l));
    clause.push_back(l);
  }
  clauses.push_back(clause);
  weights.push_back(w);
}

int DIMACS::read(istream& in) {
  clauses.clear();
  weights.clear();
  vars = 0;
  type = WPMS22;
  topv = HARDWEIGHT;
  
  int linenb = 0;
  while (in.good()) {
    ++linenb;
    in >> ws;
    if (in.peek()=='c') {
      in.ignore(numeric_limits<streamsize>::max(), '\n');
      continue;
    }
    if (in.peek()=='p') {
      string p,  tp;
      int nbcl, nbvar;
      in >> p >> tp;
      if (p!="p") {
        error="invalid p line, line " + to_string(linenb);
        return 2;
      }
      if (tp=="cnf") {
        type = SAT;
        in >> nbcl >> nbvar;
        clauses.reserve(nbcl);
        weights.reserve(nbcl);
      } else if (tp=="wcnf") {
        type = WPMS;
        in >> nbvar >> nbcl >> topv;
        clauses.reserve(nbcl);
        weights.reserve(nbcl);
        vars = nbvar;
      } else {
        error="invalid problem type specified on p line, line " + to_string(linenb);
        return 3;
      }
    } else {
      if (type == SAT) {
        read_clause(in, 1);
      } else if (in.peek() == 'h') {
        string h;
        in >> h;
        if (h!="h") {
          error="invalid h line, line "+ to_string(linenb);
          return 4;
        }
        read_clause(in);
      } else {
        uint64_t w;
        in >> w;
        if (w >= topv) w = HARDWEIGHT;
        read_clause(in, w);
      }
    }
  }
  return 0;
}

int DIMACS::read_file(string fn) {
  filename = fn;
#ifdef ZLIB
  if (fn.size()>3 && fn[fn.size()-3]=='.' && (fn[fn.size()-2]=='g' || fn[fn.size()-2]=='x' )&& fn[fn.size()-1]=='z') {
    std::ifstream f(fn, std::ios::binary);
    if (f.fail()) {
      error = "couldn't open file \"" + fn + "\"";
      return 1;
    }
    boost::iostreams::filtering_istreambuf inbf;
    if (fn[fn.size()-2]=='g') inbf.push(boost::iostreams::gzip_decompressor());
    if (fn[fn.size()-2]=='x') inbf.push(boost::iostreams::lzma_decompressor());
    inbf.push(f);
    istream in(&inbf);
    return read(in);
  }
#endif
  ifstream f(fn);
  if (f.fail()) {
    error = "couldn't open file \"" + fn +"\"";
    return 1;
  }
  return read(f);
}
