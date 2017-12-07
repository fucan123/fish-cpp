typedef struct struct_setting_system {
	char* key;                   //键[表中type字段]
	char* value;                 //值[表中value字段]
	struct_setting_system* next; //NEXT
} S_Setting_System;              //系统配置setting表中的条目

typedef struct struct_setting_line {
	int left;                  //左值[表中type字段]
	int right;                 //右值[表中value字段]
	struct_setting_line* next; //NEXT
} S_Setting_Line;              //这样的配置(1:2\r\n3:4[每行一条]), left=1 or 3, right=2 or 4

typedef struct struct_setting_value {
	int no_verify_account_timout; //没有验证帐号允许的最大时间
	int no_game_timeout;          //没有进行游戏允许的最大时间
	int no_op_timeout;            //游戏中操作间隔不能超过的时间
} S_Setting_Value;

typedef struct struct_setting_fish {
	int id;       //ID[表中ID字段]
	int no;       //编号[表中no字段]
	int coin;     //击杀得分[表中coin字段]
} S_Setting_Fish; //鱼的设置fishs表中的条目

class Setting {
private:
	S_Setting_Value*  _value;
	S_Setting_System* _system;
	S_Setting_Line* _level_list;
	S_Setting_Line* _vip_level_list;
	int _fish_qty;
	S_Setting_Fish* _fish;
public:
	static Setting* $class;
public:
	bool closeing; //是否关闭中(关闭中将不处理任何信息)
public:
	Setting();
	~Setting();
	void loadSystem(); //加载系统设置
	void loadFish();   //加载鱼的设置
	char* system(char* key) const; //获取某个设置(key键值)
	int   systemToInt(char* key, int dval=0); //获取某个设置(key键值)
	inline S_Setting_Value* values(); //values
	inline S_Setting_Fish* getFish(); //获取鱼的链表头
	inline S_Setting_Fish* getFish(int index); //获取其中的某一条[index=索引]
	inline int getFishQty(); //获取有效设置鱼的数量
	bool setValue(char* key, char* value); //设置key的值
	bool checkKey(char* key); //检查key是否有效
	void formatLine(char* text, S_Setting_Line** dst); //格式化("1:2\r\n3:3\r\n")这样的设置
	void order(S_Setting_Line* ptr, int dir = 1, int desc = 1); //排序, ptr=要排序的链表, dir[1=根据left, 2-根据right], desc[0-升序, 1-倒序]
	void freeLine(S_Setting_Line** dst);
};

Setting* Setting::$class = 0;

Setting::Setting() {
	this->closeing = false;
	this->loadFish();
	this->loadSystem();
}

char* Setting::system(char* key) const{
	S_Setting_System* ptr = this->_system;
	while (ptr) {
		if (strcmp(ptr->key, key) == 0) {
			return ptr->value;
		}
		ptr = ptr->next;
	}
	return NULL;
}

int Setting::systemToInt(char* key, int dval) {
	char* value = this->system(key);
	return value ? atoi(value) : dval;
}

S_Setting_Value* Setting::values() {
	return this->_value;
}

S_Setting_Fish* Setting::getFish() {
	return this->_fish;
}

S_Setting_Fish* Setting::getFish(int index) {
	return index < this->_fish_qty ? &this->_fish[index] : NULL;
}

int Setting::getFishQty() {
	return this->_fish_qty;
}

