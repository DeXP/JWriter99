#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#if defined(_MSC_VER)
    #define J99STDCALL_ATTR __stdcall
#else
    #define J99STDCALL_ATTR __attribute__((stdcall))
#endif

struct TJsonWrStruct {
    // For internal usage
    int _break;
    
    // Options 
    int isFlat;

    // Used internally
    int level;
    char lastChar;

    int bufLength;
    char buf[260]; // used for custom type prints only
    
    // User set variables
    void* userData;

    // Functions
    int (*jsonwr_putc) (int character, void* userData);
    int (J99STDCALL_ATTR* jsonwr_vsprintf) (char* s, const char* format, va_list arg);
};

struct TJsonMemBuffer {
    int _break;
    int size;
    int pos;
    char* data;

    // Functions
    void* (*jsonwr_realloc) (void* ptr, unsigned int size);
};

void JSymbFunc(struct TJsonWrStruct* jwr, char c);
void JNewLineFunc(struct TJsonWrStruct* jwr);
void JAddCommaFunc(struct TJsonWrStruct* jwr);
void JPrintStrFunc(struct TJsonWrStruct* jwr, const char* str, int maxLength);
void JCustomFunc(struct TJsonWrStruct* jwr, const char * format, ...);
int stat_mem_putc(int character, void* userData);
int dyn_mem_putc(int character, void* userData);


// Each JSON should start from this. Defines `_JWrInternal` object for this scope of view for usage inside of all other macroses
#define JStart(isFlatParam, putcFunc, vsprintfFunc) \
        for (struct TJsonWrStruct _JWrInternal = { \
            ._break = 1, \
            .isFlat = isFlatParam, \
            .level = 0, \
            .lastChar = 0, \
            .bufLength = 0, \
            .buf = {0}, \
            .userData = 0, \
            .jsonwr_putc = putcFunc, \
            .jsonwr_vsprintf = vsprintfFunc }; \
         _JWrInternal._break; \
         JSymb(0),_JWrInternal._break = 0)
#define JStartObj(...) JStart(__VA_ARGS__)JObjBlock
#define JStartArr(...) JStart(__VA_ARGS__)JArrBlock

#define JNewLine JNewLineFunc(&_JWrInternal)
#define JSymb(c) JSymbFunc(&_JWrInternal, c)
#define JAddComma JAddCommaFunc(&_JWrInternal)

#define JBlockInternal(startSymb, endSymb) \
       for (int _break = (JAddComma,JSymb(startSymb), 1); _break; \
         _break = 0, JSymb(endSymb))
#define JObjBlock      JBlockInternal('{','}')
#define JArrBlock      JBlockInternal('[',']')

#define JInt(x)                  JAddComma;JCustomFunc(&_JWrInternal, "%d", x);_JWrInternal.lastChar=1
#define JStr(str)                JAddComma;JSymb('"');JPrintStrFunc(&_JWrInternal, str, 2147483647);JSymb('"')
#define JCustom(...)             JAddComma;JSymb('"');JCustomFunc(&_JWrInternal, __VA_ARGS__);JSymb('"')

#define JParamStart(name)        JAddComma;JNewLine;JStr(name);JSymb(':');
#define JObj(name)               JParamStart(name);JObjBlock
#define JArr(name)               JParamStart(name);JArrBlock
#define JParam(name, value)      JParamStart(name);JStr(value)
#define JIntParam(name, value)   JParamStart(name);JInt(value)
#define JCustomParam(name, ...)  JParamStart(name);JCustom(__VA_ARGS__)


#define JPrepareStruct(PointerToStructure) struct TJsonWrStruct _JWrInternal = *PointerToStructure

#define JStaticMemOpen(JsonBuffer, MaxSize) \
        for (struct TJsonMemBuffer _JMemBufInternal = { ._break = 1, .size = MaxSize, .pos = 0, .data = JsonBuffer, .jsonwr_realloc = 0}; \
        _JMemBufInternal._break; \
        _JMemBufInternal._break=0) \
               for (int _break = ((_JWrInternal.userData = &_JMemBufInternal), 1); _break; _break = 0)

