#ifndef __CONFIG__
#define __CONFIG__

#define VERSION          20240317
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
  #define NUM_LOCALS     100
  #define CODE_SZ        (128*1024)
  #define VARS_SZ        (256*1024)
  #define NUM_REGS        0x10000
  #define NUM_FUNCS       0x10000
  #define __FILES__
  #define __BOARD__      PC
  #define __EDITOR__
#else
  /* Dev Board */
  #define STK_SZ          64
  #define RSTK_SZ         64
  #define LSTACK_SZ       30
  #define NUM_LOCALS      50
  #define CODE_SZ        ( 64*1024)
  #define VARS_SZ        (128*1024)
  #define NUM_REGS        0x4000
  #define NUM_FUNCS       0x4000
  #define __LITTLEFS__
  #define __BOARD__      TEEENSY4
  #define __EDITOR__
  #define __SERIAL__      1
  #define mySerial        Serial
#endif

#define MAX_FUNC         (NUM_FUNCS-1)
#define MAX_REG          (NUM_REGS-1)

#endif
