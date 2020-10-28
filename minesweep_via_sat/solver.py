import itertools
from typing import List, Tuple

from satispy import Variable, Cnf
from satispy.solver import Minisat
import argparse
from functools import reduce



def get_adjacent_cells(i, j, rows, cols) -> List[Tuple[int, int]]:
    def in_boundaries(p: Tuple[int, int]):
        return rows >= p[0] >= 1 and 1 <= p[1] <= cols

    cells = [(i - 1, j - 1), (i - 1, j), (i - 1, j + 1),
             (i, j - 1), (i, j + 1),
             (i + 1, j - 1), (i + 1, j), (i + 1, j + 1)
             ]
    return list(filter(in_boundaries, cells))


def number_constraint_for_cell(p, rows, cols, variables, defined_statuses):
    S = Cnf()

    if p == (1,1):
        print()
    # false imply all
    if p in defined_statuses and defined_statuses[p] == -1:  # is bomb
        return S

    cell_status = None  # is sell status defined
    if p in defined_statuses:
        cell_status = defined_statuses[p]

    adj_cells = get_adjacent_cells(p[0], p[1], rows, cols)

    if cell_status is not None and cell_status > len(adj_cells):
        return None  # more boms then adjacenet cells

    for number in range(0, len(adj_cells) + 1):
        # this means that imply already satisfied because False imply everything
        if cell_status is not None and cell_status != number:
            continue

        # number imply ( () or (...and ...) or (...and ..) ....)
        or_of_conjuncts = Cnf()

        is_have_any_solvable_conjunct = False
        is_combination_already_have_model = True
        have_model = False
        have_inter = False
        for combination in itertools.combinations(adj_cells, number):
            # one of conjunct is have model then number imply 1

            bombs_loc = [p for p in combination]
            no_bombs_loc = set(adj_cells) - set(bombs_loc)

            # NEW
            bombs_bad = [cell for cell in bombs_loc if cell in defined_statuses and defined_statuses[cell] >= 0]
            bombs_good = [cell for cell in bombs_loc if cell in defined_statuses and defined_statuses[cell] == -1]
            parametrized_pts_bomb = [variables[cell][0] for cell in bombs_loc if cell not in defined_statuses]
            parametrized_formula_conjunct_1 = reduce(lambda x, y: x & y, parametrized_pts_bomb, Cnf())

            bombs_no_bad = [cell for cell in no_bombs_loc if cell in defined_statuses and defined_statuses[cell] == -1]
            bombs_no_good = [cell for cell in no_bombs_loc if cell in defined_statuses and defined_statuses[cell] >= 0]
            parametrized_pts_no_bomb = [-variables[cell][0] for cell in no_bombs_loc if cell not in defined_statuses]
            parametrized_formula_conjunct_2 = reduce(lambda x, y: x & y, parametrized_pts_no_bomb, Cnf())

            conjunct = parametrized_formula_conjunct_1 & parametrized_formula_conjunct_2  # is conjunct for one combination

            # if we have any bad pts then skip combination
            if len(bombs_no_bad) > 0 or len(bombs_bad) > 0:
                continue
            # if we have all determined things in conjunct and they all True then break and set that right part if
            # imply is True
            if len(bombs_good) == len(bombs_loc) and len(bombs_no_good) == len(bombs_no_good):
                have_model = True
                have_inter = True
                break

            or_of_conjuncts |= conjunct
            have_inter = True

        if have_model:
            continue

        if p in defined_statuses and not defined_statuses[p] == number:
            continue
        if p in defined_statuses and defined_statuses[p] == number:
            if have_inter:
                S &= or_of_conjuncts
                continue
            else:
                return None
        if p not in defined_statuses:
            if have_inter:
                S &= (variables[p][number+1] >> or_of_conjuncts)
            else:
                S &= (-variables[p][number+1])
    return S


def bomb_constraint_for_cell(p, rows, cols, variables, defined_statuses):
    def no_zero_number_in_adjacent():
        is_p_set = p in defined_statuses
        is_p_set_to_bomb = p in defined_statuses and defined_statuses[p] == -1

        adj_cells = get_adjacent_cells(p[0], p[1], rows, cols)

        bad_pts = [cell for cell in adj_cells if cell in defined_statuses and defined_statuses[cell] == 0]
        good_pts = [cell for cell in adj_cells if cell in defined_statuses and not defined_statuses[cell] == 0]

        parametrized_pts = [-variables[cell][0] for cell in adj_cells if cell not in defined_statuses]
        parametrized_formula_conjunct = reduce(lambda x, y: x & y, parametrized_pts, Cnf())

        if is_p_set and not is_p_set_to_bomb:
            return Cnf()

        if len(bad_pts) > 0 and is_p_set_to_bomb:
            return None
        elif len(bad_pts) > 0 and is_p_set == False:
            return -variables[p][0]
        elif len(bad_pts) == 0 and is_p_set:
            return parametrized_formula_conjunct
        elif len(bad_pts) == 0 and is_p_set == False:
            return variables[p][0] >> parametrized_formula_conjunct

        raise NotImplemented()

    def at_least_one_number_greater_then_zero():
        is_p_set = p in defined_statuses
        is_p_set_to_bomb = p in defined_statuses and defined_statuses[p] == -1

        adj_cells = get_adjacent_cells(p[0], p[1], rows, cols)

        good_pts = [cell for cell in adj_cells if cell in defined_statuses and defined_statuses[cell] > 0]
        bad_pts = [cell for cell in adj_cells if cell in defined_statuses and defined_statuses[cell] <= 0]

        parametrized_pts = [reduce(lambda x, y: x | y, variables[cell][2:], Cnf()) for cell in adj_cells if
                            cell not in defined_statuses]
        parametrized_formula_disjunct = reduce(lambda x, y: x | y, parametrized_pts, Cnf())

        if len(good_pts) > 0:
            return Cnf()
        if is_p_set_to_bomb == False and is_p_set:
            return Cnf()

        if len(bad_pts) == len(adj_cells) and is_p_set_to_bomb:
            return None
        if len(bad_pts) == len(adj_cells) and is_p_set:
            return Cnf()
        if len(bad_pts) == len(adj_cells) and not is_p_set:
            return -variables[p][0]

        if is_p_set_to_bomb:
            return parametrized_formula_disjunct

        return variables[p][0] >> parametrized_formula_disjunct

        if is_p_set and not is_p_set_to_bomb:
            return Cnf()

        if len(bad_pts) > 0 and is_p_set_to_bomb:
            return None
        elif len(bad_pts) > 0 and is_p_set == False:
            return -variables[p][0]
        elif len(bad_pts) == 0 and is_p_set:
            return parametrized_formula_conjunct
        elif len(bad_pts) == 0 and is_p_set == False:
            return variables[p][0] >> parametrized_formula_conjunct

        raise NotImplemented()

    S = no_zero_number_in_adjacent()
    if S is None:
        return S
    T = at_least_one_number_greater_then_zero()
    if T is None:
        return T
    return T & S


