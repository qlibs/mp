## Metabench <a target="_blank" href="https://travis-ci.org/ldionne/metabench">![Travis status][badge.Travis]</a> <a target="_blank" href="https://ci.appveyor.com/project/ldionne/metabench">![Appveyor status][badge.Appveyor]</a>
> A simple framework for compile-time microbenchmarks


### Overview
Metabench is a single, self-contained CMake module making it easy to create
compile-time microbenchmarks. Compile-time benchmarks measure the performance
of compiling a piece of code instead of measuring the performance of running
it, as regular benchmarks do. The __micro__ part in **micro**benchmark means
that Metabench can be used to benchmark precise parts of a C++ file, such as
the instantiation of a single function. Writing benchmarks of this kind is
very useful for C++ programmers writing metaprogramming-heavy libraries, which
are known to cause long compilation times. Metabench was designed to be very
simple to use, while still allowing fairly complex benchmarks to be written.

Metabench is also a collection of compile-time microbenchmarks written using
the `metabench.cmake` module. The benchmarks measure the compile-time performance
of various algorithms provided by different metaprogramming libraries. The
benchmarks are updated nightly with the latest version of each library, and
the results are published at http://metaben.ch.

### Requirements
Metabench requires [CMake][] 3.1 or higher and [Ruby][] 2.1 or higher.
Metabench is known to work with CMake's _Unix Makefiles_ and _Ninja_
generators.

### Usage
To use Metabench, make sure you have the dependencies listed above and simply
drop the `metabench.cmake` file somewhere in your CMake search path for modules.
Then, use `include(metabench)` to include the module in your CMake file, add
individual datasets to be benchmarked using `metabench_add_dataset`, and finally
specify which datasets should be put together into a chart via `metabench_add_chart`.
For example, a minimal CMake file using Metabench would look like:

```CMake
# Make sure Metabench can be found when writing include(metabench)
list(APPEND CMAKE_MODULE_PATH "path/to/metabench/directory")

# Actually include the module
include(metabench)

# Add new datasets
metabench_add_dataset(dataset1 "path/to/dataset1.cpp.erb" "[1, 5, 10]")
metabench_add_dataset(dataset2 "path/to/dataset2.cpp.erb" "(1...15)")
metabench_add_dataset(dataset3 "path/to/dataset3.cpp.erb" "(1...20).step(5)")

# Add a new chart
metabench_add_chart(chart DATASETS dataset1 dataset2 dataset3)
```

This will create a target named `chart`, which, when run, will gather benchmark
data from each `dataset` and output JSON files for easy integration with other
tools. A HTML file is generated for easy visualization of the datasets as a
[NVD3][] chart. To understand what the `path/to/datasetN.cpp.erb` files are,
read what follows.

#### The principle
Benchmarking the compilation time of a single `.cpp` file is rather useless,
because one could simply run the compiler and time that single execution instead.
What is really useful is to have a means of running variations of the same
`.cpp` file automatically. For example, we might be interested in benchmarking
the compilation time for creating a `std::tuple` with many elements in it. To
do so, we could write the following test case:

```c++
#include <tuple>

int main() {
    auto tuple = std::make_tuple(1, 2, 3, 4, 5);
}
```

We would run the compiler and time the compilation, and then change the test
case by augmenting the number of elements in the tuple:

```c++
#include <tuple>

int main() {
    auto tuple = std::make_tuple(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}
```

We would measure the compilation time for this file, and repeat the process
until satisfactory data has been gathered. This tedious task of generating
different (but obviously related) `.cpp` files and running the compiler to
gather timings is what Metabench automates. It does this by taking a `.cpp.erb`
file written using the ERB template system, and generating a family of `.cpp`
files from that template. It then compiles these `.cpp` files and gathers
benchmark data from these compilations.

Concretely, you start by writing a `.cpp.erb` file (say `std_tuple.cpp.erb`)
that may contain ERB markup:

```c++
#include <tuple>

int main() {
    auto tuple = std::make_tuple(<%= (1..n).to_a.join(', ') %>);
}
```