void Setting::loadSystem() {
	this->_value = new S_Setting_Value;
	this->_system = NULL;
	this->_level_list = NULL;
	this->_vip_level_list = NULL;
	/******所有系统配置↓******/
	char sql[] = "SELECT type,value FROM setting";
	if (!mysql_real_query(Mysql::$class->getConn(), sql, strlen(sql))) { //没有错误
		MYSQL_RES* result = mysql_store_result(Mysql::$class->getConn());
		if (result) { //有结果
			S_Setting_System** tmp_setting = &this->_system;
			MYSQL_ROW row;
			for (int j = 0; row = mysql_fetch_row(result); j++) {

				int len0 = strlen(row[0]), len1 = strlen(row[1]);
				if (len0 > 0) { //key长度大于0才存入
					if (strcmp(row[0], "no_verify_account_timout") == 0) {
						this->_value->no_verify_account_timout = atoi(row[1]);
					}
					else if (strcmp(row[0], "no_game_timeout") == 0) {
						this->_value->no_game_timeout = atoi(row[1]);
					}
					else if (strcmp(row[0], "no_op_timeout") == 0) {
						this->_value->no_op_timeout = atoi(row[1]);
					}
					else if (strcmp(row[0], "user_level") == 0) {
						this->formatLine(row[1], &this->_level_list);
					}
					else if (strcmp(row[0], "user_vip_level") == 0) {
						this->formatLine(row[1], &this->_vip_level_list);
					}

					S_Setting_System* setting = new S_Setting_System;
					setting->key = new char[len0 + 1];
					setting->value = NULL;
					
					memcpy(setting->key, row[0], len0 + 1);
					if (len1 > 0) {
						setting->value = new char[len1 + 1];
						memcpy(setting->value, row[1], len1 + 1);
					}

					*tmp_setting = setting; //添加到链表
					tmp_setting = &((*tmp_setting)->next);
				}
			}
			mysql_free_result(result);
		}
	}
	this->order(this->_level_list, 1, 1);
	this->order(this->_vip_level_list, 1, 1);
	/******所有系统配置↑******/
}

void Setting::loadFish() {
	this->_fish_qty = 0;
	this->_fish = NULL;
	/******所有配置的鱼↓******/
	char sql[] = "SELECT id,no,coin FROM fishs WHERE status=1";
	if (!mysql_real_query(Mysql::$class->getConn(), sql, strlen(sql))) { //没有错误
		MYSQL_RES* result = mysql_store_result(Mysql::$class->getConn());
		if (result) { //有结果
			my_ulonglong num = mysql_num_rows(result);
			if (num > 0) {
				this->_fish = new S_Setting_Fish[num];
				MYSQL_ROW row;
				for (int j = 0; row = mysql_fetch_row(result); j++) {
					this->_fish[j].id = atoi(row[0]);
					this->_fish[j].no = atoi(row[1]);
					this->_fish[j].coin = atoi(row[2]);

					this->_fish_qty++;
				}
			}
			mysql_free_result(result);
		}
	}
	/******所有配置的鱼↑******/
}

bool Setting::setValue(char* key, char* value) {
	if (!this->checkKey(key)) {
		return false;
	}
	int len_key = strlen(key), len_value = strlen(value);
	if (len_key == 0) {
		return false;
	}
	S_Setting_System* ptr = this->_system;
	while (ptr) {
		if (strcmp(ptr->key, key) == 0) {
			if (ptr->value) delete[] ptr->value; //删除旧值占用的内存
			ptr->value = NULL;
			if (len_value > 0) {  //有值
				ptr->value = new char[len_value + 1];
				memcpy(ptr->value, value, len_value + 1);
			}
			goto end;
		}
		ptr = ptr->next;
	}
	//没有添加新的
	S_Setting_System* setting = new S_Setting_System;
	setting->key = new char[len_key + 1];
	setting->value = NULL;
	memcpy(setting->key, key, len_key + 1);
	if (len_value > 0) {  //有值
		setting->value = new char[len_value + 1];
		memcpy(setting->value, value, len_value + 1);
	}
end:
	if (strcmp(key, "user_level") == 0) {
		this->freeLine(&this->_level_list); //释放以前占用内存
		this->formatLine(key, &this->_level_list);
	}
	else if (strcmp(key, "user_vip_level") == 0) {
		this->freeLine(&this->_vip_level_list); //释放以前占用内存
		this->formatLine(key, &this->_vip_level_list);
	}
	return true;
}

