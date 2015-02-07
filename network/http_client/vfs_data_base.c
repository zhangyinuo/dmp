/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/

/*
 *base�ļ�����ѯ����������Ϣ��������ػ���״̬������```
 *Tracker ��Ŀ���٣�����һ����̬����
 *CS��FCS��Ŀ�϶࣬����hash����
 *CS FCS ip��Ϣ����uint32_t �洢�����ڴ洢�Ͳ���
 */
volatile extern int maintain ;		//1-ά������ 0 -����ʹ��
extern t_vfs_up_proxy g_proxy;

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

static void create_header(char *domain, char *url, char *httpheader)
{
	int l = sprintf(httpheader, "GET /%s HTTP/1.1\r\n", url);

	l += sprintf(httpheader + l, "Host: %s\r\n\r\n", domain);
}

static int get_file(char *file)
{
	int ret = -1;
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(g_config.docroot)) == NULL) 
	{
		LOG(vfs_sig_log, LOG_ERROR, "opendir %s err  %m\n", g_config.docroot);
		return ret;
	}
	LOG(vfs_sig_log, LOG_TRACE, "opendir %s ok \n", g_config.docroot);
	while((dirp = readdir(dp)) != NULL) 
	{
		if (dirp->d_name[0] == '.')
			continue;
		snprintf(file, sizeof(file), "%s/%s", g_config.docroot, dirp->d_name);
		ret = 0;
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
		char filename[256] = {0x0};
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
	}
}


