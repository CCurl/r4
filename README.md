# r4 - A minimal stack machine VM/CPU with human-readable machine language

r4 is an simple, minimal, and interactive environment where the source code IS the machine code. There is no compilation in r4.

# What is r4?

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

The reference for r4 is here:   https://github.com/CCurl/r4/blob/main/reference.txt

Examples for r4 are here: https://github.com/CCurl/r4/blob/main/examples.txt

# Why did I create r4?

There are multiple goals for r4:

1. Freedom from the need for a multiple gigabyte tool chain and the edit/compile/run/debug loop for developing everyday programs. Of course, you need one of these monsters to build and deploy r4, but at least after that, you are free of them.

2. Many programming environments use tokens and a large SWITCH statement in a loop to execute the user's program. In those systems, the machine code (aka - byte-code ... the cases in the SWITCH statement) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when looking at the code that is actually being executed. Additionally there is a compiler that is needed in order to work in that environment. In these enviromnents, there is a steep learning curve; the programmer needs to learn: (1) the user environment, (2) the hundreds or thousands of user functions (or "words" in Forth), and (3) how they work together. I wanted to avoid as much as that as possible, and have only one thing to learn: the machine code.

3. A desire for a simple, minimal, and interactive programming environment that was easy to modify and enhance.

4. An environment that could be deployed to many different types of development boards via the Arduino IDE.

5. To be able to use the same environment on my personal computer as well as development boards.

6. Short commands so that there was not a lot of typing needed.

# The implementation of r4

- The entire system is implemented in a few files, primarily: config.h, r4.h, r4.cpp, pc-main.cpp, and r4.ino.
  - There are a few additional files to support optional functionality (e.g.-WiFi and File access).
- The same code runs on Windows, Linux, and multiple development boards (via the Arduino IDE).
- See the file "config.h" for system configuration settings.

# Locals

r4 allocates 10 local variables (0-9) per function call. They are referred to, and have the same operations, like registers (r,s,i,d). For example, i4 increments local #4.

# WiFi support

Some boards, for example the ESP8266, support WiFi. For those boards, the __WIFI__ directive can be #defined to enable the boards WiFi.
Note that those boards usually also have watchdogs that need to be enabled via the __WATCHDOG__ #define.

# LittleFS support

Some development boards support LittleFS. For those boards, the __LITTLEFS__ directive can be #defined to save and load the defined code to the board, so that any user-defined words can be reloaded across boots.

# A simple block editor

r4 includes a simple block editor. Many thanks to Alain Theroux for his inspiration and help.

# Building r4

- The target machine/environment is controlled by the #defined in the file "config.h"
- For Windows, I use Microsoft's Visual Studio (Community edition). I use the x86 configuration.
- For Development boards, I use the Arduino IDE. See the file "config.h" for board-specific settings.
- For Linux systems, I use vi and clang. See the "make" script for more info.
- I do not have an Apple system, so I haven't tried to build r4 for that environment.
- However, being such a simple and minimal C program, it should not be difficult to port r4 to any environment.