#if defined(JWRITER99_EXAMPLE) && !defined(JWRITER99_IMPLEMENTATION)
    #define JWRITER99_IMPLEMENTATION
#endif

#ifdef JWRITER99_IMPLEMENTATION
    void JNewLineFunc(struct TJsonWrStruct* jwr) {
        const int JsonWr_Indent_Spaces = 2;
        if ((jwr->isFlat == 0) && (jwr->lastChar != '\n')) {
            JSymbFunc(jwr, '\n');
            for(int i=0; i< JsonWr_Indent_Spaces*(jwr->level); i++)
                JSymbFunc(jwr, ' ');
        }
    }

    void JSymbFunc(struct TJsonWrStruct* jwr, char c) {
        if ((c == '}') || (c == ']')) {
            jwr->level--;
            JNewLineFunc(jwr);
        }
        if ((c == '{') || (c == '[')) {
            if (jwr->lastChar != 0) 
                JNewLineFunc(jwr);
            jwr->level++;
        }

        jwr->jsonwr_putc(c, jwr->userData);

        if ((c != ' ') && (c != '\t')) {
            jwr->lastChar = c;
        }
        
        if ((c == '{') || (c == '[')) {
            if (jwr->lastChar != 0)
                JNewLineFunc(jwr);
        }

        if ((c == ':') && (jwr->isFlat == 0) ) {
                JSymbFunc(jwr, ' ');
        }
    }

    void JAddCommaFunc(struct TJsonWrStruct* jwr) {
        char c = jwr->lastChar;
        if (0 == c) return;
        if ('[' == c) return;
        if ('{' == c) return;
        if (':' == c) return;
        if (',' == c) return;
        if ('\n' == c) return;
        JSymbFunc(jwr, ',');
        if (jwr->isFlat == 0) JSymbFunc(jwr, ' ');
    }

    void JPrintStrFunc(struct TJsonWrStruct* jwr, const char* str, int maxLength) {
        #define JSlashedSymbInternal(c) JSymbFunc(jwr, '\\');JSymbFunc(jwr, c)
        for (int i=0; (str[i] != 0) && (i < maxLength); i++) {
            switch (str[i]) {
                case '\\': JSlashedSymbInternal('\\'); break;
                case '"' : JSlashedSymbInternal('"'); break;
                case '\n': JSlashedSymbInternal('n'); break;
                case '\r': JSlashedSymbInternal('r'); break;
                case '\t': JSlashedSymbInternal('t'); break;
                case '\b': JSlashedSymbInternal('b'); break;
                case '\f': JSlashedSymbInternal('f'); break;
                default:
                    jwr->jsonwr_putc(str[i], jwr->userData);
                break;
            }
        }
    }

    void JCustomFunc(struct TJsonWrStruct* jwr, const char * format, ...) {
        va_list args;
        va_start (args, format);
        jwr->bufLength = jwr->jsonwr_vsprintf(jwr->buf, format, args);
        va_end (args);

        jwr->buf[ jwr->bufLength ] = 0;
        
        JPrintStrFunc(jwr, jwr->buf, jwr->bufLength);
    }

    int stat_mem_putc(int character, void* userData) {
        struct TJsonMemBuffer* jbuf = (struct TJsonMemBuffer*)userData;
        if (jbuf->pos < jbuf->size) {
            jbuf->data[ jbuf->pos++ ] = (char)character;
            jbuf->data[ jbuf->pos ] = 0;
            return character;
        } else return -1;
    }

    int dyn_mem_putc(int character, void* userData) {
        struct TJsonMemBuffer* jbuf = (struct TJsonMemBuffer*)userData;
        char* data = jbuf->data;

        if ((0 == data) || ((jbuf->pos+1) >= jbuf->size)) {
            // Allocation of bigger memory required
            char* newbuf = 0;

            jbuf->size *= 2; // Grow 2 times per step
            if (0 == jbuf->size) jbuf->size = 1024; // Initial size

            newbuf = (char*) jbuf->jsonwr_realloc(data, jbuf->size);
            if (0 != newbuf) {
                data = newbuf;
                jbuf->data = newbuf;
            }
            // TODO: reallocation error handling
        }

        data[ jbuf->pos++ ] = (char)character;
        data[ jbuf->pos ] = 0;
        return character;
    }
