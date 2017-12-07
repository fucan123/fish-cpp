typedef struct struct_user_privilege {
	bool view;  //�鿴�����û�Ȩ��
	bool kick;  //���������
	bool exit;  //��ֹ������
} S_UserPrivilege;

typedef struct struct_user_task{
	int id, task_id, group_id, mutex_id;
	int status;
	int is_new; //����Ƿ���������ɵ�, ���ڲ���˼�¼�����ݿ�
	struct_user_task* next;
} S_UserTask;

typedef struct struct_user_kill_fish {
	int id;
	int no;    //����
	int value; //��ɱ����
	int total;
	int is_change;
	struct_user_kill_fish* next;
} S_UserKillFish;

class User {
private:
	S_UserTask* task;
	S_UserKillFish* killfish;
public:
	int id;               //�û�ID
	int group_id;         //�û���
	int coin;             //�û����
	int backup_coin;      //ԭ�û����(�û�׬ȡ�Ľ��=coin-backup_coin)
	char* name;           //�û�����
	int level_exp;        //�û��ȼ�����
	int vip_exp;          //�û�VIP����

	int task_total;       //�û������������
	int killfish_total;   //�û����л�ɱ�������

	int op_time;          //������ʱ��
	S_UserPrivilege privilege; //Ȩ��
public:
	inline S_UserTask* getTask();
	int  init(char* sign_key);
	int  getDbCoin(); //��ȡ���ݱ��еĽ�����
	void killFish(int no); //��ɱ��һ�����Ϊno����
	bool checkKillFish(int no, int need_num);
	bool checkTask(int group_id, int need_num);
	S_UserTask* addTask(S_UserTask* user_task);
	void save();  //������Ϣ
	void leave(); //�뿪����
	void destory();
};

S_UserTask* User::getTask() {
	return this->task;
}

