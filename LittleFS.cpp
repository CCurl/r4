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

void fileInit() {
	myFS.begin(1 * 1024 * 1024);
	printString("\r\nLittleFS: initialized");
	printStringF("\r\nBytes total: %llu, used:%llu", myFS.totalSize(), myFS.usedSize());
}

void fileOpen() {
	char *md = (char *)pop();
	char *fn = AOS;
	File x=myFS.open(fn, (md[0]=='w') ? FILE_WRITE_BEGIN : FILE_READ);
	TOS = (CELL)x;
}

void fileClose() {
	File x=File((FileImpl*)pop());
	x.close();
}

// fR (fh--c n) - File Read
// fh: File handle, c: char read, n: num chars read
// n=0: End of file or file error
void fileRead() {
	File x=File((FileImpl*)pop());
	push(0);
	NOS = TOS = 0;
	if (x) {
		char c;
		TOS = x.read(&c, 1);
		NOS = TOS ? c : 0;
	}
}

// fW (c fh--n) - File Write
// fh: File handle, c: char to write, n: num chars written
// n=0: File not open or error
void fileWrite() {
File x=File((FileImpl*)pop());
	char c = (char)TOS;
	TOS = x.write(&c,1);
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

void noFile() {}
addr codeLoad(addr x, addr h) { noFile(); return h; }
void codeSave(addr x, addr y) { noFile(); }
void blockLoad(CELL num) { noFile(); }
void loadAbort() { noFile(); }
int fileReadLine(CELL fh, char* buf) { noFile(); return -1; }

#endif // __LITTLEFS__
