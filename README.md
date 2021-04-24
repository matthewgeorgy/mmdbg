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

* Total calls to `malloc()`, `free()`, `new`, and `delete`
* Total amount of data allocated
* Memory Leaks
* Buffer Overruns/Underruns
* Free-after-free*

After calling `mmdbg_print(...)`, it will print out memory information in a format like so:

```
=========================================================
                    MMDBG OUTPUT
=========================================================
Total Mallocs: 10
Total Frees:   9
Total News:    10
Total Deletes: 9
Total Size:    80 bytes

BUFFER UNDERRUN:  0x00601B88 (test.cpp (20))
BUFFER OVERRUN:   0x00601B90 (test.cpp (20))
BUFFER UNDERRUN:  0x00601BC0 (test.cpp (19))
BUFFER OVERRUN:   0x00601BC8 (test.cpp (19))
BUFFER UNDERRUN:  0x00601BF8 (test.cpp (20))
BUFFER OVERRUN:   0x00601C00 (test.cpp (20))
DOUBLE FREE:      0x00601D14 (test.cpp (40))
UNFREED MEMORY:   0x00602084 (test.cpp (19))
UNFREED MEMORY:   0x00601FA4 (test.cpp (20))
=========================================================
                    END OF OUTPUT
=========================================================
```

The warning messages follow this format: `ERROR TYPE:   ADDRESS (FILENAME.c/cpp (LINE #))`. The meanings of each of these points tend to be consistent, but for some it varies. These will be described below.


## Reading the output
