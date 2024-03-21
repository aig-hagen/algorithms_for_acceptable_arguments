#ifndef GLOBAL_H
#define GLOBAL_H

namespace cgss2{

inline int toDimacs(int l) {
  return (l&1) ? -(l/2)-1 : (l/2)+1;
}

inline int fromDimacs(int l) {
  return l>0 ? (2*l-2) : (-2*l-1);
}

inline int posLit(int v) {
  return v<<1;
}

inline int negLit(int v) {
  return (v<<1)|1;
}

inline int litNegation(int l) {
  return l^1;
}

inline int litVariable(int l) {
  return l>>1;
}

const unsigned long long HARDWEIGHT = 1ull<<63;


}
#endif
