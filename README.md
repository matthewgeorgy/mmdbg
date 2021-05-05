# MMDbg

MMDbg is a simple memory debugging utility for use with C/C++ projects (similar to ASan - AddressSanitizer). It works by overriding `malloc()`, `free()`, `new`, and `delete` with a custom implementation to collect information regarding memory usage. It's comprised of a single header file, making it easy to integrate into projects, and only requires a few things to get up and running.


## Setup / Installation

Adding MMDbg to your project is very simple. First you just add the file itself to your project, and then `#define` a few symbols before including it:

1) Before including the file in your project, `#define` this implementation symbol in ONE C/C++ file, like so:
    
    ```
    #include <...>
    #include <...>
    #define MMDBG_IMPL
    #include <mmdbg.h>
    ```

2) Then, in any files that you wish to debug memory in (this can include the file with `#define MMDBG_IMPl`, just `#define` the debug symbol:
    
    ```
    #include <...>
    #include <...>
    #define MMDBG_DEBUG
    // #define MMDBG_IMPL (this can be here too)
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

The warning messages follow this format: `ERROR TYPE:   ADDRESS (FILENAME.c/cpp (LINE #))`. The meanings of each of these points tend to be consistent, but for some it varies. Simply navigating to the File + Line # specified in the output will always be helpful, but for the sake of completeness these will be described below.


## Reading the output

Memory Leak (UNFREED MEMORY):

* In the case of a memory leak, the address provided by MMDbg is the actual address of the memory that was allocated, either via `malloc()` or `new`.
* The File + Line # give the location in the project where this memory was allocated.

Buffer Overrun/Underrun:
    
* MMDbg works by allocating an extra 4 bytes on each end of the buffer you allocated, writes a magic number to these addresses, and then makes sure the data wasn't overwritten when `free()` or `delete` is called. As such, the pointer provided by this type of warning is the starting address of the overwritten buffer.
* File + Line # are the same as for memory leaks.

Free-after-free (DOUBLE FREE):

* Here, the pointer is like for Memory Leaks (aka, the actual address of memory allocated).
* If the double free occurred by using `free()`, then the File + Line # will refer to where this second `free()` was called. If the double free occured using `delete`, however, the File + Line # will just refer to where the memory was initially allocated. This is something that isn't accounted for in the output (as simply navigating to the file + line # will be helpful anyway), and it's a result of the way that C++ handles the `delete` operator - `new` can easily be wrapped to use `__FILE__` and `__LINE__`, but `delete` not so much. Trust me, it annoys me too :(
* On the bright side, however, because of the way that this free-after-free mechanism works, double frees won't (usually) cause your program to crash. Nice :)


## Notes

* MMDbg will probably collide with the use of `new` and object constructors (since malloc is just being called under the hood). Thus, this utility is best used for code that uses `new`/`malloc()` purely to allocate memory and not to call constructors.
* `new[]` and `delete[]` are currently unsupported by MMDbg, and probably will be for a while as I almost never use these. I might implement them at some point in the future, but don't hold your breath ¯\_(ツ)_/¯
* MMDbg has not yet been tested with classes that utilize dynamic memory allocation within constructors/destructors. It is possible that MMDbg throws warnings about memory leaks due to the `free()`/`delete` calls living in the destructor, but this hasn't been tested yet. More info soon to come.
