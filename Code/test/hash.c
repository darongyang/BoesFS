#include <stdio.h>
#include <stdint.h>

#define GETHIGH16(x) (((x) >> 16) & 0x0000FFFF)
#define GETLOW16(x) ((x) & 0x0000FFFF)
#define GETMULTIBASE(x) ((x+1) & 0x0000000F)
#define REVERSE16(x) ((GETLOW16(x) << 16) & 0xFFFF0000 | GETHIGH16(x))
#define HASHBASE1 428625750 // hash用于相加的基数
#define HASHBASE2 2004318071 // hash用于异或的基数
#define HASHBASE3 1431655765 // hash用于取模的基数
#define HASH(x) (REVERSE16(((REVERSE16(REVERSE16(REVERSE16(x)+(GETMULTIBASE(x)*HASHBASE1))) ^ HASHBASE2) % HASHBASE3)))

int hash(uint32_t t){
  return HASH(t);
}

int main(int argc, char const *argv[]){
  uint32_t t = atoi(argv[1]);
  printf("%lx %u\n", hash(t), hash(t));
  return 0;
}