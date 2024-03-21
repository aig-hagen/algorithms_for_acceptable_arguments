  #include <vector>
#include <unordered_map>

#include <iostream>
class SSEncoder { // creates totalizers using structure sharing techique
public:
    // NodeRef: index to vector nodes
    typedef unsigned int NodeRef;
    static const NodeRef undef=0xffffffff;
    static const NodeRef indexref=0x80000000;

    struct Node{
        NodeRef l;
        NodeRef r;
        int inputs;
        int ll; // number of lits in left child connected to lits
        int rl; // number of lits in right child connected to lits
        std::vector<int> lits;
        int parents; // number of nodes this node is child to
        int lleq; // number of equivalence connections to the left child
        int rreq;  // right child
        int litseq; // number of this.lits that have equivalence connections
        bool build_eqs;
        Node():l(undef),r(undef),inputs(1),ll(0),rl(0),parents(0),lleq(0),rreq(0),litseq(0),build_eqs(0){}
    };
private:
    bool is_pmres;      // relax using OLL or PMRES

    unsigned int clauses_old; // index for get_new_clauses

    // options
    bool eqtree;         // add always equivalences instead of implications only TODO: PMRES
    bool try_reuse;      // when relaxing lits, try finding if some pairs of lits are already combined in some earlier node
    int multirelax_stop_search_thresold; // stop searching for common lits in cores when maximum number of common lits gets below the thresold
    int add_eq_max_decs; // how easily equivalences is added on forced_true: number of additional inputs needed for equivalences to propagate something useful
    int add_eq_max_cost; // how many extra equivalences we are ready to pay for adding useful equivalences
    int add_eq_max_prod;   // add eq only if number_of_equivalences*additional_inputs is below some given constant

    std::vector<Node> nodes;


    int top_lit;
    std::vector<std::vector<int> > clauses;

    inline int neg(int l) { return l^1;}
    inline int new_lit() {return top_lit+=2;}
    inline int new_vars(int old_top_lit) { return (top_lit-old_top_lit)>>1;}

    // if dimacs lits used:
    /*
    inline int neg(int l) { return -l;}
    inline int new_lit() {return ++top_lit;}
    inline int new_vars(int old_top_lit) { return top_lit-old_top_lit;}
    */

    inline void add_and(int a, int b, int c) { clauses.push_back({neg(a), neg(b), c});}
    inline void add_and_neg(int a, int b, int c) { clauses.push_back({a, b, neg(c)});}
    inline void add_or(int a, int b, int c) { clauses.push_back({neg(a), c});clauses.push_back({neg(b), c});}
    inline void add_impl(int a, int b) { clauses.push_back({neg(a), b});}
    inline int  add_and(int a, int b) { clauses.push_back({neg(a), neg(b), new_lit()});return top_lit;}
    inline int  add_and_neg(int a, int b) { clauses.push_back({a, b, neg(new_lit())});return top_lit;}
    inline int  add_or(int a, int b) { clauses.push_back({neg(a), new_lit()});clauses.push_back({neg(b), top_lit});return top_lit;}



    std::unordered_map<int, NodeRef>          lit_nodes; // map from lit to node
    std::unordered_map<int, NodeRef>          lit_leafs;  // special map from lit to node:
    std::unordered_map<int, unsigned int>     lit_indices; // map from lit to lit index in node


    std::unordered_map<unsigned long long, NodeRef> parents; // map of existing node parents

    inline void remove_undefs(std::vector<std::pair<NodeRef, int> >& nodes);

    inline NodeRef lit_node(int lit) {auto n=lit_nodes.find(lit);if (n!=lit_nodes.end()) return n->second; return undef;}
    inline NodeRef lit_leaf(int lit) {auto n=lit_leafs.find(lit);if (n!=lit_leafs.end()) return n->second; return undef;}
    inline int lit_index(int lit) {auto n=lit_indices.find(lit);if (n!=lit_indices.end()) return n->second; return -1;}

    inline static unsigned long long pair_index(NodeRef a, NodeRef b){return a<b ? ((((unsigned long long)a)<<32)|b):((((unsigned long long)b)<<32)|a);}
    inline NodeRef find_parent(NodeRef a, NodeRef b){auto n=parents.find(pair_index(a, b)); if (n!=parents.end()) return n->second;return undef;}

    inline NodeRef new_node() {
        nodes.emplace_back();
        return nodes.size()-1;
    }
    inline NodeRef new_leaf(int lit) {
        unsigned int i=nodes.size();
        nodes.emplace_back();
        nodes[i].lits.push_back(lit);
        lit_leafs[lit]=i;
        return i;
    }



    void ask_n_outputs(NodeRef c, int n); // return number of added outputs (OLL only)

    NodeRef create_parent(NodeRef a, NodeRef b);
    NodeRef build_tree(std::vector<std::pair<NodeRef, int> >& tnodes);
    void    build_reusables(std::vector<std::pair<NodeRef, int> >& tnodes);

    void convert_lits_to_nodes(const std::vector<int>& lits, std::vector<std::pair<NodeRef, int> >& tnodes);

    void helper_build_tree(std::vector<std::vector<std::pair<NodeRef, int> > >& tnodes, int i);

    void get_pmres_outputs(NodeRef c, std::vector<int>& outputs);
public:
    // readonly stats
    int total_reused; // not exact value
    int new_variables;
    int equivalence_clauses;
    int shared_eq_nodes;
    int full_eq_nodes;


