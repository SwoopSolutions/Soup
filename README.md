# Soup

The everything library for C++ 17 and up.

## Building

### Visual Studio

Simply open up Soup.sln using Visual Studio 2022 and run a batch build for the Soup project to compile the static libraries, then you can use it in your own projects by adding a compiler and linker include, respectively.

### PHP Build Scripts

You can build Soup using `build_lib.php` which will produce a static lib.

### Sun

Soup can be built using our own build system, [Sun](https://github.com/calamity-inc/Sun). To include it in your own projects, we recommend the directory structure such that your own project and Soup share the same parent, then you can simply add the following to the .sun file for your project:

```
require ../Soup/soup include_dir=../Soup
```

This will make it so that you have to use `#include <soup/NAME.hpp>` in your code, which we recommend to avoid name clashes, similar to opting not to use `using namespace`. However, if you omit the `include_dir` part, you can use `#include <NAME.hpp>` directly.

### Bindings

To build Soup as a DLL/SO with C ABI exports, run `php build_lib.php` (in the root folder) or `sun dynamic` (in the "soup" folder).

For WASM, it can be done via `php wasm.php` (in the root folder) or `sun wasm` (in the "soup" folder).

## Getting Started

If you want some ideas as to how start using Soup, check out [the docs](docs).

If you're looking to use Soup from a language other than C++, have a look at [the bindings](https://github.com/calamity-inc/Soup/tree/senpai/bindings).