def build_formula(rows, cols, positions: List[Tuple[int, int, int]], position_to_check: Tuple[int, int]):
    # we will check weather or not formula satisfiable given configuration and bomb in position

    # c_{i,j,status} status = 1...9 -- (amount of bombs-1) aka shifted by one on right.
    # and 0 means --- in this cell is bomb

    # eliminate offset
    defined_statuses = {(i, j): k for (i, j, k) in positions}

    # place bomb
    defined_statuses[position_to_check] = -1

    variables = {}

    corner_cells = [(1, 1), (rows, cols), (1, cols), (rows, 1)]
    # only 5 cases - 0,1,2,3 or bomb (-1) for (1,1),(m,n), (1,n), (m,1)
    for (i, j) in corner_cells:
        if (i, j) not in defined_statuses:
            lst = [Variable('c_{}_{}_{}'.format(i, j, c)) for c in range(-1, 4) if (i, j, c) not in defined_statuses]
            variables[(i, j)] = lst

    # only 0,1,2,3,4,5 and bomb --- 7 variants for edges
    for i in range(1, rows + 1):
        for j in range(1, cols + 1):
            if (i, j) not in defined_statuses and (i == 1 or j == 1 or i == rows or j == cols) and \
                    (i, j) not in corner_cells:
                lst = [Variable('c_{}_{}_{}'.format(i, j, c)) for c in range(-1, 6)]
                variables[(i, j)] = lst

    # inner thing
    for i in range(2, rows):
        for j in range(2, cols):
            if (i, j) not in defined_statuses:
                lst = [Variable('c_{}_{}_{}'.format(i, j, c)) for c in range(-1, 9) if (i, j, c) not in positions]
                variables[(i, j)] = lst

    S1 = Cnf()
    # all cells have some status if they in cells not placed something
    for k, v in variables.items():
        have_status_ij = Cnf()
        for c_ijc in v:
            have_status_ij |= c_ijc
        S1 &= have_status_ij

    # is only one status
    S2 = Cnf()
    for k, v in variables.items():
        for pair in itertools.combinations(v, 2):
            S2 &= (-pair[0] | -pair[1])

    S3 = Cnf()

    for i in range(1, rows + 1):
        for j in range(1, cols + 1):
            c = number_constraint_for_cell((i, j), rows, cols, variables, defined_statuses)
            if c is None:
                return None
            else:
                S3 &= c

            c = bomb_constraint_for_cell((i, j), rows, cols, variables, defined_statuses)
            if c is None:
                return None
            else:
                S3 &= c

    return S1 & S2 & S3


def solve(rows, cols, positions: List[Tuple[int, int, int]], position_to_check: Tuple[int, int]):
    solver = Minisat()

    formula = build_formula(rows, cols, positions, position_to_check)

    if formula is None:
        return 'Given cell is free'
    else:
        if solver.solve(formula).success:
            c = solver.solve(formula)

            return 'Given cell is not free'
        else:
            return 'Given cell is free'


def get_positions_from_configuration(path, exclude_pos):
    positions = []
    with open(path) as f:
        lines = f.readlines()
        lines = list(map(lambda x: x.split(' '), lines))

        for i in range(1, len(lines) + 1):
            for j in range(1, len(lines[0]) + 1):
                if (i, j) == exclude_pos:
                    continue
                v = lines[i - 1][j - 1].replace('\n', '')
                if v == '?':
                    continue
                if v == '.':
                    positions.append((i, j, -1))
                    continue
                positions.append((i, j, int(v)))

    return positions


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Program checks weather or not given cell is safe for current configuration of minesweeper game')
    parser.add_argument('rows', metavar='rows', type=int, help='an number of rows')
    parser.add_argument('cols', metavar='cols', type=int, help='an number of cols')
    parser.add_argument('i', metavar='i', type=int, help='an index of row cell to check')
    parser.add_argument('j', metavar='j', type=int, help='an index of col cell to check')
    parser.add_argument('path', metavar='path', type=str, help='path to configuration')

    args = parser.parse_args()
    print(solve(args.rows, args.cols, get_positions_from_configuration(args.path, (args.i, args.j)), (args.i, args.j)))

#
# 0 1 0
# 0 1 1
# 0 0 0


# 2 ? ? 0
# ? ? 1 0
# 1 ? ? 0
# 0 0 0 0
