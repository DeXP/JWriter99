JWriter99
===========

Simple JSON writer implemented in C.

It is able to write JSON data to: console, file, memory. Available backends: stdlibc, WinAPI.

The aim of JWriter99 is to be as seet as possible. In means os syntax sugar. I.e. it can be used maximally easy.


Idea
-----------

There are two types of a JSON serialization. The first:

`myComplicatedObject.SeriailzeToJSON(file)`

It would be ideal, but not possible for C/C++ in general case since the info about class/struct members are not store in executable after compilation.

The second extreme is just print the strings like:

`printf("{'all': 'my', 'complicated': 'code'}");`

The programmer have to handle all brackets, commas etc in this case. 

I wanted something in a middle, more close to `printf` variant. I.e. do not store intermediate data anywhere, just process it and put into some place on the fly.



Implementation
-----------

The implementation is based on Serge Zaitsev's article: [SYNTACTIC SUGAR IN C - (AB)USING “FOR” LOOPS](https://zserge.com/blog/c-for-loop-tricks.html)

The code uses lots of macroses, so can be difficult to debug.

The main advantage of implementing JSON as a doman-specific language is short clear syntaxis and automatic opening/closing the brackets.


Example
-----------

```
#define JWRITER99_IMPLEMENTATION
#include "JWriter99.c"

void main() {
    JStartConsoleObj {
        JParam("string", "value");
        JIntParam("int", 5);
        JCustomParam("printf-like", "%d.%02d.%d", 31, 2, 2019);

        JArr("array"){ JInt(3), JStr("string?"), JInt(7), JInt(1); }

        JObj("subobject") {
            JIntParam("ID", 67);
            
            JArr("subarray"){
                JInt(2);

                JObjBlock {
                    JIntParam("ID", 23), JParam("color", "black");
                };

                JStr("string");
            }
        }
    }
}
```

For more complicated example see `JWriter99.c`


Name
-----------

99 in the name means C compiler required standard: C99. Let's celebrate 20 year of the standard with some code. :-)
