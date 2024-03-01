#include "r4.h"

#ifndef __FILES__
#ifndef __LITTLEFS__
void noFile() { printString("-noFile-"); }
void fileInit() { noFile(); }
void fileOpen() { noFile(); }
void fileClose() { noFile(); }
void fileDelete() { noFile(); }
void fileRead() { noFile(); }
void fileWrite() { noFile(); }
addr codeLoad(addr x) { noFile(); return x; }
void codeSave(addr x, addr y) { noFile(); }
void blockLoad(CELL num) { noFile(); }
void loadAbort() { noFile(); }
int fileReadLine(CELL fh, char* buf) { noFile(); return -1; }
int readBlock(int blk, char* buf, int sz) { noFile(); return 0; }
int writeBlock(int blk, char* buf, int sz) { noFile(); return 0; }
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
// n=0: End of file or file error
void fileDelete() {
    char* fn = AOS;
    TOS = remove(fn) == 0 ? 1 : 0;
}

// fR (fh--c n) - File Read
// fh: File handle, c: char read, n: num chars read
// n=0: End of file or file error
void fileRead() {
    FILE* fh = (FILE*)TOS;
    NOS = TOS = 0;
    push(0);
    if (fh) {
        char c;
        TOS = fread(&c, 1, 1, fh);
        NOS = TOS ? c : 0;
    }
}

// fileReadLine(fh, buf)
// fh: File handle, buf: address
// returns: -1 if EOF, else len
int fileReadLine(CELL fh, char *buf) {
    byte c, len = 0;
    while (1) {
        *(buf) = 0;
        CELL n = fread(&c, 1, 1, (FILE *)fh);
        if (n == 0) { return -1; }
        if (c == 10) { break; }
        if (c == 13) { break; }
        if (c == 9) { c = 32; }
        if (BTWI(c, 32, 126)) {
            *(buf++) = c;
            ++len;
        }
    }
    return len;
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

// fL (--) - File Load code
addr codeLoad(addr code, addr here) {
    FILE *fh = fopen("code.r4", "rt");
    if (fh) {
        vmInit();
        CELL num = fread(code, 1, CODE_SZ, fh);
        fclose(fh);
        here = code + num;
        run(code);
        printStringF("-loaded, (%d bytes)-", num);
    }
    else {
        printString("-loadFail-");
    }
    return here;
}

// fS (--) - File Save code
void codeSave(addr code, addr here) {
    FILE* fh = fopen("code.r4", "wt");
    if (fh) {
        int count = (int)(here - code);
        fwrite(code, 1, count, fh);
        fclose(fh);
        printStringF("-saved (%d)-", count);
    }
    else {
        printString("-saveFail-");
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

#endif // __FILES__
