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
int line, off, blkNum;
int cur, isDirty = 0, row, col;
char theBlock[BLOCK_SZ];
const char *msg = NULL;

void edRdBlk() {
    // for (int i=0; i<=MAX_CUR; i++) { theBlock[i]=0; }
    int r = readBlock(blkNum, theBlock, BLOCK_SZ);
    msg = (r) ? "-loaded-" : "-noFile-";
    cur = isDirty = 0;
}

void edSvBlk() {
    int r = writeBlock(blkNum, theBlock, BLOCK_SZ);
    msg = (r) ? "-saved-" : "-errWrite-";
    cur = isDirty = 0;
}

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void Color(int fg, int bg) { printStringF("\x1B[%d;%dm", bg, fg); }

void showGuide() {
    printString("\r\n    +");
    for (int i = 0; i <= MAX_X; i++) { printChar('-'); }
    printChar('+');
}

void showFooter() {
    printString("\r\n  (w) up (s) down (a) left (d) right (q) home (e) end (t) top (b) bottom");
    printString("\r\n  (x) del (i) insert (r) replace (I) Insert (R) Replace");
    printString("\r\n  (n) LF (W) Write (L) reLoad (+) next (-) prev (Q) quit");
    printString("\r\n-> \x8");
}

void normRC() {
    if (row < 1) { row = 0; }
    if (col < 1) { col = 0; }
    if (MAX_Y < row) { row = MAX_Y; }
    if (MAX_X < col) { col = MAX_X; }
}
void rcToCur() { normRC();  cur=row*LLEN+col; }

void normCur() {
    if (cur < 0) { cur = 0; }
    if (MAX_CUR < cur) { cur = MAX_CUR; }
}
void curToRC() { normCur(); row=cur/LLEN; col=cur%LLEN; }

void showEditor() {
    int cp = 0;
    CursorOff();
    GotoXY(1, 1);
    printString("   Block Editor v0.1 - ");
    printStringF("Block# %03d %c", blkNum, isDirty ? '*' : ' ');
    printStringF(" %-20s", msg ? msg : "");
    msg = NULL;
    showGuide();
    for (int i = 0; i <= MAX_Y; i++) {
        printStringF("\r\n %2d |", i+1);
        for (int j = 0; j <= MAX_X; j++) {
            int isCur = (cur==cp) ? 1 : 0;
            unsigned char c = theBlock[cp++];
            if (c == 13) { c = 174; }
            if (c == 10) { c = 241; }
            if (c ==  9) { c = 242; }
            if (c <  32) { c =  32; }
            if (isCur) { Color(30, 46); }
            printChar((char)c);
            if (isCur) { Color(0, 0); }
        }
        printString("| ");
    }
    showGuide();
    CursorOn();
}

void deleteChar() {
    for (int i = cur; i < MAX_CUR; i++) { theBlock[i] = theBlock[i+1]; }
    theBlock[MAX_CUR] = 0;
    isDirty = 1;
}

void insertChar(char c) {
    for (int i = MAX_CUR; cur < i; i--) { theBlock[i] = theBlock[i - 1]; }
    theBlock[cur] = c;
    isDirty = 1;
}

void doType(int isInsert) {
    CursorOff();
    while (1) {
        char c= key();
        if (c == 27) { --cur;  return; }
        int isBS = ((c == 127) || (c == 8));
        if (isBS) {
            if (cur) {
                theBlock[--cur] = ' ';
                if (isInsert) { deleteChar(); }
                showEditor();
            }
            continue;
        }
        if (isInsert) { insertChar(' '); }
        if (c == 13) { c = 10; }
        theBlock[cur++] = c;
        isDirty = 1;
        if (MAX_CUR < cur) { cur = MAX_CUR; }
        showEditor();
        CursorOff();
    }
}

int processEditorChar(char c) {
    printChar(c);
    curToRC();
    switch (c) {
        case  'Q': return 0;
        BCASE 'a': --col; rcToCur();
        BCASE 'd': ++col; rcToCur();
        BCASE 'w': --row; rcToCur();
        BCASE 's': ++row; rcToCur();
        BCASE 'q': col=0; rcToCur();
        BCASE 'e': col=MAX_X; rcToCur();
        BCASE 't': row=col=0; rcToCur();
        BCASE 'b': row=MAX_Y; rcToCur();
        BCASE 'r': theBlock[cur++] = key(); isDirty = 1;
        BCASE 'I': doType(1);
        BCASE 'R': doType(0);
        BCASE 'n': theBlock[cur++] = 10; isDirty = 1;
        BCASE 'x': deleteChar();
        BCASE 'X': if (cur) { cur -= cur ? 1 : 0; deleteChar(); }
        BCASE 'i': insertChar(' ');
        BCASE  9 : col += 8; rcToCur();
        BCASE 13 : ++row; col=0; rcToCur();
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
    blkNum = pop();
    blkNum = (0 <= blkNum) ? blkNum : 0;
    CLS();
    edRdBlk();
    showEditor();
    showFooter();
    while (processEditorChar(key())) {
        showEditor();
        showFooter();
    }
}
#endif
