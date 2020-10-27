
#ifndef MATHLOGIC_PARSER_H
#define MATHLOGIC_PARSER_H

#include <fstream>
#include "formula_presentation.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace parsers {

    std::pair<cnf_presentation::DisjunctSet *, std::vector<PropId> *>
    dimacs_parser_to_cnf(const std::string &file_path) {

        auto disjunct_set = new cnf_presentation::DisjunctSetViaMap();
        auto props_set = new std::unordered_set<PropId>();

        std::ifstream file(file_path);
        std::string line;

        while (std::getline(file, line)) {
            // comments
            if (line.rfind('c', 0) == 0) continue; // split comments

            std::istringstream s(line);
            std::vector<std::string> tokens{std::istream_iterator<std::string>{s},
                                            std::istream_iterator<std::string>{}};

            if (line.rfind("p cnf", 0) == 0) { continue; }

            // now process each disjunct

            auto disjunct = new cnf_presentation::Disjunct();

            for (const auto &c: tokens) {
                auto v = std::stoi(c);
                if (v < 0 && props_set->count(-v) == 0) {
                    props_set->insert(-v);
                } else if (v > 0 && props_set->count(v) == 0) {
                    props_set->insert(v);
                } else if (v == 0) break;
                disjunct->insert(v);
            }
            disjunct_set->insert_disjunct(disjunct);

        }
        auto p = std::make_pair(disjunct_set, new std::vector<PropId>(props_set->begin(), props_set->end()));
        delete props_set;
        return p;
    }

}


#endif //MATHLOGIC_PARSER_H
