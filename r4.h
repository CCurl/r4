// r4- A Minimal Interpreter

#include "config.h"
#include <stdio.h>
#include <stdarg.h>

typedef long CELL;
typedef unsigned long UCELL;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef byte *addr;

#define CELL_SZ    sizeof(CELL)
#define INDEX      reg[8]
#define TOS        dstack[dsp]
#define A          (addr)TOS
#define NOS        dstack[dsp-1]
#define R          rstack[dsp]
#define DROP1      pop()
#define DROP2      pop(); pop()
#define BTWI(n, x, y) (((x) <= (n)) && ((n) <= (y)))
#define isLocal(x) (('0' <= (x)) && ((x) <= '9'))
#define isRegChar(x) (('A' <= (x)) && ((x) <= 'Z'))
#define isReg(n) ((0 <= (n)) && ((n) < NUM_REGS))
#define isFunc(n) ((0 <= (n)) && ((n) < NUM_FUNCS))
#define ABS(x) ((x < 0) ? -x : x)

typedef struct {
    addr start;
    CELL from;
    CELL to;
    addr end;
} LOOP_ENTRY_T;

extern byte isBye;
extern byte isError;
extern addr HERE;
extern CELL dstack[];
extern ushort dsp;
extern addr func[];
extern CELL input_fp;

extern void vmInit();
extern CELL pop();
extern void push(CELL);
extern addr run(addr);
extern addr doCustom(byte, addr);
extern void printChar(const char);
extern void printString(const char*);
extern void printStringF(const char*, ...);
extern void dumpStack();
extern CELL getSeed();
extern CELL doMicros();
extern CELL doMillis();
extern void doDelay(CELL);
extern int charAvailable();
extern int getChar();

// Built-in editor
extern void doEditor();

// File support
extern void fpush(CELL);
extern CELL fpop();
extern void fileInit();
extern void fileOpen();
extern void fileClose();
extern void fileDelete();
extern void fileRead();
extern void fileWrite();
extern addr codeLoad(addr, addr);
extern void codeSave(addr, addr);
extern void blockLoad(CELL);
extern int readBlock(int blk, char* buf, int sz);
extern int writeBlock(int blk, char* buf, int sz);
extern int fileReadLine(CELL fh, char* buf);