#endif // JWRITER99_IMPLEMENTATION

// Backends: stdlib and WinAPI
#ifndef JWRITER99_NO_STDLIB
    // STDLIB backend
    #include <stdio.h>
    #include <stdlib.h>

    #define JPuts puts
    #define JFree free

    #define STDLIB_VSPRINTF (int (J99STDCALL_ATTR*)(char *, const char *, va_list))vsprintf
    
    int console_jputc(int character, void* userData);
    int file_jputc(int character, void* userData);
    int file_jclose(void* userData);

    #define JMemOpen(JsonBuffer) \
            for (struct TJsonMemBuffer _JMemBufInternal = { ._break = 1, .pos = 0, .size = 0, .data = NULL, .jsonwr_realloc = realloc}; \
            _JMemBufInternal._break; \
            _JMemBufInternal._break=0, JsonBuffer=_JMemBufInternal.data) \
                   for (int _break = ((_JWrInternal.userData = &_JMemBufInternal), 1); _break; _break = 0)

    #define JFileOpen(fileName) \
       for (int _break = ((_JWrInternal.userData = fopen(fileName, "w+")), 1); _break; \
         _break = 0,file_jclose(_JWrInternal.userData)) \
            if (NULL != _JWrInternal.userData)

    #define JStartConsole                                     JStart(0, console_jputc, STDLIB_VSPRINTF)
    #define JStartStaticMemEx(JsonBuffer, MaxSize, isFlat)    JStart(isFlat, stat_mem_putc, STDLIB_VSPRINTF)JStaticMemOpen(JsonBuffer, MaxSize)
    #define JStartMemEx(JsonBuffer, isFlat)                   JStart(isFlat, dyn_mem_putc, STDLIB_VSPRINTF)JMemOpen(JsonBuffer)
    #define JStartFileEx(fileName, isFlat)                    JStart(isFlat, file_jputc, STDLIB_VSPRINTF)JFileOpen(fileName)

    #ifdef JWRITER99_IMPLEMENTATION
        int console_jputc(int character, void* userData) {
            (void)userData; return putchar(character);
        }
        
        int file_jputc(int character, void* userData) {
            return fputc(character, (FILE *)userData);
        }

        int file_jclose(void* userData) {
            if (NULL != userData) {
                fclose((FILE *)userData);
                return 0;
            }
            return 1;
        }
    #endif // JWRITER99_IMPLEMENTATION
#endif // JWRITER99_NO_STDLIB


