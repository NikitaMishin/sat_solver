//
// Created by nikita on 24.10.2020.
//

#ifndef MATHLOGIC_MEMENTO_STATE_PATTERN_H
#define MATHLOGIC_MEMENTO_STATE_PATTERN_H

#include <stack>
#include <stdexcept>

namespace utils {

    class StateRevertOperation {
    public:
        virtual void apply() = 0;

        virtual void cancel() = 0;

        virtual ~StateRevertOperation() = default;;
    };

    class Memento {
    private:
        std::stack<std::stack<StateRevertOperation *> *> *states;

    public:
        explicit Memento() {
            states = new std::stack<std::stack<StateRevertOperation *> *>();
        }

        void add_to_state(StateRevertOperation *operation) {
            auto state = states->top();
            state->push(operation);
        }

        void make_state() {
            auto new_state = new std::stack<StateRevertOperation *>();
            states->push(new_state);
        }

        bool has_previous_state() const { return !states->empty(); }

        void revert_to_previous_state(bool with_apply = true) {
            if (!has_previous_state()) throw std::runtime_error("No states left");
            auto state = states->top();
            while (!state->empty()) {
                auto operation = state->top();
                if (with_apply) {
                    operation->apply();
                } else {
                    operation->cancel();
                }
                state->pop();
                delete operation;
            }
            states->pop();
            delete state;

        };

        ~Memento() {
            while (!states->empty()) revert_to_previous_state(false);
            delete states;
        }
    };


}


#endif //MATHLOGIC_MEMENTO_STATE_PATTERN_H
