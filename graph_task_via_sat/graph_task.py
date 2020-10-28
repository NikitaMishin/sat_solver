import itertools

from satispy import Variable, Cnf
from satispy.solver import Minisat
import argparse


def build_formula(k, n):
    variables = {}
    for i in range(1, n + 1):
        for j in range(i + 1, n + 1):
            lst = [Variable('e_{}_{}_{}'.format(i, j, c)) for c in range(1, k + 1)]
            variables[(i, j)] = lst

    S1 = Cnf()

    # all edge is colored
    for k, v in variables.items():
        have_colored_ij = Cnf()
        for e_ijc in v:
            have_colored_ij |= e_ijc
        S1 &= have_colored_ij

    # colored only once
    S2 = Cnf()
    for k, v in variables.items():
        for pair in itertools.combinations(v, 2):
            S2 &= (-pair[0] | -pair[1])

    S3 = Cnf()

    for i in range(1, n + 1):
        for j in range(i + 1, n + 1):
            for k in range(j + 1, n + 1):

                eij = variables[(j, i)] if (j, i) in variables else variables[(i, j)]
                ejk = variables[(j, k)] if (j, k) in variables else variables[(k, j)]
                eki = variables[(k, i)] if (k, i) in variables else variables[(i, k)]
                for (a, b, c) in zip(eki, ejk, eij):
                    S3 &= (-a | -b | -c)

    return S1 & S2 & S3


def solve(k):
    solver = Minisat()
    n = 3
    max_res = 3

    while True:
        formula = build_formula(k, n)
        solution = solver.solve(formula)
        if solution.success:
            max_res = n
        else:
            break
        print("Exist for {}".format(n))
        n += 1

    return max_res


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Find max size of clique for k colors such that each triangle of edges  not fully colored in same color')
    parser.add_argument('k', metavar='k', type=int, help='an number of colors')

    args = parser.parse_args()
    print(solve(args.k))
