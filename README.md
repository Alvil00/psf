# Purpose
A small command-line program that finds the mirror symmetry axes of a convex polygon.

# Usage
```bash
./psf <file> [-t <tolerance>] [-o <file>] [-v] [-h]
```
Run `./psf -h` for the options, the output format and the exit codes.

# Input file format
One polygon vertex per line: the X and Y coordinates as two numbers separated by spaces or tabs.
- Vertices go in boundary order, clockwise or counterclockwise. The polygon closes itself, so the first vertex is not repeated at the end.
- `#` starts a comment that runs to the end of the line, also after the coordinates.
- Blank lines are skipped.

```
<X0> <Y0>
<X1> <Y1>
.........
```

# Compilation with tests
Requires Conan 2, CMake 4.3.2 or newer and a C++17 compiler.
Verified with LLVM clang 22 on macOS and MSVC 14.29 (Visual Studio 2019) on Windows.

```bash
conan install --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
ctest --preset conan-release
```
On Windows (MSVC) configure with `cmake --preset conan-default`.

To build without tests, add `-c tools.build:skip_test=True` to `conan install`.

# Author
Vilenskii Alexey 2026
