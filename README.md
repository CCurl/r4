# r4 - A stack-based VM/CPU with human-readable machine language

r4 is an interactive environment where the source code IS the machine code. There is no compilation in r4.

## What is r4?

r4 is a stack-based, RPN, virtual CPU/VM that supports many registers, functions, locals, floating point, and any amount of RAM.

The number of registers, functions, and memory are configurable and can be scaled as necessary to fit into a system of any size.

For example, one might configure things like this:
- On a Leonardo,  32 registers,  32 functions,  1K of USER RAM, no  VARS RAM.
- On an ESP8266, 512 registers, 512 functions, 12K of USER RAM, 12K VARS RAM.
- On a RPI Pico, 16K registers, 16K functions, 64K of USER RAM, 64K VARS RAM.
- On a PC,       64K registers, 64K functions, 96K of USER RAM,  2M VARS RAM.

### Registers
- A register (a built-in variable) is identified by a sequence of UPPERCASE characters.
- A register can be retrieved, set, incremented, or decremented in a single operation (r,s,i,d).

### Registers and Functions
Registers and functions are identified by a sequence of UPPERCASE characters.

r4 hashes the name and uses the hashed value as the index into the array of values or addresses.

This is very fast, but poses some limitations:
- The number of registers and functions need to be powers of 2.
- r4 does NOT detect hash collisions as it does not keep key values.
  - My tests have indicated that for a large enough number of buckets, collisions are not common.
  - ':' prints "-redef-f[hash]-" when a function is redefined (a possible collision).
  - '&' prints "-redef-r[hash]-" when a register is redefined (a possible collision).
  - Use xh[NAME] to see info about [NAME].

Here is the hashing function (the djb2 hash function):
```
int getRFnum(int max) {
    UCELL hash = 5381;
    while (BTWI(*pc, 'A', 'Z')) {
        hash = (hash * 33) + *(pc++);
    }
    return hash & max;
}
```

Functions are defined in a Forth-like style, using ':', and you call them using the 'c' opcode. For example:

- 0(COPY (N F T--): copy N bytes from F to T)
- :COPY s2 s1 0[r1 C@ r2 C! i1 i2];
- 123 rF rT cCOPY 0(copy 123 bytes from rF to rT)

