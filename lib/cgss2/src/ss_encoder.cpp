#include "ss_encoder.h"
#include <queue>
#include <set>
#include <algorithm>
#include <iostream>

using namespace std;

void print_node(SSEncoder::Node& cc) {
    cout << "<";
    for (unsigned i=0;i<cc.lits.size();++i) {
        if (i) cout << ", ";
        cout << cc.lits[i];
    }cout << ">";
}


SSEncoder::NodeRef SSEncoder::create_parent(NodeRef a, NodeRef b) {
    NodeRef c=new_node();
    Node& cc=nodes[c];
    Node& aa=nodes[a];
    Node& bb=nodes[b];

    cc.build_eqs=eqtree;

    cc.l=a;
    cc.r=b;
    cc.inputs=aa.inputs+bb.inputs;

    cc.lits.push_back(add_or(aa.lits[0], bb.lits[0]));
    cc.lits.push_back(add_and(aa.lits[0], bb.lits[0]));
    cc.ll=aa.lits.size();cc.rl=bb.lits.size();

    if (!is_pmres) {
        if (aa.lits.size()>1) add_impl(aa.lits[1], cc.lits[1]);
        if (bb.lits.size()>1) add_impl(bb.lits[1], cc.lits[1]);
    }

    lit_nodes[cc.lits[0]]=c; lit_indices[cc.lits[0]]=0;
    lit_nodes[cc.lits[1]]=c; lit_indices[cc.lits[1]]=1;

    parents[pair_index(a, b)]=c;
    ++aa.parents;
    ++bb.parents;

    if (cc.build_eqs) add_equivalences(c, -1, -1);

    return c;
}



inline void SSEncoder::remove_undefs(std::vector<std::pair<NodeRef, int> >& _nodes) {
    int i=0,j=0;
    for (;i<(int)_nodes.size();++i) {
        if (_nodes[i].first==undef) continue;
        _nodes[j++]=_nodes[i];
    }
    _nodes.resize(j);
}


void SSEncoder::ask_n_outputs(NodeRef c, int n) {
    Node& cc=nodes[c];

    if (cc.inputs == (int)cc.lits.size() || (int)cc.lits.size()>=n) return;
    if (n>cc.inputs) n=cc.inputs;

    ask_n_outputs(cc.l, n);
    ask_n_outputs(cc.r, n);


    Node& l=nodes[cc.l];
    Node& r=nodes[cc.r];

    // add clauses that need to be added due to new lits in the children
    for (int li=cc.ll;li<(int)l.lits.size();++li) {
        for (int ri=-1;li+ri+1<(int)cc.lits.size() && ri<(int)r.lits.size();++ri) {
            if (ri==-1) add_impl(l.lits[li], cc.lits[li+ri+1]);
            else        add_and(l.lits[li], r.lits[ri], cc.lits[li+ri+1]);
        }
    }
    for (int ri=cc.rl;ri<(int)r.lits.size();++ri) {
        for (int li=-1;li+ri+1<(int)cc.lits.size() && li<cc.ll;++li) {
            if (li==-1) add_impl(r.lits[ri], cc.lits[li+ri+1]);
            else        add_and(l.lits[li], r.lits[ri], cc.lits[li+ri+1]);
        }
    }

    // add new lits and their clauses
    for (int i=cc.lits.size();i<n;++i) {
        int nl = new_lit();
        cc.lits.push_back(nl);

        lit_nodes[nl]=c;
        lit_indices[nl]=cc.lits.size()-1;

        for (int li=-1;li<=i && li<(int)l.lits.size();++li) {
            int ri=i-li-1;
            if (ri>=(int)r.lits.size()) continue;

            if (li==-1)      add_impl(r.lits[ri], cc.lits[i]);
            else if (ri==-1) add_impl(l.lits[li], cc.lits[i]);
            else             add_and(l.lits[li], r.lits[ri], cc.lits[i]);
        }
    }

    cc.ll=l.lits.size();
    cc.rl=r.lits.size();

    if (cc.build_eqs) add_equivalences(c, -1, -1);
}


