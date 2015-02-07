/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/

/*
 *base文件，查询基础配置信息，设置相关基础状态及其它```
 *Tracker 数目较少，放在一个静态数组
 *CS和FCS数目较多，放在hash链表
 *CS FCS ip信息采用uint32_t 存储，便于存储和查找
 */
volatile extern int maintain ;		//1-维护配置 0 -可以使用
extern t_vfs_up_proxy g_proxy;
static char filename[256] = {0x0};

static inline int isDigit(const char *ptr) 
{
	return isdigit(*(unsigned char *)ptr);
}

static int active_connect(char *ip, int port)
{
	int fd = createsocket(ip, port);
	if (fd < 0)
	{
		LOG(vfs_sig_log, LOG_ERROR, "connect %s:%d err %m\n", ip, port);
		return -1;
	}
	if (svc_initconn(fd))
	{
		LOG(vfs_sig_log, LOG_ERROR, "svc_initconn err %m\n");
		close(fd);
		return -1;
	}
	add_fd_2_efd(fd);
	LOG(vfs_sig_log, LOG_NORMAL, "fd [%d] connect %s:%d\n", fd, ip, port);
	return fd;
}

static int create_header(char *domain, char *url, char *httpheader)
{
	int l = sprintf(httpheader, "GET /%s HTTP/1.1\r\n", url);

	l += sprintf(httpheader + l, "Host: %s\r\nConnection: Close\r\n\r\n", domain);

	return l;
}

static int get_file(char *file)
{
	int ret = -1;
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(g_config.docroot)) == NULL) 
	{
		LOG(vfs_sig_log, LOG_ERROR, "opendir %s err  %m %d\n", g_config.docroot, sizeof(g_config.docroot));
		return ret;
	}
	LOG(vfs_sig_log, LOG_TRACE, "opendir %s ok \n", g_config.docroot);
	while((dirp = readdir(dp)) != NULL) 
	{
		if (dirp->d_name[0] == '.')
			continue;
		snprintf(file, 256, "%s/%s", g_config.docroot, dirp->d_name);
		ret = 0;
		LOG(vfs_sig_log, LOG_NORMAL, "%s:%s:%d\n", ID, FUNC, LN);
		break;
	}
	closedir(dp);
	return ret;
}

void check_task()
{
	static FILE * lastfp = NULL;

	if (lastfp == NULL)
	{
		if (get_file(filename))
			return ;

		lastfp = fopen(filename, "r");
		if (lastfp == NULL)
		{
			LOG(vfs_sig_log, LOG_ERROR, "open %s err  %m\n", filename);
			return;
		}
	}

	int once = 0;
	char buf[2048] = {0x0};

	while(fgets(buf, sizeof(buf), lastfp))
	{
		LOG(vfs_sig_log, LOG_NORMAL, "%s:%s:%d\n", ID, FUNC, LN);
		char *t = strchr(buf, '/');
		if (t == NULL)
			continue;

		*t = 0x0;
		char ip[16] = {0x0};
		if (get_uint32_ip(buf, ip) == INADDR_NONE)
			continue;
		LOG(vfs_sig_log, LOG_NORMAL, "%s:%s:%d\n", ID, FUNC, LN);
		int fd = active_connect(ip, 80);
		if (fd < 0)
			continue;

		LOG(vfs_sig_log, LOG_NORMAL, "%s:%s:%d\n", ID, FUNC, LN);
		char httpheader[1024] = {0x0};
		create_header(buf, t + 1, httpheader);
		active_send(fd, httpheader);

		once++;
		if (once >= 19)
			break;
	}

	if (once < 20)
		return;

	fclose(lastfp);
	lastfp = NULL;
	unlink(filename);
}


