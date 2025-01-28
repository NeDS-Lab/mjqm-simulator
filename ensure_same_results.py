import sys
from pathlib import Path

def load_results(path: Path):
    data = {}
    with open(path, 'r') as file:
        columns = file.readline().strip().split(';')
        for line in file:
            values = line.strip().split(';')
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
        print('Missing stat in ours')
        print(set(columns2) - set(columns1))
        return
    if any(map(lambda x: x not in data1.keys(), data2.keys())):
        print('Missing keys in ours')
        print(set(data2.keys()) - set(data1.keys()))
        return
    different = False
    for key in data2.keys():
        for column in columns2[1:]:
            if data1[key][column] != data2[key][column]:
                if column == 'Run Duration ConfInt':
                    continue
                if column == 'Run Duration':
                    ours = float(data1[key][column])
                    theirs = float(data2[key][column])
                    if ours < theirs:
                        if ours == 0:
                            print(f'Speedup: {theirs}/0 ({columns2[0]} {key})')
                        else:
                            print(f'Speedup: {(1 - ours / theirs)*100:.2f}% ({columns2[0]} {key})')
                    else:
                        print(f'Slowdown: {(ours / theirs - 1)*100:.2f}% ({columns2[0]} {key})')
                    continue
                print(f'{columns2[0]}: {key}, Column: {column} '
                      f'Data1: {data1[key][column]} ')
                print(f'{columns2[0]}: {key}, Column: {column} '
                      f'Data2: {data2[key][column]} ')
                different = True
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
