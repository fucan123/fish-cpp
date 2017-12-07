#define ARTICLE_COIN      1
#define ARTICLE_LEVEL_EXP 2
#define ARTICLE_VIP_EXP   3
#define ARTICLE_ICE       4

typedef struct struct_task_condition {
	int param;       //��������
	int param2;      //��������2
	int type;        //��������
	struct_task_condition* next;
} S_TaskCondition;

typedef struct struct_task {
	int id;                      //ID
	int pre_id;                  //ǰ������ID
	int group_id;                //������
	int mutex_id;                //������
	int priority;                //���ȼ�(Խ��Խ����)
	int auto_give;               //�Ƿ��Զ�����
	int auto_use;                //�Ƿ��Զ�ʹ��
	int reapt;                   //�Ƿ���ظ����
	int start_time;              //��ʼʱ��
	int end_time;                //����ʱ��
	int article_ids[10];         //������Ʒ
	S_TaskCondition* condition;  //�����������
	struct_task* next;
} S_Task;

typedef struct struct_article {
	int id;    //ID
	int param; //����
	int type;  //����(1-��ҿ�, 2-�ȼ����鿨, 3-VIP���鿨, 4-������)
	struct_article* next;
} S_Article;

class Task {
private:
	S_Task* _tasks;
	S_Article* _article;
protected:
	bool conditionIsTrue(int param, int param2, int type, Player*, S_Task*);
	int giveArticle(User* user, S_Article* article);
public:
	inline S_Task* getTasks(); //��������
	S_Task* find(int id); //��������(id=����ID)
	bool hasMutex(int id, int mutex_id); //���Ҵ������Ƿ�����ͬ����������������
	bool complete(int param, int param2, int type, Player* player);
	void load(bool first=1);
};

S_Task* Task::getTasks() {
	return this->_tasks;
}

S_Task* Task::find(int id) {
	S_Task* data = NULL;
	S_Task* task = this->_tasks;
	while (task) {
		if (task->id == id) {
			data = task;
			break;
		}
		task = task->next;
	}
	return data;
}

bool Task::hasMutex(int id, int mutex_id) {
	S_Task* mutext_task = this->find(id);
	return mutext_task ? mutext_task->mutex_id == mutex_id : false;
}

bool Task::conditionIsTrue(int param, int param2, int type, Player* player, S_Task* task) {
	bool type_exists = false, completed = true;
	S_TaskCondition* condi = task->condition;
	while (condi) {
		if (condi->type == type) {
			type_exists = true;
			break;
		}
		condi = condi->next;
	}
	if (!type_exists) {
		return false;
	}

	condi = task->condition;
	while (condi) {
		switch (condi->type) {
		case 2:
			completed = completed && ((User*)(player->user))->checkKillFish(condi->param2, condi->param);
			break;
		case 3:
			completed = completed && ((User*)(player->user))->checkTask(condi->param2, condi->param);
			break;
		default:
			completed = false;
			break;
		}
		if (!completed) {
			break;
		}
		condi = condi->next;
	}
	return completed;
}

int Task::giveArticle(User* user, S_Article* article) {
	switch (article->type) {
	case ARTICLE_COIN: //��ҿ�
		user->coin += article->param;
		break;
	case ARTICLE_LEVEL_EXP: //�ȼ����鿨
		user->level_exp += article->param;
		break;
	case ARTICLE_VIP_EXP: //vip���鿨
		user->vip_exp += article->param;
		break;
	case ARTICLE_ICE: //������
		break;
	default:
		break;
	}
	return 0;
}

bool Task::complete(int param, int param2, int type, Player* player) {
	User* user = (User*)player->user;
	S_UserTask* user_task = user->getTask();
	long time = gettime();
	S_Task* task = this->_tasks;
	while (task) {
		if (task->start_time > time || task->end_time < time) { //������Ч��
			continue;
		}

		bool completed = this->conditionIsTrue(param, param2, type, player, task);
		S_TaskCondition* condition = task->condition;
		while (condition) {
			completed = param >= condition->param && param2 == condition->param2 && type == condition->type;
			if (!completed) {
				break;
			}
			if (type == condition->type) {

			}
			condition = condition->next;
		}
		if (completed) { //��������
			if (!task->reapt) { //�����ظ����
				while (user_task) {
					if (user_task->task_id == task->id) { //����ɴ�����
						completed = false;
						break;
					}
					if (task->mutex_id > 0 && user_task->mutex_id == task->mutex_id) { //����ͬ���������������
						completed = false;
						break;
					}
					user_task = user_task->next;
				}
			}
			if (completed) { //�����
				int article_i = 10;
				for (int i = 0; i < article_i; i++) { //ѭ��Ҫ���͵���Ʒ
					if (task->article_ids[i] == 0) { //��ƷID������(�Ժ��Ҳ�������)
						break;
					}
					S_Article* article = this->_article; //������Ʒ�б�
					while (article) {
						if (article->id == task->article_ids[i]) { //�ҵ�����ƷID
							this->giveArticle(user, article);

							S_UserTask* user_task_new = (S_UserTask*)_malloc(sizeof(S_UserTask));
							user_task_new->id = 0;
							user_task_new->task_id = task->id;
							user_task_new->group_id = task->group_id;
							user_task_new->mutex_id = task->mutex_id;
							user_task_new->status = task->auto_give;
							user_task_new->is_new = 1;
							user->addTask(user_task_new);
							
							break;
						}
						article = article->next;
					}
				}
			}
		}

		task = task->next;
	}
	return true;
}

