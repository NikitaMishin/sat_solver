//
// Created by nikita on 24.10.2020.
//

#ifndef MATHLOGIC_SOLVER_H
#define MATHLOGIC_SOLVER_H

#include <utility>
#include "constansts_and_typedefs.h"
#include "interpretation.h"
#include "formula_presentation.h"

class SatSolver {
protected:
    virtual std::pair<TRINARY, Interpretation *>
    is_sat(cnf_presentation::DisjunctSet *disjunct_set, std::vector<PropId> *props_ids) = 0;
};

class RecDPLL : public SatSolver {
private:

public:

    std::pair<TRINARY, Interpretation *>
    is_sat(cnf_presentation::DisjunctSet *disjunct_set, std::vector<PropId> *props_ids) override {
        auto m = new Interpretation(props_ids, false);
        m->start_making_state();
        m->make_state();
        disjunct_set->start_making_state();
        disjunct_set->make_state();

        return rec_is_sat(disjunct_set, m, 0);
    }

private:
    std::pair<TRINARY, Interpretation *>
    rec_is_sat(cnf_presentation::DisjunctSet *disjunct_set, Interpretation *m, LiteralId guess_literal) {
        auto status = disjunct_set->is_model(m);
        if (status == TOP) return std::make_pair(status, m);
        if (status == BOT) return std::make_pair(status, m);

        if (guess_literal != 0) {
            auto status = disjunct_set->delete_disjuncts_with_literal_id(guess_literal);
            if (status == BOT) return std::make_pair(BOT, m);
            status = disjunct_set->delete_literal_in_disjuncts(-guess_literal);
            if (status == BOT) return std::make_pair(BOT, m);

        }

        auto disjuncts_of_size_one = disjunct_set->get_disjunct_of_size(1);

        while (!disjuncts_of_size_one->empty()) {

            for (auto disjunct: *disjuncts_of_size_one) {
                if (disjunct == nullptr) break; ///already deleted

                auto literal_id = *std::begin(*disjunct);
                auto status = m->get_value_of_literal_id(literal_id);

                if (status == TOP) {
                    //toDo think
                    continue;// already processed
                }
                if (status == BOT) {
                    delete disjuncts_of_size_one;
                    return std::make_pair(BOT, m);
                }


                if (m->is_have_conflict(literal_id, TOP)) {
                    delete disjuncts_of_size_one;
                    return std::make_pair(BOT, m);
                }

                m->set_value_to_prop(literal_id, TOP);

                //we check that okey


                status = disjunct_set->delete_disjuncts_with_literal_id(literal_id);
                if (status == BOT) {
                    delete disjuncts_of_size_one;
                    return std::make_pair(BOT, m);
                }

                status = disjunct_set->delete_literal_in_disjuncts(-literal_id);
                if (status == BOT) {
                    delete disjuncts_of_size_one;
                    return std::make_pair(BOT, m);
                }

            }

            delete disjuncts_of_size_one;
            disjuncts_of_size_one = disjunct_set->get_disjunct_of_size(1);
        }

        delete disjuncts_of_size_one;

        status = disjunct_set->is_model(m);
        if (status == TOP || status == BOT) {
            return std::make_pair(status, m);
        }

        //guess literal
        m->make_state();
        disjunct_set->make_state();

        auto prop_id = m->get_any_unset_prop();
        m->set_value_to_prop(prop_id, TOP);
        auto dpll_rec_res = rec_is_sat(disjunct_set, m, prop_id);
        if (dpll_rec_res.first == TOP) return dpll_rec_res; //find model

        disjunct_set->revert_to_previous_state(true); //revert all actions that have been deep in recursion
        m->revert_to_previous_state();

        m->set_value_to_prop(prop_id, BOT);

//        disjunct_set->make_state();
//        m->make_state();


        dpll_rec_res = rec_is_sat(disjunct_set, m, -prop_id);
        if (dpll_rec_res.first == TOP) return dpll_rec_res; //find model

//        disjunct_set->revert_to_previous_state(true); //revert all actions that have been deep in recursion
//        m->revert_to_previous_state();


        return dpll_rec_res;

    }
};


