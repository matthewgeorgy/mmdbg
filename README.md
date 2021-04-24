# MMDbg

MMDbg is a simple memory debugging utility for use with C/C++ projects. It works by overriding `malloc()`, `free()`, `new`, and `delete` with a custom implementation to collect information regarding memory usage. It's comprised of a single header file, making it easy to integrate into projects, and only requires a few things to get up and running

## Setup / Installation

Adding MMDbg to your project is very simple. First you just add the file itself to your project, and then `#define` a few symbols before including it:

1) Before including the file in your project, `#define` this implementation symbol in ONE C/C++ file, like so:
    
    ```
    #include <...>
    #include <...>
    #define MMDBG_IMPL
    #include <mmdbg.h>
    ```

2) Then, in any files that you wish to debug memory in (this includes the file with `#define MMDBG_IMPl`, just `#define` the debug symbol:
    
    ```
    #include <...>
    #include <...>
    #define MMDBG_DEBUG
    #include <mmdbg.h>
    ```
