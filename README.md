# r4 - A stack-based VM/CPU with human-readable machine language

r4 is an simple, minimal, and interactive environment where the source code IS the machine code. There is no compilation in r4.

## What is r4?

r4 is a stack-based, RPN, virtual CPU/VM that supports many registers, functions, locals, and any amount of user ram.

A register (a built-in variable) is identified by up to 3 upper-case characters, so there is a maximum of (26x26x26) = 17576 registers available. They can be retrieved, set, increment, or decremented in a single operation (r,s,i,d).

Similarly, a function is also identified by up to 3 upper-case characters, so there is a maximum of (26x26x26) = 17576 functions available. A function is defined in a Forth-like style, using ':', and you call it using 'c'. For example:

- 0(CPY (N F T--): copy N bytes from F to T)
- :CPY s2 s1 1[r1 C@ r2 C! i1 i2];
- 123 1000 2000 cCPY 0(copy 123 bytes from 1000 to 2000)

The number of registers, functions, and user memory can be scaled as necessary to fit into a system of any size. For example, on an ESP8266 board, a typical configuration might be 676 (26*26) registers and functions, and 24K of user ram. In such a system, the register names would be in the range of [AA..ZZ], and function names would be in the range of [AA..ZZ]. On a Arduino Leonardo, you might configure the system to have 13 registers, 26 functions, and 1K user ram. On a RPI Pico, you can have 676 registers and functions, with 64K ram.

