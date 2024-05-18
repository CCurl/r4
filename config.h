#ifndef __CONFIG__
#define __CONFIG__

#define PC 1

#ifdef _WIN32
  #define __WINDOWS__
  #define  _CRT_SECURE_NO_WARNINGS
  #include <Windows.h>
  #include <conio.h>
  #define __PC__
#endif

#ifdef __LINUX__
  #include <time.h>
  #define __PC__
#endif

#ifdef __PC__
  #define STK_SZ         256
  #define RSTK_SZ        256
  #define LSTACK_SZ       60
  #define FSTK_SZ         10
  #define NUM_LOCALS     100
  #define CODE_SZ       (128*1024)
  #define VARS_SZ       (256*1024)
  #define NUM_REGS        0x10000
  #define NUM_FUNCS       0x10000
  #define __FILES__
  #define __BOARD__      PC
  #define __EDITOR__
  #define MAX_LINES      200
#else
  /* Dev Board */
  #define STK_SZ          64
  #define RSTK_SZ         64
  #define LSTACK_SZ       30
  #define FSTK_SZ         10
  #define NUM_LOCALS      50
  #define CODE_SZ        (32*1024)
  #define VARS_SZ        (64*1024)
  #define NUM_REGS        0x4000
  #define NUM_FUNCS       0x2000
  // #define __TEENSY_FS__
  // #define __PISO_FS__
  // #define __NO_FS__
  // #define __FILES__
  #define __BOARD__      TEEENSY4
  #define __EDITOR__
  #define MAX_LINES      64
  #define __SERIAL__      1
  #define mySerial        Serial
#endif

#define MAX_FUNC         (NUM_FUNCS-1)
#define MAX_REG          (NUM_REGS-1)

#endif