SSEncoder::NodeRef SSEncoder::build_tree(std::vector<std::pair<NodeRef, int> >& tnodes) {
//     std::cout << "build_tree {"; for (int i=0;i<tnodes.size();++i) std::cout << tnodes[i] << " "; std::cout << "} -> ";
    if (try_reuse) build_reusables(tnodes);


    auto cmp = [this](std::pair<NodeRef, int>& a, std::pair<NodeRef, int>& b){return this->nodes[a.first].inputs == this->nodes[b.first].inputs ? a.second<b.second :this->nodes[a.first].inputs > this->nodes[b.first].inputs;};
    std::priority_queue<std::pair<NodeRef, int>, std::vector<std::pair<NodeRef, int> >, decltype(cmp)> nodeq(cmp);


    for (int i=0;i<(int)tnodes.size();++i) nodeq.push(tnodes[i]);

    // build tree and return the root
    while (nodeq.size()>1) {
        int pr=nodeq.top().second;
        NodeRef a=nodeq.top().first;nodeq.pop();
        NodeRef b=nodeq.top().first;nodeq.pop();
        nodeq.push({create_parent(a, b), pr});
    }
//     std::cout << tnodes.top() << std::endl;
    return nodeq.top().first;
}


void SSEncoder::build_reusables(std::vector<std::pair<NodeRef, int> >& tnodes) {
    for (int i=1;i<(int)tnodes.size();++i) {
        if (tnodes[i].first==undef) continue;
        for (int j=0;j<i;++j) {
            if (tnodes[j].first==undef) continue;
            NodeRef c=find_parent(tnodes[i].first, tnodes[j].first);
            if (c!=undef) {
                tnodes.emplace_back(c, 0);
                tnodes[i].first=undef;
                tnodes[j].first=undef;
                ++total_reused;
                break;
            }
        }
    }
    remove_undefs(tnodes);
}


void SSEncoder::convert_lits_to_nodes(const std::vector<int>& lits, std::vector<std::pair<NodeRef, int> >& tnodes) {
    tnodes.resize(lits.size());
    // create leaf nodes
    for (int i=0;i<(int)lits.size();++i) {
        NodeRef node=lit_leaf(lits[i]);
        if (node==undef) node=new_leaf(lits[i]);
        tnodes[i]={node, i};
    }
}




void SSEncoder::helper_build_tree(std::vector<std::vector<std::pair<NodeRef, int> > >& tnodes, int i) {
    if (tnodes[i].size()==1 && tnodes[i][0].first<indexref) return;
    //std::cout << tnodes.size() << ", " << i << std::endl;

    for (int j=tnodes[i].size()-1;j>=0 && tnodes[i][j].first>=indexref;--j) {
        int k=tnodes[i][j].first-indexref;
        if (tnodes[k].size()>1 || tnodes[k][0].first>=indexref) helper_build_tree(tnodes, k);
        if (k>(int)tnodes.size() || tnodes[k].size()!=1) std::cout << "ERROR!~!" << std::endl;
        tnodes[i][j]=tnodes[k][0];
        //std::cout << "tnodes [" << i << "][" << j << "]: " << tnodes[i][j] << std::endl;
    }

    //std::cout << "tnodes[" << i << "].size()=" << tnodes[i].size() << std::endl;
    //for (int j=0;j<tnodes[i].size();++j) std::cout << tnodes[i][j] <<" ";std::cout << std::endl;
    NodeRef node=build_tree(tnodes[i]);
    tnodes[i].resize(1);
    tnodes[i][0].first=node;
}


void SSEncoder::get_pmres_outputs(NodeRef c, std::vector<int>& outputs) {
    if (c==undef) return;
    Node& cc=nodes[c];
    if (cc.lits.size()<2) return;
    outputs.push_back(cc.lits[1]);

    get_pmres_outputs(cc.l, outputs);
    get_pmres_outputs(cc.r, outputs);
}

// public functions

// relax single core

int SSEncoder::relax(const std::vector<int>& core, NodeRef& root, int& toplit) {
    top_lit=toplit;
    std::vector<std::pair<NodeRef, int> > tnodes;tnodes.clear();

    convert_lits_to_nodes(core, tnodes);
    root = build_tree(tnodes);

    new_variables+=new_vars(toplit);
    toplit=top_lit;

    return nodes[root].lits[1];
}

