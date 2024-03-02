// r4- A Minimal Interpreter

#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

typedef long CELL;
typedef unsigned long UCELL;
typedef uint16_t ushort;
typedef uint8_t byte;
typedef byte *addr;
typedef union { double f; CELL i; char *c; } ST_T;

#define CELL_SZ    sizeof(CELL)
#define TOS        dstack[dsp].i
#define NOS        dstack[dsp-1].i
#define AOS        dstack[dsp].c
#define FTOS       dstack[dsp].f
#define FNOS       dstack[dsp-1].f
#define L0         lstack[lsp]
#define L1         lstack[lsp-1]
#define L2         lstack[lsp-2]
#define DROP2      pop(); pop()
#define NEXT       goto next
#define NCASE      NEXT; case
#define BCASE      break; case
#define RCASE      return; case
#define BTWI(n,x,y)   (((x) <= (n)) && ((n) <= (y)))
#define isLocal(x)    BTWI(x,'0','9')
#define isReg(n)      ((0 <= (n)) && ((n) < NUM_REGS))
#define isFunc(n)     ((0 <= (n)) && ((n) < NUM_FUNCS))
#define ABS(x)        ((x < 0) ? -x : x)

extern byte isBye;
extern byte isError;
extern addr HERE;
extern ST_T dstack[];
extern int dsp;
extern addr func[];
extern CELL input_fp;

extern void vmInit();
extern CELL pop();
extern void push(CELL);
extern CELL doHash(CELL max);
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
extern int qkey();
extern int key();

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
extern void loadAbort();
extern int readBlock(int blk, char* buf, int sz);
extern int writeBlock(int blk, char* buf, int sz);
extern int fileReadLine(CELL fh, char* buf);
extern addr pc;