int User::init(char* sign_key) {
	this->killfish_total = 0;
	this->task_total = 0;
	this->coin = 0;
	this->backup_coin = 0;
	this->name = NULL;
	this->task = NULL;
	this->killfish = NULL;
	this->op_time = gettime();
	this->privilege = { false, false, false };

	mysql_ping(Mysql::$class->getConn());
	/******�û�������Ϣ��******/
	char sql_basic[256];
	char sql_basic_f[] = "SELECT "\
		"u.id,u.group_id,u.name,u.nick,u.level_exp,u.vip_exp"\
		" FROM user_sign AS us INNER JOIN users AS u ON u.id=us.user_id WHERE us.sign_key='%s'";
	sprintf(sql_basic, sql_basic_f, sign_key);
	MYSQL_ROW basic = Mysql::$class->getRow(sql_basic);
	//printf("%s(%d), %p\n", sql_basic, strlen(sql_basic), basic);
	if (basic && strlen(basic[0]) > 0) {
		this->id = atoi(basic[0]);
		this->group_id = atoi(basic[1]);
		int index = strlen(basic[3]) > 0 ? 3 : 2;
		int len = strlen(basic[index]);
		if (len > 0) {
			this->name = (char*)_malloc(len + 1);
			memcpy(this->name, basic[index], len + 1);
		}
		this->level_exp = atoi(basic[4]);
		this->vip_exp = atoi(basic[5]);
		//printf("name: %s.\n", this->name);
		if (this->group_id == -1) {
			this->privilege.view = true;
			this->privilege.kick = true;
			this->privilege.exit = true;
		}
		else if (this->group_id > 0) {
			char sql_p[128];
			sprintf(sql_p, "SELECT id FROM user_privileges WHERE user_group_id=%d AND privilege_key='%s'", this->group_id, "game_view_user");
			if (Mysql::$class->getItem(sql_p)) {
				this->privilege.view = true;
			}
			sprintf(sql_p, "SELECT id FROM user_privileges WHERE user_group_id=%d AND privilege_key='%s'", this->group_id, "game_kick");
			if (Mysql::$class->getItem(sql_p)) {
				this->privilege.kick = true;
			}
		}
	}
	else {
		return 0;
	}
	/******�û�������Ϣ��******/
	char uid_str[12];
	sprintf(uid_str, "%d", this->id);
	/******�û��Ѵ��ڵ������******/
	char sql_task[200] = "SELECT a.id,a.task_id,a.status,b.group_id,b.mutex_id FROM user_tasks AS a WHERE INNER JOIN tasks b ON b.id=a.task_id a.user_id=";
	strcat(sql_task, uid_str);
	if (!mysql_real_query(Mysql::$class->getConn(), sql_task, strlen(sql_task))) { //û�д���
		MYSQL_RES* result_task = mysql_store_result(Mysql::$class->getConn());
		if (result_task) { //�н��
			S_UserTask** tmp_task = &this->task;
			MYSQL_ROW row_task;
			for (int j = 0; row_task = mysql_fetch_row(result_task); j++) {
				S_UserTask* task_x = (S_UserTask*)_malloc(sizeof(S_UserTask));
				task_x->id = atoi(row_task[0]);       //ID
				task_x->task_id = atoi(row_task[1]);  //����ID
				task_x->status = atoi(row_task[2]);   //���״̬(0-�����, 1-�����[����ֻҪ�д˼�¼, �Ͳ�������¼�¼])
				task_x->group_id = atoi(row_task[3]); //������
				task_x->mutex_id = atoi(row_task[4]); //������
				task_x->is_new = 0;
				task_x->next = NULL;

				*tmp_task = task_x; //��ӵ�����
				tmp_task = &((*tmp_task)->next);

				this->task_total++;
			}
			mysql_free_result(result_task);
		}
	}
	/******�û��Ѵ��ڵ������******/
	/******�û��ѻ�ɱ����������******/
	char sql_value[100] = "SELECT id,param,value FROM user_values WHERE type='kill_fish' AND user_id=";
	strcat(sql_value, uid_str);
	if (!mysql_real_query(Mysql::$class->getConn(), sql_value, strlen(sql_value))) { //û�д���
		MYSQL_RES* result_killfish = mysql_store_result(Mysql::$class->getConn());
		if (result_killfish) { //�н��
			S_UserKillFish** tmp_killfish = &this->killfish;
			MYSQL_ROW row_killfish;
			for (int j = 0; row_killfish = mysql_fetch_row(result_killfish); j++) {
				int no = atoi(row_killfish[1]);
				if (!no) {
					continue;
				}
				S_UserKillFish* killfish_x = (S_UserKillFish*)_malloc(sizeof(S_UserKillFish));
				killfish_x->id = atoi(row_killfish[0]);
				killfish_x->no = no;      //����
				killfish_x->value = atoi(row_killfish[2]);   //��ɱ����
				killfish_x->is_change = 0;
				killfish_x->next = NULL;

				*tmp_killfish = killfish_x; //��ӵ�����
				tmp_killfish = &((*tmp_killfish)->next);

				this->killfish_total += killfish_x->value;
			}
		}
	}
	/******�û��ѻ�ɱ����������******/
	this->coin = this->getDbCoin();
	this->backup_coin = this->coin;
	return this->id;
}

int User::getDbCoin() {
	char sql[128];
	sprintf(sql, "SELECT balance FROM user_coin WHERE user_id=%d", this->id);
	return Mysql::$class->getItemToInt(sql);
}

void User::killFish(int no) {
	S_UserKillFish** ptr = &this->killfish;
	while (*ptr) {
		if ((*ptr)->no == no) {
			(*ptr)->value += 1;
			(*ptr)->is_change = 1;
			return;
		}
		ptr = &((*ptr)->next);
	}

	*ptr = (S_UserKillFish*)_malloc(sizeof(S_UserKillFish));
	(*ptr)->id = 0;
	(*ptr)->no = no;
	(*ptr)->value = 1;
	(*ptr)->is_change = 1;
	(*ptr)->next = NULL;
}

bool User::checkKillFish(int no, int need_num) {
	if (no == 0) {
		return this->killfish_total >= need_num;
	}
	S_UserKillFish** ptr = &this->killfish;
	while (*ptr) {
		if ((*ptr)->no == no) {
			return (*ptr)->value >= need_num;
		}
		ptr = &((*ptr)->next);
	}
	return false;
}

bool User::checkTask(int group_id, int need_num) {
	int num = 0;
	if (group_id <= 0) {
		num = this->task_total;
	}
	else {
		S_UserTask* task = this->task;
		while (task) {
			if (task->group_id == group_id) {
				num++;
			}
			task = task->next;
		}
	}
	return num >= need_num;
}

