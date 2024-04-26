xml-tokenizer.h
===============

This is a single-header-file [STB-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt) library that tokenize xml file for C (also works in C++)

The easiest way to install the library to your C/C++ project is to copy 'n' paste the *xml-tokenizer.h* to your project and do this in *one* C or C++ file:

``` C
#define XML_TOKENIZER_IMPLEMENTATION
#include "xml-tokenizer.h"
```

COMPILE-TIME OPTIONS
--------------------

These defines only need to be set in the file containing XML_TOKENIZER_IMPLEMENTATION

``` C
#define XML_REALLOC(context,ptr,size) better_realloc
#define XML_FREE(context,ptr)         btter_free
```

By default the stdlib realloc() and free() is used. You can defines your own by defining these symbols. You must either define both, or neither.

Note that at the moment, 'context' will always be NULL.

``` C
#define XML_FOPEN(fp,filename,mode) better_fopen
#define XML_FGETC(fp)               better_fgetc
#define XML_FCLOSE(fp)              better_fclose
```

By default the stdlib fopen(), fgetc() and fclose() are used. You can defines you own by defining these symbols. You most either define all three, or neither.

EXAMPLE
-------

You can find examples how to parser a book_cataloge.xml file that contains a book catalog using C and C++ (C++11).

```
example/parser_catalog.c
example/xml_dom.hpp
```

LICENSE
-------

See LICENSE.txt
