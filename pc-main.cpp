// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include "r4.h"

#ifdef __PC__

#ifdef __WINDOWS__
CELL doMillis() { return (CELL)GetTickCount(); }
CELL doMicros() { return (CELL)doMillis()*1000; }
void doDelay(CELL ms) { Sleep(ms); }
#else
CELL doMillis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (CELL)((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
}
CELL doMicros() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (CELL)(ts.tv_nsec);
}
void doDelay(CELL ms) { 
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL); 
}
int _kbhit() { return 0; }
int _getch() { return 0; }
#endif

static char buf[256];
static CELL t1, t2;

void printChar(const char c) { printf("%c", c); }
void printString(const char* str) { printf("%s", str); }
CELL getSeed() { return doMillis(); }

int getChar() { return _getch(); }
int charAvailable() { return _kbhit(); }

addr doCustom(byte ir, addr pc) {
    switch (ir) {
    case 'Q': isBye = 1;                       break;
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}

void ok() {
    printString("\r\nr4:");
    dumpStack();
    printString(">");
}

void rtrim(char* cp) {
    char* x = cp;
    while (*x) { ++x; }
    --x;
    while (*x && (*x < 32) && (cp <= x)) { *(x--) = 0; }
}

void loadCode(const char* src) {
    addr here = (addr)HERE;
    addr tib = here;
    while (*src) {
        *(tib++) = *(src++);
    }
    *tib = 0;
    run(here);
}

void doHistory(char* str) {
    FILE* fp = fopen("history.txt", "at");
    if (fp) {
        fputs(str, fp);
        fclose(fp);
    }
}

void loop() {
    if (input_fp) {
        int n = fileReadLine(input_fp, buf);
        if (n == -1) {
            fclose((FILE *)input_fp);
            input_fp = fpop();
        }
    } else {
        ok();
        fgets(buf, sizeof(buf), stdin);
        doHistory(buf);
        rtrim(buf);
    }
    loadCode(buf);
}

int main(int argc, char** argv) {
    vmInit();
    loadCode(":C 0U xIH D[I C@#,59=(I P C@ 58=(N))];");
    loadCode("0 bL");
    while (!isBye) { loop(); }
    return 0;
}

#endif