Code contained inside `<%= ... %>` is just normal Ruby code. When the file
will be rendered, the contents of `<%= ... %>` will be replaced with the
result of evaluating this Ruby code, which will look like:

```c++
#include <tuple>

int main() {
    auto tuple = std::make_tuple(1, 2, 3, ..., n);
}
```

The ERB markup language has many other features; we encourage readers to take
a look at the [Wikipedia page][ERB]. What happens is that Metabench will
generate a `.cpp` file for different values of `n`, and will gather benchmark
data for each of these values. Now, this isn't the whole story. More often
than not, we're only interested in benchmarking part of a C++ file. Indeed,
if we benchmark the whole file in our example above, we'll end up measuring
the time required to `#include` the `<tuple>` header in addition to the time
required for creating the `std::tuple`. While this might be negligible in our
example, this situation arises in nontrivial examples, and would make the
resulting data nearly worthless. Hence, we have to tell Metabench what part(s)
of the file it should measure. This is done by guarding the relevant part(s)
of the code with a preprocessor `#if`:

```c++
#include <tuple>

int main() {
#if defined(METABENCH)
    auto tuple = std::make_tuple(<%= (1..n).to_a.join(', ') %>);
#endif
}
```

What Metabench will actually do is compile the file once with the macro defined
(and hence with the content of the block), and once without it. It will then
subtract the time for compiling the file without the content of the block to
the time for compiling the whole file, which should represent a good
approximation of the time for compiling what's inside the block.

On the C++ side of things, the `.cpp` file will be compiled (to benchmark it)
as if it were located in the directory containing the `.cpp.erb` file, so that
relative include paths can be used. Furthermore, it will be compiled as if the
`.cpp` file were part of a CMake executable added in the same directory as the
call to `metabench_add_dataset`. This way, any variable or property set in CMake
will also apply when benchmarking the file. In other words, Metabench tries to
create the illusion that the code is actually compiled as if it were written
in the `.cpp.erb` file.

This is it for the basic usage of the module! The `example/` directory contains
a fully working example of using Metabench to create benchmarks. For a more
involved example, you can take a look at the benchmark suite in the `benchmark/`
directory. Note that only the most basic usage of Metabench was covered here.
To know all the features provided by the module, you should read the reference
documentation provided as comments inside the CMake module.

#### A note on benchmark resolution
Like any measurement tool, Metabench has a limited resolution. For example, when
the code being measured (inside the `#ifdef METABENCH`/`#endif` pair) takes only
a few milliseconds to compile, the timings reported by Metabench may be completely
inside the noise. Typically, the resolution of timings taken by Metabench is
similar to that of the `time` command. A good technique to make sure the results
of a benchmark are not inside the noise is to reduce the _relative_ uncertainty
of the measurement. This can be done by increasing the total compilation time
of the measured block, by repeating the same thing (or a similar one) multiple
times:

```c++
#include <tuple>

int main() {
#if defined(METABENCH)
    auto tuple1 = std::make_tuple(<%= (1..n).to_a.join(', ') %>);
    auto tuple2 = std::make_tuple(<%= (1..n).to_a.join(', ') %>);
    auto tuple3 = std::make_tuple(<%= (1..n).to_a.join(', ') %>);
    auto tuple4 = std::make_tuple(<%= (1..n).to_a.join(', ') %>);
#endif
}
```

### History
Metabench was initially developed inside the [Boost.Hana][] library as a
mean to benchmark compile-time algorithms. After seeing that a self-standing
framework would be useful to the general C++ community, it was decided to
extract it into its own project.

### License
Please see [LICENSE.md](LICENSE.md).


<!-- Links -->
[badge.Appveyor]: https://ci.appveyor.com/api/projects/status/github/ldionne/metabench?svg=true&branch=master
[badge.Travis]: https://travis-ci.org/ldionne/metabench.svg?branch=master
[Boost.Hana]: http://github.com/boostorg/hana
[CMake]: http://www.cmake.org
[ERB]: http://en.wikipedia.org/wiki/ERuby
[Ruby]: https://www.ruby-lang.org/en/
[NVD3]: http://nvd3.org/
