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

3) Finally, use the function `mmdbg_print(FILE *stream)` to print debugging information to a specified stream (ie, `stdout`, a `FILE *` handle, etc). To collect the most amount of information, call this function at the end of your `main()` function:

    ```
    int main()
    {
        // ..
        
        mmdbg_print(stdout);
        return 0;
    }
    ```


## Features

Currently, MMDbg can report the following memory related information:

    - Total calls to `malloc()`, `free()`, `new`, and `delete
    - Total amount of data allocated
    - Memory Leaks
    - Buffer Overruns/Underruns
    - Free-after-free*