void Task::load(bool first) {
	this->_tasks = NULL;
	this->_article = NULL;

	char sql[] = 
		"SELECT  "\
		"id,pre_id,group_id,mutex_id,priority,auto_give,auto_use,reapt,start_time,end_time,article_ids"\
		" FROM tasks WHERE status=1 ORDER BY priority DESC";
	int error = mysql_real_query(Mysql::$class->getConn(), sql, strlen(sql));
	if (error) {
		return;
	}

	MYSQL_RES* result = mysql_store_result(Mysql::$class->getConn());
	if (result == NULL) {
		return;
	}

	if (!first) {

	}

	S_Task** tmp_task = &this->_tasks;

	long time = gettime();
	MYSQL_ROW row;
	for (int i = 0; row = mysql_fetch_row(result); i++) {
		int start_time = atoi(row[8]), end_time = atoi(row[9]);
		if (start_time > 0 && start_time > time) { //δ��ʼ
			printf("δ��ʼ: %d.\n", time);
			continue;
		}
		if (end_time > 0 && end_time < time) { //�ѽ���
			printf("�ѽ���: %d, end time: %d.\n", time, end_time);
			continue;
		}

		S_Task* task = (S_Task*)_malloc(sizeof(S_Task));
		task->id = atoi(row[0]);        //ID
		task->pre_id = atoi(row[1]);    //����ǰ��ID
		task->group_id = atoi(row[2]);  //������
		task->mutex_id = atoi(row[3]);  //������
		task->priority = atoi(row[4]);  //���ȼ�
		task->auto_give = atoi(row[5]); //�Ƿ��Զ�������Ʒ
		task->auto_use = atoi(row[6]);  //�Ƿ��Զ�ʹ����Ʒ
		task->reapt = atoi(row[7]);     //�Ƿ���ظ����
		task->start_time = start_time;  //��Ч��ʼʱ��
		task->end_time = end_time;      //��Ч����ʱ��
		task->condition = NULL;         //���������������
		task->next = NULL;              //��һ������

		int x = 0, n = 0;
		char* atids = row[10]; //������Ʒ��ID, ��ʽ(1,2,3)
		while (true) {
			if (*atids >= 48 && *atids <= 57) { //������
				n = (n * 10) + (*atids - 48);   //ת������
			}
			if (*atids == ',' || *atids == NULL) { //�����ָ������ѽ���
				task->article_ids[x] = n; //��ӵ���ƷID����
				//printf("��ƷID: %d\n", n);
				x++;   //����+1
				n = 0; //ID������
				if (*atids == NULL) { //�ѽ���, ��ֹ
					while (x < 10) { //��ʣ���������Ϊ0
						task->article_ids[x] = 0;
						x++;
					}
					break;
				}
			}
			atids++;
		}

		/**********��ѯ����������������**********/
		char sql2[100] = "SELECT param,param2,type FROM task_conditions WHERE task_id=";
		strcat(sql2, row[0]);
		int error = mysql_real_query(Mysql::$class->getConn(), sql2, strlen(sql2));
		if (error) {
			_free(task);
			continue;
		}
		MYSQL_RES* result2 = mysql_store_result(Mysql::$class->getConn());
		if (result2 == NULL) {
			_free(task);
			continue;
		}

		S_TaskCondition** tmp_condition = &task->condition;
		MYSQL_ROW row2;
		for (int j = 0; row2 = mysql_fetch_row(result2); j++) {
			S_TaskCondition* condition = (S_TaskCondition*)_malloc(sizeof(S_TaskCondition));
			condition->param = atoi(row2[0]);  //����1
			condition->param2 = atoi(row2[1]); //����2
			condition->type = atoi(row2[2]);   //��������
			condition->next = NULL;

			*tmp_condition = condition; //��ӵ�����
			tmp_condition = &((*tmp_condition)->next);
		}
		mysql_free_result(result2);
		/**********��ѯ���������������� END**********/

		if (task->condition == NULL) { //û������
			_free(task);
			continue;
		}

		*tmp_task = task; //��ӵ�����
		tmp_task = &((*tmp_task)->next);
	}
	/**********������Ʒ**********/
	char sql_article[] = "SELECT id,param,type FROM articles WHERE status=1";
	if (!mysql_real_query(Mysql::$class->getConn(), sql_article, strlen(sql_article))) { //û�д���
		MYSQL_RES* result_article = mysql_store_result(Mysql::$class->getConn());
	}
	MYSQL_RES* result_article = mysql_store_result(Mysql::$class->getConn());
	if (result_article) {
		S_Article** tmp_article = &this->_article;
		MYSQL_ROW row2;
		for (int j = 0; row2 = mysql_fetch_row(result_article); j++) {
			S_Article* article = (S_Article*)_malloc(sizeof(S_Article));
			article->id = atoi(row2[0]);  //����1
			article->param = atoi(row2[1]); //����2
			article->type = atoi(row2[2]);   //��������
			article->next = NULL;

			*tmp_article = article; //��ӵ�����
			tmp_article = &((*tmp_article)->next);
		}
		mysql_free_result(result_article);
	}
	/**********������Ʒ END**********/
	//printf("�������.\n");
	mysql_free_result(result);
}