#if !defined(JFree) && defined(_WIN32)
    // WinAPI backend
    #include <windows.h>

    #define JFree GlobalFree
    #define JPuts(str) WJPutsEx(str, 1)
    
    int WJPutsEx(const char* str, int appendNewLine);
    void* winapi_jrealloc(void* ptr, unsigned int size);
    int winapi_jfile_puts(const char* filename, const char* buf);

    #define JStartConsole \
        for (char _break = 1, *_WinAPIBufInternal = 0; _break; \
             _break = 0, JPuts(_WinAPIBufInternal), JFree(_WinAPIBufInternal) ) \
            JStartMem(_WinAPIBufInternal)

    #define JMemOpen(JsonBuffer) \
            for (struct TJsonMemBuffer _JMemBufInternal = { ._break = 1, .size = 0, .pos = 0, .data = NULL, .jsonwr_realloc = winapi_jrealloc}; \
            _JMemBufInternal._break; \
            _JMemBufInternal._break=0, JsonBuffer=_JMemBufInternal.data) \
                   for (int _break = ((_JWrInternal.userData = &_JMemBufInternal), 1); _break; _break = 0)
            
    #define JStartFileEx(fileName, isFlat) \
        for (char _break = 1, *_WinAPIBufInternal = 0; _break; \
             _break = 0, winapi_jfile_puts(fileName, _WinAPIBufInternal), JFree(_WinAPIBufInternal) ) \
            JStartMemEx(_WinAPIBufInternal, isFlat)

    #define JStartStaticMemEx(JsonBuffer, MaxSize, isFlat)    JStart(isFlat, stat_mem_putc, wvsprintf)JStaticMemOpen(JsonBuffer, MaxSize)
    #define JStartMemEx(JsonBuffer, isFlat)                   JStart(isFlat, dyn_mem_putc, wvsprintf)JMemOpen(JsonBuffer)


    #ifdef JWRITER99_IMPLEMENTATION
        int WJPutsEx(const char* str, int appendNewLine) {
            DWORD written = 0, writtenLn = 0, len = 0;
            while (str[len] != 0) len++; // get rid of reference to lstrlen()

            WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), str, len, &written, NULL);
            
            if (appendNewLine) {
                WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &writtenLn, NULL);
            }

            return (int)(written + writtenLn);
        }
        
        void* winapi_jrealloc(void* ptr, unsigned int size) {
            if (0 == ptr) return GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)size);
                else return GlobalReAlloc((HGLOBAL)ptr, (SIZE_T)size, GMEM_MOVEABLE);
        }
        
        int winapi_jfile_puts(const char* filename, const char* buf) {
            unsigned long bufsize = 0;
            while (buf[bufsize] != 0) bufsize++;
            HANDLE out = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (out != INVALID_HANDLE_VALUE) {
                DWORD written = 0;
                WriteFile(out, buf, bufsize, &written, NULL);
                CloseHandle(out);
                if (written < bufsize) return bufsize - written;
            }
            return 0;
        }
    #endif // JWRITER99_IMPLEMENTATION
#endif // WINAPI backend


/// Helper macroses for end-user usage

// Console
#define JStartConsoleObj        JStartConsole JObjBlock
#define JStartConsoleArr        JStartConsole JArrBlock

// Static memory macroses
#define JStartStaticMemObjEx(JsonBuffer, MaxSize, isFlat) JStartStaticMemEx(JsonBuffer, MaxSize, isFlat)JObjBlock
#define JStartStaticMemArrEx(JsonBuffer, MaxSize, isFlat) JStartStaticMemEx(JsonBuffer, MaxSize, isFlat)JArrBlock

#define JStartStaticMem(JsonBuffer, MaxSize)              JStartStaticMemEx(JsonBuffer, MaxSize, 0)
#define JStartStaticMemObj(JsonBuffer, MaxSize)           JStartStaticMem(JsonBuffer, MaxSize)JObjBlock
#define JStartStaticMemArr(JsonBuffer, MaxSize)           JStartStaticMem(JsonBuffer, MaxSize)JArrBlock

#define JStartStaticMemCompact(JsonBuffer, MaxSize)              JStartStaticMemEx(JsonBuffer, MaxSize, 1)
#define JStartStaticMemObjCompact(JsonBuffer, MaxSize)           JStartStaticMemCompact(JsonBuffer, MaxSize)JObjBlock
#define JStartStaticMemArrCompact(JsonBuffer, MaxSize)           JStartStaticMemCompact(JsonBuffer, MaxSize)JArrBlock

//Dynamic memory macroses
#define JStartMemObjEx(JsonBuffer, isFlat) JStartMemEx(JsonBuffer, isFlat)JObjBlock
#define JStartMemArrEx(JsonBuffer, isFlat) JStartMemEx(JsonBuffer, isFlat)JArrBlock

#define JStartMem(JsonBuffer)              JStartMemEx(JsonBuffer, 0)
#define JStartMemObj(JsonBuffer)           JStartMem(JsonBuffer)JObjBlock
#define JStartMemArr(JsonBuffer)           JStartMem(JsonBuffer)JArrBlock

