/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/

#include "uri_decode.h"
#include "c_api.h"
#include "myconv.h"

FILE *fp_sou = NULL;
FILE *fp_url = NULL;
FILE *fp_head = NULL;
FILE *fp_body = NULL;

#define MAX_CONVERT 204800

char convert_dst[MAX_CONVERT];

enum {SOU = 0, URL, HEAD, BODY};

int active_send(int fd, char *data)
{
	LOG(vfs_sig_log, LOG_DEBUG, "send %d cmdid %s\n", fd, data);
	set_client_data(fd, data, strlen(data));
	modify_fd_event(fd, EPOLLOUT);
	return 0;
}

static int init_stock()
{
	fp_sou = fopen("../path/sou", "w");
	if (fp_sou == NULL)
	{
		LOG(vfs_sig_log, LOG_ERROR, "fopen fp_sou err %m\n");
		return -1;
	}

	fp_url = fopen("../path/url", "w");
	if (fp_url == NULL)
	{
		LOG(vfs_sig_log, LOG_ERROR, "fopen fp_url err %m\n");
		return -1;
	}

	fp_head = fopen("../path/head", "w");
	if (fp_head == NULL)
	{
		LOG(vfs_sig_log, LOG_ERROR, "fopen fp_head err %m\n");
		return -1;
	}

	fp_body = fopen("../path/body", "w");
	if (fp_body == NULL)
	{
		LOG(vfs_sig_log, LOG_ERROR, "fopen fp_body err %m\n");
		return -1;
	}
	return 0;
}

static void do_print_process(int type, char *data)
{
	if (type == SOU)
		fprintf(fp_sou, "%s\n", data);
	else if (type == URL)
		fprintf(fp_url, "%s\n", data);
	else if (type == HEAD)
		fprintf(fp_head, "%s\n", data);
	else if (type == BODY)
		fprintf(fp_body, "%s\n", data);
	return;
}

static int check_sou(char *domain)
{
	if (strcasestr(domain, "baidu.com"))
		return 0;
	if (strcasestr(domain, "sou.com"))
		return 0;
	if (strcasestr(domain, "soso.com"))
		return 0;
	if (strcasestr(domain, "haosou.com"))
		return 0;
	return -1;
}

static void do_process_req(char *domain, char *url)
{
	u_char durl[1024] = {0x0};
	u_char *ddurl = durl;
	ngx_unescape_uri(&ddurl, (u_char **)&url, strlen(url), 0);

	if (check_sou(domain))
		do_print_process(URL, (char *)durl);
	else
		do_print_process(SOU, (char *)durl);
}

static int get_title(char *src, int srclen, char *dst)
{
	char *s = strcasestr(src, "<title>");
	if (s == NULL)
		return -1;

	char *e = strcasestr(s, "</title>");
	if (e == NULL)
		return -1;

	LOG(vfs_sig_log, LOG_DEBUG, "prepare head!\n");
	if (e - s - 7 > 1024)
	{
		LOG(vfs_sig_log, LOG_DEBUG, "prepare head error!\n");
		return -1;
	}

	strncpy(dst, s + 7, e - s - 7);
	LOG(vfs_sig_log, LOG_DEBUG, "get head [%s]!\n", dst);
	return 0;

}

static void do_process_sub(char *data, int len)
{
	if (isprint(*data) == 0)
		return;

	char title[1024] = {0x0};

	if (get_title(data, len, title) == 0)
		do_print_process(HEAD, title);
	do_print_process(BODY, data);
}

static void do_process(char *data, size_t len, int isutf8)
{

	if (isutf8 == 0)
	{
		LOG(vfs_sig_log, LOG_DEBUG, "process %.*s\n", len, data);
		do_process_sub(data, len);
		return;
	}
	char *dst = convert_dst;

	memset(dst, 0, MAX_CONVERT);

	if (utf8_to_gbk(data, len, dst, MAX_CONVERT) == 0)
	{
		LOG(vfs_sig_log, LOG_DEBUG, "process utf8 %s\n", dst);
		do_process_sub(dst, MAX_CONVERT);
	}
	else
		LOG(vfs_sig_log, LOG_ERROR, "utf8_to_gbk err %m\n");

}
