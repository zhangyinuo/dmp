#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <iconv.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int utf8_to_gbk(char *sourcebuf, size_t  sourcelen, char  *destbuf, size_t  destlen )
{
	iconv_t cd;
	if ( (cd = iconv_open( "gbk" , "utf-8"  )) ==0  )
		return  -1;
	memset(destbuf,0,destlen);
	char  **source = &sourcebuf;
	char  **dest = &destbuf;
	if (-1 == iconv(cd,source,&sourcelen,dest,&destlen))
	{
		iconv_close(cd);
		return  -1;
	}
	iconv_close(cd);
	return  0;
} 

int  gbk_to_utf8( char  *sourcebuf, size_t  sourcelen, char  *destbuf, size_t  destlen )
{
	iconv_t cd;
	if ( (cd = iconv_open( "utf-8" , "gbk"  )) ==0  )
		return  -1;
	memset(destbuf,0,destlen);
	char  **source = &sourcebuf;
	char  **dest = &destbuf;
	if (-1 == iconv(cd,source,&sourcelen,dest,&destlen))
	{
		iconv_close(cd);
		return  -1;
	}
	iconv_close(cd);
	return  0;
} 

/*
int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "a.out infile\n");
		return -1;
	}

	char *from = malloc (1024);

	char *to = malloc(2048);

	FILE *fp = fopen(argv[1], "r");
	if (!fp)
	{
		fprintf(stderr, "open %s error %s\n", argv[1], strerror(errno));
		return -1;
	}
	while (fgets(from, 1024, fp))
	{
		char *t = strchr(from, '\n');
		if (t)
			*t = 0x0;
		//fprintf(stdout, "%s\n", from);
		if (utf8_to_gbk(from, strlen(from), to, 2048))
		{
			fprintf(stderr, "utf8_to_gbk err %m\n");
		}
		else
			fprintf(stdout, "%s\n", to);
	}
	fclose(fp);
	return 0;
}
*/