#define JStartMemCompact(JsonBuffer)       JStartMemEx(JsonBuffer, 1)
#define JStartMemObjCompact(JsonBuffer)    JStartMemCompact(JsonBuffer)JObjBlock
#define JStartMemArrCompact(JsonBuffer)    JStartMemCompact(JsonBuffer)JArrBlock

// File-based macro defines
#define JStartFileObjEx(fileName, isFlat) JStartFileEx(fileName, isFlat)JObjBlock
#define JStartFileArrEx(fileName, isFlat) JStartFileEx(fileName, isFlat)JArrBlock

#define JStartFile(fileName)              JStartFileEx(fileName, 0)
#define JStartFileObj(fileName)           JStartFile(fileName)JObjBlock
#define JStartFileArr(fileName)           JStartFile(fileName)JArrBlock

#define JStartFileCompact(fileName)       JStartFileEx(fileName, 1)
#define JStartFileObjCompact(fileName)    JStartFileCompact(fileName)JObjBlock
#define JStartFileArrCompact(fileName)    JStartFileCompact(fileName)JArrBlock


// Example start
#ifdef JWRITER99_EXAMPLE
    // http://stackoverflow.com/questions/1167253/implementation-of-rand
    unsigned int myrand(void) {
        static unsigned int m_w = 12345;
        static unsigned int m_z = 54321;
        m_z = 36969 * (m_z & 65535) + (m_z >> 16);
        m_w = 18000 * (m_w & 65535) + (m_w >> 16);
        return (m_z << 16) + m_w;
    }


    void json_example(void) {
        char* mybuf = NULL; // Used for dynamic memory only
        
        char staticMemory[1024] = {0};
        int staticMemorySize = sizeof(staticMemory) / sizeof(staticMemory[0]);
        (void)staticMemorySize;
        
        JPuts("-= Start example =-");

        //JStartStaticMemObj(staticMemory, staticMemorySize) {
        //JStartMemObj(mybuf) {
        //JStartFileObj("file.json") {
        JStartConsoleObj {
            JParam("Title", "My \"JSON\n demo");
            JCustomParam("Date", "%d.%02d.%d", 31, 2, 2019);
            JParam("Cool string", "={^_^}=\n\r\t\b\f \" /(\\_/)\\");

            JObj("Window") {
                JParam("state", "maximized");
                JIntParam("left", 120);
                JIntParam("top", 70);

                JArr("IntegerArray") {
                    int n = 3 + myrand() % 10;
                    for (int i = 0; i < n; i++) {
                        int value = myrand() % 100;

                        JInt(value);
                    }
                    JStr("Ha ha! Not int");
                    JInt(n);
                }

                JArr("ObjectsArray") {
                    int n = 1 + myrand() % 3;
                    for (int i = 0; i < n; i++) {
                        JObjBlock {
                            JIntParam("ID", myrand() % 10000);
                            JParam("visible", myrand()%2 ? "true": "false");
                        }
                    }
                }
                
                JParam("author", "DeXPeriX");
            };
        };

        if (NULL != mybuf) {
            JPuts(mybuf);
            JFree(mybuf);
        }
        
        if (0 != staticMemory[0]) {
            JPuts(staticMemory);
        }
    }

    #ifndef JWRITER99_NO_STDLIB
        int main(void) {
            json_example();
            return 0;
        }
    #else
        int dx_start() {
            json_example();
            ExitProcess(0);
            return 0;
        }

        #if !defined(_MSC_VER)
        __asm__(
            ".globl __main\n"
            "__main:\n"
            #if defined(WIN64) || defined(__amd64__)
                "call dx_start\n"
            #else
                "call _dx_start\n"
            #endif
            "movl $1, %eax\n"
            "xorl %ebx, %ebx\n"
            "int $0x80"
        );
        #endif // _MSC_VER
    #endif // JWRITER99_NO_STDLIB

#endif // JWRITER99_EXAMPLE

#ifdef __cplusplus
}
#endif