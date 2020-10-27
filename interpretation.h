//
// Created by nikita on 24.10.2020.
//

#ifndef MATHLOGIC_INTERPRETATION_H
#define MATHLOGIC_INTERPRETATION_H


#include <unordered_map>
#include <string>
#include <unordered_set>
#include <vector>
#include "memento_state_pattern.h"
#include "constansts_and_typedefs.h"


typedef std::unordered_map<LiteralId, std::string> IdToVarName;


class Interpretation {
private:
    utils::Memento *memento;
    std::unordered_map<LiteralId, TRINARY> *mapping;
    bool with_saving_states;

    class RevertOperationSingle : public utils::StateRevertOperation {
    private:
        std::unordered_map<LiteralId, TRINARY> *mapping;
        LiteralId key;
        TRINARY value;


    public:
        explicit RevertOperationSingle(std::unordered_map<LiteralId, TRINARY> *mapping,
                                       LiteralId key, TRINARY value) : mapping(mapping), key(key), value(value) {};

        void apply() override { (*mapping)[key] = value; }

        void cancel() override {};

        ~RevertOperationSingle() override = default;

    };

public:

    explicit Interpretation(std::vector<PropId> const *props_ids, bool with_saving_states = true)
            : with_saving_states(with_saving_states) {
        mapping = new std::unordered_map<PropId, TRINARY>;
        for (auto prop_id:*props_ids) {
            if (mapping->count(prop_id) == 0) mapping->insert({prop_id, NOT_SET});
        }
        if (with_saving_states) {
            memento = new utils::Memento();
        }
    }

    ~Interpretation() {
        delete mapping;
        if (with_saving_states) {
            delete memento;
        }
    }

    void set_value_to_prop(LiteralId literal_id, TRINARY value) {
        auto is_neg = (literal_id < 0);
        auto prop_id = is_neg ? -literal_id : literal_id;
        if (value == TOP && is_neg) {
            value = BOT;
        } else if (value == BOT && is_neg) value = TOP;

        if (with_saving_states) {
            auto op = new RevertOperationSingle(mapping, prop_id, mapping->at(prop_id));
            memento->add_to_state(op);
        }
        (*mapping)[prop_id] = value;

    }

    bool is_all_props_set() {
        for (auto pair :*mapping) {
            if (pair.second == TRINARY::NOT_SET) return false;
        }
        return true;
    }

    TRINARY get_value_of_literal_id(LiteralId literal_id) {
        auto is_neg = (literal_id < 0);
        auto prop_id = is_neg ? -literal_id : literal_id;
        auto status = (*mapping)[prop_id];
        if (status == TOP && is_neg) return BOT;
        if (status == BOT && is_neg) return TOP;
        return status;
    }


    LiteralId get_any_unset_prop() {
        for (auto &pair :*mapping) {
            if (pair.second == TRINARY::NOT_SET) return pair.first;
        }
        throw std::runtime_error("All props are set");
    }

    void make_state() {
        if (!with_saving_states) throw std::runtime_error("No states due to with_saving_states set to false");
        memento->make_state();
    }


    void revert_to_previous_state() {
        if (!with_saving_states) throw std::runtime_error("No states due to with_saving_states set to false");
        memento->revert_to_previous_state(true);
    };

    bool is_have_conflict(LiteralId literal_id, TRINARY value) {
        auto is_neg = (literal_id < 0);
        auto prop_id = is_neg ? -literal_id : literal_id;
        if (value == TOP && is_neg) {
            value = BOT;
        } else if (value == BOT && is_neg) {
            value = TOP;
        }


        if (mapping->count(prop_id) == 0) {
            return false;
        }

        auto cur_value = mapping->at(prop_id);
        return (cur_value == TOP && value == BOT) || (cur_value == BOT && value == TOP);
    }

    void start_making_state() {
        memento = new utils::Memento();
        with_saving_states = true;
    };

    void stop_making_state() {
        with_saving_states = false;
    };

};


#endif //MATHLOGIC_INTERPRETATION_H
