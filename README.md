# :dragon: macro-expand

<p align="center">
  <br><br>
  A clang tool for expanding preprocessor directives to make it MISRA 16.X.X complaint.
  <br><br>
   <img alt="Docker Pulls" src="https://img.shields.io/docker/pulls/mw-smenon/macro-expand.svg"/>  <img alt="Github All Releases" src="https://img.shields.io/github/downloads/mw-smenon/macro-expand/total.svg"/> <img alt="GitHub release" src="https://img.shields.io/github/release/mw-smenon/macro-expand.svg"/>
</p>

## Overview

This tool (*macro-expand*) helps you to refactor your code and make sure that it is compliant with MISRA Rule #16.0.4, #16.2.1 and #16.2.2.  

In other words, this tool does the following things:
1. It expands all #define statements that are used in the working code, these could be function-like macros or object-like macros.
2. It goes through the macro definitions and removes any unused #define statement that is among the source files. It also removes any #undefs for corresponding definitions that have been removed.


1. Replace parameters with respective argument expressions. That is, for a
function `f(int x)` that you call with `f(5)`, macro-expand will rewrite every
occurrence of `x` inside `f` to `5`. Note that since macro-expand uses clang, it
actually understands C++ and knows what occurrences of `x` are parameter
references and what aren't. Default arguments are replaced as well.

<p align="center">
<table align="center">
<tr><th colspan="2">Given</th></tr>
<tr valign="top"><td colspan="2"><sub><pre lang="cpp">
#include &lt;vector&gt;
#include &lt;iostream&gt;
#include &lt;string&gt;
#include &lt;cstdio.h&gt;
#define ERASE(A,B) A.erase(B)
#define ONE 1
#define CAT(a,b) a##b
#define MY_PRINTF(…) fprintf (stdout, __VA_ARGS__)

template&lt;typename Range&gt;
void magic(Range& range, int meaning_of_life = 42) {
  auto iterator = std::find(range.begin(), range.end(), meaning_of_life);
  if (iterator != range.end()) {
    ERASE(range, iterator);
    std::cout << "Successfully erased all meaning of life\n";
  }
}
int main()
{
    std::vector<int> v{ONE, 42, 3};
    magic(v);
#define STRINGOF(IN) std::string(#IN)
    std::cout << STRINGOF(MeaningOfLife) << CAT(U,"Undefined");
#undef STRINGOF
    MY_PRINTF("All %d operations done", 3);
    return 0;
}
</pre></sub></td></tr>
<tr><th>Object-Like Expanded</th><th>Function-Like Expanded</th></tr>
<tr valign="top">
<td><sub><pre lang="cpp">
#include &lt;vector&gt;
#include &lt;iostream&gt;
#include &lt;string&gt;
#include &lt;cstdio.h&gt;
#define ERASE(A,B) A.erase(B)
#define CAT(a,b) a##b
#define MY_PRINTF(…) fprintf (stdout, __VA_ARGS__)

template&lt;typename Range&gt;
void magic(Range& range, int meaning_of_life = 42) {
  auto iterator = std::find(range.begin(), range.end(), meaning_of_life);
  if (iterator != range.end()) {
    ERASE(range, iterator);
    std::cout << "Successfully erased all meaning of life\n";
  }
}
int main()
{
    std::vector<int> v{1, 42, 3};
    magic(v);
#define STRINGOF(IN) std::string(#IN)
    std::cout << STRINGOF(MeaningOfLife) << CAT(U,"Undefined");
#undef STRINGOF
    MY_PRINTF("All %d operations done", 3);
    return 0;
}
</pre></sub></td>
<td><sub><pre lang="cpp">
#include &lt;vector&gt;
#include &lt;iostream&gt;
#include &lt;string&gt;
#include &lt;cstdio.h&gt;
#define ONE 1

template&lt;typename Range&gt;
void magic(Range& range, int meaning_of_life = 42) {
  auto iterator = std::find(range.begin(), range.end(), meaning_of_life);
  if (iterator != range.end()) {
    range.erase(iterator);
    std::cout << "Successfully erased all meaning of life\n";
  }
}
int main()
{
    std::vector<int> v{ONE, 42, 3};
    magic(v);
    std::cout << std::string("MeaningOfLife") << U"Undefined";
    fprintf(stdout, "All %d operations done", 3);
    return 0;
}
</pre></sub></td>
</tr>
</table>
</p>

## Usage

macro-expand is implemented as a command-line tool targeted at building editor integrations. The tool itself has the following help text (excerpt):

```
$ macro-expand -help
USAGE: macro-expand [options] <source0> [... <sourceN>]

OPTIONS:

macro-expand options:

  -fcnExp=     - [true] Whether to replace function like macros. For example, "#define USTR(a) U ## a".
  -objExp=     - [true] Whether to replace object like macros. For example, "#define PI 3.14159"
  -remUnused=  - [true] Whether to remove unused macro definitions from non-system source files
  -rewrite=    - [true] Whether to rewrite the original source files
```

Basically, you have to pass it any sources you want the tool to look for definitions in as arguments.
Additionally, you have to pass any options required to compile the files at the
end, following `--`. For example, given:

`foo.h`:
```cpp
#define MeaningOfLife 42
int foo();
```