class NonRecDPLL : public SatSolver {
private:

public:

    std::pair<TRINARY, Interpretation *>
    is_sat(cnf_presentation::DisjunctSet *disjunct_set, std::vector<PropId> *props_ids) override {
        auto m = new Interpretation(props_ids, false);
        m->start_making_state();
        disjunct_set->start_making_state();

        return is_sat(disjunct_set, m);
    }

private:
    std::pair<TRINARY, Interpretation *>
    is_sat(cnf_presentation::DisjunctSet *disjunct_set, Interpretation *m) {

        m->make_state();
        disjunct_set->make_state();


        auto literal_guess = m->get_any_unset_prop();

        // literal_id, value_to_be_set,is_already_cheked_opposite assignemnt; signal for rec
        auto stack = new std::stack<LiteralId>();

        stack->push(0);
        stack->push(-literal_guess);
        stack->push(literal_guess);

        while (!stack->empty()) {

            auto value_stack = stack->top();
            if (value_stack == 0) {
                stack->pop();
                m->revert_to_previous_state();
                disjunct_set->revert_to_previous_state(true);
                continue;
            }
            m->make_state();
            disjunct_set->make_state();


            auto status = disjunct_set->is_model(m);

            if (status == BOT) {
                stack->pop();
                m->revert_to_previous_state();
                disjunct_set->revert_to_previous_state(true);
                continue;
            }

            if (status == TOP) {
                delete stack;
                return std::make_pair(status, m);
            }



            // emulate guess step
            auto guess_literal = value_stack;
            m->set_value_to_prop(guess_literal, TOP);
            status = disjunct_set->delete_disjuncts_with_literal_id(guess_literal);
            if (status == BOT) {
                stack->pop();
                m->revert_to_previous_state();
                disjunct_set->revert_to_previous_state(true);
                continue;
            }

            status = disjunct_set->delete_literal_in_disjuncts(-guess_literal);
            if (status == BOT) {
                stack->pop();
                m->revert_to_previous_state();
                disjunct_set->revert_to_previous_state(true);
                continue;
            }

            auto disjuncts_of_size_one = disjunct_set->get_disjunct_of_size(1);

            auto is_any_conflict_occured = false;

            while (!disjuncts_of_size_one->empty()) {

                for (auto disjunct: *disjuncts_of_size_one) {
                    if (disjunct == nullptr) break; ///already deleted

                    auto literal_id = *std::begin(*disjunct);
                    auto status = m->get_value_of_literal_id(literal_id);

                    if (status == TOP) {
                        //toDo think
                        continue;// already processed
                    }

                    if (status == BOT) {
                        is_any_conflict_occured = true;
                        break;
                    }


                    if (m->is_have_conflict(literal_id, TOP)) {
                        is_any_conflict_occured = true;
                        break;
                    }

                    m->set_value_to_prop(literal_id, TOP);

                    //we check that okey

                    status = disjunct_set->delete_disjuncts_with_literal_id(literal_id);
                    if (status == BOT) {
                        is_any_conflict_occured = true;
                        break;
                    }

                    status = disjunct_set->delete_literal_in_disjuncts(-literal_id);
                    if (status == BOT) {
                        is_any_conflict_occured = true;
                        break;
                    }

                }

                delete disjuncts_of_size_one;
                disjuncts_of_size_one = disjunct_set->get_disjunct_of_size(1);
            }

            delete disjuncts_of_size_one;
            if (is_any_conflict_occured) {
                stack->pop();
                m->revert_to_previous_state();
                disjunct_set->revert_to_previous_state(true);
                continue;
            }

            status = disjunct_set->is_model(m);
            if (status == TOP) {
                delete stack;
                return std::make_pair(status, m);
            }


            stack->pop();


            auto prop_id = m->get_any_unset_prop();
            stack->push(0); // marker
            stack->push(-prop_id);
            stack->push(prop_id);


        }

        delete stack;

        return std::make_pair(BOT, m);
    }
};


#endif //MATHLOGIC_SOLVER_H
