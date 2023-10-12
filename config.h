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
  #define STK_SZ          32
  #define RSTK_SZ         16
  #define LSTACK_SZ       10*3
  #define USER_SZ        (128*1024)
  #define VARS_SZ        (256*1024)
  #define NUM_REGS       (26*26*26)
  #define NUM_FUNCS      (26*26*26)
  #define __FILES__
  #define __BOARD__      PC
  #define __EDITOR__
#else
  /* Dev Board */
  #define _NEEDS_ALIGN_   1
  #define STK_SZ          32
  #define RSTK_SZ         16
  #define LSTACK_SZ       4
  #define __LITTLEFS__
  #define __EDITOR__
  // #define USER_SZ        (24*1024) // XIAO
  #define USER_SZ        (96*1024) // RPI PICO
  #define NUM_REGS       (26*26*13) // MZZ
  #define NUM_FUNCS      (26*26*13) // MZZ
  //#define NUM_REGS       (26*26)
  //#define NUM_FUNCS      (26*26)
  #define __SERIAL__      1
  #define mySerial        Serial
#endif

#endif
