#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <scws/scws.h>

static scws_t s;

enum {HEAD = 0, BODY, SOU, URL, MAX_COUNT};

typedef struct {
	char stock[128];
	int count[MAX_COUNT];
	int total;
	list_head_t hlist;
} t_stock_msg;

#define MAX_LIST 0x3FF

list_head_t alllist[1024];

static uint32_t r5hash(const char *p) 
{
	uint32_t h = 0;
	while(*p) {
		h = h * 11 + (*p<<4) + (*p>>4);
		p++;
	}
	return h;
}

static void add_node(char *item)
{
	int idx = r5hash(item) & MAX_LIST;
	list_head_t *hlist = &(alllist[idx]);
	
	t_stock_msg *stock_msg = (t_stock_msg *) malloc (sizeof(t_stock_msg));
	if (stock_msg == NULL)
	{
		perror("malloc");
		exit(1);
	}

	memset(stock_msg, 0, sizeof(t_stock_msg));
	INIT_LIST_HEAD(&(stock_msg->hlist));

	strcpy(stock_msg->stock, item);
	list_add_head(&(stock_msg->hlist), hlist);
}

static int init_stock()
{
	int i = 0;
	for (; i <= MAX_LIST; i++)
	{
		INIT_LIST_HEAD(&(alllist[i]));
	}

	char *stockfile = "./stock_file.txt";

	FILE *fp = fopen(stockfile, "r");
	if (fp == NULL)
	{
		perror("open");
		return -1;
	}

	char buf[128] = {0x0};

	while (fgets(buf, sizeof(buf), fp))
	{
		char *t = strrchr(buf, '\n');
		if (t)
			*t = 0x0;

		add_node(buf);
		memset(buf, 0, sizeof(buf));
	}

	fclose(fp);

	return 0;
}

static int p_item(char *item, int type)
{
	int idx = r5hash(item) & MAX_LIST;
	list_head_t *hlist = &(alllist[idx]);

	list_head_t *l;
	t_stock_msg *mc;

	int f = 0;
	list_for_each(l, hlist)
	{
		mc = list_entry(l, t_stock_msg, hlist);
		if (strcmp(mc->stock, item) == 0)
		{
			f = 1;
			break;
		}
	}

	if (f == 0)
		return -1;
	mc->count[type]++;
	return 0;
}

static void p_txt(char *text, int type)
{
	scws_res_t res, cur;
	scws_send_text(s, text, strlen(text));
	while (res = cur = scws_get_result(s))
	{
		while (cur != NULL)
		{
			char buf[128] = {0x0};
			snprintf(buf, sizeof(buf), "%.*s", cur->len, text+cur->off);
			p_item(buf, type);
			cur = cur->next;
		}
		scws_free_result(res);
	}
}

static void p_file(char *file, int type)
{
	FILE *fp = fopen(file, "r");
	if (fp == NULL)
	{
		perror("open");
		return;
	}

	char buf[20480] = {0x0};

	while (fgets(buf, sizeof(buf), fp))
	{
		char *t = strrchr(buf, '\n');
		if (t)
			*t = 0x0;

		p_txt(buf, type);
		memset(buf, 0, sizeof(buf));
	}

	fclose(fp);
}

int main()
{
	if (!(s = scws_new())) {
		printf("ERROR: cann't init the scws!\n");
		return -1;
	}

	if (init_stock()) 
	{
		printf("ERROR: init_stock!\n");
		return -1;
	}
	scws_set_charset(s, "gbk");
	scws_set_dict(s, "./full.xdb", SCWS_XDICT_XDB);
	scws_set_rule(s, "/ott/install/scws/etc/rules.gbk.ini");

	p_file("./head", HEAD);
	p_file("./body", BODY);
	p_file("./sou", SOU);
	p_file("./url", URL);

	scws_free(s);
	return 0;
}
