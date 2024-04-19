#!/usr/bin/env python

from pathlib import Path
import os, sys, time, subprocess
import numpy as np

def at(N):
    str = f"template<int> struct x;\n"
    for i in range(N):
        str += f"using x_{i} = at<{i}, {','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def drop(N):
    str = f"template<int> struct x;\n"
    for i in range(N):
        str += f"using x_{i} = drop<{i}, {','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def erase(N):
    str = f"template<int> struct x;\n"
    for i in range(N):
        str += f"using x_{i} = erase<{i}, {','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def filter(N):
    str = f"template<int N> struct x {{ static constexpr auto value = N; }};\n"
    for i in range(N):
        str += f"using x_{i} = filter<{','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def insert(N):
    str = f"template<int> struct x;\n"
    for i in range(N):
        str += f"using x_{i} = insert<{i}, void, {','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def reverse(N):
    str = f"template<int> struct x;\n"
    for i in range(N):
        str += f"using x_{i} = reverse<{','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def take(N):
    str = f"template<int> struct x;\n"
    for i in range(N):
        str += f"using x_{i} = take<{i}, {','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def unique(N):
    str = f"template<int> struct x;\n"
    for i in range(N):
        str += f"using x_{i} = unique<{','.join([f'x<{n}>' for n in range(N)])}, {','.join([f'x<{n}>' for n in range(N)])}>;\n"
    return str

def timeit(command):
    start_time = time.time()
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    print(str(result).replace('\\n', '\n'))
    if result.returncode != 0:
        sys.exit(result.returncode)
    end_time = time.time()
    execution_time = end_time - start_time
    return execution_time

def bench(n, step, runs, cxx, filter):
    results = {}
    for dir in Path('benchmark').iterdir():
        if dir.is_dir():
            results[dir.name] = {}
            for file in dir.iterdir():
                sut = f'{file.name}'
                if sut not in filter:
                    continue

                results[dir.name][sut] = {}
                for i in range(n, step):
                    path = f'{dir.name}_{file.name}_{i}.cpp'
                    with open(path, 'w') as tmp:
                        with open(f'benchmark/{dir.name}/{file.name}', 'r') as f:
                            for line in f:
                                tmp.write(line)
                            tmp.write(eval(dir.name)(i))

                    time = []
                    for _ in range(runs):
                        time.append(timeit(f'{cxx} -c {path}'))

                    results[dir.name][sut][i] = np.mean(time)

    return results

def results(cxx, bench):
    for result in bench:
        path = f'results/{cxx}'
        if os.path.exists(path):
            os.rmdir(path)
            os.mkdir(path)

        with open(f'{path}/{result}.csv', 'w') as csv:
            csv.write('n,' + ','.join(bench[result]) + '\n')
            data = bench[result]
            all_keys = set().union(*(d.keys() for d in data.values()))
            for key in sorted(all_keys):
                str = f'{key}'
                for name, values in zip(data.keys(), (data[name].get(key) for name in data.keys())):
                    str += f',{values}'
                csv.write(str + '\n')

results('gcc-13:', bench(n=100, step=10, runs=3, cxx="g++-13 -std=c++20", ['mp', 'mp11', 'nth_pack_element']))
results('clang-17:', bench(n=100, step=10, runs=3, cxx="clang++-17 -std=c++20", ['mp', 'mp11', 'nth_pack_element', 'type_pack_element']))
results('clang-p2996', bench(n=100, step=10, runs=3, cxx="clang++-19 -std=c++2c -stdlib=libc++ -freflection", ['mp', 'mp11', 'nth_pack_element', 'p1858', 'p2996', 'type_pack_element']))
