#ifndef __UTIL_H__
#define __UTIL_H__

int be2se_2bytes(const char *src, char *dst);
int be2se_4bytes(const char *src, char *dst);
int be2se_8bytes(const char *src, char *dst);

int se2be_2bytes(const char *src, char *dst);
int se2be_3bytes(const char *src, char *dst);
int se2be_4bytes(const char *src, char *dst);
int se2be_8bytes(const char *src, char *dst);

#endif