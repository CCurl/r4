// editor.cpp - A simple block editor
//
// NOTE: A huge thanks to Alain Theroux. This editor was inspired by
//       his editor and is a shameful reverse-engineering of it. :D

#include "r4.h"

#ifndef __EDITOR__
void doEditor() { printString("-noEdit-"); }
#else
#define LLEN        128
#define MAX_X       (LLEN-1)
#define MAX_Y       31
#define BLOCK_SZ    (LLEN)*(MAX_Y+1)
#define MAX_CUR     (BLOCK_SZ-1)
#define REDRAW      redraw=1
#define DIRTY       isDirty=1; REDRAW
int blkNum, cur, isDirty, row, col, redraw;
char theBlock[BLOCK_SZ];
const char *msg = NULL;

void edRdBlk() {
    int r = readBlock(blkNum, theBlock, BLOCK_SZ);
    msg = (r) ? "-loaded-" : "-noFile-";
    cur = isDirty = 0; REDRAW;
}

void edSvBlk() {
    int r = writeBlock(blkNum, theBlock, BLOCK_SZ);
    msg = (r) ? "-saved-" : "-errWrite-";
    cur = isDirty = 0;
}

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void CursorOff() { printString("\x1B[?25l"); }
void CursorOn()  { printString("\x1B[?25h"); }
void Color(int fg, int bg) { printStringF("\x1B[%d;%dm", bg, fg); }

void showGuide() {
    printString("\r\n    +");
    for (int i = 1; i <= LLEN; i++) { printChar(i%5?'-':' '); }
    printChar('+');
}

int edChar(int x) {
    if (x == 13) { return 174; }
    if (x == 10) { return 241; }
    if (x ==  9) { return 242; }
    if (x <  32) { return  32; }
    return x;
}
int charAt(int x) { return edChar(theBlock[x]); }

void showHeader() {
    GotoXY(1, 1);
    printString("   Block Editor v0.1 - ");
    printStringF("Block# %03d %c", blkNum, isDirty ? '*' : ' ');
    printStringF(" %-20s", msg ? msg : "");
}

void showFooter() {
    printString("\r\n  (w) up (s) down (a) left (d) right (q) home (e) end (t) top (b) bottom");
    printString("\r\n  (x) del (i) insert (r) replace (I) Insert (R) Replace");
    printString("\r\n  (n) LF (W) Write (L) reLoad (+) next (-) prev (Q) quit");
    printString("\r\n-> \x8");
}

int minMax(int n, int min, int max) { return (n<min) ? min : (n>max) ? max : n; }
void normCur() { cur=minMax(cur, 0, MAX_CUR); }
void normRC() { row=minMax(row, 0, MAX_Y); col=minMax(col, 0, MAX_X); }
void curToRC() { normCur(); row=cur/LLEN; col=cur%LLEN; }
void rcToCur() { normRC();  cur=row*LLEN+col; }

void showEditor() {
    if (!redraw) { return; }
    int cp = 0;
    redraw = 0;
    showHeader();
    showGuide();
    for (int i = 0; i <= MAX_Y; i++) {
        printStringF("\r\n %2d |", i+1);
        for (int j = 0; j <= MAX_X; j++) {
            unsigned char c = charAt(cp++);
            printChar((char)c);
        }
        printString("| ");
    }
    showGuide();
}

void deleteChar() {
    for (int i = cur; i < MAX_CUR; i++) { theBlock[i] = theBlock[i+1]; }
    theBlock[MAX_CUR] = 0;
    DIRTY;
}

void insertChar(char c) {
    for (int i = MAX_CUR; cur < i; i--) { theBlock[i] = theBlock[i - 1]; }
    theBlock[cur] = c;
    DIRTY;
}

void showCur() {
    showHeader();
    char c = charAt(cur);
    GotoXY(col+6, row+3);
    Color(30, 46);
    printChar(c); 
    Color(0, 0);
}

void unShowCur() {
    char c = charAt(cur);
    GotoXY(col+6, row+3);
    Color(0, 0);
    printChar(c);
    printChar(8);
    msg = NULL;
}

int edKey() {
        showCur();
        int c = key();
        unShowCur();
        return c;
}

void doType(int isInsert) {
    while (1) {
        char c = edKey();
        if (c == 27) { --cur;  return; }
        if (c == 13) { c = 10; }
        int isBS = ((c == 127) || (c == 8));
        if (isBS) {
            if (cur) {
                theBlock[--cur] = ' ';
                curToRC();
                if (isInsert) { deleteChar(); }
                showEditor();
            }
            continue;
        }
        if (isInsert) { insertChar(' '); }
        else { printChar(edChar(c)); }
        theBlock[cur++] = c;
        curToRC();
        if (isInsert) { showEditor(); }
    }
}

int processEditorChar(char c) {
    // printChar(c);
    curToRC();
    switch (c) {
        // Movement
        case  'Q': return 0;
        BCASE 'a': --col; rcToCur();
        BCASE 'd': ++col; rcToCur();
        BCASE 'w': --row; rcToCur();
        BCASE 's': ++row; rcToCur();
        BCASE 'q': col=0; rcToCur();
        BCASE 'e': col=MAX_X; rcToCur();
        BCASE 't': row=col=0; rcToCur();
        BCASE 'b': row=MAX_Y; col=0; rcToCur();
        BCASE  9 : col += 8; rcToCur();
        BCASE 13 : ++row; col=0; rcToCur();
        // Actions
        BCASE 'r': c=edKey(); if (c==13) { c=10; }
                theBlock[cur++]=c;
                printChar(edChar(c));
        BCASE 'R': doType(0);
        BCASE 'i': insertChar(' ');
        BCASE 'I': doType(1);
        BCASE 'n': theBlock[cur++]=10;  printChar(edChar(10));
        BCASE 'x': deleteChar();
        BCASE 'X': if (cur) { cur -= cur ? 1 : 0; deleteChar(); }
        BCASE 'L': edRdBlk();
        BCASE 'W': edSvBlk();
        BCASE '+': if (isDirty) { edSvBlk(); } ++blkNum; edRdBlk();
        BCASE '-': if (isDirty) { edSvBlk(); }
                blkNum -= (blkNum) ? 1 : 0;
                edRdBlk();
                break;
    }
    return 1;
}

void doEditor() {
    int ok = 1;
    blkNum = (int)pop();
    blkNum = (0 <= blkNum) ? blkNum : 0;
    CLS();
    CursorOff();
    edRdBlk();
    showEditor(); showFooter();
    while (ok) {
        char c = edKey();
        ok = processEditorChar(c);
        curToRC();
        if (redraw) { showEditor(); showFooter(); }
    }
    CLS();
    CursorOn();
}
#endif //  __EDITOR__
