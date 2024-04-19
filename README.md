## Meta-Programming benchmark (https://boost-ext.github.io/mp)

- https://godbolt.org/z/6KzzEMGbe

### Libraries

- [boost.mp11](https://github.com/boostorg/mp11)
- [mp](https://github.com/boost-ext/mp)

### Proposals

- [P2996 - Reflection for C++26](https://wg21.link/P2996)
- [P1858 - Generalized pack declaration and usage](https://wg21.link/P1858)

### Compilers

- [gcc](https://gcc.gnu.org)
- [clang](https://clang.llvm.org)
- [clang-p2996](https://github.com/bloomberg/clang-p2996)
- [circle](https://www.circle-lang.org)

---

### Usage

```sh
git clone https://github.com/boost-ext/mp
git co benchmark
```

```sh
./benchmark.py # to run benchmarks
python3 -m http.server && open localhost:8000 # to open results
```