- Example 1: "Hello World!" - the standard "hello world" program.
- Example 2: 1 sA 2 sB 3 sC rA rB rC ++ . -would print 6.
- Example 4: 32 126\[13,10,rI#." - ",\] - would print the ASCII table
- Example 3: The typical Arduino "blink" program is a one-liner, except this version stops when a key is pressed:

    1000 sS 13 xPO 1{\ 0 1[rI 13 xPWD rS xW] K? 0=} K@ \

Examples for r4 are here: https://github.com/CCurl/r4/blob/main/examples.txt

## Why did I create r4?

There are multiple goals for r4:

1. Freedom from the need for a multiple gigabyte tool chain and the edit/compile/run/debug loop for developing everyday programs. Of course, you need one of these monsters to build and deploy r4, but at least after that, you are free of them.

2. Many programming environments use tokens and a large SWITCH statement in a loop to execute the user's program. In those systems, the machine code (aka - byte-code ... the cases in the SWITCH statement) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when looking at the code that is actually being executed. Additionally there is a compiler that is needed in order to work in that environment. In these enviromnents, there is a steep learning curve; the programmer needs to learn: (1) the user environment, (2) the hundreds or thousands of user functions (or "words" in Forth), and (3) how they work together. I wanted to avoid as much as that as possible, and have only one thing to learn: the machine code.

3. A desire for a simple, minimal, and interactive programming environment that was easy to modify and enhance.

4. An environment that could be deployed to many different types of development boards via the Arduino IDE.

5. To be able to use the same environment on my personal computer as well as development boards.

6. Short commands so that there was not a lot of typing needed.

## The implementation of r4

- The entire system is implemented in a few files, primarily: config.h, r4.h, r4.cpp, pc-main.cpp, and r4.ino.
  - There are a few additional files to support optional functionality (e.g.-WiFi and File access).
- The same code runs on Windows, Linux, and multiple development boards (via the Arduino IDE).
- See the file "config.h" for system configuration settings.

## Locals

r4 allocates 10 local variables (0-9) per function call. They are referred to, and have the same operations, like registers (r,s,i,d). For example, i4 increments local #4.

## WiFi support

Some boards, for example the ESP8266, support WiFi. For those boards, the __WIFI__ directive can be #defined to enable the boards WiFi.
Note that those boards usually also have watchdogs that need to be enabled via the __WATCHDOG__ #define.

## LittleFS support

Some development boards support LittleFS. For those boards, the __LITTLEFS__ directive can be #defined to save and load the defined code to the board, so that any user-defined words can be reloaded across boots.

## A simple block editor

r4 includes a simple block editor. Many thanks to Alain Theroux for his inspiration and help.

## Building r4

- The target machine/environment is controlled by the #defined in the file "config.h"
- For Windows, I use Microsoft's Visual Studio (Community edition). I use the x86 configuration.
- For Development boards, I use the Arduino IDE. See the file "config.h" for board-specific settings.
- For Linux systems, I use vi and clang. See the "make" script for more info.
- I do not have an Apple system, so I haven't tried to build r4 for that environment.
- However, being such a simple and minimal C program, it should not be difficult to port r4 to any environment.

##  r4 Reference

### ARITHMETIC
|OP |Stack |Description|
|:-- |:-- |:--|
| + |  (a b--n)   |n: a+b - addition|
| - |  (a b--n)   |n: a-b - subtraction|
| * |  (a b--n)   |n: a*b - multiplication|
| / |  (a b--q)   |q: a/b - division|
| M |  (a b--r)   |r: a%b - modulo|
| S |  (a b--q r) |q: div(a,b), r: modulo(a,b)  (SLASH-MOD)|

### FLOATING POINT
|OP |Stack |Description|
|:-- |:-- |:--|
| F< | (n--)     |Float: data -> float stack|
| F> | (n--)     |Float: float -> data stack|
| F+ | (a b--n)  |Float: add|
| F- | (a b--n)  |Float: subtract|
| F* | (a b--n)  |Float: multiply|
| F/ | (a b--n)  |Float: divide|
| F. | (n--)     |Float: print top of fload stack|


### BIT MANIPULATION
|OP |Stack |Description|
|:-- |:-- |:--|
| b& | (a b--n)   |n: a and b|
| b| | (a b--n)   |n: a or b|
| b^ | (a b--n)   |n: a xor b|
| b~ | (a--b)     |b: not a      (e.g - 1011 => 0100)|
| L  | (a n--b)   |b: a << n     (Left-Shift)|
| R  | (a n--b)   |b: a >> n     (Right-Shift)|


### STACK
|OP |Stack |Description|
|:-- |:-- |:--|
| #  | (a--a a)       |Duplicate TOS             (DUP)|
| \  | (a b--a)       |Drop TOS                  (DROP)|
| $  | (a b--b a)     |Swap top 2 stack items    (SWAP)|
| %  | (a b--a b a)   |Push 2nd                  (OVER)|
| ~  | (a--b)         |b: -a                     (Negate)|
| D  | (a--b)         |b: a-1                    (Decrement TOS)|
| I  | (a--b)         |b: a+1                    (Increment TOS)|
| A  | (a--b)         |b: abs(a)                 (Absolute value)|


### MEMORY
|OP |Stack |Description|
|:-- |:-- |:--|
| @   | (a--n)      |Fetch CELL n from address a|
| !   | (n a--)     |Store CELL n  to  address a|
| C@  | (a--n)      |Fetch BYTE n from address a|
| C!  | (n a--)     |Store BYTE n  to  address a|


### REGISTERS
#### NOTES:
- A register reference is 1-3 UPPERCASE characters [A..ZZZ]
- The number of registers is controlled by the NUM_REGS #define in "config.h"
- Register A is the same as register AAA, B <-> AAB, Z <-> AAZ
- r"TEST" will push the value of register AAA and then print TEST

|OP |Stack |Description|
|:-- |:-- |:--|
| rABC  | (--v)      |v: value of register ABC.|
| sABC  | (v--)      |v: store v to register ABC.|
| iABC  | (--)       |Increment register ABC.|
| dABC  | (--)       |Decrement register ABC.|


### LOCALS
#### NOTES:
- On each function call, 10 locals [r0..r9] are allocated.
- Locals are NOT initialized.

|OP |Stack |Description|
|:-- |:-- |:--|
| rN  | (--v)  |v: value of local #N.|
| sN  | (v--)  |v: store v to local #N.|
| iN  | (--)   |Increment local N.|
| dN  | (--)   |Decrement local n.|


### FUNCTIONS
#### NOTES:
- A function reference is 1-3 UPPERCASE characters [A..ZZZ]
- The number of functions is controlled by the NUM_FUNCS #define in "config.h"
- Function A is the same as function AAA, B <-> AAB, Z <-> AAZ
- :"TEST"; will define function #0 (AAA).
- Returning while inside of a loop is not supported; behavior will be undefined. 
  - Use '^' to exit the loop first.

|OP |Stack |Description|
|:-- |:-- |:--|
| :ABC  | (--)   |Define function ABC. Copy chars to (HERE++) until closing ';'.
| cABC  | (--)   |Call function ABC. Handles "tail call optimization"
| ;     | (--)   |Return: PC = rpop()


### INPUT/OUTPUT
|OP |Stack |Description|
|:-- |:-- |:--|
| .     | (N--)    |Output N as a decimal number
| ,     | (N--)    |Output N as a character (EMIT)
| "     | (--)     |Output characters until the next '"'
| B     | (--)     |Output a single SPACE (32,)
| N     | (--)     |Output a single NEWLINE (13,10,)
| K?    | (--f)    |f: non-zero if char is ready to be read, else 0.
| K@    | (--n)    |n: Key char, wait if no char is available.
| 0..9  | (--N)    |Scan DECIMAL number N until non digit
|       |          |- to specify multiple values, separate them by space (4711 3333)
|       |          |- to enter a negative number, use "negate" (eg - 490~)
|hXXX   | (--N)    |Scan HEX number N until non hex-digit ([0-9,A-F] only ... NOT [a-f])
| 'C    | (n)      |n: the ASCII value of C
| `x    | (a--a b) |Copy following chars until closing '`' to (a++).
|       |          |a: address, b next byte after trailing NULL.


### LOGICAL/CONDITIONS/FLOW CONTROL
|OP |Stack |Description|
|:-- |:-- |:--|
| <  | (a b--f)    |f: (a < b) ? 1 : 0;
| =  | (a b--f)    |f: (a = b) ? 1 : 0;
| >  | (a b--f)    |f: (a > b) ? 1 : 0;
| _  | (x--f)      |f: (x = 0) ? 1 : 0; (logical NOT)
| (  | (f--)       |if (f != 0), execute code in '()', else skip to matching ')'
| X  | (a--)       |if (a != 0), execute/call function at address a
| G  | (a--)       |if (a != 0), go/jump) to function at address a


### FOR LOOPS
|OP |Stack |Description|
|:-- |:-- |:--|
| [   | (F T--)   |FOR: start a For/Next loop. if (T < F), swap T and F
| rI  | (--n)     |n: the index of the current FOR loop
| sI  | (n--)     |n: a new value for the index of the current FOR loop
| ^   | (--)      |EXIT for loop
| ]   | (--)      |NEXT: increment index (I) and loop if (I <= T)


### WHILE LOOPS
|OP |Stack |Description|
|:-- |:-- |:--|
| {  | (f--f)      |BEGIN: if (f == 0) skip to matching '}'
| ^  | (--)        |EXIT while loop
| }  | (f--f?)     |WHILE: if (f != 0) jump to matching '{', else drop f and continue


### FILES
|OP |Stack |Description|
|:-- |:-- |:--|
| fO  | (nm md--fh)  |FILE: Open, nm: name, md: mode, fh: fileHandle
| fC  | (fh--)       |FILE: Close, fh: fileHandle
| fD  | (nm--)       |FILE: Delete
| fR  | (fh--c n)    |FILE: Read, fh: fileHandle, c: char, n: num
| fW  | (c fh--n)    |FILE: Write, fh: fileHandle, c: char, n: num
| fS  | (--)         |FILE: Save Code
| fL  | (--)         |FILE: Load Code
| bL  | (n--)        |BLOCK: Load code from block file (Block-nnn.r4). This can be nested.


### OTHER
|OP |Stack |Description|
|:-- |:-- |:--|
| xIAF  | (--a)     |INFO: Address where the function vectors begin
| xIAH  | (--a)     |INFO: Address of the HERE variable
| xIAR  | (--a)     |INFO: Address where the registers begin
| xIAU  | (--a)     |INFO: Address there the user area begins
| xIC   | (--n)     |INFO: CELL size
| xIF   | (--n)     |INFO: Number of functions
| xIH   | (--a)     |INFO: HERE
| xIR   | (--n)     |INFO: Number of registers
| xIU   | (--n)     |INFO: Size of user area
| xLA   | (--)      |PC: Load Abort: to stop loading a block (eg - if the block has already been loaded)
| xPI   | (p--)     |Arduino: pin input  (pinMode(p, INPUT))
| xPU   | (p--)     |Arduino: pin pullup (pinMode(p, INPUT_PULLUP))
| xPO   | (p--)     |Arduino: pin output (pinMode(p, OUTPUT)
| xPRA  | (p--n)    |Arduino: pin read analog  (n = analogRead(p))
| xPRD  | (p--n)    |Arduino: pin read digital (n = digitalRead(p))
| xPWA  | (n p--)   |Arduino: pin write analog  (analogWrite(p, n))
| xPWD  | (n p--)   |Arduino: pin write digital (digitalWrite(p, n))
| xSR   | (--)      |R4 System Reset
| xT    | (--n)     |Time (Arduino: millis(), Windows: GetTickCount())
| xN    | (--n)     |Time (Arduino: micros(), Windows: N/A)
| xW    | (n--)     |Wait (Arduino: delay(),  Windows: Sleep())
| xR    | (n--r)    |r: a random number between 0 and n
|       |           |NOTE: when n=0, r is the entire 32-bit number
| xQ    | (--)      |PC: Exit R4