void SSEncoder::relax(const std::vector<int>& core, std::vector<int>& outputs, NodeRef& root, int& toplit) {
    top_lit=toplit;
    std::vector<std::pair<NodeRef, int> > tnodes;tnodes.clear();

    convert_lits_to_nodes(core, tnodes);
    root=build_tree(tnodes);

    new_variables+=new_vars(toplit);
    toplit=top_lit;

    if (!is_pmres) outputs=nodes[root].lits;
    else get_pmres_outputs(root, outputs);
}

/* Relax group of cores, try to reuse variables&clauses
 * vector outputs will contain 2-output-lits for each core
 *
 */
void SSEncoder::relax(const std::vector<std::vector<int> >& cores, std::vector<std::vector<int> >& outputs, vector<NodeRef>& roots, int& toplit) {
    top_lit=toplit;

    std::vector<std::vector<std::pair<NodeRef, int> > > tnodes;tnodes.clear();
    std::vector<int> times;
    std::priority_queue<std::pair<std::pair<int, int>, std::pair<std::pair<int, int>, std::pair<int, int> > > > commonq; // values: <<common, -tmz>, <<index1, time>, <index2, time> >

    int n=0;
    tnodes.resize(cores.size());
    times.resize(cores.size());
    // convert cores to vector of sorted vectors of NodeRefs (variable 'tnodes') and  // pair of NodeRefs in same core (nodepairs)
    for (int i=0;i<(int)cores.size();++i) {
        convert_lits_to_nodes(cores[i], tnodes[i]);
        times[i]=0;
        std::sort(tnodes[i].begin(), tnodes[i].end());
    }


    n=tnodes.size();
    outputs.resize(n);


    for (int i=0;i<(int)tnodes.size();++i) {
        if ((int)tnodes[i].size()<multirelax_stop_search_thresold) continue;
        for (int j=i+1;j<(int)tnodes.size();++j) {
            if ((int)tnodes[j].size()<multirelax_stop_search_thresold) continue;
            int ii=0;
            int jj=0;
            int common=0;
            int tmz=std::min(tnodes[i].size(), tnodes[j].size());

            while (ii<(int)tnodes[i].size() && jj<(int)tnodes[j].size()) {
                if (tnodes[i][ii].first<tnodes[j][jj].first) ++ii;
                else if (tnodes[i][ii].first>tnodes[j][jj].first)++jj;
                else ++common,++ii,++jj;
            }
            if (common<multirelax_stop_search_thresold) continue;
            commonq.push({{common, -tmz}, {{i, times[i]},{j, times[j]}}});
        }
    }

    int T=0;
    while (commonq.size()) {
        int max_common=commonq.top().first.first;
        int a=commonq.top().second.first.first;
        int at=commonq.top().second.first.second;
        int b=commonq.top().second.second.first;
        int bt=commonq.top().second.second.second;
        commonq.pop();
        if (times[a]!=at || times[b]!=bt) continue;
        total_reused+=max_common*2-1;

        int c=a;

        if (max_common==(int)tnodes[a].size() || max_common==(int)tnodes[b].size()) {
            if (max_common==(int)tnodes[a].size() && max_common==(int)tnodes[b].size()) {
                tnodes[a].resize(1);
                tnodes[a][0].first=indexref+b;
            } else {
                if (max_common==(int)tnodes[a].size()) std::swap(a, b);
                int ii=0;
                int jj=0;
                int ap=0;
                while (ii<(int)tnodes[a].size() && jj<(int)tnodes[b].size()) {
                    if (tnodes[a][ii].first<tnodes[b][jj].first) ++ii;
                    else {
                        tnodes[a][ii].first=undef;
                        ap=tnodes[a][ii].second;
                        ++ii;++jj;
                    }
                }
                tnodes[a].emplace_back(indexref+b, ap);
                remove_undefs(tnodes[a]);
            }
        } else {
            c=tnodes.size();
            tnodes.emplace_back();
            times.emplace_back();
            int ii=0;
            int jj=0;
            int ap=0;
            int bp=0;
            while (ii<(int)tnodes[a].size() && jj<(int)tnodes[b].size()) {
                if (tnodes[a][ii].first<tnodes[b][jj].first) ++ii;
                else if (tnodes[a][ii].first>tnodes[b][jj].first)++jj;
                else {
                    tnodes.back().emplace_back(tnodes[a][ii]);
                    tnodes[a][ii].first=undef;
                    tnodes[b][jj].first=undef;
                    ap=tnodes[a][ii].second;
                    bp=tnodes[b][jj].second;
                    ++ii;++jj;
                }
            }
            tnodes[a].emplace_back(indexref+tnodes.size()-1, ap);
            tnodes[b].emplace_back(indexref+tnodes.size()-1, bp);
            remove_undefs(tnodes[a]);
            remove_undefs(tnodes[b]);
        }

        ++T;
        times[a]=T;
        times[b]=T;
        times[c]=T;


        for (int i=0;i<(int)tnodes.size();++i) {
            if ((int)tnodes[i].size()<multirelax_stop_search_thresold) continue;
            if (i!=a && (int)tnodes[a].size()>=multirelax_stop_search_thresold) {
                int j=a;
                int ii=0;
                int jj=0;
                int common=0;
                int tmz=std::min(tnodes[i].size(), tnodes[j].size());

                while (ii<(int)tnodes[i].size() && jj<(int)tnodes[j].size()) {
                    if (tnodes[i][ii].first<tnodes[j][jj].first) ++ii;
                    else if (tnodes[i][ii].first>tnodes[j][jj].first)++jj;
                    else ++common,++ii,++jj;
                }
                if (common>=multirelax_stop_search_thresold)
                    commonq.push({{common, -tmz}, {{i, times[i]},{j, times[j]}}});
            }
            if (i!=b && (int)tnodes[b].size()>=multirelax_stop_search_thresold) {
                int j=b;
                int ii=0;
                int jj=0;
                int common=0;
                int tmz=std::min(tnodes[i].size(), tnodes[j].size());

                while (ii<(int)tnodes[i].size() && jj<(int)tnodes[j].size()) {
                    if (tnodes[i][ii].first<tnodes[j][jj].first) ++ii;
                    else if (tnodes[i][ii].first>tnodes[j][jj].first)++jj;
                    else ++common,++ii,++jj;
                }
                if (common>=multirelax_stop_search_thresold)
                    commonq.push({{common, -tmz}, {{i, times[i]},{j, times[j]}}});
            }
            if (i!=c && a!=c && (int)tnodes[c].size()>=multirelax_stop_search_thresold) {
                int j=c;
                int ii=0;
                int jj=0;
                int common=0;
                int tmz=std::min(tnodes[i].size(), tnodes[j].size());

                while (ii<(int)tnodes[i].size() && jj<(int)tnodes[j].size()) {
                    if (tnodes[i][ii].first<tnodes[j][jj].first) ++ii;
                    else if (tnodes[i][ii].first>tnodes[j][jj].first)++jj;
                    else ++common,++ii,++jj;
                }
                if (common>=multirelax_stop_search_thresold)
                    commonq.push({{common, -tmz}, {{i, times[i]},{j, times[j]}}});
            }
        }
    }

    roots.clear();
    roots.reserve(tnodes.size());
    for (int i=0;i<(int)tnodes.size();++i) {
        helper_build_tree(tnodes, i);
        if (i<n) {
            if (!is_pmres) outputs[i]=nodes[tnodes[i][0].first].lits;
            else get_pmres_outputs(tnodes[i][0].first, outputs[i]);
            roots.push_back(tnodes[i][0].first);
        }
    }

    new_variables+=new_vars(toplit);
    toplit=top_lit;
}


