// R4 - A Minimal Interpreter

#include "r4.h"
#include "config.h"

byte ir, isBye = 0, isError = 0;
addr pc, HERE;
CELL n1, t1, seed = 0;
int dsp, rsp, lsp, locStart;
ST_T   dstack[STK_SZ+1];
CELL   reg[NUM_REGS], locals[(RSTK_SZ+1)*10], lstack[LSTACK_SZ+1];
addr   rstack[RSTK_SZ+1], func[NUM_FUNCS];
byte   user[USER_SZ], vars[VARS_SZ];

void push(CELL v) { if (dsp < STK_SZ) { dstack[++dsp].i = v; } }
CELL pop() { return (dsp) ? dstack[dsp--].i : 0; }

void rpush(addr v) { if (rsp < RSTK_SZ) { rstack[++rsp] = v; } }
addr rpop() { return (rsp) ? rstack[rsp--] : 0; }

void vmInit() {
    dsp = rsp = lsp = locStart = 0;
    for (int i = 0; i < NUM_REGS; i++) { reg[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { user[i] = 0; }
    // for (int i = 0; i < NUM_FUNCS; i++) { func[i] = 0; }
    HERE = &user[0];
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
    static char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

void dumpStack() {
    printChar('(');
    for (int i = 1; i <= dsp; i++) {
        printStringF("%s%ld", (i > 1 ? " " : ""), (CELL)dstack[i].i);
    }
    printChar(')');
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
    CELL f = pop(), t=pop();
    lsp += 3; if (LSTACK_SZ < lsp) { printString("-[lsp]-"); lsp=LSTACK_SZ; }
    L0 = (f<t)?f:t; L1 = (t>f)?t:f; L2 = (CELL)pc;
}

int isOk(int exp, const char* msg) {
    isError = (exp == 0); if (isError) { printString(msg); }
    return (isError == 0);
}

void doFloat() {
    ir = *(pc++);
    switch (ir) {
        case  'F': FTOS = (double)TOS;
        RCASE 'I': TOS = (CELL)FTOS;
        RCASE '+': FNOS += FTOS; pop();
        RCASE '-': FNOS -= FTOS; pop();
        RCASE '*': FNOS *= FTOS; pop();
        RCASE '/': if (isOk(FTOS!=0, "-0div-")) { FNOS /= FTOS; pop(); }
        RCASE '<': FNOS = (FNOS<FTOS)  ? 1 : 0; pop();
        RCASE '>': FNOS = (FNOS>FTOS)  ? 1 : 0; pop();
        RCASE '=': FNOS = (FNOS==FTOS) ? 1 : 0; pop();
        RCASE '.': printStringF("%g", FTOS); pop();
        return; default:
            isError = 1;
            printStringF("-flt:%c?-", ir);
    }
}

int getRFnum(int max) {
    CELL n = 0;
    while (isRegChar(*pc)) { n = (n * 26) + *(pc++)-'A'; }
    if (BTWI(n,0,max-1)) { push(n); return 1; }
    printString("-RF#-");
    return 0;
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
                if (ir == 'U') { push((CELL)&user[0]); }
                if (ir == 'V') { push((CELL)&vars[0]); }
                return;
            };
            if (ir == 'C') { push(CELL_SZ); }
            if (ir == 'F') { push(NUM_FUNCS); }
            if (ir == 'H') { push((CELL)HERE); }
            if (ir == 'R') { push(NUM_REGS); }
            if (ir == 'U') { push(USER_SZ); }
            if (ir == 'V') { push(VARS_SZ); }
        RCASE 'K': dumpStack();
        RCASE 'S': if (*pc == 'R') { ++pc; vmInit(); }
        RCASE 'M': push(doMicros());
        RCASE 'T': push(doMillis());
        RCASE 'W': if (0 < TOS) { doDelay(TOS); } pop();
        RCASE 'R': if (!seed) { seed = getSeed(); }
                seed ^= (seed << 13);
                seed ^= (seed >> 17);
                seed ^= (seed << 5);
                TOS = (TOS) ? ABS(seed) % TOS : seed;
                return;
        default:
        pc = doCustom(ir, pc);
    }
}

addr run(addr start) {
    pc = start;
    isError = 0;
    rsp = lsp = locStart = 0;
next:
    if (isError) { return pc; }
    ir = *(pc++);
    switch (ir) {
        case 0:  return pc;
        NCASE ' ': while (*(pc) == ' ') { pc++; }
        NCASE '!': setCell((byte*)TOS, NOS); DROP2;
        NCASE '"': while (*(pc)!=ir) { printChar(*(pc++)); }; ++pc;
        NCASE '#': push(TOS);
        NCASE '$': t1 = TOS; TOS = NOS; NOS = t1;
        NCASE '%': push(NOS);
        // NCASE '&': /*FREE*/
        NCASE '\'': push(*(pc++));
        NCASE '(': if (!TOS) { skipTo(')', 0); } pop();
        NCASE ')': /* nothing to do here */
        NCASE '*': t1 = pop(); TOS *= t1;
        NCASE '+': t1 = pop(); TOS += t1;
        NCASE ',': printChar((char)pop());
        NCASE '-': t1 = pop(); TOS -= t1;
        NCASE '.': printStringF("%ld", (CELL)pop());
        NCASE '/': if (isOk(TOS, "-0div-")) { NOS /= TOS; pop(); }
        NCASE '0': case '1': case '2': case '3': case '4': case  '5': case '6':
        case '7': case '8': case '9': push(ir-'0');
            while (BTWI(*pc, '0', '9')) { TOS = (TOS * 10) + *(pc++) - '0'; }
            NCASE ':': if (!getRFnum(NUM_FUNCS)) { NEXT; }
            while (*(pc) == ' ') { pc++; }
            func[pop()] = (addr)pc;
            skipTo(';', 0);
            HERE = (HERE < pc) ? pc : HERE;
        NCASE ';': pc = rpop(); locStart -= (9<locStart) ? 10 : 0; if (!pc) return pc;
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
        // NCASE 'E': /*FREE*/
        NCASE 'F': doFloat();
        NCASE 'G': if (TOS) { pc = (addr)TOS; } pop();
        // NCASE 'H': /*FREE*/
        NCASE 'I': push(L0);
        // NCASE 'J': /*FREE*/
        NCASE 'K': ir = *(pc++);
            if (ir == '?') { push(qkey()); }
            else if (ir == '@') { push(key()); }
        NCASE 'L': t1 = pop(); TOS = (TOS << t1);
        NCASE 'M': if (isOk(TOS, "-0div-")) { t1 = pop(); TOS %= t1; }
        NCASE 'N': printString("\r\n");
        // NCASE 'O': /*FREE*/
        NCASE 'P': ++TOS;
        // NCASE 'Q': /*FREE*/
        NCASE 'R': t1 = pop(); TOS = (TOS >> t1);
        NCASE 'S': if (isOk(TOS, "-0div-")) { t1 = TOS; TOS = NOS % t1; NOS /= t1; }
        // NCASE 'T': /*FREE*/
        NCASE 'U': TOS += (CELL)&user[0];
        NCASE 'V': TOS += (CELL)&vars[0];
        // NCASE 'W': /*FREE*/
        NCASE 'X': if (TOS) { rpush(pc); pc = (addr)TOS; } pop();
        // NCASE 'Y': /*FREE*/
        NCASE 'Z': printString((char*)pop());
        NCASE '[': doFor();
        NCASE '\\': pop();
        NCASE ']': if ((++L0)<L1) { pc=(addr)L2; NEXT; } /* fall through */
        case '^': lsp = 3; if (lsp<0) { lsp=0; }
        NCASE '_': TOS = (TOS) ? 0 : 1;
        NCASE '`': push(TOS);
            while ((*pc) && (*pc != ir)) { *(AOS++) = *(pc++); }
            *(AOS++) = 0; pc++;
        // NCASE 'a': /*FREE*/
        NCASE 'b': ir = *(pc++);                    // BIT operations
            if (ir == '&') { NOS &= TOS; pop(); }           // AND
            else if (ir == '|') { NOS |= TOS; pop(); }      // OR
            else if (ir == '^') { NOS ^= TOS; pop(); }      // XOR
            else if (ir == '~') { TOS = ~TOS; }             // NOT (COMPLEMENT)
            else if (ir == 'L') { blockLoad(pop()); }       // Block Load
            else if (ir == 'A') { loadAbort(); }            // Block Load Abort
            else if (ir == 'E') { doEditor(); }             // Block Edit
        NCASE 'c': if (getRFnum(NUM_FUNCS) && func[TOS]) {
            if (*pc != ';') { rpush(pc); locStart += 10; }
            pc = func[TOS];
        } pop();
        NCASE 'd': if (isLocal(*pc)) { --locals[*(pc++) - '0' + locStart]; }
                  else { if (getRFnum(NUM_REGS)) { --reg[pop()]; } }
        // NCASE 'e': /*FREE*/
        NCASE 'f': ir = *(pc++);
            if (ir == 'O') { fileOpen(); }
            else if (ir == 'C') { fileClose(); }
            else if (ir == 'R') { fileRead(); }
            else if (ir == 'W') { fileWrite(); }
            else if (ir == 'S') { codeSave(user, HERE); }
            else if (ir == 'L') { HERE = codeLoad(user, HERE); }
        // NCASE 'g': /*FREE*/
        NCASE 'h': push(0); while (1) {
                t1 = BTWI(*pc,'0','9') ? (*pc)-'0' : -1;
                t1 = BTWI(*pc,'A','F') ? (*pc)-'A'+10 : t1;
                if (t1 < 0) { NEXT; }
                TOS = (TOS * 16) + t1; ++pc;
            }
        NCASE 'i': if (isLocal(*pc)) { ++locals[*(pc++) - '0' + locStart]; }
                  else { if (getRFnum(NUM_REGS)) { ++reg[pop()]; } }
        // NCASE 'j': /*FREE*/
        // NCASE 'k': /*FREE*/
        // NCASE 'l': /*FREE*/
        // NCASE 'm': /*FREE*/
        // NCASE 'n': /*FREE*/
        // NCASE 'o': /*FREE*/
        NCASE 'p': L0 += pop();
        // NCASE 'q': /*FREE*/
        NCASE 'r': if (isLocal(*pc)) { push(locals[*(pc++)-'0'+locStart]); }
                  else { if (getRFnum(NUM_REGS)) { TOS = reg[TOS]; } }
        NCASE 's': if (isLocal(*pc)) { locals[*(pc++)-'0'+locStart] = pop(); }
                  else { if (getRFnum(NUM_REGS)) { reg[TOS] = NOS; DROP2; } }
        // NCASE 't': /*FREE*/
        // NCASE 'u': /*FREE*/
        // NCASE 'v': /*FREE*/
        // NCASE 'w': /*FREE*/
        NCASE 'x': doExt();
        // NCASE 'y':
        // NCASE 'z':
        NCASE '{': if (!TOS) { skipTo('}', 0); NEXT; }
                lsp += ((lsp+2) < LSTACK_SZ) ? 3 : 0;
                L0 = 0; L1 = 1; L2 = (CELL)pc;
        // NCASE '|':  /*FREE*/
        NCASE '}': if (TOS) { pc=(addr)L2; } else { pop(); lsp = (2<lsp) ? lsp-3 : 0; }
        NCASE '~': TOS = -TOS;
        NEXT;
        default: printStringF("-[ir:%d?]-", ir); NEXT;
    }
    return pc;
}
