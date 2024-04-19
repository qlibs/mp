#!/usr/bin/env python

from pathlib import Path
import os, sys, time, subprocess, shutil
import numpy as np

def at(N):
    str = f"template<int> struct x{{}};\n"
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

def benchmark(rng, runs, cxx, test):
    results = {}
    for dir in Path('benchmark').iterdir():
        if dir.is_dir():
            results[dir.name] = {}
            for file in dir.iterdir():
                sut = f'{file.name}'
                if sut not in test:
                    continue

                results[dir.name][sut] = {}
                for i in rng:
                    path = f'/tmp/{dir.name}_{file.name}_{i}'

                    with open(f'benchmark/{dir.name}/{file.name}', 'r') as f:
                        with open(path + '_include.cpp', 'w') as include:
                            with open(path + '.cpp', 'w') as cpp:
                                for line in f:
                                    if "#include" in line or "#define" in line:
                                        include.write(line)
                                    cpp.write(line)
                                cpp.write(eval(dir.name)(i))

                    include = []
                    for _ in range(runs):
                        include.append(timeit(f'{cxx} -c {path}_include.cpp -o {path}.o'))

                    cpp = []
                    for _ in range(runs):
                        cpp.append(timeit(f'{cxx} -c {path}.cpp -o {path}.o'))

                    results[dir.name][sut][i] = (np.mean(include), np.mean(cpp))

    return results

def save(name, results):
    path = f'results/{name}'
    if os.path.exists(path):
        shutil.rmtree(path)
    os.mkdir(path)

    for result in results:
        with open(f'{path}/{result}.csv', 'w') as csv:
            data = results[result]
            csv.write('n,' + ','.join(data) + '\n')
            keys = set().union(*(d.keys() for d in data.values()))
            for key in sorted(keys):
                str = f'{key}'
                for name, time in zip(data.keys(), (data[name].get(key) for name in data.keys())):
                    str += f',{time[1]}'
                csv.write(str + '\n')

        with open(f'{path}/{result}.raw.csv', 'w') as csv:
            data = results[result]
            csv.write('n,' + ','.join(data) + '\n')
            keys = set().union(*(d.keys() for d in data.values()))
            for key in sorted(keys):
                str = f'{key}'
                for name, time in zip(data.keys(), (data[name].get(key) for name in data.keys())):
                    str += f',{max(0, time[1]-time[0])}'
                csv.write(str + '\n')

save('clang-p2996', benchmark(range(0, 110, 10), runs=3, cxx="clang++-19 -std=c++2c -stdlib=libc++ -freflection", test=['mp', 'mp11', 'nth_pack_element', 'p1858', 'p2996', 'type_pack_element']))
save('clang-17', benchmark(range(0, 110, 10), runs=3, cxx="clang++-17 -std=c++20", test=['mp', 'mp11', 'nth_pack_element', 'type_pack_element']))
save('gcc-13', benchmark(range(0, 110, 10), runs=3, cxx="g++-13 -std=c++20", test=['mp', 'mp11', 'nth_pack_element']))
save('circle', benchmark(range(0, 110, 10), runs=3, cxx="circle -std=c++20", test=['circle', 'mp11', 'nth_pack_element', 'p1858']))