void SSEncoder::combine(const std::vector<int>& inputs, const std::vector<NodeRef>& input_nodes, std::vector<int>& outputs, NodeRef& root, int& toplit) {
    top_lit=toplit;

    std::vector<std::pair<NodeRef, int>> tnodes;
    tnodes.reserve(inputs.size());
    for (int i=0; i<(int)inputs.size(); ++i) {
      int lit = inputs[i];
      NodeRef node=lit_node(lit);
      if (node==undef) {
        node=lit_leaf(lit);
        if (node==undef) node=new_leaf(lit);
      }
      tnodes.push_back({node, i});
    }
    for (int i=0; i<(int)input_nodes.size(); ++i) {
      tnodes.push_back({input_nodes[i], i+inputs.size()});
    }
    root = build_tree(tnodes);
    outputs = nodes[root].lits;

    toplit = top_lit;
}





void SSEncoder::add_equivalences(NodeRef c, int leq, int req) {
    if (c==undef) return;
//     cout << "add_equivalences ";print_node(nodes[c]); cout << " " << leq << ", " << req << endl;
    Node& cc = nodes[c];
    if (cc.l==undef) return;
    Node& ll=nodes[cc.l];
    Node& rr=nodes[cc.r];
    if (leq>(int)ll.lits.size() || leq<0) leq=ll.lits.size();
    if (req>(int)rr.lits.size() || req<0) req=rr.lits.size();
    if (leq<cc.lleq) leq=cc.lleq;
    if (req<cc.rreq) req=cc.rreq;
//     cout << "add_equivalences, add really: " << leq << ", " << req << ", " << cc.lleq << ", " << cc.rreq << endl;
//     cout << ll.lits.size() << ", " << rr.lits.size() << endl;

    // add equivalences that are needed because cc.lits.size() has grown
    for (;cc.litseq<(int)cc.lits.size();++cc.litseq) {
        for (int ri=0;ri<=cc.litseq;++ri) {
            int li=cc.litseq-ri;
            if (ri<(int)cc.rreq && li==ll.inputs)       add_impl(cc.lits[li+ri], rr.lits[ri]),++equivalence_clauses;
            else if (li<(int)cc.lleq && ri==rr.inputs) add_impl(cc.lits[li+ri], ll.lits[li]),++equivalence_clauses;
            else if ((ri<cc.rreq && li<(int)ll.lits.size()) || (li<cc.lleq && ri<(int)rr.lits.size()))  add_and_neg(ll.lits[li], rr.lits[ri], cc.lits[li+ri]),++equivalence_clauses;
        }
    }

    // add equivalences that are needed because cc.lleq and/or cc.rreq grow
    for (int li=cc.lleq;li<leq;++li) {
        for (int ri=0;li+ri<(int)cc.lits.size() && ri<=(int)rr.lits.size();++ri) {
            if (ri==rr.inputs)               add_impl(cc.lits[li+ri], ll.lits[li]),++equivalence_clauses;
            else if (ri<(int)rr.lits.size()) add_and_neg(ll.lits[li], rr.lits[ri], cc.lits[li+ri]),++equivalence_clauses;
        }
    }
    for (int ri=cc.rreq;ri<req;++ri) {
        for (int li=0;li+ri<(int)cc.lits.size() && li<=(int)ll.lits.size();++li) {
            if (li>=cc.lleq && li<leq) continue;
            if (li==ll.inputs)               add_impl(cc.lits[li+ri], rr.lits[ri]),++equivalence_clauses;
            else if (li<(int)ll.lits.size()) add_and_neg(ll.lits[li], rr.lits[ri], cc.lits[li+ri]),++equivalence_clauses;
        }
    }

    cc.lleq=leq;
    cc.rreq=req;
    cc.litseq=cc.lits.size();
}