bool Setting::checkKey(char* key) {
	if (!key) {
		return false;
	}
	if (strcmp(key, "no_verify_account_timeout") == 0) {
		return true;
	}
	if (strcmp(key, "no_game_timeout") == 0) {
		return true;
	}
	if (strcmp(key, "no_op_timeout") == 0) {
		return true;
	}
	if (strcmp(key, "user_level") == 0) {
		return true;
	}
	if (strcmp(key, "user_vip_level") == 0) {
		return true;
	}
	return false;
}

void Setting::formatLine(char* text, S_Setting_Line** dst) {
	S_Setting_Line** tmp_line = dst;
	char* ptr = text;
	while (*ptr) {
		int end_pos = 0;
		int line_pos = strpos(ptr, "\n", 0);
		int maohao_pos = strpos(ptr, ":", 0, line_pos > -1 ? line_pos : 0);
		if (maohao_pos > -1) {
			//printf("ptr:%c\n", *ptr);
			S_Setting_Line* line = new S_Setting_Line;
			line->left = 0;
			line->right = 0;
			line->next = NULL;

			*tmp_line = line; //添加到链表
			tmp_line = &(*tmp_line)->next;


			char* end_ptr = line_pos > -1 ? ptr + line_pos : 0;
			int left = 0, right = 0, i;
			for (i = 0; i < maohao_pos; i++) {
				if (*ptr >= 48 && *ptr <= 57) { //是数字
					line->left = (line->left * 10) + (*ptr - 48);   //转成数字
				}
				ptr++;
			}
			ptr++;
			while (*ptr) {
				if (ptr == end_ptr) {
					break;
				}
				if (*ptr >= 48 && *ptr <= 57) { //是数字
					line->right = (line->right * 10) + (*ptr - 48);   //转成数字
				}
				ptr++;
			}
			if (end_ptr) {
				ptr++;
			}
			//printf("left: %d, right: %d\n", left, right);
		}
		else {
			//printf("line pos: %d\n", line_pos);
			if (line_pos > -1) {
				ptr += line_pos;
			}
			ptr++;
		}
	}
}

void Setting::order(S_Setting_Line* ptr, int dir, int desc) {
	if (!ptr) {
		return;
	}
	S_Setting_Line* p = ptr;
	while (p->next) {
		//printf("fuck\n");
		S_Setting_Line* pp = ptr;
		while (pp) {
			//printf("fuck2\n");
			S_Setting_Line* pp2 = pp->next;
			if (pp2) {
				int v = pp->left, v2 = pp2->left;
				//printf("v:%d, v2:%d, desc:%d\n", v, v2, desc);
				if (dir == 2) {
					v = pp->right, v2 = pp2->right;
				}
				bool swap = !desc ? v > v2 : v < v2; //升序v大于v2才交换, 降序v小于v2才交换
				if (swap) { //1,2,3
					int tl = pp->left, tr = pp->right;
					pp->left = pp2->left;
					pp->right = pp2->right;
					pp2->left = tl;
					pp2->right = tr;
					//printf("swap\n");
				}
			}
			pp = pp->next;
		}
		p = p->next;
	}
}

void Setting::freeLine(S_Setting_Line** ptr) {
	S_Setting_Line* p = *ptr;
	while (p) {
		S_Setting_Line* tmp = p;
		p = p->next;

		delete tmp;
	}
	*ptr = NULL;
}
 
Setting::~Setting() {
	if (this->_fish) delete[] this->_fish; //释放鱼的内存
    /******释放系统设置内存↓******/
	S_Setting_System* system_ptr = this->_system;
	while (system_ptr) {
		S_Setting_System* tmp = system_ptr;
		if (system_ptr->key) delete[] system_ptr->key;
		if (system_ptr->value) delete[] system_ptr->value;
		system_ptr = system_ptr->next;

		delete tmp;
	}
	/******释放系统设置内存↑******/
	/******释放系统设置-用户等级配置内存↓******/
	this->freeLine(&this->_level_list);
	/******释放系统设置-用户等级配置内存↑******/
	/******释放系统设置-用户VIP等级配置内存↓******/
	this->freeLine(&this->_vip_level_list);
	/******释放系统设置-用户VIP等级配置内存↑******/
}