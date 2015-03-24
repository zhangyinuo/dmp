/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/

#ifndef _VFS_TASK_H_
#define _VFS_TASK_H_

/*
 *������ƣ�ҵ���߳����Ѻ���������ݷ��� CLEAN������У�Agent�̶߳�ʱ����ö������ݣ�����HOME��ҵ���߳�ѭ������
 *��ʱ������ҵ���߳��Լ�������������CLEAN���У���AGENT�̻߳���
 */

#include "list.h"
#include "vfs_init.h"
#include "nm_app_vfs.h"
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#define SYNCIP 256
#define DOMAIN_PREFIX "fcs"

enum {TASK_MOD_UP = 0, TASK_MOD_DOWN, TASK_MOD_CANCEL};

enum {OPER_IDLE, OPER_GET_REQ, OPER_GET_RSP, OPER_PUT, SYNC_2_GROUP};  /*�������� GET FCS-��CS�� CS-��CS��PUT CS-��CS*/

enum {TASK_DELAY = 0, TASK_WAIT, TASK_WAIT_TMP, TASK_Q_SYNC_DIR, TASK_Q_SYNC_DIR_TMP, TASK_RUN, TASK_FIN, TASK_CLEAN, TASK_HOME, TASK_SEND, TASK_RECV, TASK_UNKNOWN}; /*�������*/  /*add TASK_Q_SYNC_DIR_REQ TASK_Q_SYNC_DIR_RSP for thread sync dir */

enum {TASK_ADDFILE = '0', TASK_DELFILE, TASK_MODFILE, TASK_LINKFILE, TASK_SYNCDIR};  /* �������� */

enum {OVER_UNKNOWN = 0, OVER_OK, OVER_E_MD5, OVER_PEERERR, TASK_EXIST, OVER_PEERCLOSE, OVER_UNLINK, OVER_TIMEOUT, OVER_MALLOC, OVER_SRC_DOMAIN_ERR, OVER_SRC_IP_OFFLINE, OVER_E_OPEN_SRCFILE, OVER_E_OPEN_DSTFILE, OVER_E_IP, OVER_E_TYPE, OVER_SEND_LEN, OVER_TOO_MANY_TRY, OVER_DISK_ERR, OVER_LAST};  /*�������ʱ��״̬*/

enum {GET_TASK_ERR = -1, GET_TASK_OK, GET_TASK_NOTHING};  /*��ָ������ȡ����Ľ��*/

enum {TASK_DST = 0, TASK_SOURCE, TASK_SRC_NOSYNC, TASK_SYNC_ISDIR, TASK_SYNC_VOSS_FILE}; /*���������Ƿ���Ҫ��ͬ�����ͬ�� */

extern const char *task_status[TASK_UNKNOWN]; 
extern const char *over_status[OVER_LAST]; 
typedef struct {
	char url[1024];
	char filename[256];
	char tmpfile[256];
	char filemd5[33];
	char src_domain[16];
	off_t offsize;
	uint32_t dstip;    /*��������Ŀ��ip����GET����PUT_FROMʱ����ip�Ǳ���ip*/
	time_t starttime;
	time_t mtime;
	time_t ctime;
	off_t fsize;
	char okindex;
	char iscallback;
	char type;         /*�ļ��任���ͣ�ɾ����������*/
	int8_t retry;     /*����ִ��ʧ��ʱ�����������Ƿ�ִ�����·��������Ѿ����Դ��������ܳ����趨���Դ���*/
	uint8_t overstatus; /*����״̬*/
	uint8_t isp;     /*voss sync ʹ��*/
	mode_t fmode;   /*stupid Ϊ�˼��ݾɰ汾��û��������*/
//	char bk[7];
}t_task_base;

typedef struct {
	off_t processlen; /*��Ҫ��ȡ���߷��͵����ݳ���*/
	off_t lastlen; /*��һ������ ����ĳ��ȣ���ʼ��Ϊ0 */
	time_t   lasttime; /*�ϸ�����ʱ���*/
	time_t   starttime; /*��ʼʱ���*/
	time_t	 endtime; /*����ʱ���*/
	char peerip[16];  /*�Զ�ip*/
	uint16_t peerport;
	uint8_t oper_type; /*�Ǵ�FCS GET�ļ���������ͬ��CS  GET�ļ� */
	uint8_t need_sync; /*TASK_SOURCE����������Դͷ��TASK_DST����������Ŀ��֮һ */
	uint8_t sync_dir;  /**/
	uint8_t isp;      /**/
	uint8_t archive_isp;      /**/
	uint8_t bk[3];
}t_task_sub;

typedef struct {
	t_task_base base;
	t_task_sub  sub;
	void *user;
}t_vfs_taskinfo;

typedef struct {
	t_vfs_taskinfo task;
	list_head_t llist;
	list_head_t hlist;
	list_head_t userlist;
	uint32_t upip;
	uint8_t status;
	uint8_t bk[3];
} t_vfs_tasklist;

typedef struct {
	char domain[64];
	int d1;
	int d2;
	uint32_t ip;
	time_t task_stime; /*for time_out */
	time_t starttime; /*ͬ����ʼʱ��� ��CS��˵����д Ŀ¼�ϴ�ͬ����ʱ�������FCS��˵����д 0*/ 
	time_t endtime; /*ͬ������ʱ��㣬��CS��˵ ��д 0����FCS��˵����дFCS���һ��������ʱ��� */ 
	char type;  /*ADDFILE, DELFILE ɾ��ͬ����ֻ��CS�Żᷢ������FCS���ᷢ��ɾ��ͬ��*/
} t_vfs_sync_task;

typedef struct {
	uint8_t trycount;
	t_vfs_sync_task sync_task;
	list_head_t list;
} t_vfs_sync_list;

typedef void (*timeout_task)(t_vfs_tasklist *task);

int vfs_get_task(t_vfs_tasklist **task, int status);

int vfs_set_task(t_vfs_tasklist *task, int status);

int init_task_info();

int add_task_to_alltask(t_vfs_tasklist *task);

int check_task_from_alltask(t_task_base *base, t_task_sub *sub);

int mod_task_level(char *filename, int type);

int get_task_from_alltask(t_vfs_tasklist **task, t_task_base *base);

int get_timeout_task_from_alltask(int timeout, timeout_task cb);

int scan_some_status_task(int status, timeout_task cb);

int get_task_count(int status);

void do_timeout_task();

void report_2_nm();

#endif
