# r4 - A stack-based VM/CPU with a human-readable machine language
r4 is an interactive environment where the source code IS the machine code. There is no compilation in r4.

## What is r4?
r4 is a stack-based, RPN, virtual CPU/VM that supports many registers, functions, locals, floating point, and any amount of RAM.

The number of registers, functions, and memory are configurable and can be scaled as necessary to fit into a system of any size.

For example, one might configure things like this:
- On a Leonardo,  16 registers,  32 functions,   1K of CODE RAM,  no  VARS RAM.
- On an ESP8266, 512 registers, 512 functions,  12K of CODE RAM,  12K VARS RAM.
- On a RPI Pico, 16K registers, 16K functions,  64K of CODE RAM,  64K VARS RAM.
- On a PC,       64K registers, 64K functions, 128K of CODE RAM, 256K VARS RAM.

## Why did I create r4?
There are multiple reasons for creating r4, including:

- Freedom from the need for a multiple gigabyte tool chain and the edit/compile/run/debug loop for developing everyday programs. Of course, you need one of these monsters to build r4, but at least after that, you are free of them.
- Many interpreters use tokens and a large SWITCH statement in a loop to execute the program. In those systems, there needs to be a compiler to parse the input text and generate the machine code that the VM executes. Additionally, the VM's opcodes (the cases in the SWITCH statement) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when inspecting the code that is actually being executed. I wanted to avoid as much of that as possible, and have only one thing to learn: the VM's opcodes.
- A minimal implementation that is "intuitively obvious upon casual inspection" and easy to extend as necessary.
- An interactive programming environment.
- The ability to use the same environment on my personal computer as well as development boards.
- An environment that can be deployed to many different types of development boards via the Arduino IDE.
- Short commands so that there is not a lot of typing needed.

## Registers
A register name is of the form `[A-Z][A-Z0-9]*` (e.g. - REG12).
- They can be retrieved, set, incremented, or decremented in a single operation (r[REG],s[REG],i[REG],d[REG]).
- Example: - `12 sTMP1 34 sTMP2 rTMP1 rTMP2 + .` will print `46`.

## Temporary registers (aka - LOCALS)
A reference to a temporary register is of the form `[rsid][0-9]` (e.g. r6).
- They are allocated and freed in groups of 10.
- Working with these are very fast, as the name does not need to be hashed.
- `T+` allocates 10 new temporary registers (r0-r9).
- `T-` frees the most recently allocated set.
- They have the same operations [rsid]. For example: `i4` increments register #4.
- They are used like other registers. For example: `r7` pushes the value of register #7.
- They can be used as local variables inside a function (using T+ and T-), or across functions.

## Functions
A function name is of the form `[A-Z][A-Z0-9]*` (e.g. - BTW10AND20).

Functions are defined in a Forth-like style, using ':', and are called using the 'c' opcode.

## For example:
```
0(COPY (F T N--): copy N bytes from F to T)
:COPY T+ s3 s2 s1 r3 0[r1 C@ r2 C! i1 i2] T-;
rFROM rTO rNUMBER cCOPY
```
## Some more Examples

