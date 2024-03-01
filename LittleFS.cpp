#include "r4.h"

#ifdef __LITTLEFS__
#include "LittleFS.h"

// shared with __FILE__
static byte fdsp = 0;
static CELL fstack[STK_SZ + 1];
CELL input_fp;

void fpush(CELL v) { if (fdsp < STK_SZ) { fstack[++fdsp] = v; } }
CELL fpop() { return (fdsp) ? fstack[fdsp--] : 0; }
// shared with __FILE__

#define MAX_FILES 10
File files[MAX_FILES+1];
int numFiles = 0;
File f;

int freeFile() {
  for (int i = 1; i <= MAX_FILES; i++) {
    if (!files[i]) { return i; }
  }
  isError = 1;
  printString("-fileFull-");
  return 0;
}

void fileInit() {
    // for (int i = 0; i < MAX_FILES; i++) { files[i] = NULL; }
    LittleFS.begin();
    FSInfo fs_info;
    LittleFS.info(fs_info);
    printStringF("\r\nLittleFS: Total: %ld", fs_info.totalBytes);
    printStringF("\r\nLittleFS: Used: %ld", fs_info.usedBytes);
}

void fileOpen() {
    char* md = (char*)pop();
    char* fn = (char*)pop();
    int i = freeFile();
    if (i) {
        files[i] = LittleFS.open(fn, md);
        if (!files[i]) {
            i = 0;
            isError = 1;
            printString("-openFail-");
        }
    }
    push(i);
}

void fileClose() {
    int fn = (int)pop();
    if (BetweenI(fn, 1, MAX_FILES) && (files[fn])) {
        files[fn].close();
        // files[fn] = NULL;
    }
}

void fileDelete() {
    char* fn = (char*)TOS;
    TOS = LittleFS.remove(fn) ? 1 : 0;
}

// (fh--c n) -fh: file handle, c: char, n: num read (0 => EOF)
void fileRead() {
    int fn = (int)TOS;
    TOS = 0;
    push(0);
    if (BetweenI(fn, 1, MAX_FILES) && (files[fn])) {
        byte c;
        TOS = files[fn].read(&c, 1);
        NOS = (CELL)c;
    }
}

// fileReadLine(fh, buf)
// fh: File handle, buf: address
// return: -1 if EOF, else len
int fileReadLine(CELL fh, char* buf) {
    // char t[24];
    // sprintf(t, "-fh=%d-", fh);
    // printString(t);
    *(buf) = 0;
    if (!BetweenI(fh, 1, MAX_FILES)) { return -1; }
    if (!files[fh]) { return -1; }
    byte c, len = 0, n;
    while (1) {
        n = files[fh].read(&c, 1);
        if (n == 0) { 
            // printString("-n=0-");
            files[fh].close();
            // files[fh] = NULL;
            break; 
        }
        // printStringF("-%d-", c);
        if (c == 13) { break; }
        if (c == 10) { break; }
        if (BetweenI(c, 32, 126)) {
            *(buf++) = c;
            ++len;
        }
    }
    *(buf) = 0;
    return (n == 0) ? -1 : len;
}

void fileWrite() {
    int fn = (int)pop();
    byte c = (byte)TOS;
    TOS = 0;
    if (BetweenI(fn, 1, MAX_FILES) && (files[fn])) {
        TOS = files[fn].write(&c, 1);
    }
}

addr codeLoad(addr u, addr h) {
    File f = LittleFS.open("/Code.R4", "r");
    if (f) {
        vmInit();
        int num = f.read(u, CODE_SZ);
        f.close();
        h = u + num;
        run(u);
        printStringF("-loaded, (%d)-", num);
    }
    else {
        printString("-loadFail-");
    }
    return h;
}

void codeSave(addr u, addr h) {
    File f = LittleFS.open("/Code.R4", "w");
    if (f) {
        int count = h - u;
        f.write(u, count);
        f.close();
        printString("-saved-");
    }
    else {
        printString("-saveFail-");
    }
}

void blockLoad(CELL blk) {
    char fn[24];
    int i = freeFile();
    if (!i) { return; }
    sprintf(fn, "/Block-%03d.R4", blk);
    // printString(fn);
    files[i] = LittleFS.open(fn, "r");
    if (files[i]) {
        // printStringF("-opened-%d-", i);
        if (input_fp) { fpush(input_fp); }
        input_fp = i;
    }
}

int readBlock(int blk, char* buf, int sz) {
    char fn[24];
    sprintf(fn, "/Block-%03d.R4", blk);
    for (int i = 0; i < sz; i++) { buf[i] = 32; }
    File f = LittleFS.open(fn, "r");
    if (f) {
        int n = f.read((uint8_t*)buf, sz);
        f.close();
        return 1;
    } else {
        return 0;
    }
}

int writeBlock(int blk, char* buf, int sz) {
    char fn[24];
    sprintf(fn, "/Block-%03d.R4", blk);
    File f = LittleFS.open(fn, "w");
    if (f) {
        int n = f.write((uint8_t *)buf, sz);
        f.close();
        return 1;
    }
    else {
        return 0;
    }
}

#endif // __LITTLEFS__
