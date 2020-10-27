//
// Created by nikita on 18.10.2020.
//

#ifndef MATHLOGIC_FORMULA_PRESENTATION_H
#define MATHLOGIC_FORMULA_PRESENTATION_H


#include <iostream>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stack>
#include "constansts_and_typedefs.h"
#include "interpretation.h"


namespace formula_presentation {
    enum FormulaType {
        Neg, Implication, And, Or, Equiv, Var
    };


    class Formula {
    public:


        FormulaType op_type;

        virtual bool is_literal() const = 0;

        virtual bool is_prop() const = 0;


        virtual void print(std::ostream &os) const = 0;

        virtual ~Formula() {}

    protected:
        explicit Formula(FormulaType op_type) : op_type(op_type) {}


    };


    class Variable : public Formula {
    private:
        std::unordered_map<LiteralId, std::string> *literal_id_to_pretty_name;

    public:
        int id;

        explicit Variable(int id, std::unordered_map<LiteralId,
                std::string> *id_to_string_mapper) : Formula(Var), id(id),
                                                     literal_id_to_pretty_name(id_to_string_mapper) {}

        void print(std::ostream &os) const override {
            os << (*literal_id_to_pretty_name)[id];
        }

        bool is_literal() const override { return true; }

        bool is_prop() const override { return true; }

        Variable *copy(){return new Variable(id,literal_id_to_pretty_name);};

    };

    class Negation : public Formula {
    public:
        Formula *formula;

        void print(std::ostream &os) const override {
            os << "!";
            if (formula->op_type == Var || formula->op_type == Neg) {
                formula->print(os);
            } else {
                os << "(";
                formula->print(os);
                os << ")";
            }
            os << "";
        }

        bool is_prop() const override { return false; }

        bool is_literal() const override { return formula->is_prop(); }


        explicit Negation(Formula *f) : Formula(Neg), formula(f) {}

        ~Negation() override {
            delete formula;
        }


    };

    class BinaryOperator : public Formula {
    public:
        Formula *left_formula;
        Formula *right_formula;

        void print(std::ostream &os) const override {
            if (left_formula->op_type == Neg || left_formula->op_type == Var) {
                left_formula->print(os);
            } else {
                os << "(";
                left_formula->print(os);
                os << ")";
            }
            switch (op_type) {
                case And:
                    os << "&&";
                    break;
                case Or:
                    os << "||";
                    break;
                case Equiv:
                    os << "<->";
                    break;
                case Implication:
                    os << "->";
                    break;
            }

            if (right_formula->op_type == Neg || right_formula->op_type == Var) {
                right_formula->print(os);
            } else {
                os << "(";
                right_formula->print(os);
                os << ")";
            }

        }

        bool is_literal() const override { return false; }

        bool is_prop() const override { return false; }


        BinaryOperator(Formula *left_f, Formula *right_f, FormulaType op) : Formula(op), left_formula(left_f),
                                                                            right_formula(right_f) {}


        ~BinaryOperator() override {
            delete left_formula;
            delete right_formula;
        }
    };


}


namespace cnf_presentation {
    // stores literals id, //hint  negation =  - id
    typedef std::unordered_set<LiteralId> Disjunct;

    class DisjunctSet {

    public:

        virtual void print(std::ostream &os) const = 0;


        virtual std::vector<Disjunct *> *get_disjunct_of_size(int n) const = 0;


        /**
         * Get all disjunct in set that have size not in sizes
         * @param n
         */
        virtual std::vector<Disjunct *> *get_all_disjuncts_except_of_size(int size) const = 0;


        virtual TRINARY delete_literal_in_disjuncts(int literal_id) = 0;

        virtual TRINARY delete_disjuncts_with_literal_id(LiteralId literal_id) = 0;

        virtual void delete_disjunct(Disjunct *disjunct) = 0;


        /**
         * Checks weather or not interpretation is model for disjunct set
         * @param model
         * @return TRUE/SAT(TOP), FALSE/UNSAT(BOT) or NOT_SAT (not all variables have interpretation)
         */
        virtual TRINARY is_model(Interpretation *interpretation) const = 0;

        virtual void insert_disjunct(Disjunct *disjunct) = 0;

        virtual void start_making_state() = 0;

        virtual void stop_making_state() = 0;


        virtual void make_state() = 0;

        virtual void revert_to_previous_state(bool should_apply_revert_operations) = 0;

        virtual  ~DisjunctSet() = default;;

    };

    class DisjunctSetViaMap : public DisjunctSet {
    private:
        utils::Memento *memento;
        std::unordered_set<Disjunct *> *set;
        bool is_state_saving;


        class DisjunctAddition : public utils::StateRevertOperation {
        private:
            std::unordered_set<Disjunct *> *set;
            Disjunct *disjunct;
        public:

            explicit DisjunctAddition(Disjunct *disjunct, std::unordered_set<Disjunct *> *set) : StateRevertOperation(),
                                                                                                 disjunct(disjunct),
                                                                                                 set(set) {};

            void apply() override { set->insert({disjunct}); }

            void cancel() override {};

