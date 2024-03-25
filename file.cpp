#include "r4.h"

// #define __FILES__ 12345

#ifndef __FILES__
#ifndef __LITTLEFS__
void noFile() { printString("-noFile-"); }
void fileInit() { noFile(); }
void fileOpen() { noFile(); }
void fileClose() { noFile(); }
void fileDelete() { noFile(); }
void fileRead() { noFile(); }
void fileWrite() { noFile(); }
void blockLoad(CELL num) { noFile(); }
void loadAbort() { noFile(); }
int fileReadLine(CELL fh, char* buf) { noFile(); return -1; }
int readBlock(int blk, char* buf, int sz) { noFile(); return 0; }
void readBlock1() { noFile(); }
int writeBlock(int blk, char* buf, int sz) { noFile(); return 0; }
void writeBlock1() { noFile(); }
#endif // __LITTLEFS__
#else
// shared with __LITTLEFS__
static int fdsp = 0;
static CELL fstack[STK_SZ + 1];
CELL input_fp;

void fpush(CELL v) { if (fdsp < STK_SZ) { fstack[++fdsp] = v; } }
CELL fpop() { return (0 < fdsp) ? fstack[fdsp--] : 0; }
// shared with __LITTLEFS__

void fileInit() {}

// fO (nm md--fh) - File Open
// fh: File handle, nm: File name, md: mode
// fh=0: File not found or error
void fileOpen() {
    char* md = (char *)pop();
    char* fn = AOS;
    TOS = (CELL)fopen(fn, md);
}

// fC (fh--) - File Close
// fh: File handle
void fileClose() {
    FILE* fh = (FILE*)pop();
    if (fh) { fclose(fh); }
}

// fD (nm--) - File Delete
// nm: File name
void fileDelete() {
    char *fn = (char*)pop();
    remove(fn);
}

// fR (fh--c n) - File Read
// fh: File handle, c: char read, n: num chars read
// n=0: End of file or file error
void fileRead() {
    FILE *fh = (FILE*)TOS;
    push(0);
    NOS = 0;
    if (fh) {
        char c;
        TOS = fread(&c, 1, 1, fh);
        NOS = TOS ? c : 0;
    }
}

// fL - fileReadLine(fh, buf)
// fh: File handle, buf: address
// returns: -1 if EOF, else len
int fileReadLine(CELL fh, char *buf) {
    byte c, l=0;
    if (fgets(buf, 256, (FILE*)fh) == buf) {
        while (buf[l]) { if (buf[l]<32) { buf[l]=32; } ++l; }
        while (l && (buf[l-1]<33)) { --l; buf[l]=0; }
        return l;
    }
    buf[0]=0;
    return -1;
}

// fW (c fh--n) - File Write
// fh: File handle, c: char to write, n: num chars written
// n=0: File not open or error
void fileWrite() {
    FILE* fh = (FILE*)pop();
    char c = (char)TOS;
    TOS = 0;
    if (fh) {
        TOS = fwrite(&c, 1, 1, fh);
    }
}

// fB (n--) - File: block load
// Loads a block file
void blockLoad(CELL num) {
    char buf[24];
    sprintf(buf, "block-%03d.r4", (int)num);
    FILE* fp = fopen(buf, "rb");
    if (fp) {
        if (input_fp) { fpush(input_fp); }
        input_fp = (CELL)fp;
    }
}

void loadAbort() {
    if (input_fp) {
        fclose((FILE*)input_fp);
        input_fp = fpop();
    }
}

int readBlock(int blk, char *buf, int sz) {
    int cn = 0;
    char fn[24];
    sprintf(fn, "block-%03d.r4", blk);
    for (int i = 0; i < sz; i++) { buf[i] = 0; }
    FILE *fp = fopen(fn, "rb");
    if (fp) {
        // Read in one byte at a time, to strip out CR
        while (cn < sz) {
            CELL n = fread(fn, 1, 1, fp);
            if (n == 0) { break; }
            if (fn[0] == 13) { continue; }
            buf[cn++] = fn[0];
        }
        fclose(fp);
    }
    return fp ? 1 : 0;
}

// bR (num addr sz--f)
void readBlock1() {
    CELL t1 = pop();
    char *n1 = (char*)pop();
    TOS = readBlock(TOS, (char*)n1, t1);
}

int writeBlock(int blk, char *buf, int sz) {
    char fn[24];
    sprintf(fn, "block-%03d.r4", blk);
    FILE *fp = fopen(fn, "wb");
    if (fp) {
        fwrite(buf, 1, sz, fp);
        fclose(fp);
    }
    return fp ? 1 : 0;
}

// bW (num addr sz--f)
void writeBlock1() {
    CELL t1 = pop();
    char *n1 = (char*)pop();
    TOS = writeBlock(TOS, (char*)n1, t1);
}

#endif // __FILES__
