// r4 - A minimal human-readable interpreter

#include "r4.h"
#include <stdlib.h>

#ifdef __PC__

#ifdef __WINDOWS__
CELL doMillis() { return (CELL)GetTickCount(); }
CELL doMicros() { return (CELL)doMillis()*1000; }
void doDelay(CELL ms) { Sleep((DWORD)ms); }
int qkey() { return _kbhit(); }
int key() { return _getch(); }
#else
// Support for Linux
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
#include <unistd.h>
#include <termios.h>
static struct termios normT, rawT;
static int isTtyInit = 0;
void ttyInit() {
    tcgetattr( STDIN_FILENO, &normT);
    cfmakeraw(&rawT);
    isTtyInit = 1;
}
void ttyModeNorm() {
    if (!isTtyInit) { ttyInit(); }
    tcsetattr( STDIN_FILENO, TCSANOW, &normT);
}
void ttyModeRaw() {
    if (!isTtyInit) { ttyInit(); }
    tcsetattr( STDIN_FILENO, TCSANOW, &rawT);
}
int qkey() {
    struct timeval tv;
    fd_set rdfs;
    ttyModeRaw();
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    int x = FD_ISSET(STDIN_FILENO, &rdfs);
    ttyModeNorm();
    return x;
}
int key() {
    ttyModeRaw();
    int x = getchar();
    ttyModeNorm();
    return x;
}
#endif

static char buf[256];
static CELL t1, t2;

void printChar(const char c) { printf("%c", c); }
void printString(const char* str) { printf("%s", str); }
CELL getSeed() { return doMillis(); }

addr doCustom(byte ir, addr pc) {
    switch (ir) {
        case 'Q': isBye = 1;                 break;
        case 's': system((char*)pop());      break;
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
    char *x = cp;
    while (*x) { ++x; }
    --x;
    while (*x && (*x < 32) && (cp <= x)) { *(x--) = 0; }
}

void loadCode(const char* src) {
    addr in = HERE;
    while (*src) {
        char c = *(src++);
        *(in++) = (c<32) ? 32 : c;
    }
    *in = 0;
    run(HERE);
}

// #define __HISTORY__

void doHistory(char* str) {
#ifdef __HISTORY__
    FILE* fp = fopen("history.txt", "at");
    if (fp) {
        fputs(str, fp);
        fclose(fp);
    }
#endif
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
    // doHistory(buf);
    loadCode(buf);
}

int main(int argc, char** argv) {
    vmInit();
    if (1 < argc) { input_fp = (CELL)fopen(argv[1], "rt"); }
    if (!input_fp) {
        loadCode(":CD 0UxIH[IC@#,';=(IPC@':=(N))];");
        loadCode("0bL");
    }
    while (!isBye) { loop(); }
    return 0;
}

#endif
