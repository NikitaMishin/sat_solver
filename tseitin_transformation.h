//
// Created by nikita on 18.10.2020.
//

#ifndef MATHLOGIC_TSEITIN_TRANSFORMATION_H
#define MATHLOGIC_TSEITIN_TRANSFORMATION_H


#include "formula_presentation.h"


std::pair<LiteralId, cnf_presentation::DisjunctSet *> get_cnf(
        formula_presentation::Formula *f, cnf_presentation::DisjunctSet *context, LiteralId max_value) {
    if (f->op_type == formula_presentation::Var) {
        auto variable = reinterpret_cast<formula_presentation::Variable *>(f);
        auto literal_id = variable->id;
        return std::make_pair(literal_id, context);
    }

    if (f->op_type == formula_presentation::Neg) {
        auto negation = reinterpret_cast<formula_presentation::Negation *>(f);
        auto pair = get_cnf(negation->formula, context, max_value);
        return std::make_pair(-pair.first, pair.second);
    }

    auto bin_formula = reinterpret_cast<formula_presentation::BinaryOperator *>(f);

    // evaluate first
    auto tuple_left = get_cnf(bin_formula->left_formula, context, max_value);
    auto tuple_second = get_cnf(bin_formula->right_formula, tuple_left.second,
                                std::max(std::abs(tuple_left.first), max_value) + 1);

    auto new_literal_id = std::abs(tuple_second.first) + 1;
    auto new_context = tuple_second.second;

    auto l = tuple_left.first;
    auto r = tuple_second.first;
    auto p = new_literal_id;

    switch (bin_formula->op_type) {
        case formula_presentation::Implication:
            context->insert_disjunct(new cnf_presentation::Disjunct({-l, r, -p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({l, p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({-r, p}));
            break;
        case formula_presentation::And:
            context->insert_disjunct(new cnf_presentation::Disjunct({-l, -r, p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({l, -p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({r, -p}));
            break;
        case formula_presentation::Or:
            context->insert_disjunct(new cnf_presentation::Disjunct({l, r, -p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({-l, p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({-r, p}));
            break;
        case formula_presentation::Equiv:
            context->insert_disjunct(new cnf_presentation::Disjunct({-l, -r, p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({l, p, r}));
            context->insert_disjunct(new cnf_presentation::Disjunct({l, -r, -p}));
            context->insert_disjunct(new cnf_presentation::Disjunct({-l, r, -p}));
            break;
    }

    return std::make_pair(new_literal_id, new_context);


}

cnf_presentation::DisjunctSet *get_cnfs(formula_presentation::Formula *f, LiteralId max_value) {
    auto set = new cnf_presentation::DisjunctSetViaMap();
    auto new_disjunct = new cnf_presentation::Disjunct();
    auto res = get_cnf(f, set, max_value);
    new_disjunct->insert(res.first);
    res.second->insert_disjunct(new_disjunct);
    return res.second;
}

#endif //MATHLOGIC_TSEITIN_TRANSFORMATION_H
