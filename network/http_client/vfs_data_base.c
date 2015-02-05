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

static void create_header(char *httpheader, t_task_base *base, t_task_sub *sub, int len)
{
	char *t = base->url + 7;

	char *e = strchr(t, '/');
	int l = snprintf(httpheader, len, "GET /%s HTTP/1.1\r\n", t);

	if (e)
	{
		l = snprintf(httpheader, len, "GET /%s HTTP/1.1\r\n", e+1);
		*e = 0x0;
		l = snprintf(httpheader + l, len - l, "Host: %s\r\n\r\n", t); 
		*e = '/';
	}
	else
	{
		l = snprintf(httpheader, len, "GET /%s HTTP/1.1\r\n", t);
		l = snprintf(httpheader + l, len - l, "Host: %s\r\n\r\n", sub->peerip); 
	}
}

void check_task()
{
	t_vfs_tasklist *task = NULL;
	int ret = 0;
	while (1)
	{
		ret = vfs_get_task(&task, g_queue_tmp);
		if (ret != GET_TASK_OK)
		{
			LOG(vfs_sig_log, LOG_TRACE, "vfs_get_task get notihng %d\n", ret);
			break;
		}
		vfs_set_task(task, g_queue);
	}

	uint16_t once_run = 0;
	while (1)
	{
		once_run++;
		if (once_run >= g_config.cs_max_task_run_once)
			return;
		ret = vfs_get_task(&task, g_queue);
		if (ret != GET_TASK_OK)
		{
			LOG(vfs_sig_log, LOG_TRACE, "vfs_get_task get notihng %s\n", task_status[g_queue]);
			return ;
		}
		t_task_base *base = &(task->task.base);
		t_task_sub *sub = &(task->task.sub);
		LOG(vfs_sig_log, LOG_DEBUG, "vfs_get_task get from %s %s\n", task_status[g_queue], base->filename);
		int fd = active_connect(sub->peerip, sub->peerport);
		if (fd < 0)
		{
			LOG(vfs_sig_log, LOG_ERROR, "connect %s %d error %m\n", sub->peerip, sub->peerport);
			base->overstatus = OVER_TOO_MANY_TRY;
			vfs_set_task(task, TASK_FIN);  
			continue;
		}
		sub->lastlen = 0;
		char httpheader[1024] = {0x0};
		create_header(httpheader, base, sub, sizeof(httpheader));
		active_send(fd, httpheader);
		struct conn *curcon = &acon[fd];
		vfs_cs_peer *peer = (vfs_cs_peer *) curcon->user;
		peer->recvtask = task;
		peer->sock_stat = RECV_HEAD_ING;
	}
}