- Example 1: "Hello World!" - the standard "hello world" program.
- Example 2: :MIN %%>($)\; :MAX %%<($)\;
- Example 3: :BTW s3 s2 s1 r1 r2 > r1 r3 < b&;
- Example 4: 32 126\[13,10,I#." - ",\] - would print the ASCII table
- Example 5: The Arduino "blink" program is a one-liner, except this version stops when a key is pressed:

    1000 sS 13 xPO 1{\ 0 1[I 13 xPWD rS xW] K? 0=} K@ \

Examples for r4 are here: https://github.com/CCurl/r4/blob/main/examples.txt

## Why did I create r4?

There are multiple goals for r4:

- Freedom from the need for a multiple gigabyte tool chain and the edit/compile/run/debug loop for developing everyday programs. Of course, you need one of these monsters to build r4, but at least after that, you are free of them.
- Many programming environments use tokens and a large SWITCH statement in a loop to execute the user's program. In those systems, the machine code (aka - the byte-code ... the cases in the SWITCH statement) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when inspecting the code that is actually being executed. In these enviromnents, there is a steep learning curve; the programmer needs to learn: (1) the user environment, (2) the hundreds or thousands of user functions (or "words" in Forth), and (3) how they work together. I wanted to avoid as much of that as possible, and have only one thing to learn: the machine code.
- A minimal implementation that is "intuitively obvious upon casual inspection" and easy to extend as necessary.
- An interactive programming environment.
- The ability to use the same environment on my personal computer as well as development boards.
- An environment that can be deployed to many different types of development boards via the Arduino IDE.
- Short commands so that there is not a lot of typing needed.

## The implementation of r4

- The entire system is implemented in a few files, primarily: config.h, r4.h, r4.cpp, pc-main.cpp, and r4.ino.
  - There are a few additional files to support optional functionality (e.g.-WiFi and File access).
- The same code runs on Windows, Linux, and multiple development boards (via the Arduino IDE).
- See the file "config.h" for system configuration settings.

## Memory areas
There are 2 memory areas in r4:
- USER: r4 function code goes in this area. Default size is 128K.
- VARS: use with 'V' opcode. Default size is 256K.
- The USER area can also be used for data. See the 'U' opcode.
- The VARS area is not used by the r4 system at all, 100% free.

## Locals

r4 allocates 10 local variables (0-9) per function call. They are referred to like registers and have the same operations (r,s,i,d). For example, i4 increments local #4.

## WiFi support

Some boards, (eg - the ESP8266), support WiFi. For those boards, the __WIFI__ directive can be #defined to enable the boards WiFi. This adds additional opcodes.

Note that those boards usually also have watchdogs that need to be enabled via the __WATCHDOG__ #define.

## LittleFS support

Some development boards support LittleFS. For those boards, the __LITTLEFS__ directive can be #defined persist data to the board, including supporting automatically loading blocks on boot.

## A simple block editor

r4 includes a simple block editor. Many thanks to Alain Theroux for his inspiration.

## Building r4

- The target machine/environment is controlled by the #defines in the file "config.h"
- For Windows, There is a Visual Studio solution. Use the x86 configuration (32-bit).
- For Development boards, I use the Arduino IDE. See the file "config.h" for board-specific settings.
- For Linux systems, there is a makefile. Default is 64-bit, or you can change it to 32-bit.
- I do not have an Apple system, so I haven't tried to build r4 for that environment.
  - However, being such a simple and minimal C program, it should not be difficult to port r4 to any environment.

##  r4 Reference

### INTEGER OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| +  |  (a b--n)   |n: a+b - addition
| -  |  (a b--n)   |n: a-b - subtraction
| *  |  (a b--n)   |n: a*b - multiplication
| /  |  (a b--q)   |q: a/b - division
| M  |  (a b--r)   |r: a%b - modulo
| S  |  (a b--q r) |q: div(a,b), r: modulo(a,b)  (SLASH-MOD)


### FLOATING POINT OPERATIONS
#### NOTES:
- r4 uses double wide (64-bit) floating point numbers

| OP |Stack |Description|
|:-- |:--   |:--|
| FF | (i--f)    |Convert TOS from integer to float
| FI | (f--i)    |Convert TOS from float to integer
| F+ | (a b--n)  |Float: add
| F- | (a b--n)  |Float: subtract
| F* | (a b--n)  |Float: multiply
| F/ | (a b--n)  |Float: divide
| F< | (a b--f)  |f: if (a < b), else 0
| F= | (a b--f)  |f: if (a = b), else 0
| F> | (a b--f)  |f: if (a > b), else 0
| F. | (n--)     |Float: print top of fload stack
| F_ | (a--b)    |b: -a


### BIT MANIPULATION OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| b& | (a b--n)   |n: a and b
| b| | (a b--n)   |n: a or b
| b^ | (a b--n)   |n: a xor b
| b~ | (a--b)     |b: not a      (e.g - 1011 => 0100)
| L  | (a n--b)   |b: a << n     (Left-Shift)
| R  | (a n--b)   |b: a >> n     (Right-Shift)


### STACK OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| #  | (a--a a)       |Duplicate TOS (DUP)
| \  | (a b--a)       |Drop TOS (DROP)
| $  | (a b--b a)     |Swap top 2 stack items (SWAP)
| %  | (a b--a b a)   |Push 2nd (OVER)
| _  | (a--b)         |b: -a (NEGATE)
| D  | (a--b)         |b: a-1 (1-)
| P  | (a--b)         |b: a+1 (1+)
| A  | (a--b)         |b: absolute value of a (ABS)
| xK | (--)           |Display the stack (.S)


### MEMORY OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| @  | (a--n)      |Fetch CELL n from address a
| !  | (n a--)     |Store CELL n  to  address a
| C@ | (a--n)      |Fetch BYTE n from address a
| C! | (n a--)     |Store BYTE n  to  address a
| U  | (n--a)      |a: address of byte n in the USER area.
| V  | (n--a)      |a: address of byte n in the VARS area.


### REGISTERS OPERATIONS
#### NOTES:
- A register reference is consecutive UPPERCASE characters.
- The number of registers is controlled by the NUM_REGS #define in "config.h"

| OP |Stack |Description|
|:-- |:--   |:--|
| &ABC | (N--)  |Define register rABC with initial value of N.
|      |        |If register rABC has a value <> 0, print "-redef-r[hash]-".
| rABC | (--v)  |v: value of register ABC.
| sABC | (v--)  |v: store v to register ABC.
| iABC | (--)   |Increment register ABC.
| dABC | (--)   |Decrement register ABC.


### LOCALS OPERATIONS
#### NOTES:
- On each function call, 10 locals [r0..r9] are allocated.
- They are de-allocated when ';' executes.
- Locals are NOT initialized.

| OP |Stack |Description|
|:-- |:--   |:--|
| rN | (--v)  |v: value of local #N.|
| sN | (v--)  |v: store v to local #N.|
| iN | (--)   |Increment local N.|
| dN | (--)   |Decrement local n.|


### FUNCTIONS OPERATIONS
#### NOTES:
- A function reference is any number of consecutive UPPERCASE characters.
- The number of functions is controlled by the NUM_FUNCS #define in "config.h"
- Returning while inside of a loop will eventually cause a problem.
  - Use '^' to unwind the loop stack first.

| OP |Stack |Description|
|:-- |:--   |:--|
| :ABC  | (--)   |Define function ABC. Copy chars to (HERE++) until closing ';'.
|       |        |If function ABC has a value <> 0, print "-redef-f[hash]-".
| cABC  | (--)   |Call function ABC. Handles "tail call optimization".
| ;     | (--)   |Return: PC = rpop()

 
### INPUT/OUTPUT OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| .     | (N--)    |Output N as a decimal number.
| ,     | (N--)    |Output N as a character (EMIT)
| "str" | (--)     |Output characters until the next '"'
| B     | (--)     |Output a single SPACE (32,)
| N     | (--)     |Output a single NEWLINE (13,10,)
| K?    | (--f)    |f: non-zero if char is ready to be read, else 0.
| K@    | (--n)    |n: Key char, wait if no char is available.
| <N>   | (--N)    |Scan DECIMAL number N until non digit
| <N.N> | (--F)    |- Use NNN.NNN (eg - 3.14) to enter a floating point number F
|       |          |- to specify multiple values, separate them by space (4711 3333)
|       |          |- to enter a negative number, use "negate" (eg - 490_)
|hXXX   | (--N)    |Scan HEX number N until non hex-digit ([0-9,A-F] only ... NOT [a-f])
| 'C    | (--n)    |n: the ASCII value of C
| `x`   | (a--a b) |Copy following chars until closing '`' to (a++).
|       |          |- a: address, b next byte after trailing NULL.


### LOGICAL/CONDITIONS/FLOW CONTROL OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| <  | (a b--f)    |f: if (a < b) then 1, else 0;
| =  | (a b--f)    |f: if (a = b) then 1, else 0;
| >  | (a b--f)    |f: if (a > b) then 1, else 0;
| ~  | (x--f)      |f: if (x = 0) then 1, else 0; (logical NOT)
| (  | (f--)       |if (f != 0), execute code in '()', else skip to matching ')'
| X  | (a--)       |if (a != 0), execute/call function at address a
| G  | (a--)       |if (a != 0), go/jump to function at address a


### FOR/NEXT LOOPING OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| [  | (F T--)   |FOR: start a For/Next loop. if (T < F), swap T and F
| I  | (--i)     |i: the index of the current FOR loop
| p  | (i--)     |i: number to add to "I"
| ^  | (--)      |un-loop, use with ';'. Example: rSrK>(^;)
| ]  | (--)      |NEXT: increment index (I) and loop if (I < T)


### BEGIN/WHILE LOOPING OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| {  | (f--f)      |BEGIN: if (f == 0) skip to matching '}'
| ^  | (--)        |un-loop, used with ';'. Example: rX0=(^;)
| }  | (f--f?)     |WHILE: if (f != 0) jump to matching '{', else drop f and continue