S_UserTask* User::addTask(S_UserTask* user_task) {
	S_UserTask** task = &this->task;
	while (*task) {
		task = &((*task)->next);
	}
	*task = user_task;

	this->task_total += 1;

	return user_task;
}

void User::save() {
	if (!this->id) {
		return;
	}
	MYSQL* mysql = Mysql::$class->getConn();
	mysql_ping(mysql);
	this->op_time = gettime();
	/******�����û���Ϣ��******/
	char sql[256];
	sprintf(sql, "UPDATE users SET op_time=%d WHERE id=%d", 
		this->op_time, this->id);
	mysql_query(mysql, sql);
	/******�����û���Ϣ��******/
	/******�����û�����******/
	if (this->coin != this->backup_coin) { //����б䶯
		sprintf(sql, "SELECT balance FROM user_coin WHERE user_id=%d", this->id);
		int balance = Mysql::$class->getItemToInt(sql);
		int win_coin = this->coin - this->backup_coin; //׬ȡ�Ľ��
		this->coin = balance + win_coin;
		this->backup_coin = this->coin;
		//�������
		sprintf(sql, "UPDATE user_coin SET balance=%d WHERE user_id=%d", balance + win_coin, this->id);
		mysql_query(mysql, sql);
		//�������䶯�ռ� 0xe5, 0x86, 0x85,->��
		char remark[] = {0xe6, 0xb8, 0xb8, 0xe6, 0x88, 0x8f, 0xe5, 0x8f, 0x98, 0xe5, 0x8A, 0xA8, 0}; //�ַ�'��Ϸ�䶯'��utf8����
		sprintf(sql, "INSERT INTO user_coin_logs(user_id,op_user_id,count,balance,`remark`,type,created_at) VALUES (%d,%d,%d,%d,'%s','%s',%d)",
			this->id, this->id, win_coin, balance + win_coin, remark, "game", this->op_time);
		printf("sql:%s(%d)\n", sql, strlen(sql));
		mysql_query(mysql, sql);
	}
	/******�����û�����******/
	/******��������ɵ������******/
	S_UserTask* utask = this->task;
	while (utask) {
		if (utask->is_new) { //����ɵ�����
			sprintf(sql, "INSERT user_tasks (user_id,task_id,status,created_at,updated_at) VALUES (%d,%d,%d,%d,%d)", 
				this->id, utask->task_id, utask->status, this->op_time, this->op_time);
			mysql_query(mysql, sql);
		}
		utask = utask->next;
	}
	/******��������ɵ������******/
	/******���»�ɱ���������MYSQL��******/
	S_UserKillFish* kfish = this->killfish;
	while (kfish) {
		if (kfish->is_change) {
			if (kfish->id) { //��idΪ����
				sprintf(sql, "UPDATE user_values SET value=%d WHERE id=%d", kfish->value, kfish->id);
			}
			else { //û���²���һ��
				sprintf(sql, "INSERT user_values (user_id,value,param,type) VALUES (%d,%d,%d,'kill_fish')", this->id, kfish->value, kfish->no);
			}
			kfish->is_change = 0;
			mysql_query(mysql, sql);
		}
		kfish = kfish->next;
	}
	/******���»�ɱ���������MYSQL��******/
	//this->destory();
}

void User::leave() {}

void User::destory() {
	if (this->name) {
		_free(this->name);
		printf("free user name.\n");
	}
	/******�ͷ�task�����ڴ��******/
	S_UserTask* task_x = this->task;
	while (task_x) {
		S_UserTask* tmp_task = task_x;
		task_x = task->next;
		_free(tmp_task);
	}
	this->task = NULL;
	/******�ͷ�task�����ڴ��******/
	/******�ͷ�killfish�ڴ��******/
	S_UserKillFish* killfish_x = this->killfish;
	while (killfish_x) {
		S_UserKillFish* tmp_killfish = killfish_x;
		killfish_x = killfish_x->next;
		_free(tmp_killfish);
	}
	this->killfish = NULL;
	/******�ͷ�killfish�ڴ��******/
	printf("user->destory\n");
}