#include "r4.h"

#ifdef __TEENSY_FS__

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
	for (int i=1;i<=NFILES;i++) {
		if ((bool)files[i] == false) { return i; }
	}
	return 0;
}

CELL_T fileOpenI(const char *fn, const char *md) {
	int fh = freeFile();
	if (0 < fh) {
		files[fh] = myFS.open(fn, (md[0]=='w') ? FILE_WRITE_BEGIN : FILE_READ);
        if (md[0]=='w') { files[fh].truncate(); }
	}
	return fh;
}

// fO (fn md -- fh) - FileOpen
void fileOpen() {
	char *md = (char *)pop();
	char *fn = AOS;
	TOS = fileOpenI(fn, md);
}

// fC (fh--) - File Close
void fileClose() {
	CELL_T fh = pop();
	if (BTWI(fh, 1, NFILES) &&((bool)files[fh])) {
		files[fh].close();
	}
}

// fD (nm--) - File Delete
void fileDelete() {
	char *nm = (char *)pop();
    myFS.remove(nm);
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
	sprintf(fn, BLOCK_FN, num);
	num = 0;
	File x = myFS.open(fn, FILE_READ);
	if ((bool)x) {
		num = x.read(blk, sz);
		x.close();
	}
	return num;
}

// bR (num addr sz--f)
void readBlock1() {
    CELL_T t1 = pop();
    char *n1 = (char*)pop();
    TOS = readBlock(TOS, (char*)n1, t1);
}

int writeBlock(int num, char *blk, int sz) {
	char fn[32];
	sprintf(fn, BLOCK_FN, num);
	num = 0;
	File x = myFS.open(fn, FILE_WRITE_BEGIN);
	if ((bool)x) {
        x.truncate();
        num = x.write(blk, sz);
		x.close();
	}
	return num;
}

// bW (num addr sz--f)
void writeBlock1() {
    CELL_T t1 = pop();
    char *n1 = (char*)pop();
    TOS = writeBlock(TOS, (char*)n1, t1);
}

// fL - File readLine
int fileReadLine(CELL_T fh, char *buf) {
	int n = -1;
	buf[0] = 0;
	if (BTWI(fh, 1, NFILES) && (0 < files[fh].available())) {
		n = files[fh].readBytesUntil(10, buf, 256);
		buf[n] = 0;
	}
	return n;
}

// bL - Block Load
void blockLoad(CELL_T num) {
	char fn[32];
	sprintf(fn, BLOCK_FN, num);
	CELL_T fh = fileOpenI(fn, "r");
	if (files[fh].available()) {
		if (input_fp) {
			fpush(input_fp);
		}
		input_fp = fh;
	}
}

// bA - Block load Abort
void loadAbort() {}

#endif // __TEENSY_FS__
