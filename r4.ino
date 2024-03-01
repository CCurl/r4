#include "r4.h"

#if __SERIAL__
    int charAvailable() { return mySerial.available(); }
    int getChar() { 
        while (!charAvailable()) {}
        return mySerial.read();
    }
    void printChar(char c) { mySerial.print(c); }
    void printString(const char* str) { mySerial.print(str); }
#else
    int charAvailable() { return 0; }
    int getChar() { return 0; }
    void printString(const char* str) { }
    void printChar(char c) { }
#endif

CELL getSeed() { return millis(); }
CELL doMicros() { return micros(); }
CELL doMillis() { return millis(); }
void doDelay(CELL ms) { return delay(ms); }

addr doCustom(byte ir, addr pc) {
    CELL pin;
    switch (ir) {
    case 'P': ir = *(pc++);
        pin = pop();
        if (ir == 'I') { pinMode(pin, INPUT); }
        if (ir == 'O') { pinMode(pin, OUTPUT); }
        if (ir == 'U') { pinMode(pin, INPUT_PULLUP); }
        if (ir == 'R') {
            ir = *(pc++);
            if (ir == 'A') { push(analogRead(pin)); }
            if (ir == 'D') { push(digitalRead(pin)); }
        } else if (ir == 'W') {
            CELL val = pop();
            ir = *(pc++);
            if (ir == 'A') { analogWrite(pin, val); }
            if (ir == 'D') { digitalWrite(pin, val); }
        }
        break;
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}

void loadCode(const char* src) {
    addr here = (addr)HERE;
    addr here1 = here;
    while (*src) {
        *(here1++) = *(src++);
    }
    *here1 = 0;
    run(here);
}

// ********************************************
// * HERE is where you load your default code *
// ********************************************

#define SOURCE_STARTUP \
    X(1000, ":C xIAU xIH1-[rI C@#,59=(rI1+C@58=(N))];") \
    X(1010, ":R 0xIR1-[rI4*xIAR+@#s1(rI26S$26S$'A+,'A+,'A+,':,Br1.N)];") \
    X(1040, ":U xIH xIAU-;") \
    X(1045, ":SI N\"System: \"xIR.\" registers, \"xIF.\" functions: \"xIU.\" bytes code memory.\";") \
    X(2000, ":Q iT rA#*rS/sC rB#*rS/sD rCrD+rK>(rJsM;)rArB*100/rY+sB rCrD-rX+sA iJ;") \
    X(2010, ":L 0sA 0sB 0sJ rS sM 1{\\cQ rJ rM<};") \
    X(2020, ":O cL rJ 40+# 126>(\\32),;") \
    X(2030, ":X 490_sX 1 95[  cO rX 8+sX];") \
    X(2040, ":Y 300_sY 1 31[N cX rY20+sY];") \
    X(2050, ":M cI 0sT xT cY xT$- N rT.\" iterations, \" . \" ms\";") \
    X(2060, ":I 200 sS 1000000 sK;") \
    X(9999, "1000 xW cSI 0 fL")

#define X(num, val) const PROGMEM char str ## num[] = val;
SOURCE_STARTUP

#undef X
#define X(num, val) str ## num,
const char *bootStrap[] = {
    SOURCE_STARTUP
    NULL
};

void loadBaseSystem() {
    for (int i = 0; bootStrap[i] != NULL; i++) {
        loadCode(bootStrap[i]);
    }
}

void ok() {
    printString("\r\nr4:("); 
    dumpStack(); 
    printString(")>");
}

// PuTTY sends a 127 for backspace
int isBackspace(char c) {
    if (c == 8) { return 1; }
    if (c == 127) { return 1; }
    return 0;
}

void handleInput(char c) {
    static addr here = (addr)NULL;
    static addr here1 = (addr)NULL;
    if (here == NULL) { 
        here = (addr)HERE; 
        here1 = here; 
    }
    if (c == 13) {
        printString(" ");
        *(here1) = 0;
        run(here);
        here = (addr)NULL;
        ok();
        return;
    }

    if (isBackspace(c) && (here < here1)) {
        here1--;
        char b[] = {8, 32, 8, 0};
        printString(b);
        return;
    }
    if (c == 9) { c = 32; }
    if (32 <= c) {
        *(here1++) = (byte)c;
        char b[] = {c, 0};
        printString(b);
    }
}

void setup() {
#ifdef __SERIAL__
    while (!mySerial) {}
    mySerial.begin(19200);
    while (mySerial.available()) { char c = mySerial.read(); }
#endif
    vmInit();
    fileInit();
    loadBaseSystem();
    ok();
}

void loop() {
    static int iLed = 0;
    static long nextBlink = 0;
    static int ledState = LOW;
    long curTm = millis();
    
    if (iLed == 0) {
        iLed = LED_BUILTIN;
        pinMode(iLed, OUTPUT);
    }
    if (nextBlink < curTm) {
        ledState = (ledState == LOW) ? HIGH : LOW;
        digitalWrite(iLed, ledState);
        nextBlink = curTm + 1000;
    }

    if (input_fp) {
        // printString("-inputFp-");
        int n = fileReadLine(input_fp, (char *)HERE);
        // printStringF("\r\n%s", (char *)HERE);
        if (n < 0) { input_fp = fpop(); }
        run(HERE);
        if (input_fp == 0) { ok(); }
        return;
    } else {
        while ( charAvailable() ) {
            handleInput(getChar());
        }
    }

    addr a = func[NUM_FUNCS-1];
    if (a) { run(a); }
}
