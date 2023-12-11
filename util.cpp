#include "util.h"

int be2se_2bytes(const char *src, char *dst)
{
    if (!src || !dst)
    {
        return -1;
    }

    dst[1] = src[0];
    dst[0] = src[1];
    return 0;
}

int be2se_4bytes(const char *src, char *dst)
{
    if (!src || !dst)
    {
        return -1;
    }
    dst[3] = src[0];
    dst[2] = src[1];
    dst[1] = src[2];
    dst[0] = src[3];
    return 0;
}

int be2se_8bytes(const char *src, char *dst)
{
    if (!src || !dst)
    {
        return -1;
    }
    dst[7] = src[0];
    dst[6] = src[1];
    dst[5] = src[2];
    dst[4] = src[3];
    dst[3] = src[4];
    dst[2] = src[5];
    dst[1] = src[6];
    dst[0] = src[7];
    return 0;
}

int se2be_2bytes(const char *src, char *dst)
{
    if (!src || !dst)
    {
        return -1;
    }
    dst[0] = src[1];
    dst[1] = src[0];
    return 0;
}

int se2be_3bytes(const char *src, char *dst)
{
    if (!src || !dst)
    {
        return -1;
    }
    dst[0] = src[2];
    dst[1] = src[1];
    dst[2] = src[0];
    return 0;
}

int se2be_4bytes(const char *src, char *dst)
{
    if (!src || !dst)
    {
        return -1;
    }
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
    return 0;
}

int se2be_8bytes(const char *src, char *dst)
{
    if (!src || !dst)
    {
        return -1;
    }
    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
    return 0;
}