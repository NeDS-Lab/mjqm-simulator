import sys
from pathlib import Path

import math

print_speedup = True


def load_results(path: Path):
    data = {}
    with open(path, 'r') as file:
        columns = file.readline().strip().split(';')
        for line in file:
            values = line.strip().split(';')
            if values[0]:
                data[values[0]] = {}
                for column, value in zip(columns[1:], values[1:]):
                    data[values[0]][column] = value
    return columns, data


def print_with_columns_key(columns, data, filter_column=None):
    for key, values in data.items():
        print()
        print(f'{columns[0]}: {key}')
        for column, value in zip(columns[1:], values):
            if filter_column is None or filter_column(column):
                print(f'{column}: {value}')


def compare_results(data1, data2):
    columns1, data1 = data1
    columns2, data2 = data2
    if any(map(lambda x: x not in columns1, columns2)):
        print('Missing stat in curr')
        print(set(columns2) - set(columns1))
    if any(map(lambda x: x not in data1.keys(), data2.keys())):
        print('Missing keys in curr')
        print(set(data2.keys()) - set(data1.keys()))
    different = False
    for key in data2.keys():
        if key not in data1:
            continue
        key_header = False
        duration_diff = ""
        for column in columns2[1:]:
            if not column or column not in data1[key] or column.endswith('ConfInt') or not data2[key][column]:
                continue
            curr = float(data1[key][column])
            prev = float(data2[key][column])
            if not math.isclose(curr, prev, rel_tol=1e-3):
                if column == 'Run Duration':
                    if curr < prev:
                        if curr == 0:
                            duration_diff = f'Speedup: {prev}/0'
                        else:
                            duration_diff = f'Speedup: {(1 - curr / prev) * 100:.2f}%'
                    else:
                        duration_diff = f'Slowdown: {(curr / prev - 1) * 100:.2f}%'
                    continue
                if not key_header:
                    print(f'{columns2[0]}: {key}')
                    key_header = True
                print(f'\t{column}:')
                if "Stability Check" not in column:
                    print(f'\t\tcurr: {data1[key][column]} {data1[key][column + " ConfInt"]}')
                    print(f'\t\tprev: {data2[key][column]} {data2[key][column + " ConfInt"]}')
                else:
                    print(f'\t\tcurr: {data1[key][column]}')
                    print(f'\t\tprev: {data2[key][column]}')
                different = True
        if print_speedup and duration_diff:
            if not key_header:
                print(f'{columns2[0]}: {key}')
            print(f'\t{duration_diff} (curr: {data1[key]["Run Duration"]} ; prev: {data2[key]["Run Duration"]})')
    if not different:
        print('Data is the same')


if __name__ == '__main__':
    # get the two paths from arguments
    path1 = Path(sys.argv[1])
    path2 = Path(sys.argv[2])
    data1 = load_results(path1)
    data2 = load_results(path2)
    # print(path1)
    # print_with_columns_key(*data1, filter_column=lambda x: 'Run Duration' in x)
    # print()
    # print(path2)
    # print_with_columns_key(*data2, filter_column=lambda x: 'Run Duration' in x)
    compare_results(data1, data2)
