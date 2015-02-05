#include <stdio.h>
#include <scws/scws.h>
//#define SCWS_PREFIX     "/usr/local/scws"

void main()
{
	scws_t s;
	scws_res_t res, cur;
	char *text = "大家好，我是中国人，来自安徽";

	if (!(s = scws_new())) {
		printf("ERROR: cann't init the scws!\n");
		exit(-1);
	}
	scws_set_charset(s, "utf8");
	scws_set_dict(s, "./dict.utf8.xdb", SCWS_XDICT_XDB);
	scws_set_rule(s, "/ott/install/scws/etc/rules.utf8.ini");

	scws_send_text(s, text, strlen(text));
	while (res = cur = scws_get_result(s))
	{
		while (cur != NULL)
		{
			printf("WORD: %.*s/%s (IDF = %4.2f)\n", cur->len, text+cur->off, cur->attr, cur->idf);
			cur = cur->next;
		}
		scws_free_result(res);
	}
	scws_free(s);
}
