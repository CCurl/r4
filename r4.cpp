// R4 - A Minimal Interpreter

#include "r4.h"

byte ir, isBye = 0, isError = 0;
static char buf[96];
addr pc, HERE;
CELL n1, t1, seed = 0;
ushort dsp, rsp, lsp, locStart;
CELL   dstack[STK_SZ + 1];
addr   rstack[RSTK_SZ + 1];
LOOP_ENTRY_T lstack[LSTACK_SZ + 1];
addr   func[NUM_FUNCS];
CELL   reg[NUM_REGS];
byte   user[USER_SZ];
CELL locals[RSTK_SZ * 10];

void push(CELL v) { if (dsp < STK_SZ) { dstack[++dsp] = v; } }
CELL pop() { return (dsp) ? dstack[dsp--] : 0; }

inline void rpush(addr v) { if (rsp < RSTK_SZ) { rstack[++rsp] = v; } }
inline addr rpop() { return (rsp) ? rstack[rsp--] : 0; }

static float flstack[STK_SZ+1];
static int flsp = 0;

#define fT flstack[flsp]
#define fN flstack[flsp-1]
#define fDROP1 flpop()
#define fDROP2 flpop(); flpop();
static void flpush(float v) { if (flsp < STK_SZ) { flstack[++flsp] = v; } }
static float flpop() { return (flsp) ? flstack[flsp--] : 0; }

#define lAt() (&lstack[lsp])
inline LOOP_ENTRY_T* lpush() { if (lsp < STK_SZ) { ++lsp; } return lAt(); }
inline LOOP_ENTRY_T *ldrop() { if (0 < lsp) { --lsp; } return lAt(); }

