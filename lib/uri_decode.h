#ifndef __VFS_URI_DECODE_H_
#define __VFS_URI_DECODE_H_
#define NGX_UNESCAPE_URI       1
#define NGX_UNESCAPE_REDIRECT  2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

typedef unsigned char u_char;

void ngx_unescape_uri(u_char **dst, u_char **src, size_t size, int type);

void ngx_escape_uri(u_char *dst, u_char *src, size_t size, int type);

#endif

