#include "types.h"
#include "stat.h"
#include "user.h"

char* strcpy(char *s, const char *t) {
  char *os = s;
  while((*s++ = *t++) != 0);
  return os;
}

int strcmp(const char *p, const char *q) {
  while(*p && *p == *q) p++, q++;
  return (uchar)*p - (uchar)*q;
}

char* strchr(const char *s, char c) {
  for(; *s; s++) if(*s == c) return (char*)s;
  return 0;
}

void* memset(void *dst, int c, uint n) {
  char *cdst = (char *) dst;
  uint i;
  for(i = 0; i < n; i++) cdst[i] = c;
  return dst;
}

void* memmove(void *dst, const void *src, uint n) {
  const char *s = src;
  char *d = dst;
  if(s < d && s + n > d){
    s += n; d += n;
    while(n-- > 0) *--d = *--s;
  } else {
    while(n-- > 0) *d++ = *s++;
  }
  return dst;
}

uint strlen(const char *s) {
  uint n;
  for(n = 0; s[n]; n++);
  return n;
}

char* gets(char *buf, int max) {
  int i, cc;
  char c;
  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1) break;
    buf[i++] = c;
    if(c == '\n' || c == '\r') break;
  }
  buf[i] = '\0';
  return buf;
}