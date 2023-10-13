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
int cur, isDirty = 0;
char theBlock[BLOCK_SZ];
const char *msg = NULL;

void edRdBlk() {
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
    printString("\r\n     (q)home (w)up (e)end (a)left (s)down (d)right (t)top (l)last");
    printString("\r\n     (x)del char (i)insert char (r)replace char (I)Insert (R)Replace");
    printString("\r\n     (n)LF (S)Save (L)reLoad (+)next (-)prev (Q)quit");
    printString("\r\n-> \x8");
}

void showEditor() {
    int cp = 0;
    CursorOff();
    GotoXY(1, 1);
    printString("     Block Editor v0.1 - ");
    printStringF("Block# %03d %c", blkNum, isDirty ? '*' : ' ');
    printStringF(" %-20s", msg ? msg : "");
    msg = NULL;
    showGuide();
    for (int i = 0; i <= MAX_Y; i++) {
        printStringF("\r\n %2d |", i);
        for (int j = 0; j <= MAX_X; j++) {
            int isCur = (cur==cp) ? 1 : 0;
            unsigned char c = theBlock[cp++];
            if (c == 13) { c = 174; }
            if (c == 10) { c = 241; }
            if (c ==  9) { c = 242; }
            if (c <  32) { c = '.'; }
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
    for (int i = cur; i < MAX_CUR; i++) { theBlock[i] = theBlock[i + 1]; }
    theBlock[MAX_CUR - 1] = 32;
    theBlock[MAX_CUR] = 10;
}

void insertChar(char c) {
    for (int i = MAX_CUR; cur < i; i--) { theBlock[i] = theBlock[i - 1]; }
    theBlock[cur] = c;
    theBlock[MAX_CUR] = 10;
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
        if (MAX_CUR < cur) { cur = MAX_CUR; }
        showEditor();
        CursorOff();
    }
}

int processEditorChar(char c) {
    printChar(c);
    switch (c) {
    case 'Q': return 0;                                  break;
    case 'a': --cur;                                     break;
    case 'd': ++cur;                                     break;
    case 'w': cur -= LLEN;                               break;
    case 's': cur += LLEN;                               break;
    case 'q': cur -= (cur % LLEN);                       break;
    case 'e': cur -= (cur % LLEN); cur += MAX_X;         break;
    case 't': cur = 0;                                   break;
    case 'b': cur = MAX_CUR - MAX_X;                     break;
    case 'r': isDirty = 1; theBlock[cur++] = key();      break;
    case 'I': isDirty = 1; doType(1);                    break;
    case 'R': isDirty = 1; doType(0);                    break;
    case 'n': isDirty = 1; theBlock[cur++] = 10;         break;
    case 'x': isDirty = 1; deleteChar();                 break;
    case 'i': isDirty = 1; insertChar(' ');              break;
    case 'L': edRdBlk();                                 break;
    case 'S': edSvBlk();                                 break;
    case '+': if (isDirty) { edSvBlk(); }
            ++blkNum;
            edRdBlk();
            break;
    case '-': if (isDirty) { edSvBlk(); }
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
        if (cur < 0) { cur = 0; }
        if (MAX_CUR < cur) { cur = MAX_CUR; }
        showEditor();
        showFooter();
    }
}
#endif