### FILE OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| fO  | (nm md--fh)  |FILE: Open, nm: name, md: mode, fh: fileHandle
| fC  | (fh--)       |FILE: Close, fh: fileHandle
| fD  | (nm--)       |FILE: Delete
| fR  | (fh--c n)    |FILE: Read, fh: fileHandle, c: char, n: num
| fW  | (c fh--n)    |FILE: Write, fh: fileHandle, c: char, n: num
| fS  | (--)         |FILE: Save Code
| fL  | (--)         |FILE: Load Code


### BLOCK OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| bL  | (n--)        |BLOCK: Load code from block file (block-nnn.r4). This can be nested.
| bA  | (--)         |BLOCK: Load Abort - stop loading the current block (eg - if already loaded)
| bE  | (n--)        |BLOCK: Edit block N (file name is block-nnn.r4)


### OTHER OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| xIAF  | (--a)     |INFO: Address where the function vectors begin
| xIAH  | (--a)     |INFO: Address of the HERE variable
| xIAR  | (--a)     |INFO: Address where the registers begin
| xIAU  | (--a)     |INFO: Address there the USER area begins
| xIAV  | (--a)     |INFO: Address there the VARS area begins
| xIC   | (--n)     |INFO: CELL size
| xIF   | (--n)     |INFO: Number of functions (NUM_FUNCS)
| xIH   | (--a)     |INFO: HERE
| xIR   | (--n)     |INFO: Number of registers (NUM_REGS)
| xIU   | (--n)     |INFO: Size of USER area (USER_SZ)
| xIV   | (--n)     |INFO: Size of VARS area (VARS_SZ)
| xPI   | (p--)     |Arduino: pin input  (pinMode(p, INPUT))
| xPU   | (p--)     |Arduino: pin pullup (pinMode(p, INPUT_PULLUP))
| xPO   | (p--)     |Arduino: pin output (pinMode(p, OUTPUT)
| xPRA  | (p--n)    |Arduino: pin read analog  (n = analogRead(p))
| xPRD  | (p--n)    |Arduino: pin read digital (n = digitalRead(p))
| xPWA  | (n p--)   |Arduino: pin write analog  (analogWrite(p, n))
| xPWD  | (n p--)   |Arduino: pin write digital (digitalWrite(p, n))
| xs    | (a--)     |PC: call "system(a)"
| xSR   | (--)      |R4 System Reset
| xT    | (--n)     |Time in milliseconds (Arduino: millis(), Windows: GetTickCount())
| xh[S] | (--)      |Print hash, reg, and func value for [S] (eg - xhALLOT or xhVH)
| xM    | (--n)     |Time in microseconds (Arduino: micros())
| xW    | (n--)     |Wait (Arduino: delay(),  Windows: Sleep())
| xR    | (n--r)    |r: a pseudo-random number between 0 and n (uses XOR-shift)
|       |           |NOTE: when n=0, r is the entire CELL-sized number
| xQ    | (--)      |PC: Exit R4