`foo.cpp`:
```cpp
int foo() { return MeaningOfLife; }
```

`main.cpp`:
```cpp
#include "foo.h"
auto main() -> int {
  auto x = foo();
}
```

The following command would do the job:

```bash
$ macro-expand main.cpp foo.cpp -rewrite=false -- -I/path/to/include -std=c++14
```

which will output:

```json
{
  "call": {
    "begin": {
      "column": 3,
      "line": 3
    },
    "end": {
      "column": 17,
      "line": 3
    }
  },
  "declaration": {
    "location": {
      "filename": "/path/to/foo.h",
      "offset": {
        "column": 5,
        "line": 1
      }
    },
    "name": "foo",
    "text": "int foo();"
  },
    "macro": true,
    "rewritten": "int x = 42;",
    "text": "int foo() { return 42; }"
  }
}
```

Yes, JSON! But why so much output? Well, since macro-expand has to find the
declaration and definition of a function you want to expand, it might as well
also return the location and full text for either (because why not). As such,
macro-expand can also be used as a backend for
"go-to-definition"/"show-declaration" functions inside a text editor (though an
indexed-solution like [ctags](https://en.wikipedia.org/wiki/Ctags) is likely
faster for just that).

For expanding, what's most interesting here is the `call` section and the
`definition.rewritten` field. The former is the entire range (defined by two
`(line, column)` pairs) in the source code that you'll want to replace with the
expansion. The latter is the text to insert instead.


## Limitations

`macro-expand` does not (yet) recursively expand macros. ie. Function like macros that invoke other function like macros. What this means is that macro-expand needs to be called multiple times until there is no more replacement required.

## Building

If you just want to use macro-expand, you can grab the executable from the
[Release](https://github.com/goldsborough/macro-expand/releases) page.

To build from source, macro-expand uses cmake and requires a C++14-capable compiler.
It also depends on the full [LLVM](https://github.com/llvm-mirror/llvm) and
[clang](https://github.com/llvm-mirror/clang) source, so you will need to download
those, as explained [here](http://llvm.org/docs/GettingStarted.html#for-developers-to-work-with-a-git-monorepo).
Clang/LLVM does have support for gcc/msvc. Just make sure you compile
the LLVM and clang libraries with the same compiler and standard library as you do macro-expand.

Once you have all that, you can build with:

```bash
$ mkdir build && cd build
$ cmake -DLLVM_PATH=/path/to/llvm/ -DFIND_LLVM_VERBOSE_CONFIG=on ..
```

### Docker

I provide Dockerfiles for Debian, Ubuntu, Fedora and OpenSUSE based images that, once built, have LLVM and clang libraries installed and compiled and contain build scripts to compile the project inside the Docker containers. While this is mainly to make it easier to create reproducible, fast and isolated releases of macro-expand on each of these distributions, these containers may actually be the easiest way for you to compile the project and make changes to it. To build a single container, run something like:

```sh
$ docker build --compress --memory 2G --tag macro-expand:<os> --file docker/<os>.Dockerfile
```

where `os` is in `{ubuntu, debian, fedora, opensuse}`.

To then build the project inside the container, you can run:

```sh
$ docker run -v build:/home/build -v llvm-build:/llvm/build -v $PWD:/home/project -v $PWD/bin:/home/build/bin macro-expand:<os> ./build.sh <os>
```

where `os` is again one of the above. To explain the volumes we are mounting here:

1. The named volume `build` is where the project will be built with cmake,
2. The named volume `llvm-build` is where LLVM and clang will be built with cmake,
3. `$PWD:/home/macro-expand` mounts your local macro-expand directory under /home,
4. `$PWD/bin:/home/build/bin` is where all the binaries go. Mount it on the host if you want to keep the binaries.

You can also just run `docker-compose up` (provided you have `docker-compose` installed) from the project root to build macro-expand on all distributions. Our cmake also has a docker target, so `make docker` does the same as `docker-compose up`.

### Windows

On Windows, you'll want to use cmake-gui and let it generate a solution for Visual Studio 2017 (make sure `llvm-config` is either in the `PATH` or set the variable manually). Depending on how you've built LLVM you could have to add `mincore.lib` as an additional library dependency to the clang-generate project to fix some undefined external symbols.

You could also run these commands:
```sh
$ cd build/
$ cmake -G "Visual Studio 15 2017 Win64" ../
```
now open the generated project.
I have hard coded some paths in the CMAKELists.txt file for where I locally built LLVM on Windows, you should probably replace those as well.

*In you are using windows with Clang 3.8.0, make sure you also work around some imcompatabilities with clang and Windows!*

Here's how - when you invoke `macro-expand`, do it in this manner:
```sh
$ macro-expand.exe *.cpp -- -std=c++14 -fms-compatibility-version=19.0 -D__is_assignable=__is_trivially_assignable
```

## Documentation

macro-expand has very extensive in-source documentation which can be generated
with [Doxygen](http://www.doxygen.org). Run `make docs` inside the `build`
folder. You don't need to compile the project for this, just run `cmake ..` and
then `make` the `docs` target.

## License

TBD

## Authors

[Siddharth Menon](https://nam.delve.office.com/?u=2b5cc3f5-f271-4363-ab56-2d883b4f1f37&v=work)
:heart:
