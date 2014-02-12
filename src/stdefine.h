/* ��׼ͷ�ļ� */
#ifndef _STDEFINE_H_
#define _STDEFINE_H_

#if defined(__MINGW32__)
#include <windows.h>
#undef RGB // undef RGB
#else

/* �������� */
#define TRUE   1
#define FALSE  0

/* ��׼�����Ͷ��� */
typedef int BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef long     int   LONG;

#endif

#define offsetof(type, member)          ((size_t)&((type*)0)->member)
#define container_of(ptr, type, member) (type*)((char*)(ptr) - offsetof(type, member))

/* �ú�������������δʹ�õľ��� */
#define DO_USE_VAR(var)  do { var = var; } while (0)

#endif

