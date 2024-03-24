#include "r4.h"

#ifdef __LITTLEFS__

// shared with __FILE__
static byte fdsp = 0;
static CELL fstack[STK_SZ + 1];
CELL input_fp;

void fpush(CELL v) { if (fdsp < STK_SZ) { fstack[++fdsp] = v; } }
CELL fpop() { return (fdsp) ? fstack[fdsp--] : 0; }
// shared with __FILE__

// LittleFS uses NEXT
#undef NEXT
#include <LittleFS.h>

LittleFS_Program myFS;

#define NFILES 20
File files[NFILES+1];

void fileInit() {
	myFS.begin(1 * 1024 * 1024);
  // myFS.quickFormat();
	printString("\r\nLittleFS: initialized");
	printStringF("\r\nBytes total: %llu, used: %llu", myFS.totalSize(), myFS.usedSize());
  for (int i=0;i<=NFILES;i++) { files[i] = File(); }
}

int freeFile() {
  for (int i=1;i<=NFILES;i++) { if (files[i].name()[0] == (char)0) { return i; } }
  return 0;
}

CELL fileOpenI(const char *fn, const char *md) {
  int fh = freeFile();
  if (BTWI(fh, 1, NFILES)) {
	  files[fh] = myFS.open(fn, (md[0]=='w') ? FILE_WRITE_BEGIN : FILE_READ);  
  }
	return fh;
}

// fO (fn md -- fh) - FileOpen
void fileOpen() {
	char *md = (char *)pop();
	char *fn = AOS;
  TOS = fileOpenI(fn, md);
}

void fileClose() {
  CELL fh = pop();
	if (BTWI(fh, 1, NFILES)) {
    files[fh].close();
    files[fh] = File();
  }
}

// fR (fh--c n) - File Read
// fh: File handle, c: char read, n: num chars read
// n=0: End of file or file error
void fileRead() {
	int ndx = TOS;
	push(0);
	NOS = TOS = 0;
	if (BTWI(ndx, 1, NFILES)) {
		char c;
		TOS = files[ndx].read(&c, 1);
		NOS = TOS ? c : 0;
	}
}

// fW (c fh--n) - File Write
// fh: File handle, c: char to write, n: num chars written
// n=0: File not open or error
void fileWrite() {
	int fh = pop();
  char c = (char)TOS;
  TOS = 0;
	if (BTWI(fh, 1, NFILES)) {
	  TOS = files[fh].write(&c,1);
  }
}

int readBlock(int num, char *blk, int sz) {
	char fn[32];
	sprintf(fn, "block-%03d", num);
	num = 0;
	File x=myFS.open(fn, FILE_READ);
	if (x) {
		num = x.read(blk, sz);
		x.close();
	}
	return num;
}

int writeBlock(int num, char *blk, int sz) {
	char fn[32];
	sprintf(fn, "block-%03d", num);
	num = 0;
	File x=myFS.open(fn, FILE_WRITE_BEGIN);
	if (x) {
		num = x.write(blk, sz);
		x.close();
	}
	return num;
}

int fileReadLine(CELL fh, char* buf) {
  int n = -1;
  buf[0] = 0;
  if (BTWI(fh, 1, NFILES) && (0 < files[fh].available())) {
    n = files[fh].readBytesUntil(10, buf, 256);
    buf[n] = 0;
  }
  return n;
}

void blockLoad(CELL num) {
	char fn[32];
	sprintf(fn, "block-%03ld", num);
  CELL fh = fileOpenI(fn, "r");
  if (files[fh].available()) {
    if (input_fp) {
      fpush(input_fp);
    }
    input_fp = fh;
  }
}

void loadAbort() {}

#endif // __LITTLEFS__
