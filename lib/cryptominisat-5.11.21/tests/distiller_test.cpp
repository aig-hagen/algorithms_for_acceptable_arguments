/******************************************
Copyright (C) 2009-2020 Authors of CryptoMiniSat, see AUTHORS file

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#include "gtest/gtest.h"

#include "solver.h"
#include "distillerlong.h"
#include "distillerlitrem.h"
#include "solverconf.h"
using namespace CMSat;
#include "test_helper.h"

struct distill_test : public ::testing::Test {
    distill_test()
    {
        must_inter.store(false, std::memory_order_relaxed);
        SolverConf conf;
        //conf.verbosity = 20;
        s = new Solver(&conf, &must_inter);
        distill_long_cls = s->distill_long_cls;
        distill_lit_rem = s->distill_lit_rem;
    }
    ~distill_test()
    {
        delete s;
    }

    Solver* s;
    DistillerLong* distill_long_cls;
    DistillerLitRem* distill_lit_rem;
    std::atomic<bool> must_inter;
};

//BY-BY 1

TEST_F(distill_test, long_by1)
{
    s->new_vars(4);
    s->add_clause_outside(str_to_cl("1, -2"));
    s->add_clause_outside(str_to_cl("1, 2, 3, 4"));

    distill_long_cls->distill(false);
    check_irred_cls_contains(s, "1, 3, 4");
}

TEST_F(distill_test, long_by1_transitive)
{
    s->new_vars(5);
    s->add_clause_outside(str_to_cl("1, -5"));
    s->add_clause_outside(str_to_cl("5, -2"));
    s->add_clause_outside(str_to_cl("1, 2, 3, 4"));

    distill_long_cls->distill(false);
    check_irred_cls_contains(s, "1, 3, 4");
}

TEST_F(distill_test, long_by1_2)
{
    s->new_vars(4);
    s->add_clause_outside(str_to_cl("2, -3"));
    s->add_clause_outside(str_to_cl("1, 2, 3, 4"));

    distill_long_cls->distill(false);
    check_irred_cls_contains(s, "1, 2, 4");
}

TEST_F(distill_test, long_by1_nodistill
)
{
    s->new_vars(5);
    s->add_clause_outside(str_to_cl("-1, 3"));
    s->add_clause_outside(str_to_cl("1, 2, 3, 4"));

    distill_long_cls->distill(false);
    check_irred_cls_contains(s, "1, 2, 3, 4");
}

TEST_F(distill_test, long_by1_nodistill2
)
{
    s->new_vars(5);
    s->add_clause_outside(str_to_cl("-1, -2"));
    s->add_clause_outside(str_to_cl("1, 2, 3, 4"));

    distill_long_cls->distill(false);
    check_irred_cls_contains(s, "1, 2, 3, 4");
}

TEST_F(distill_test, tri)
{
    s->new_vars(5);
    s->add_clause_outside(str_to_cl("1, 2, -3"));
    s->add_clause_outside(str_to_cl("1, 2, 3"));

    distill_long_cls->distill(false);
    check_irred_cls_contains(s, "1, 2");
}

/*
 * Can actually fail depending on literal ordering, clause skipping, etc.
 *
TEST_F(distill_test, tri_transitive)
{
    s->new_vars(5);
    s->add_clause_outside(str_to_cl("1, 2, 4"));
    s->add_clause_outside(str_to_cl("-4, -3"));
    s->add_clause_outside(str_to_cl("1, 2, 3"));

    distill_long_cls->distill(false);
    check_irred_cls_contains(s, "1, 2");
}*/

TEST_F(distill_test, remove_long)
{
    s->new_vars(10);
    s->add_clause_outside(str_to_cl("1, 5"));
    s->add_clause_outside(str_to_cl("1, 5, 6, 7"));
    check_irred_cls_contains(s, "1, 5, 6, 7");

    distill_long_cls->distill(false, true);
    check_irred_cls_doesnt_contain(s, "1, 5, 6, 7");
}

TEST_F(distill_test, remove_long_trans)
{
    s->new_vars(10);
    s->add_clause_outside(str_to_cl("1, -9"));
    s->add_clause_outside(str_to_cl("9, 6"));
    s->add_clause_outside(str_to_cl("1, 5, 6, 7"));
    check_irred_cls_contains(s, "1, 5, 6, 7");

    distill_long_cls->distill(false, true);
    check_irred_cls_doesnt_contain(s, "1, 5, 6, 7");
}

TEST_F(distill_test, litrem_1)
{
    s->new_vars(5);
    s->add_clause_outside(str_to_cl("1, 2, 3"));
    s->add_clause_outside(str_to_cl("1, -2, 3"));

    distill_lit_rem->distill_lit_rem();
    check_irred_cls_contains(s, "1, 3");
}

TEST_F(distill_test, litrem_2)
{
    s->new_vars(5);
    s->add_clause_outside(str_to_cl("1, 2, 3, 4"));
    s->add_clause_outside(str_to_cl("1, -2, 3, 4"));

    distill_lit_rem->distill_lit_rem();
    check_irred_cls_contains(s, "1, 3, 4");
}

TEST_F(distill_test, litrem_3)
{
    s->new_vars(10);
    s->add_clause_outside(str_to_cl("1, 2, 3, 4"));
    s->add_clause_outside(str_to_cl("1, -2, 3, 4"));
    s->add_clause_outside(str_to_cl("1, 2, 3, 7"));
    s->add_clause_outside(str_to_cl("1, 2, -3, 7"));

    distill_lit_rem->distill_lit_rem();
    check_irred_cls_contains(s, "1, 3, 4");
    check_irred_cls_contains(s, "1, 2, 7");
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
