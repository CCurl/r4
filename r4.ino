#include "r4.h"

#if __SERIAL__
    int qkey() { return mySerial.available(); }
    int key() {
        while (!qkey()) {}
        return mySerial.read();
    }
    void printChar(char c) { mySerial.print(c); }
    void printString(const char* str) { mySerial.print(str); }
#else
    int qkey() { return 0; }
    int key() { return 0; }
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
void loadBaseSystem() {
    // loadCode(":CD 0U xIH[I C@ #, 59=(I P C@ 58=(N))];");
    // loadCode(":S0 xIR\"%d registers, \" xIF\"%d functions, \";");
    // loadCode(":S1 xIU\"%d bytes code, \" xIV\"%d bytes user\";");
    // loadCode(":SI N\"r4 - \" cS0 cS1;");
    // loadCode(":FT rVH `block-000``r`\\ fO;");
    // loadCode(":Q i6 rA#*rS/sC rB#*rS/sD rCrD+rK>(rJsM;)rArB*100/rY+sB rCrD-rX+sA iJ;");
    // loadCode(":L 0sA 0sB rS sM 1{\\cQ rJ rM<};");
    // loadCode(":O 0sJ cL rJ 40+# 126>(\\32),;");
    // loadCode(":X 490_ sX 1 95[  cO rX  8+sX];");
    // loadCode(":Y 300_ sY 1 31[N cX rY 20+sY];");
    // loadCode(":I 200 sS 1000000 sK;");
    // loadCode(":M cI 0s6 xT cY xT$- N r6\"%d iterations, %d ms\";");
    loadCode("0 bL");
}

void ok() {
    printString("\r\nr4:"); 
    dumpStack(); 
    printString(">");
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
    if (here == (addr)NULL) {
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
    if (BTWI(c, 32, 126)) {
        *(here1++) = (byte)c;
        char b[] = {c, 0};
        printString(b);
    }
}

void setup() {
#ifdef __SERIAL__
    while (!mySerial) {}
    mySerial.begin(19200);
    while (mySerial.available()) { mySerial.read(); }
#endif
    vmInit();
    fileInit();
    loadBaseSystem();
    ok();
}

void autoRun() {
    pc = (addr)"AUTORUN";
    CELL h = doHash(MAX_FUNC);
    if (func[h]) { run(func[h]); }
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
        //printString("-inputFp-");
        int n = fileReadLine(input_fp, (char *)HERE);
        //printStringF("\r\n%s", (char *)HERE);
        if (n < 0) {
          push(input_fp); fileClose();
          input_fp = fpop();
        }
        else { run(HERE); }
        if (input_fp == 0) { ok(); }
        return;
    } else {
        while ( qkey() ) {
            handleInput(key());
        }
    }
    autoRun();
}
