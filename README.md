# Modular C

This project is currently extremely alpha quality software.
Everything is a work in progress.

This project is an attempt to create a preprocessor for `C` that
adds some features for easier modularity. These include:

* [x] Automatic namespacing for imports
* [x] Automatic header generation 
* [x] Automatic passthrough exports (export types used in an export)
* [x] Specify build flags
* More as I think of them...

# Syntax:

The preprocessor recognizes the following syntaxes:

## Imports
> Import a `c` file without using a header

### Syntax:

```javascript
import a from "./awesome.c";
```

### Description
All the exported functions/types from `awesome.c` are now available
under the namespace `a`. So if the function `void do(void);` is
exported from `awesome.c`, after the above import, it can be used
as `a.do();`

### Generated Syntax

`import a from "./awesome.c";` -> the contents of `awesome.c`   
`a.do();` -> `awesome_do();`   



## Exports
> Export a symbol from your module

### Syntax:
```javascript
export void do(void){
  ...
}
```

### Description
Running `export` on a symbol prefixes it with
the module's namespace prefix (currently based on the filename).

### Generated Syntax

`export void do(void){` -> `void awesome_do(void){`


## Build Options
> Change the generated makefile.

### Syntax:
```
/* Set variables in the Makefile */
build set VAR "VALUE"
build add CFLAGS "-g"
build set default LIBRARY_INCLUDE_DIR "/path/to/default"

/* Depend on external c files */
build depends "/path/to/some/file.c"
```

### Description
`build (set | set default | add) NAME "VALUE"` will use `:=`, `?=`, `+=` respectively to set the variables in the
generated makefile whenever the module is included in the build.

`build depends "file.c"` adds `file.c` to the dependency tree whenever the module is imported. It will be added to the
generated makefile, so you can still just build your whole project with `mpp -m root.modul.c`

### Generated Syntax
the example above would result in the following lines being added to the generated makefile.
```make
VAR := "VALUE"
CFLAGS += "-g"
LIBRARY_INCLUDE_DIR ?= "/path/to/default"
```

`build depends` will add the `file.o` to the dependencies of the current module, so it will be included in the build of
the final executable.

# Build Instructions:

The project probably works on all `posix` based systems. It has been tested on `OSX`, `Linux`, and `FreeBSD`.

## Dependencies:
* `flex`
* `libc`
* `C` compiler.
* `make`

to build, simply run:
```sh
make
```

this will generate the `mpp` binary.

# Usage:

`mpp (-m | -f) <root module>`

the `root module` is the source for either an executable or library you would like to turn into a c project. 
`mpp` generates a `.c` and `.h` file for each module in the depencency tree of `root module`. Furthermore it generates a
`.mk` file which specifies the depencencies between all the generated files and their respective objects, it also
contains a rule to build either a shared library or an executable for modules where the name is `main`.

To run the compilitation based on the makefile, simply run `make -f <root>.mk <root>` where `<root>` refers to the base
name of the root module. 
## flags

* `-m` if this flag is specified, `mpp` calls `make -f <root>.mk <root>` which compiles the generated files according to
  the generated makefile. (currently not working on linux)

* `-v` verbose mode. This prints out debug information, and is primarily for use during the development of the `mpp`
  tool.
