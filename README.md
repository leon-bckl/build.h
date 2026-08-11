# build.h

Building C and C++ projects often requires an external build system like CMake, Meson and many more. This unnecesssarily complicates the build when all that should really be needed is a compiler.

This is where this single-header build system comes in. It lets you define your build targets and sources in C code that can be compiled into an executable with a simple comiler command. When run, the resulting program will automatically compile the project using the compiler that it was built with while managing toolchain-specific flags.

This project is inspired by [nob.h](https://github.com/tsoding/nob.h) but aims to abstract away the raw compiler commands, making the build more portable across compilers and operating systems. It is written in C89 to maximize compatibility and has zero external dependencies.

Right now only the compilation on windows is implemented but macOS and Linux will follow. The supported compilers are `msvc`, `clang` and `gcc`.

## Creating a project

A project is defined using the `begin_project` and `end_project` macros. `begin_project` has a single parameter which is an identifier for the name of the project. In between these two macros you can define targets like executables and libraries.

Here's a simple project called 'Test' which builds an executable called 'MyProgram':

```cpp
#include "build.h"

begin_project(Test)
	add_executable("MyProgram", "main.c");
end_project
```

That's really all that's needed for a simple project. Just build the file with a compiler of your choice and run the resulting executable. E.g., `clang build.c -o build && ./build`

## Command-line arguments

The following arguments can be used when calling the compiled build system executable:
	
- `-v` enables verbose output (e,g., printing compiler and linker command-lines).
- `-j<num>` defines the maximum number of parallel compilation jobs. The default is the number of cpu cores.
- `--debug` disables optimization and generates debug information.
- `--release` produces optimized binaries without debug information.
- `--profiling` is the same as release but with debug information.
- `-b<dirname>` specifies the build directory where the artifacts are created. By default it is `./.build`.
- `--incremental` only compiles the source files that have changed by looking at their lastmodified timestamp and comparing it against the compiled object file. This does not consider changes in included headers yet.
- `<target>` (without `-`) only builds the target with that name and its dependencies. Multiple targets can be specified.

## Targets

A target corresponds with an output binary like an executable or library. The target name defines the name of the binary file. Target names must be unique for the entire project and are case-insensitive.

Targets are created using `add_executable`, `add_shared_library` and `add_static_library`. The first parameter is always the target name, followed by a list of source files which can also be `NONE` and then specified later using `target_sources`.

The commands that work with targets all have the form `target_*` and take a target or target name as the first parameter. Most of these commands have a non-target specific variant that applies to all targets like for example `target_include_paths` and `include_paths` or `target_defines` and `defines`.

```cpp
add_executable("MyProgram", "main.c");
target_defines("MyProgram", "FOO=1");

/* Or alternatively using a target value instead of the name */

Target myProgram = add_executable("MyProgram", "main.c");
target_defines(myProgram, "FOO=1");
```

## Lists

Commands such as `target_sources`, `include_paths` and `defines` accept a list of multiple items as a parameter. Lists are strings where individual entries are separated from each other using whitespace, commas or quotes. A list entry that is quoted may contain whitespace and commas.

```cpp
target_sources(target, "foo.c bar.c");
include_paths("/usr/local/include \"/path with spaces/include\"");
defines("FOO,"
        "BAR,"
        "DEBUG=1");
```


## Sub-projects

It is possible to add projects defined in a separate source file. They need to either be compiled as their own translation unit or be included _after_ the main project. `add_project` is used to register the sub-project:

```cpp
#include "build.h"

begin_project(Main)
	add_project(Other); /* Add the targets defined in the 'Other' project */
end_project

#include "other.c" /* Defines a project called 'Other'. Needs to also include build.h */
```

## Logging

Log output is written using `log_msg`, `log_err` and `log_raw`. `log_msg` and `log_err` always start on a new line. The latter adds the string `ERROR: ` in front of the message.

`log_raw` is used to continue writing to the previous log line, allowing formatted log lines.

```cpp
log_msg("Hello World!");
log_err("Something went wrong: ");
log_raw(errorMsg);
log_msg("foo");
/*
Output:

Hello World!
build.c:10 - ERROR: Something went wrong: File not found
foo
*/
```

`log_err` always prints the file and line where the error is logged. `log_msg` only includes that information if the verbose command-line option (`-v`) was used.
