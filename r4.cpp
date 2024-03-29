// r4 - A minimal human-readable interpreter

#include "r4.h"
#include "config.h"
#include <cmath>

byte ir, isBye = 0, isError = 0;
addr pc, HERE;
CELL n1, t1, seed = 0;
int dsp, rsp, lsp, locBase;
ST_T   dstack[STK_SZ+1];
CELL   reg[NUM_REGS], locs[NUM_LOCALS], lstack[LSTACK_SZ+1];
addr   rstack[RSTK_SZ+1], func[NUM_FUNCS];
byte   code[CODE_SZ], vars[VARS_SZ];
static char buf[128];

void push(CELL v) { if (dsp < STK_SZ) { dstack[++dsp].i = v; } }
CELL pop() { return (dsp) ? dstack[dsp--].i : 0; }

void rpush(addr v) { if (rsp < RSTK_SZ) { rstack[++rsp] = v; } }
addr rpop() { return (rsp) ? rstack[rsp--] : 0; }

void vmInit() {
    dsp = rsp = lsp = locBase = 0;
    for (int i = 0; i < NUM_REGS;  i++) { reg[i]  = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { func[i] = 0; }
    for (int i = 0; i < CODE_SZ;   i++) { code[i] = 0; }
    HERE = &code[0];
}

void setCell(byte* to, CELL val) {
#ifdef _NEEDS_ALIGN_
    *(to++) = (byte)val; 
    for (int i = 1; i < CELL_SZ; i++) {
        val = (val >> 8);
        *(to++) = (byte)val;
    }
#else
    * ((CELL *)to) = val;
#endif
}

CELL getCell(byte* from) {
    CELL val = 0;
#ifdef _NEEDS_ALIGN_
    from += (CELL_SZ - 1);
    for (int i = 0; i < CELL_SZ; i++) {
        val = (val << 8) + *(from--);
    }
#else
    val = *((CELL*)from);
#endif
    return val;
}

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

char *num2Str(CELL v, int b) {
    b = b ? b : 10;
    char *c = &buf[sizeof(buf)-1], n=((v<0) && (b==10))?1:0;
    UCELL u = (n) ? -v : v;
    *(c) = 0;
    do {
        *(--c) = (char)((u % b)+'0');
        if (*c>'9') { *c += 7; }
        u /= b;
    } while (u);
    if (n) { *(--c) = '-'; }
    return (char*)c;
}

void printBase(CELL v, int b) {
    printString(num2Str(v, b));
}

addr dotQ(addr str) {
    addr y = str ? str : pc;
    while (*y && (*y != '"')) {
        char c = *(y++);
        if (c == '%') {
            c = *(y++);
            if (c == 'd') { printBase(pop(), 10); }
            else if (c == 'c') { printChar((int)pop()); }
            else if (c == 'e') { printChar(27); }
            else if (c == 'f') { printStringF("%f", FTOS); pop(); }
            else if (c == 'g') { printStringF("%g", FTOS); pop(); }
            else if (c == 'n') { printChar(13); printChar(10); }
            else if (c == 'q') { printChar('"'); }
            else if (c == 's') { printString((char*)pop()); }
            else if (c == 'b') { printBase(pop(), 2); }
            else if (c == 'x') { printBase(pop(), 16); }
            else if (c == 'B') { int t = (int)pop(); printBase(pop(), t); }
            else { printChar(c); }
        }
        else { printChar(c); }
    }
    return y;
}

void dumpStack() {
    printChar('(');
    for (int i = 1; i <= dsp; i++) {
        printStringF("%s%ld", (i > 1 ? " " : ""), dstack[i].i);
    }
    printChar(')');
}

CELL doHash(CELL max) {
    CELL hash = 5381;
    if (!ISALPHA(*pc)) { return 0; }
    while (ISALPHANUM(*pc)) { hash = (hash * 33) ^ *(pc++); }
    return hash & max;
}

void skipTo(byte to, int isCreate) {
    while (*pc) {
        ir = *(pc++);
        if (isCreate) { *(HERE++) = ir; }
        if ((to == '"') && (ir != to)) { continue; }
        if ((to == '`') && (ir != to)) { continue; }
        if (ir == to) { return; }
        if (ir == '\'') { ++pc; continue; }
        if (ir == '(') { skipTo(')', isCreate); continue; }
        if (ir == '[') { skipTo(']', isCreate); continue; }
        if (ir == '"') { skipTo('"', isCreate); continue; }
        if (ir == '`') { skipTo('`', isCreate); continue; }
    }
    isError = 1;
}

void doFor() {
    CELL f = pop(), t = pop();
    lsp += 3; if (LSTACK_SZ < lsp) { printString("-[lsp]-"); lsp=LSTACK_SZ; }
    L0 = (f<t)?f:t; L1 = (t>f)?t:f; L2 = (CELL)pc;
}

int isOk(CELL exp, const char* msg) {
    isError = (exp == 0); if (isError) { printString(msg); }
    return (isError == 0);
}

void doFloat() {
    ir = *(pc++);
    switch (ir) {
        case  'F': FTOS = (FLT_T)TOS;
        RCASE 'I': TOS = (CELL)FTOS;
        RCASE '+': FNOS += FTOS; pop();
        RCASE '-': FNOS -= FTOS; pop();
        RCASE '*': FNOS *= FTOS; pop();
        RCASE '/': if (isOk(FTOS!=0, "-0div-")) { FNOS /= FTOS; pop(); }
        RCASE '<': NOS = (FNOS<FTOS)  ? 1 : 0; pop();
        RCASE '>': NOS = (FNOS>FTOS)  ? 1 : 0; pop();
        RCASE '=': NOS = (FNOS==FTOS) ? 1 : 0; pop();
        RCASE '_': FTOS = -FTOS;
        RCASE '.': printStringF("%g", FTOS); pop();
        RCASE 'Q': FTOS = sqrt(FTOS);
        RCASE 'T': FTOS = tanh(FTOS);
        return; default:
            isError = 1;
            printStringF("-flt:%c?-", ir);
    }
}

void doExt() {
    ir = *(pc++);
    switch (ir) {
        case 'I': ir = *(pc++);
            if (ir == 'A') { 
                ir = *(pc++);
                if (ir == 'F') { push((CELL)&func[0]); }
                if (ir == 'H') { push((CELL)&HERE); }
                if (ir == 'R') { push((CELL)&reg[0]); }
                if (ir == 'U') { push((CELL)&code[0]); }
                if (ir == 'V') { push((CELL)&vars[0]); }
                return;
            };
            if (ir == 'C') { push(CELL_SZ); }
            if (ir == 'F') { push(NUM_FUNCS); }
            if (ir == 'H') { push((CELL)HERE); }
            if (ir == 'L') { push(NUM_LOCALS); }
            if (ir == 'R') { push(NUM_REGS); }
            if (ir == 'U') { push(CODE_SZ); }
            if (ir == 'V') { push(VARS_SZ); }
        RCASE 'h': t1=doHash(-1); push(t1);
                push(reg[t1&MAX_REG]);
                push((CELL)func[t1&MAX_FUNC]);
        RCASE 'K': dumpStack();
        RCASE 'S': if (*pc == 'R') { vmInit(); }
        RCASE 'M': push(doMicros());
        RCASE 'T': push(doMillis());
        RCASE 'W': if (0 < TOS) { doDelay(TOS); } pop();
        RCASE 'R': if (!seed) { seed = getSeed(); }
                seed ^= (seed << 13);
                seed ^= (seed >> 17);
                seed ^= (seed << 5);
                TOS = (TOS) ? ABS(seed) % TOS : seed;
        RCASE 'V': push(VERSION);
        return; default: pc = doCustom(ir, pc);
    }
}

addr run(addr start) {
    pc = start;
    isError = 0;
    rsp = lsp = 0;
next:
    if (isError) { return pc; }
    ir = *(pc++);
    switch (ir) {
        case  0:  return pc;
        case ' ': while (*(pc) == ' ') { pc++; }
        NCASE '!': setCell((byte*)TOS, NOS); DROP2;
        NCASE '"': pc = dotQ(pc); if (*pc == ir) { ++pc; }
        NCASE '#': push(TOS);
        NCASE '$': t1 = TOS; TOS = NOS; NOS = t1;
        NCASE '%': push(NOS);
        NCASE '&': t1 = doHash(MAX_REG);
            if (reg[t1]) { printStringF("-redef-r:[%ld]-",t1); }
            reg[t1] = pop();
        NCASE '\'': push(*(pc++));
        NCASE '(': if (!TOS) { skipTo(')', 0); } pop();
        NCASE ')': /* nothing to do here */
        NCASE '*': t1 = pop(); TOS *= t1;
        NCASE '+': t1 = pop(); TOS += t1;
        NCASE ',': printChar((char)pop());
        NCASE '-': t1 = pop(); TOS -= t1;
        NCASE '.': printStringF("%ld", (CELL)pop());
        NCASE '/': if (isOk(TOS, "-0div-")) { NOS /= TOS; pop(); }
        NCASE '0': case '1': case '2': case '3': case '4':
        case  '5': case '6': case '7': case '8': case '9': push(ir-'0');
            while (ISNUM(*pc)) { TOS = (TOS * 10) + *(pc++) - '0'; }
            if (*pc=='.') {
                FLT_T x=10;
                FTOS=(FLT_T)TOS; ++pc;
                while (ISNUM(*pc)) { FTOS += (*pc-'0')/x; x*=10; ++pc; }
            }
        NCASE ':': t1 = doHash(MAX_FUNC);
            while (*(pc) == ' ') { pc++; }
            if (func[t1] && t1) { printStringF("-redef-f:[%ld]-",t1); }
            func[t1] = (addr)pc;
            skipTo(';', 0);
            HERE = (HERE < pc) ? pc : HERE;
        NCASE ';': pc = rpop(); if (!pc) return pc;
        NCASE '<': t1 = pop(); TOS = (TOS <  t1) ? 1 : 0;
        NCASE '=': t1 = pop(); TOS = (TOS == t1) ? 1 : 0;
        NCASE '>': t1 = pop(); TOS = (TOS >  t1) ? 1 : 0;
        // NCASE '?': /*FREE*/
        NCASE '@': TOS = getCell((byte*)TOS);
        NCASE 'A': if (TOS < 0) { TOS = -TOS; }
        NCASE 'B': printChar(' ');
        NCASE 'C': ir = *(pc++);
            if (ir == '@') { TOS = *(byte*)TOS; }
            else if (ir == '!') { *(byte*)TOS = (byte)NOS; DROP2; }
        NCASE 'D': --TOS;
        NCASE 'F': doFloat();
        NCASE 'G': if (TOS) { pc = (addr)TOS; } pop();
        NCASE 'I': push(L0);
        NCASE 'J': push(L3);
        NCASE 'K': ir = *(pc++);
            if (ir == '?') { push(qkey()); }
            else if (ir == '@') { push(key()); }
        NCASE 'L': t1 = pop(); TOS = (TOS << t1);
        NCASE 'M': if (isOk(TOS, "-0div-")) { t1 = pop(); TOS %= t1; }
        NCASE 'N': printString("\r\n");
        NCASE 'P': ++TOS;
        NCASE 'R': t1 = pop(); TOS = (TOS >> t1);
        NCASE 'S': if (isOk(TOS, "-0div-")) { t1 = TOS; TOS = NOS % t1; NOS /= t1; }
        NCASE 'T': ir = *(pc++);
            if (ir == '+') { locBase = MIN(locBase+10, NUM_LOCALS-10); }
            else if (ir == '-') { locBase = MAX(locBase-10, 0); }
        NCASE 'U': TOS += (CELL)&code[0];
        NCASE 'V': TOS += (CELL)&vars[0];
        NCASE 'X': if (TOS) { rpush(pc); pc = (addr)TOS; } pop();
        NCASE 'Z': printString((char*)pop());
        NCASE '[': doFor();
        NCASE '\\': pop();
        NCASE ']': if ((++L0)<L1) { pc=(addr)L2; NEXT; } /* fall through */
        case '^': lsp = (lsp<3) ? 0: lsp-3;
        NCASE '_': TOS = -TOS;
        NCASE '`': push(TOS);
            while ((*pc) && (*pc != ir)) { *(AOS++) = *(pc++); }
            *(AOS++) = 0; pc++;
        NCASE 'b': ir = *(pc++);                    // BIT and Block operations
            if (ir == '&') { NOS &= TOS; pop(); }           // AND
            else if (ir == '|') { NOS |= TOS; pop(); }      // OR
            else if (ir == '^') { NOS ^= TOS; pop(); }      // XOR
            else if (ir == '~') { TOS = ~TOS; }             // NOT (COMPLEMENT)
            else if (ir == 'L') { blockLoad(pop()); }       // Block Load
            else if (ir == 'A') { loadAbort(); }            // Block Load Abort
            else if (ir == 'E') { doEditor(); }             // Block Edit
            else if (ir == 'R') { readBlock1(); }           // Block Read
            else if (ir == 'W') { writeBlock1(); }          // Block Write
        NCASE 'c': t1=doHash(MAX_FUNC);
            if (func[t1]) {
                if (*pc != ';') { rpush(pc); }
                pc = func[t1];
            }
        NCASE 'd': if (isLocal(*pc)) { --locs[*(pc++)-'0'+locBase]; }
                   else { --reg[doHash(MAX_REG)]; }
        NCASE 'f': ir = *(pc++);
            if (ir == 'O') { fileOpen(); }
            else if (ir == 'C') { fileClose(); }
            else if (ir == 'D') { fileDelete(); }
            else if (ir == 'R') { fileRead(); }
            else if (ir == 'W') { fileWrite(); }
            else if (ir == 'L') { t1=pop(); TOS = fileReadLine(t1, AOS); }
        NCASE 'h': push(0); while (1) {
                t1 = ISNUM(*pc) ? (*pc)-'0' : -1;
                t1 = BTWI(*pc,'A','F') ? (*pc)-'A'+10 : t1;
                if (t1 < 0) { NEXT; }
                TOS = (TOS*16) + t1; ++pc;
            }
        NCASE 'i': if (isLocal(*pc)) { ++locs[*(pc++)-'0'+locBase]; }
                   else { ++reg[doHash(MAX_REG)]; }
        NCASE 'p': L0 += pop();
        NCASE 'r': if (isLocal(*pc)) { push(locs[*(pc++)-'0'+locBase]); }
                   else { push(reg[doHash(MAX_REG)]); }
        NCASE 's': if (isLocal(*pc)) { locs[*(pc++)-'0'+locBase] = pop(); }
                   else { reg[doHash(MAX_REG)] = pop(); }
        NCASE 'x': doExt();
        NCASE '{': if (!TOS) { skipTo('}', 0); NEXT; }
                push(0); push(0); doFor();
        // NCASE '|':  /*FREE*/
        NCASE '}': if (TOS) { pc=(addr)L2; } else { pop(); lsp=(lsp<3)?0:lsp-3; }
        NCASE '~': TOS = (TOS) ? 0 : 1;
        NEXT;
        default: printStringF("-[ir:%d?]-", ir); NEXT;
    }
    return pc;
}