void SSEncoder::add_full_eqs(NodeRef c) {
    add_equivalences(c ,-1, -1);
    Node& cc=nodes[c];
    full_eq_nodes+=!cc.build_eqs;
    cc.build_eqs=1;
    if (cc.l != undef) add_full_eqs(cc.l), add_full_eqs(cc.r);
}

/* forced_true calls add_partial_eqs(NodeRef, nof_true, cost)
 *      parameter nof_true tells how many inputs are known to be true on the subtree (can be negative)
 *      paremter cost tells how much it costs to add full equivalences to other subtrees so that information about false inputs there can be known in this subtree
 *
 *      adding equivalences is considered useful, if they can propagate something to a shared subtree
 *
 *      options add_eq_max_decs, add_eq_max_cost and add_eq_max_prod limit the number of equivalences added
 *          add_eq_max_decs limits the number of needed false inputs on other subtrees to propagate information to a shared subtree
 *          add_eq_max_cost limits the equivalences added to other subtrees
 *          add_eq_max_prod limits the product of the two
 */
int eval_cost(int inputs) { return inputs*inputs; }// TODO: is this a good way to evaluate the number of equivalences...

int SSEncoder::add_partial_eqs(NodeRef c, int nof_true, int cost) {
//     cout << "add_partial_eqs, Node: "; print_node(nodes[c]);
//     cout << ", nof_true: " << nof_true;
//     cout << ", cost: " << cost << endl;

    if (nof_true<=-add_eq_max_decs || (nof_true<=0 && cost>add_eq_max_cost)) return 0;
    if ((nof_true<=0) && -nof_true*cost>add_eq_max_prod) return 0;

    Node& cc=nodes[c];
    if (cc.l==undef) {++shared_eq_nodes;return nof_true>0?1:-1;} // or: return aa.parents>1; ?

    Node& ll=nodes[cc.l];
    Node& rr=nodes[cc.r];


    int eql=add_partial_eqs(cc.l, nof_true-rr.inputs, cost+eval_cost(rr.inputs));
    int eqr=add_partial_eqs(cc.r, nof_true-ll.inputs, cost+eval_cost(ll.inputs));
    if (!eql && !eqr && cc.parents<2) return 0;
    if (cc.parents>1) ++shared_eq_nodes;

    if (eql<0) add_full_eqs(cc.r);
    if (eqr<0) add_full_eqs(cc.l);

    //  add equivalences :
    add_equivalences(c, abs(eql), abs(eqr)); // TODO: is this good?


    if (cost>add_eq_max_cost) return min((int)cc.lits.size(), nof_true); // do not build ful equivalences to the other subtree
    return -min((int)cc.lits.size(), nof_true+add_eq_max_decs);
}