    int relax(const std::vector<int>& core, NodeRef& root, int& toplit); // relax single core TODO: PMRES
    void relax(const std::vector<int>& core, std::vector<int>& outputs, NodeRef& root, int& toplit); // relax single core TODO: PMRES
    void relax(const std::vector<std::vector<int> >& cores, std::vector<std::vector<int> >& outputs, std::vector<NodeRef>& roots, int& toplit); // relax group of cores

    void combine(const std::vector<int>& inputs, const std::vector<NodeRef>& nodes, std::vector<int>& outputs, NodeRef& root, int& toplit); //
    // toplit: greatest lit that is already in use and cannot be used.
private:
    void add_equivalences(NodeRef c, int leq, int req); // add equivalence clauses to node c


    void add_full_eqs(NodeRef c); //make subtree cf node c to a full equicalence subtree

    int add_partial_eqs(NodeRef c, int nof_true, int cost); // forced_true calls this, adds equivalences recursively when they are needed
public:
    void forced_true(int var, int& toplit); // an output variable is forced to be true: add equivalences


    // gettin next and previous outputs, OLL only:
    int next_output(int output, int& toplit);
    int previous_output(int output); // TODO: this is a bit ad hoc way of getting outputs...
    int get_output(int lit, int ix, int& toplit);


    void get_new_clauses(std::vector<std::vector<int> >& clauses);
    void get_new_clauses(std::vector<std::vector<int> >::iterator& begin, std::vector<std::vector<int> >::iterator& end);

    void stats(std::string b, std::ostream& out) {
        // options
        out << b << "ssenc.is_pmres: " << (int)is_pmres << std::endl;
        out << b << "ssenc.eqtree: " << (int)eqtree << std::endl;
        out << b << "ssenc.try_reuse: " << (int)try_reuse << std::endl;
        out << b << "ssenc.multirelax_stop_search_thresold: " << multirelax_stop_search_thresold << std::endl;
        out << b << "ssenc.add_eq_max_decs: " << add_eq_max_decs << std::endl;
        out << b << "ssenc.add_eq_max_cost: " << add_eq_max_cost << std::endl;
        out << b << "ssenc.add_eq_max_prod: " << add_eq_max_prod << std::endl;


        // statistics
        out << b << "ssenc.nodes.size(): " << nodes.size() << std::endl;
        int ttl_lits=0; int ttl_lit_capacity=0;
        for (unsigned int i=0;i<nodes.size();++i) {
            ttl_lits+=nodes[i].lits.size();
            ttl_lit_capacity+=nodes[i].lits.capacity();
        }
        out << b << "ssenc.new_variables: " << new_variables << std::endl;
        out << b << "ssenc.ttl_lits: " << ttl_lits << std::endl;
        out << b << "ssenc.ttl_lit_capacity: " << ttl_lit_capacity << std::endl;
        out << b << "ssenc.lit_nodes.size(): " << lit_nodes.size() << std::endl;
        out << b << "ssenc.lit_leafs.size(): " << lit_leafs.size() << std::endl;
        out << b << "ssenc.lit_indices.size(): " << lit_indices.size() << std::endl;
        out << b << "ssenc.parents.size(): " << parents.size() << std::endl;

        out << b << "ssenc.clauses.size(): " << clauses.size() << std::endl;
        int clauses_lits=0;int clauses_lits_capacity=0;
        for (unsigned int i=0;i<clauses.size();++i) {
            clauses_lits+=clauses[i].size();
            clauses_lits_capacity+=clauses[i].capacity();
        }
        out << b << "ssenc.clauses_lits: " << clauses_lits << std::endl;
        out << b << "ssenc.clauses_lits_capacity: " << clauses_lits_capacity << std::endl;
        out << b << "ssenc.total_reused: " << total_reused << std::endl;
        out << b << "ssenc.equivalence_clauses: " << equivalence_clauses << std::endl;
        out << b << "ssenc.shared_eq_nodes: " << shared_eq_nodes << std::endl;
        out << b << "ssenc.full_eq_nodes: " << full_eq_nodes << std::endl;

    }

    void enable_reusing(){try_reuse=1;}
    void disable_reusing(){try_reuse=0;}
    void set_multirelax_search_thresold(int thre){multirelax_stop_search_thresold=thre;}
    void set_add_eq_max_decs(int decs){add_eq_max_decs=decs;}
    void set_add_eq_max_cost(int cost){add_eq_max_cost=cost;}
    void set_add_eq_max_prod(int prod){add_eq_max_prod=prod;}

    void disable_eqs() {add_eq_max_decs = 0; add_eq_max_cost = 0; add_eq_max_prod = 0;}
    void enable_full_eqs() {add_eq_max_decs = 2147483647; add_eq_max_cost = 2147483647; add_eq_max_prod = 2147483647;}

    SSEncoder(bool _is_pmres=0, bool _eqtree=0):
            is_pmres(_is_pmres),
            clauses_old(0),
            // options
            eqtree(_eqtree),
            try_reuse(1),
            multirelax_stop_search_thresold(16),
            add_eq_max_decs(50),
            add_eq_max_cost(50),
            add_eq_max_prod(2500),

            // statistics
            total_reused(0),
            new_variables(0),
            equivalence_clauses(0),
            shared_eq_nodes(0),
            full_eq_nodes(0)
            {

            }

};