void vmInit() {
    dsp = rsp = lsp = flsp = locStart = 0;
    for (int i = 0; i < NUM_REGS; i++) { reg[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { user[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { func[i] = 0; }
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
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

void dumpStack() {
    for (UCELL i = 1; i <= dsp; i++) {
        printStringF("%s%ld", (i > 1 ? " " : ""), (CELL)dstack[i]);
    }
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
    CELL t = (NOS < TOS) ? TOS : NOS;
    CELL f = (NOS < TOS) ? NOS : TOS;
    DROP2;
    LOOP_ENTRY_T *x = lpush();
    x->start = pc;
    INDEX = x->from = f;
    x->to = t;
    x->end = 0;
}

void doNext() {
    LOOP_ENTRY_T* x = lAt();
    x->from = ++INDEX;
    if (x->from <= x->to) {
        x->end = pc;
        pc = x->start;
    } else {
        INDEX = ldrop()->from;
    }
}

void loopExit() {
    if (!lsp) { isError = 1; return; }
    LOOP_ENTRY_T* x = lAt();
    ldrop();
    char c = ((x->from) || (x->to)) ? ']' : '}';
    if (c == '}') { pop(); }
    if (x->end) { pc = x->end; }
    else { 
        skipTo(c, 0);
    }
}

int isOk(int exp, const char* msg) {
    isError = (exp == 0); if (isError) { printString(msg); }
    return (isError == 0);
}

void doFloat() {
    switch (*(pc++)) {
    case '<': flpush((float)pop());                             return;
    case '>': push((CELL)flpop());                              return;
    case '+': fN += fT; fDROP1;                                 return;
    case '-': fN -= fT; fDROP1;                                 return;
    case '*': fN *= fT; fDROP1;                                 return;
    case '/': if (isOk(fT!=0, "-0div-")) { fN /= fT; fDROP1; }  return;
    case '.': printStringF("%g", flpop());                      return;
    default:
        isError = 1;
        printString("-flt?-");
    }
}

int getRFnum(int isReg) {
    push(0);
    while (isRegChar(*pc)) { TOS = (TOS * 26) + *(pc++)-'A'; }
    if ((isReg == 1) && (NUM_REGS <= TOS)) { isError = 1; printString("-#reg-"); }
    if ((isReg == 0) && (NUM_FUNCS <= TOS)) { isError = 1; printString("-#func-"); }
    if (isError) { pop(); }
    return isError == 0;
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
            return;
        };
        if (ir == 'C') { push(CELL_SZ); }
        if (ir == 'F') { push(NUM_FUNCS); }
        if (ir == 'H') { push((CELL)HERE); }
        if (ir == 'R') { push(NUM_REGS); }
        if (ir == 'U') { push(USER_SZ); }
        return;
    case 'E': doEditor();                            return;
    case 'S': if (*pc == 'R') { ++pc; vmInit(); }    return;
    case 'N': push(doMicros());                      return;
    case 'T': push(doMillis());                      return;
    case 'W': if (0 < TOS) { doDelay(TOS); } pop();  return;
    case 'R': if (!seed) { seed = getSeed(); }
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
    while (!isError && pc) {
        ir = *(pc++);
        switch (ir) {
        case 0:                                                    return pc;
        case ' ': while (*(pc) == ' ') { pc++; }                   break;  // 32
        case '!': setCell((byte*)TOS, NOS); DROP2;                 break;  // 33 STORE
        case '"': while (*(pc)!=ir) { printChar(*(pc++)); }; ++pc; break;  // 34 PRINT
        case '#': push(TOS);                                       break;  // 35 DUP
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                   break;  // 36 SWAP
        case '%': push(NOS);                                       break;  // 37 OVER
        case '&': /*FREE*/                                         break;  // 38
        case '\'': push(*(pc++));                                  break;  // 39 CHAR-LIT
        case '(': if (!TOS) { skipTo(')', 0); } pop();             break;  // 40 IF
        case ')': /* endIf() */                                    break;  // 41 ENDIF
        case '*': t1 = pop(); TOS *= t1;                           break;  // 42 MULTIPLY
        case '+': t1 = pop(); TOS += t1;                           break;  // 43 ADD
        case ',': printChar((char)pop());                          break;  // 44 EMIT
        case '-': t1 = pop(); TOS -= t1;                           break;  // 45 SUBTRACT
        case '.': printStringF("%ld", (CELL)pop());                break;  // 46 DOT
        case '/': if (isOk(TOS, "-0div-")) { NOS /= TOS; pop(); }  break;  // 47 DIVIDE
        case '0': case '1': case '2': case '3': case '4':                  // 48-57 NUMBER
        case '5': case '6': case '7': case '8': case '9':
            push(ir-'0');
            while (BTWI(*pc, '0', '9')) {
                TOS = (TOS * 10) + *(pc++) - '0';
            } break;
        case ':': if (getRFnum(0)) {                                       // 58 CREATE
                func[pop()] = pc;
                skipTo(';', 0);
                HERE = (HERE < pc) ? pc : HERE;
            } break;
        case ';': pc = rpop(); locStart -= (locStart) ? 10 : 0;    break;  // 59 RETURN
        case '<': t1 = pop(); TOS = (TOS <  t1) ? 1 : 0;           break;  // 60 LESS-THAN
        case '=': t1 = pop(); TOS = (TOS == t1) ? 1 : 0;           break;  // 61 EQUALS
        case '>': t1 = pop(); TOS = (TOS >  t1) ? 1 : 0;           break;  // 62 GREATER-THAN
        case '?': /*FREE*/                                         break;  // 63
        case '@': TOS = getCell((byte*)TOS);                       break;  // 64 FETCH
        case 'A': if (TOS < 0) { TOS = -TOS; }                     break;  // ABS
        case 'B': printChar(' ');                                  break;  // PRINT BLANK
        case 'C': ir = *(pc++);                                            // C@, C!
            if (ir == '@') { TOS = *(byte*)TOS; }
            if (ir == '!') { *(byte*)TOS = (byte)NOS; DROP2; }
            break;
        case 'D': --TOS;                                           break;  // DECREMENT
        case 'E': /*FREE*/                                         break;
        case 'F': doFloat();                                       break;  // FLOAT ops
        case 'G': if (TOS) { pc = (addr)TOS; } pop();              break;  // GOTO
        case 'H': /*FREE*/                                         break;
        case 'I': ++TOS;                                           break;  // INCREMENT
        case 'J': /*FREE*/                                         break;
        case 'K': ir = *(pc++);                                            // KEYBOARD ops
            if (ir == '?') { push(charAvailable()); }
            if (ir == '@') { push(getChar()); }
            break;
        case 'L': t1 = pop(); TOS = (TOS << t1);                   break;  // LEFT-SHIFT
        case 'M': if (isOk(TOS, "-0div-")) { t1 = pop(); TOS %= t1; }  break;  // MOD
        case 'N': printString("\r\n");                             break;  // NEW-LINE
        case 'O': /*FREE*/                                         break;
        case 'P': /*FREE*/                                         break;
        case 'Q': /*FREE*/                                         break;
        case 'R': t1 = pop(); TOS = (TOS >> t1);                   break;  // RIGHT-SHIFT
        case 'S': if (TOS) { t1 = TOS; TOS = NOS % t1; NOS /= t1; }                  // /MOD
                else { isError = 1; printString("-0div-"); }       break;
        case 'T': /*FREE*/                                         break;
        case 'U': /*FREE*/                                         break;
        case 'V': /*FREE*/                                         break;
        case 'W': /*FREE*/                                         break;
        case 'X': if (TOS) { rpush(pc); pc = (addr)TOS; } pop();   break;  // eXecute
        case 'Y': /*FREE*/                                         break;
        case 'Z': printString((char *)pop());                      break;  // ZPRINT
        case '[': doFor();                                         break;  // 91 FOR
        case '\\': pop();                                          break;  // 92 DROP
        case ']': doNext();                                        break;  // 93 NEXT
        case '^': loopExit();                                      break;  // 94 Exit LOOP
        case '_': TOS = (TOS) ? 0 : 1;                             break;  // 95 NOT (LOGICAL)
        case '`': push(TOS);                                                 // 96 ZCOPY
            while ((*pc) && (*pc != ir)) { *(A++) = *(pc++); }
            *(A++) = 0; pc++;
            break;
        case 'a': /*FREE*/                                         break;
        case 'b': ir = *(pc++);                                            // BIT operations
            if (ir == '&') { NOS &= TOS; pop(); }      // AND
            if (ir == '|') { NOS |= TOS; pop(); }      // OR
            if (ir == '^') { NOS ^= TOS; pop(); }      // XOR
            if (ir == '~') { TOS = ~TOS; }             // NOT (COMPLEMENT)
            if (ir == 'L') { blockLoad(pop()); }       // Block Load
            break;
        case 'c': if (getRFnum(0) && func[TOS]) {                            // FUNCTION CALL
            if (*pc != ';') { rpush(pc); locStart += 10; }
            pc = func[TOS];
        } pop(); break;
        case 'd': if (isLocal(*pc)) { --locals[*(pc++) - '0' + locStart]; }
                else { if (getRFnum(1)) { --reg[pop()]; } }        break;  // REG DECREMENT
        case 'e': /*FREE*/                                         break;
        case 'f': ir = *(pc++);
            if (ir == 'O') { fileOpen(); }
            if (ir == 'C') { fileClose(); }
            if (ir == 'R') { fileRead(); }
            if (ir == 'W') { fileWrite(); }
            if (ir == 'S') { codeSave(user, HERE); }
            if (ir == 'L') { HERE = codeLoad(user, HERE); }
            break;
        case 'g': /*FREE*/                                         break;
        case 'h': push(0); while (1) {                                     // HEX number
                t1 = BTWI(*pc,'0','9') ? (*pc)-'0' : -1;
                t1 = BTWI(*pc,'A','F') ? (*pc)-'A'+10 : t1;
                if (t1 < 0) { break; }
                TOS = (TOS * 16) + t1; ++pc;
            } break;
        case 'i': if (isLocal(*pc)) { ++locals[*(pc++) - '0' + locStart]; }
                else { if (getRFnum(1)) { ++reg[pop()]; } }        break;  // SET-REGISTER
        case 'j': /*FREE*/                                         break;
        case 'k': /*FREE*/                                         break;
        case 'l': /*FREE*/                                         break;
        case 'm': /*FREE*/                                         break;
        case 'n': /*FREE*/                                         break;
        case 'o': /*FREE*/                                         break;
        case 'p': /*FREE*/                                         break;
        case 'q': /*FREE*/                                         break;
        case 'r': if (isLocal(*pc)) { push(locals[*(pc++)-'0'+locStart]); } 
                else { if (getRFnum(1)) { TOS = reg[TOS]; } }      break;  // READ-REGISTER
        case 's': if (isLocal(*pc)) { locals[*(pc++)-'0'+locStart] = pop(); }
              else { if (getRFnum(1)) { reg[TOS] = NOS; DROP2; } } break;  // SET-REGISTER
        case 't': /*FREE*/                                         break;
        case 'u': /*FREE*/                                         break;
        case 'v': /*FREE*/                                         break;
        case 'w': /*FREE*/                                         break;
        case 'x': doExt();                                         break;  // EXTENDED ops
        case 'y': /*FREE*/                                         break;
        case 'z': /*FREE*/                                         break;
        case '{': { LOOP_ENTRY_T *x = lpush();                             // 123 BEGIN
                x->start = pc; x->end = 0; x->from = x->to = 0;
                if (!TOS) { skipTo('}', 0); }
            } break;
        case '|':  /*FREE*/                                        break;  // 124
        case '}': if (!TOS) { ldrop(); pop(); }                            // 125 WHILE
            else { lAt()->end = pc; pc = lAt()->start; }
            break;
        case '~': TOS = -TOS;                                      break;  // 126 NEGATE
        }
    }
    return pc;
}