| Code | Description |
|:--|:--|
| "Hello World!"                             | The standard "hello world" program.
| :MIN %%>($)\\; :MAX %%<(<SPAN>$</SPAN>)\\; | Define functions MIN and MAX
| :BTW T+ s3 s2 s1 r1 r2 > r1 r3 < b& T-;    | Using temporary registers
| :BTW10AND20 9 21 cBTW;                     | Define a function with an alphanumeric name
| 32 127[I###"%n[%c] - %d, %x, %b"]          | Loop to print the ASCII table

The Arduino "blink" program is a one-liner, except this version stops when a key is pressed:

`1000 sDELAY 13 sLED rLED xPO 1{\ 0 2[I rLED xPWD rDELAY xW] K? ~} K@ \`

  Explanation of the blink program:
  | Code           | Description|
  | :--            | :--|
  | 1000 sDELAY    | Set register DELAY to 1000
  | 13 sLED        | Set register LED to 13, the typical port number of the built-in LED
  | rLED xPO       | Push register LED (13) on the stack and open that port
  | 1{             | Begin a WHILE loop; DROP control value
  | \\ 0 2 [       | Begin a FOR loop from 0 to 2
  | I rLED xPWD    | Write the loop index (I 0 or 1) to port LED (13)
  | rDELAY xW]     | Wait for DELAY (1000) milliseconds; end FOR loop
  | K? ~}          | Check if a key was pressed; if not then continue, else exit loop
  | K@ \\          | Read the key that was pressed and discard it

There are more examples here: https://github.com/CCurl/r4/blob/main/examples.txt

## Memory areas
There are 2 memory areas in r4:
- CODE: r4 function code goes in this area. Default size is 128K.
- VARS: use with 'V' opcode. Default size is 256K.
- `xIH` (Info: HERE) returns the first unused address in the CODE area.
- The CODE area can also be used for data. See the 'U' opcode.
- The VARS area is not used by the r4 system at all, it is 100% free.

## LittleFS support
Some development boards support LittleFS. For those boards, the __LITTLEFS__ directive can be #defined to persist data to the board, including supporting automatically loading blocks on boot.

## A simple block editor
r4 includes a simple block editor. It has a VI-like feel to it.

## The implementation of r4

- The entire system is implemented in a few files, primarily: config.h, r4.h, r4.cpp, pc-main.cpp, and r4.ino.
  - There are a few additional files to support optional functionality (e.g.-File access).
- The same code runs on Windows, Linux, and multiple development boards (via the Arduino IDE).
- See the file "config.h" for system configuration settings.

r4 hashes register and function names and uses the hashed value as the index into the array of values or addresses.

Here is the hashing function (the DJB2a hash function using XOR):
```
int doHash(int max) {
    UCELL hash = 5381;
    if (!ISALPHA(*pc)) { return 0; }
    while (ISALPHANUM(*pc)) {
        hash = (hash * 33) ^ *(pc++);
    }
    return hash & max;
}
```

This is very fast, but poses some limitations:
- The maximum number of registers and functions need to be powers of 2.
- r4 does NOT detect hash collisions as it does not keep key values.
  - My tests have indicated that for a large enough number of buckets, collisions are not common.
  - ':' prints "-redef-f:[hash]-" when the given function name already has a value (a possible collision).
  - '&' prints "-redef-r:[hash]-" when a the given register name already has a value (a possible collision).
  - Use `xh[NAME]` to see info about [NAME]. For example: `xhTEST`

## Building r4
- The target machine/environment is controlled by the #defines in the file "config.h"
- For Windows, There is a Visual Studio solution. Use the x86 configuration (32-bit).
- For Development boards, I use the Arduino IDE. See the file "config.h" for board-specific settings.
- For Linux systems, there is a makefile. Default is 64-bit, or you can change it to 32-bit.
- I do not have an Apple system, so I haven't tried to build r4 for that environment.
  - However, being such a minimal C program, it should not be difficult to port r4 to any environment.

##  r4 Reference
### INTEGER OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| +  |  (a b--n)   | n: a+b - addition
| -  |  (a b--n)   | n: a-b - subtraction
| *  |  (a b--n)   | n: a*b - multiplication
| /  |  (a b--q)   | q: a/b - division
| M  |  (a b--r)   | r: a%b - modulo
| S  |  (a b--q r) | q: div(a,b), r: modulo(a,b)  (SLASH-MOD)


### FLOATING POINT OPERATIONS
#### NOTES:
- r4 uses double wide (64-bit) floating point numbers

| OP |Stack |Description|
|:-- |:--   |:--|
| FF | (i--f)    | Convert TOS from integer to float
| FI | (f--i)    | Convert TOS from float to integer
| F+ | (a b--n)  | Float: add
| F- | (a b--n)  | Float: subtract
| F* | (a b--n)  | Float: multiply
| F/ | (a b--n)  | Float: divide
| F< | (a b--f)  | f: if (a < b) then 1 , else 0
| F= | (a b--f)  | f: if (a = b) then 1 , else 0
| F> | (a b--f)  | f: if (a > b) then 1 , else 0
| F. | (n--)     | Float: print top of float stack
| F_ | (a--b)    | b: -a


### BIT MANIPULATION OPERATIONS
| OP |Stack |Description|
|:--  |:--   |:--|
| b&  | (a b--n)   | n: a AND b
| b\| | (a b--n)   | n: a OR b
| b^  | (a b--n)   | n: a XOR b
| b~  | (a--b)     | b: complement of a (e.g - 1011 => 0100)
| L   | (a n--b)   | b: a << n (Left-Shift)
| R   | (a n--b)   | b: a >> n (Right-Shift)


### STACK OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| #  | (a--a a)       | Duplicate TOS (DUP)
| \  | (a b--a)       | Drop TOS (DROP)
| $  | (a b--b a)     | Swap top 2 stack items (SWAP)
| %  | (a b--a b a)   | Push 2nd (OVER)
| _  | (a--b)         | b: -a (NEGATE)
| D  | (a--b)         | b: a-1 (DECREMENT)
| P  | (a--b)         | b: a+1 (INCREMENT)
| A  | (a--b)         | b: absolute value of a (ABS)
| xK | (--)           | Display the stack (.S)


### MEMORY OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| @  | (a--n)      | Fetch CELL n from address a
| !  | (n a--)     | Store CELL n  to  address a
| C@ | (a--n)      | Fetch BYTE n from address a
| C! | (n a--)     | Store BYTE n  to  address a
| U  | (n--a)      | a: address of byte n in the CODE area.
| V  | (n--a)      | a: address of byte n in the VARS area.


### REGISTERS OPERATIONS
#### NOTES:
- A register name is of the form `[A-Z][A-Z0-9]*` (e.g. - REG43).
- The number of registers is controlled by the NUM_REGS #define in "config.h".

| OP |Stack |Description|
|:-- |:--   |:--|
| &ABC | (N--)  | Define register rABC with initial value of N.
|      |        | *** NOTE: If register rABC has a value <> 0, r4 prints "-redef-r[hash]-".
|      |        |     This can be used to check if there is a hashing collision.
| rABC | (--v)  | v: value of register ABC.
| sABC | (v--)  | v: store v to register ABC.
| iABC | (--)   | Increment register ABC.
| dABC | (--)   | Decrement register ABC.


### TEMPORARY REGISTERS OPERATIONS
#### NOTES:
- A reference to a temporary register is of the form `[rsid][0-9]` (e.g. r7).

| OP |Stack |Description|
|:-- |:--   |:--|
| T+ | (--)   | Allocate 10 temporary registers
| rN | (--v)  | v: value of register #N.
| sN | (v--)  | v: store v to register #N.
| iN | (--)   | Increment register #N.
| dN | (--)   | Decrement register #N.
| T- | (--)   | Free the most recently allocated temporary registers


### FUNCTIONS OPERATIONS
#### NOTES:
- A function name is of the form `[A-Z][A-Z0-9]*` (e.g. - TMP23).
- The number of functions is controlled by the NUM_FUNCS #define in "config.h".
- A function definition is limited to ONE line.
- Returning while inside of a loop will eventually cause a problem.
  - Use '^' to unwind the loop stack first.

| OP    |Stack   |Description|
| :--   |:--     |:--|
| :ABC  | (--)   | Define function ABC. Copy chars to (HERE++) until closing ';'.
|       |        | If function ABC has a value <> 0, print "-redef-f[hash]-".
| cABC  | (--)   | Call function ABC. Handles "tail call optimization".
|       |        |  *** NOTE: this allocates 10 local variables, which are freed on ';'
| ;     | (--)   | Return: PC = rpop()

 
### INPUT/OUTPUT OPERATIONS
| OP    |Stack     |Description|
|:--    |:--       |:--|
| .     | (N--)    | Output N as a decimal number.
| ,     | (N--)    | Output N as a character (EMIT)
| "str" | (--)     | Output formatted characters until the next '"' see (1)
| B     | (--)     | Output a single SPACE (32,)
| N     | (--)     | Output a single NEWLINE (13,10,)
| K?    | (--f)    | f: non-zero if char is ready to be read, else 0.
| K@    | (--n)    | n: Key char, wait if no char is available.
| 0-9   | (--N)    | Scan DECIMAL number N until non digit
| N.N   | (--F)    | - Use NNN.NNN (eg - 3.14) to enter a floating point number F
|       |          | - to specify multiple values, separate them by space (4711 3333)
|       |          | - to enter a negative number, use "negate" (eg - 490_)
| hXXX  | (--N)    | Scan HEX number N until non hex-digit ([0-9,A-F] only ... NOT [a-f])
| 'C    | (--n)    | n: the ASCII value of C
| \`x\` | (a--a b) | Copy following chars until closing '`' to (address a).
|       |          | - a: address, b next byte after trailing NULL.

(1) Output formatting:
- Similar to 'C' formatting, the '%' char identifies additional processing.
  - %c: (N--)   Output N as a char
  - %b: (N--)   Output N as a number in base 2
  - %d: (N--)   Output N as a number in base 10
  - %x: (N--)   Output N as a number in base 16
  - %B: (N B--) Output N as a number in base B
  - %s: (A--)   Output A as a NULL-terminated string
  - %e: (--)    Output an escape (27)
  - %q: (--)    Output a quote (34)
  - %n: (--)    Output a CR/LF (13,10)
  - %f: (F--)   Output F as a double/float using "%f"
  - %g: (F--)   Output F as a double/float using "%g"
  - any other char after the % is output as is (eg "%%" outputs a single '%')

### LOGICAL/CONDITIONS/FLOW CONTROL OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| <  | (a b--f)    | f: if (a < b) then 1, else 0;
| =  | (a b--f)    | f: if (a = b) then 1, else 0;
| >  | (a b--f)    | f: if (a > b) then 1, else 0;
| ~  | (x--f)      | f: if (x = 0) then 1, else 0; (logical NOT)
| (  | (f--)       | if (f != 0), execute code in '()', else skip to matching ')'
| X  | (a--)       | if (a != 0), execute/call function at address a
| G  | (a--)       | if (a != 0), go/jump to function at address a


### FOR/NEXT LOOPING OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| [  | (F T--)   | FOR: start a For/Next loop. 
|    |           |  *** NOTE: if (F > T), F and T are swapped
| I  | (--n)     | n: the index of the current FOR loop
| J  | (--n)     | n: the index of the next outer FOR loop
| p  | (n--)     | n: number to add to "I"
| ^  | (--)      | UNLOOP, use with ';'. Example: `rS rK>(^;)`
| ]  | (--)      | NEXT: increment index (I) and loop if (I < T)


### BEGIN/WHILE LOOPING OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| {  | (f--f)      | BEGIN: if (f == 0) skip to ending '}'
| ^  | (--)        | UNLOOP, used with ';'. Example: `rX ~(^;)`
| }  | (f--f?)     | WHILE: if (f != 0) jump to starting '{', else drop f and continue


