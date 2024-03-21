#include "cgss2.h"


extern "C" {

#include "ipamir.h"

static cgss2::CGSS2* import (void * s) { return (cgss2::CGSS2 *) s; }

const char * ipamir_signature () { return cgss2::CGSS2::ipamir_signature(); }
void * ipamir_init () { return cgss2::CGSS2::ipamir_init(); }
void ipamir_release (void * s) { delete import(s); }
void ipamir_add_hard (void * s, int32_t l) { import(s)->ipamir_add_hard(l); }
void ipamir_add_soft_lit (void * s, int32_t l, uint64_t w) { import(s)->ipamir_add_soft_lit(l,w); }
void ipamir_assume (void * s, int32_t l) { import(s)->ipamir_assume(l); }
int32_t ipamir_solve (void * s) { return import(s)->ipamir_solve(); }
uint64_t ipamir_val_obj (void * s) { return import(s)->ipamir_val_obj(); }
int32_t ipamir_val_lit (void * s, int32_t l) { return import(s)->ipamir_val_lit(l); }
void ipamir_set_terminate (void * s, void * state, int (*terminate)(void * state)) {import(s)->ipamir_set_terminate(state, terminate);}

};
