# msgmap
msgmap is a header-only localization system for C that uses a
NodeJS script to generate headers from YAML files.

## Usage

```
USAGE: node msgmap.js [options] <dir>
       node msgmap.js <--help|-h>

Options:
    --help, -h:
        Displays this help message.

    --out-dir, -o:
        Output directory for the translations.
        "build" by default.
        
    --wide, -w:
        Use wide character (wchar_t) strings for
        the translations. The produced headers will
        only work with the Microsoft Visual C++
        compiler.
        
    --default-lang, -d:
        Default language to use if the user does
        not prefer any of the provided languages.
        "en_US" by default.
```

## Installation

**Prerequesites**:
- Git
- NodeJS

1. Add msgmap as a submodule in your project:
   
   ```
   git submodule add https://github.com/aubymori/msgmap
   ```

2. `cd` to the directory where msgmap is, and download the necessary modules:
   
   ```
   npm i
   ```

3. Set up the translations directory. You can use the `test` directory in this
   repository as a reference.

4. Make sure the `include` folder in this repository is in your project's include
   directories list.

5. Build translation files. It is recommended that you make a shell script that
   calls the `msgmap.js` script with a set of options so that multiple contributors
   can work on translations.

6. Create a separate source code file for translation implementation, for example, `translations.c`.
   The file should look something like this:

   ```c
   #define MSGMAP_IMPL
   #include "msgmap.h"
   #include "translations/xxx.h"
   #include "translations/yyy.h"
   ```

   This is necessary, because msgmap is implemented in an [STB](https://github.com/nothings/stb) manner,
   which avoids duplication of function implementations in the final binary and can prevent access
   to symbols not meant for public consumption.

## API

See `include/msgmap.h` and your generated header files.