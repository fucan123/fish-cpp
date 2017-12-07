#define ARTICLE_COIN      1
#define ARTICLE_LEVEL_EXP 2
#define ARTICLE_VIP_EXP   3
#define ARTICLE_ICE       4

typedef struct struct_task_condition {
	int param;       //条件参数
	int param2;      //条件参数2
	int type;        //条件类型
	struct_task_condition* next;
} S_TaskCondition;

typedef struct struct_task {
	int id;                      //ID
	int pre_id;                  //前置任务ID
	int group_id;                //任务组
	int mutex_id;                //互斥组
	int priority;                //优先级(越大越优先)
	int auto_give;               //是否自动赠送
	int auto_use;                //是否自动使用
	int reapt;                   //是否可重复完成
	int start_time;              //开始时间
	int end_time;                //结束时间
	int article_ids[10];         //赠送物品
	S_TaskCondition* condition;  //完成所需条件
	struct_task* next;
} S_Task;

typedef struct struct_article {
	int id;    //ID
	int param; //参数
	int type;  //类型(1-金币卡, 2-等级经验卡, 3-VIP经验卡, 4-冰冻卡)
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
	inline S_Task* getTasks(); //任务链表
	S_Task* find(int id); //查找任务(id=任务ID)
	bool hasMutex(int id, int mutex_id); //查找此任务是否已有同属互斥组的任务完成
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
	case ARTICLE_COIN: //金币卡
		user->coin += article->param;
		break;
	case ARTICLE_LEVEL_EXP: //等级经验卡
		user->level_exp += article->param;
		break;
	case ARTICLE_VIP_EXP: //vip经验卡
		user->vip_exp += article->param;
		break;
	case ARTICLE_ICE: //冰冻卡
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
		if (task->start_time > time || task->end_time < time) { //不在有效期
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
		if (completed) { //任务可完成
			if (!task->reapt) { //不可重复完成
				while (user_task) {
					if (user_task->task_id == task->id) { //已完成此任务
						completed = false;
						break;
					}
					if (task->mutex_id > 0 && user_task->mutex_id == task->mutex_id) { //已有同属互斥组任务完成
						completed = false;
						break;
					}
					user_task = user_task->next;
				}
			}
			if (completed) { //可完成
				int article_i = 10;
				for (int i = 0; i < article_i; i++) { //循环要赠送的物品
					if (task->article_ids[i] == 0) { //物品ID不存在(以后的也不会存在)
						break;
					}
					S_Article* article = this->_article; //所有物品列表
					while (article) {
						if (article->id == task->article_ids[i]) { //找到此物品ID
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
		if (start_time > 0 && start_time > time) { //未开始
			printf("未开始: %d.\n", time);
			continue;
		}
		if (end_time > 0 && end_time < time) { //已结束
			printf("已结束: %d, end time: %d.\n", time, end_time);
			continue;
		}

		S_Task* task = (S_Task*)_malloc(sizeof(S_Task));
		task->id = atoi(row[0]);        //ID
		task->pre_id = atoi(row[1]);    //任务前置ID
		task->group_id = atoi(row[2]);  //任务组
		task->mutex_id = atoi(row[3]);  //互斥组
		task->priority = atoi(row[4]);  //优先级
		task->auto_give = atoi(row[5]); //是否自动赠送物品
		task->auto_use = atoi(row[6]);  //是否自动使用物品
		task->reapt = atoi(row[7]);     //是否可重复完成
		task->start_time = start_time;  //有效开始时间
		task->end_time = end_time;      //有效结束时间
		task->condition = NULL;         //任务完成所需条件
		task->next = NULL;              //下一个任务

		int x = 0, n = 0;
		char* atids = row[10]; //赠送物品的ID, 格式(1,2,3)
		while (true) {
			if (*atids >= 48 && *atids <= 57) { //是数字
				n = (n * 10) + (*atids - 48);   //转成数字
			}
			if (*atids == ',' || *atids == NULL) { //遇到分隔符或已结束
				task->article_ids[x] = n; //添加到物品ID数组
				//printf("物品ID: %d\n", n);
				x++;   //索引+1
				n = 0; //ID数重置
				if (*atids == NULL) { //已结束, 终止
					while (x < 10) { //把剩余的数组置为0
						task->article_ids[x] = 0;
						x++;
					}
					break;
				}
			}
			atids++;
		}

		/**********查询任务完成所需的条件**********/
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
			condition->param = atoi(row2[0]);  //参数1
			condition->param2 = atoi(row2[1]); //参数2
			condition->type = atoi(row2[2]);   //条件类型
			condition->next = NULL;

			*tmp_condition = condition; //添加到链表
			tmp_condition = &((*tmp_condition)->next);
		}
		mysql_free_result(result2);
		/**********查询任务完成所需的条件 END**********/

		if (task->condition == NULL) { //没有条件
			_free(task);
			continue;
		}

		*tmp_task = task; //添加到链表
		tmp_task = &((*tmp_task)->next);
	}
	/**********加载物品**********/
	char sql_article[] = "SELECT id,param,type FROM articles WHERE status=1";
	if (!mysql_real_query(Mysql::$class->getConn(), sql_article, strlen(sql_article))) { //没有错误
		MYSQL_RES* result_article = mysql_store_result(Mysql::$class->getConn());
	}
	MYSQL_RES* result_article = mysql_store_result(Mysql::$class->getConn());
	if (result_article) {
		S_Article** tmp_article = &this->_article;
		MYSQL_ROW row2;
		for (int j = 0; row2 = mysql_fetch_row(result_article); j++) {
			S_Article* article = (S_Article*)_malloc(sizeof(S_Article));
			article->id = atoi(row2[0]);  //参数1
			article->param = atoi(row2[1]); //参数2
			article->type = atoi(row2[2]);   //条件类型
			article->next = NULL;

			*tmp_article = article; //添加到链表
			tmp_article = &((*tmp_article)->next);
		}
		mysql_free_result(result_article);
	}
	/**********加载物品 END**********/
	//printf("加载完成.\n");
	mysql_free_result(result);
}