            ~DisjunctAddition() override = default;

        };

        class DisjunctsAddition : public utils::StateRevertOperation {
        private:
            std::vector<Disjunct *> *vector_with_disjuncts;
            std::unordered_set<Disjunct *> *set;
        public:

            explicit DisjunctsAddition(std::vector<Disjunct *> *vector_with_disjuncts,
                                       std::unordered_set<Disjunct *> *set) :
                    StateRevertOperation(), vector_with_disjuncts(vector_with_disjuncts), set(set) {};

            void apply() override { for (auto disjunct :*vector_with_disjuncts) set->insert(disjunct); }

            void cancel() override {};

            ~DisjunctsAddition() override {
                delete vector_with_disjuncts;
            }

        };

        class LiteralIdAdditionToDisjuncts : public utils::StateRevertOperation {
        private:
            std::vector<Disjunct *> set_to_add;
            LiteralId literal_id;

        public:
            explicit LiteralIdAdditionToDisjuncts(LiteralId literal_id, std::vector<Disjunct *> set_to_add)
                    : set_to_add(std::move(set_to_add)), literal_id(literal_id) {};

            void cancel() override {}

            void apply() override {
                for (Disjunct *disjunct: set_to_add) { disjunct->insert(literal_id); }
            }

            ~LiteralIdAdditionToDisjuncts() override = default;

        };

    public:

        DisjunctSetViaMap() : DisjunctSet(), is_state_saving(false) {
            memento = new utils::Memento();
            set = new std::unordered_set<Disjunct *>;
        }

        void start_making_state() override { is_state_saving = true; }

        void stop_making_state() override { is_state_saving = false; }

        void make_state() override { memento->make_state(); }

        void revert_to_previous_state(bool should_apply_revert_operations) override {
            memento->revert_to_previous_state(should_apply_revert_operations);
        }

        void insert_disjunct(Disjunct *disjunct) override { set->insert(disjunct); }

        void delete_disjunct(Disjunct *disjunct) override {
            set->erase(disjunct);
            if (is_state_saving) {
                memento->add_to_state(new DisjunctAddition(disjunct, set));
            }
        }

        TRINARY delete_literal_in_disjuncts(int literal_id) override {
            std::vector<Disjunct *> disjuncts_where_removed_literals;
            if (is_state_saving) disjuncts_where_removed_literals = std::vector<Disjunct *>();

            auto status = TOP;

            for (auto disjucnt:*set) {
                if (disjucnt->count(literal_id) > 0) {
                    if (disjucnt->size() == 1) status = BOT;
                    if (is_state_saving) disjuncts_where_removed_literals.push_back(disjucnt);
                    disjucnt->erase(literal_id);
                }
            }

            if (is_state_saving && !disjuncts_where_removed_literals.empty()) {
                memento->add_to_state(new LiteralIdAdditionToDisjuncts(literal_id, disjuncts_where_removed_literals));
            }
            return status;
        }

        TRINARY delete_disjuncts_with_literal_id(LiteralId literal_id) override {

            std::vector<Disjunct *> *removed_disjuncts;
            removed_disjuncts = new std::vector<Disjunct *>;

            for (auto disjunct:*set) {
                if (disjunct->count(literal_id) > 0) {
                    if (is_state_saving) removed_disjuncts->push_back(disjunct);
                }
            }

            // remove them
            for (auto disjunct:*removed_disjuncts) { set->erase(disjunct); }

            if (is_state_saving && !removed_disjuncts->empty()) {
                memento->add_to_state(new DisjunctsAddition(removed_disjuncts, set));
            } else { delete removed_disjuncts; }

            return TOP;
        }


        std::vector<Disjunct *> *get_all_disjuncts_except_of_size(int size) const override {
            auto v = new std::vector<Disjunct *>;
            for (auto disjunct: *set) {
                if (disjunct->size() != size) v->push_back(disjunct);
            }
            return v;
        }


        std::vector<Disjunct *> *get_disjunct_of_size(int n) const override {
            auto v = new std::vector<Disjunct *>;
            for (auto disjunct: *set) {
                if (disjunct->size() == n) v->push_back(disjunct);
            }
            return v;
        }


        ~DisjunctSetViaMap() override {

            for (auto disjunct: *set) {
                delete disjunct;
            }

            delete set;
            delete memento;

        }


        TRINARY is_model(Interpretation *interpretation) const override {
            for (auto disjunct: *set) {
                bool is_true = false;
                for (auto literal: *disjunct) {
                    auto status = interpretation->get_value_of_literal_id(literal);
                    if (status == TOP) {
                        is_true = true;
                        break;
                    } else if (status == NOT_SET) { return NOT_SET; }
                }
                if (!is_true) return BOT;
            }
            return TOP;
        }


        void print(std::ostream &os) const override {
            for (auto disjunct: *set) {
                if (disjunct->empty()) continue;
                os << "{";

                for (auto lit:*disjunct) {
                    os << lit << " ";
                }

                os << "}" << std::endl;


            }
        }
    };


}


#endif //MATHLOGIC_FORMULA_PRESENTATION_H
