

#include "formula_presentation.h"
#include "tseitin_transformation.h"
#include "solver.h"
#include "parser.h"



int main() {

    auto cnf = parsers::dimacs_parser_to_cnf("/home/nikita/projects/mathlogic/example_sat1");

    auto solver = new NonRecDPLL();

    auto res = solver->is_sat(cnf.first,cnf.second);
    std::cout << "\n";

    if(res.first==BOT){
        std::cout<<"UNSATISFIABLE";
    }
    std::cout << "\n";

    std::sort(cnf.second->begin(),cnf.second->end());
    if (res.first == TOP) {
        std::cout<<"SATISFIABLE"<<std::endl;
        for (const auto& it: *cnf.second) {
            auto status = res.second->get_value_of_literal_id(it);
            if (status==TOP) std::cout<<it;
            if (status==BOT) std::cout<<-it;
            if (status==NOT_SET) std::cout<<it;
            std::cout<<" ";
        }
    }
    std::cout<<std::endl;

    delete solver;
    delete cnf.second;
    delete cnf.first;
    delete res.second;




//
//    auto mapper = new std::unordered_map<int, std::string>();
//    mapper->insert({{1, "q1"},
//                    {2, "q2"},
//                    {3, "q3"}});
//
//    auto prosp_id = new std::vector<PropId>({1,2,3,4,5});
//    auto m = new Interpretation(prosp_id, false);
//
//    auto q1 = new formula_presentation::Variable(1, mapper);
//    auto q2 = new formula_presentation::Variable(2, mapper);
//    auto q3 = new formula_presentation::Variable(3, mapper);
//
//    auto q3_neg = new formula_presentation::Negation(q3);
//    auto q2vq3_neg = new formula_presentation::BinaryOperator(q2, q3_neg, formula_presentation::And);
//    auto q1and = new formula_presentation::BinaryOperator(q1, q2vq3_neg, formula_presentation::And);
//    auto q1_neq = new formula_presentation::Negation(q1->copy());
//    auto q1and_neg = new formula_presentation::BinaryOperator(q1and,q1_neq,formula_presentation::And);
//
//
//
//
//    q1and_neg->print(std::cout);
//    auto cnf = get_cnfs(q1and_neg, 3);
//
//    std::cout << "\n";
//
//    cnf->print(std::cout);
//
//
//    std::cout << "\n";
//
//    auto solver = RecDPLL();
//
//    auto res = solver.is_sat(cnf,prosp_id);
//    std::cout << "\n";
//
//    std::cout<<(res.first==TOP);
//    std::cout << "\n";
//
//    if (res.first == TOP) {
//        for (const auto& it:*mapper) {
//            std::cout<<it.second<<":";
//            auto status = res.second->get_value_of_literal_id(it.first);
//            if (status==TOP) std::cout<<1;
//            if (status==BOT) std::cout<<0;
//            if (status==NOT_SET) std::cout<<-1;
//            std::cout<<std::endl;
//        }
//    }
////    cnf->start_making_state();
////    cnf->make_state();
//
//
//
//    delete res.second;
//    delete  q1and_neg;
//    delete  cnf;
//    delete mapper;
//    delete m;
//    delete prosp_id;
//

    return 0;
}