### FILE OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| fO | (nm md--fh)  | FILE: Open, nm: name, md: mode, fh: fileHandle
| fC | (fh--)       | FILE: Close, fh: fileHandle
| fD | (nm--)       | FILE: Delete
| fR | (fh--c n)    | FILE: Read, fh: fileHandle, c: char, n: num
| fW | (c fh--n)    | FILE: Write, fh: fileHandle, c: char, n: num
| fS | (--)         | FILE: Save Code
| fL | (--)         | FILE: Load Code


### BLOCK OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| bL | (n--)        | BLOCK: Load code from block file (block-nnn.r4). This can be nested.
| bA | (--)         | BLOCK: Load Abort - stop loading the current block (eg - if already loaded)
| bE | (n--)        | BLOCK: Edit block N (file name is block-nnn.r4)


### OTHER OPERATIONS
| OP |Stack |Description|
|:-- |:--   |:--|
| xV    | (--n)     | r4 version number (YYYYMMDD)
| xIAF  | (--a)     | INFO: Address where the function vectors begin
| xIAH  | (--a)     | INFO: Address of the HERE variable
| xIAR  | (--a)     | INFO: Address where the registers begin
| xIAU  | (--a)     | INFO: Address there the CODE area begins
| xIAV  | (--a)     | INFO: Address there the VARS area begins
| xIC   | (--n)     | INFO: CELL size
| xIF   | (--n)     | INFO: Number of functions (NUM_FUNCS)
| xIH   | (--a)     | INFO: Current HERE value
| xIR   | (--n)     | INFO: Number of registers (NUM_REGS)
| xIU   | (--n)     | INFO: Size of CODE area (CODE_SZ)
| xIV   | (--n)     | INFO: Size of VARS area (VARS_SZ)
| xPI   | (p--)     | Arduino: open pin input  (pinMode(p, INPUT))
| xPU   | (p--)     | Arduino: open pin pullup (pinMode(p, INPUT_PULLUP))
| xPO   | (p--)     | Arduino: open pin output (pinMode(p, OUTPUT)
| xPRA  | (p--n)    | Arduino: pin read analog  (n = analogRead(p))
| xPRD  | (p--n)    | Arduino: pin read digital (n = digitalRead(p))
| xPWA  | (n p--)   | Arduino: pin write analog  (analogWrite(p, n))
| xPWD  | (n p--)   | Arduino: pin write digital (digitalWrite(p, n))
| xs    | (a--)     | PC: call "system(a)"
| xSR   | (--)      | R4 System Reset
| xT    | (--n)     | Time in milliseconds (Arduino: millis(), Windows: GetTickCount())
| xh[S] | (--)      | Print hash, reg, and func value for [S] (eg - `xhALLOT` or `xhVH`)
| xM    | (--n)     | Time in microseconds (Arduino: micros())
| xW    | (n--)     | Wait (Arduino: delay(),  Windows: Sleep())
| xR    | (n--r)    | r: a pseudo-random number between 0 and n (uses XOR-shift)
|       |           |  *** NOTE: when n=0, r is the entire CELL-sized number
| xQ    | (--)      | PC: Exit R4
