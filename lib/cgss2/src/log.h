#ifndef LOG_H
#define LOG_H

#include <ostream>
#include <iostream>

namespace cgss2{

class Log{
private:
  std::ostream* out;
  int vlevel;
  int clevel;
public:
  template<typename T>
	Log& operator<<(T t) {
		if (clevel<=vlevel) *out << t;
    return *this;
	}

  void flush() {out->flush(); }

  int level() {return vlevel;}
  bool level(int l) {return vlevel>=l;}

  Log l(int l) {
    Log o(*this, l);
    return o;
  }

  void setv(int v){
    vlevel = v;
  }

  Log() :                        out(&std::cout), vlevel(0),        clevel(1)        { }
  Log(std::ostream& o) :         out(&o),         vlevel(0),        clevel(1)        { }
  Log(int vl) :                  out(&std::cout), vlevel(vl),       clevel(1)        { }
  Log(std::ostream& o, int vl) : out(&o),         vlevel(vl),       clevel(1)        { }
  Log(const Log& o) :            out(o.out),      vlevel(o.vlevel), clevel(o.clevel) { }
  Log(const Log& o, int cl) :    out(o.out),      vlevel(o.vlevel), clevel(cl)       { }

  Log& operator=(const Log& o) {
    out = o.out;
    vlevel = o.vlevel;
    clevel = o.clevel;
    return *this;
  }
};

}

#endif