/*
 * forced_true: idea: output variable var is known to be true, add equivalences in the tree structures that may cause useful propagation
 *
 */
void SSEncoder::forced_true(int var, int& toplit) {
    NodeRef node=lit_node(var);
    if (node==undef) return;
    int ix=lit_index(var);
    if (ix==-1) return; // this should never happen unless there is a terrible BUG! if lit_index not found, node shouldn't be found either...

    top_lit=toplit;
    add_partial_eqs(node, ix+1, 0);

    new_variables+=new_vars(toplit);
    toplit=top_lit;

}


int SSEncoder::next_output(int output, int& toplit) {
    NodeRef node=lit_node(output);
    if (node==undef) return 0;
    int ix=lit_index(output);
    if (ix==-1) return 0; // this should never happen unless there is a terrible BUG! if lit_index not found, node shouldn't be found either...

    // try to add lits so that there will be enough
    top_lit=toplit; ask_n_outputs(node, ix+2); new_variables+=new_vars(toplit); toplit=top_lit;

    if ((int)nodes[node].lits.size()<ix+2) return 0;
    return nodes[node].lits[ix+1];
}

int SSEncoder::previous_output(int output) {
    NodeRef node=lit_node(output);
    if (node==undef) return 0;
    int ix=lit_index(output);
    if (ix==-1) return 0; // this should never happen unless there is a terrible BUG! if lit_index not found, node shouldn't be found either...
    if (ix==0) return 0; // previous index does not exist...
    return nodes[node].lits[ix-1];
}

int SSEncoder::get_output(int lit, int ix, int& toplit) {
    NodeRef node=lit_node(lit);
    if (node==undef) return 0;

    top_lit=toplit; ask_n_outputs(node, ix+1); new_variables+=new_vars(toplit); toplit=top_lit;

    if ((int)nodes[node].lits.size()<ix+1) return 0;
    return nodes[node].lits[ix];
}


void SSEncoder::get_new_clauses(std::vector<std::vector<int> >& new_clauses) {
    for (;clauses_old<clauses.size();++clauses_old) {
        new_clauses.emplace_back(clauses[clauses_old]);
    }
}
void SSEncoder::get_new_clauses(std::vector<std::vector<int> >::iterator& begin, std::vector<std::vector<int> >::iterator& end) {
    begin=clauses.begin()+clauses_old;
    end=clauses.end();
    clauses_old=clauses.size();
}
