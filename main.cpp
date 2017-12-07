#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINDOWS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "win32fixes.c"
#include "header/common.h"
#include "init/cnf.hpp"
#include "init/memory.hpp"
#include "header/mystring.h"
#include "db/redis.hpp"
#include "db/mysql.hpp"
#include "init/setting.hpp"
#include "header/source.h"

#define PORT       6000 
#define MSGSIZE    1024
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "hiredis.lib")
#pragma comment(lib, "libmysql.lib")


int main() {
	Cnf* cnf = new Cnf;
	if (!cnf->parse("fish.cnf")) {
		system("pause" );
		return 0;
	}
	if (!cnf->check()) {
		system("pause");
		return 0;
	}
	int max_room = cnf->get_int("max_room");
	if (max_room <= 0) {
		max_room = 100;
	}
	rooms.setRooms(max_room);

	int memory_size = cnf->get_memory_size();
	//printf("m size: %d\n", memory_size);
	if (!memory_init(memory_size)) {
		printf("内存申请失败(%.2fKB)", (double)memory_size/1024.00);
		system("pause");
		return 0;
	}

	for (int i = 0; i < 100; i++) {
		//printf("r:%d", random(100, 120));
	}

	WSADATA wsaData;
	WSAStartup(0x0202, &wsaData);

	Redis::$class = new Redis(cnf->get("redis_host"), cnf->get_int("redis_port"));
	if (!Redis::_this()->redis()) {
		printf("连接Redis失败(地址: %s, 端口: %d).\n", cnf->get("redis_host"), cnf->get_int("redis_port"));
		system("pause");
		return 0;
	}
	if (Redis::_this()->redis()->err) {
		printf("连接Redis失败(地址: %s, 端口: %d); 错误信息: %s.\n", cnf->get("redis_host"), cnf->get_int("redis_port"), Redis::_this()->redis()->errstr);
		system("pause");
		return 0;
	}
	Redis::$class->set("x", "xx");

	Mysql::$class = new Mysql(cnf->get("mysql_host"), cnf->get("mysql_user"), cnf->get("mysql_pword"), cnf->get("mysql_dbase"), cnf->get_int("mysql_port"));
	if (Mysql::$class->error()) {
		printf("连接Mysql失败; 错误码: %d.\n", Mysql::$class->error());
		system("pause");
		return 0;
	}
	Mysql::$class->setChar(cnf->get("mysql_char"));

	Setting::$class = new Setting;
	//printf("fish qty: %d.\n", Setting::$class->getFishQty());
	if (Setting::$class->getFishQty() == 0) {
		printf("找不到鱼的配置, 请检查(Mysql数据库fishs里面是否有数据, 且status字段为1).\n", Mysql::$class->error());
		system("pause");
		return 0;
	}

	//printf("size: %d.\n", sizeof(Bullet));
	/*S_Setting_Fish* fishs = Setting::$class->getFish();
	for (int i = 0; i < Setting::$class->getFishQty(); i++) {
		printf("id:%d, no:%d, coin:%d.\n", fishs[i].id, fishs[i].no, fishs[i].coin);
	}
	S_Setting_Line* line = NULL;
	Setting::$class->formatLine("3:2\r\n1:2223\r\n2:0002\r\n", &line);
	Setting::$class->order(line, 1, 1);
	while (line) {
		printf("left: %d, right: %d\n", line->left, line->right);
		line = line->next;
	}*/

	char sql[] = "SELECT nick,name FROM p_member WHERE id=2";
	MYSQL_ROW row = Mysql::$class->getRow("SELECT id FROM users WHERE id=1 LIMIT 1");
	//MYSQL_ROW row2 = Mysql::$class->getRow("SELECT name FROM users WHERE id=1 LIMIT 1");
	//printf("row: %s\n", row[0]);
	char* p = Mysql::$class->getItem("SELECT id FROM users WHERE id=122 LIMIT 1");
	//printf("%p\n", p);
	/*Mysql::$class->setLock();
	Mysql::$class->getOrItem(sql, 3);
	Mysql::$class->freeLock();
	char* name = Mysql::$class->getOrItem(sql, 3);
	printf("name:%s\n", name);
	
	MYSQL_ROW row = Mysql::$class->getRow(sql);

	char sql2[] = "SELECT nick,name FROM p_member WHERE id=3";
	MYSQL_ROW row2 = Mysql::$class->getRow(sql2);
	
	printf("row[1]: %s\n", row);

	Task task;
	task.load();
	S_Task* task_list = task.getTasks();
	while (task_list) {
		printf("task id: %d.\n", task_list->id);
		S_TaskCondition* condition = task_list->condition;
		while (condition) {
			printf("----条件: %d, %d, %d.\n", condition->param, condition->param2, condition->type);
			condition = condition->next;
		}
		task_list = task_list->next;
	}*/

	
	
	//mysql_real_connect(&mysql, "127.0.0.1", "root", "123456", "piao", 3306, NULL, 0);
	//printf("%d\n", strlen("{'op':'to shot', 'status':1, 'uid':%d, 'seat':%d, 'point':%d, 'x':%d, 'y':%d, 'angle':%.2f, 'id':%d}"));
	char* t = "abc";
	//printf("t: %s\n", copy_str(t, t + 2));
	Json json;
	//json.parseString("{ 'xms' : '2 xxx' , y:  3.6  , z: {'zx': {zxy:'abc'}}, c: 3}");
	//z:{'x':{'q':6.99}, 'b':'168'},
	/*json.parseString("{'op':'set setting', 'data':{'vip_level':'xxx', 'level':'bbb'}}");
	JsonValue* data = json.getJsonValue("data");
	if (data) {
		JsonValue* data_child = data->child;
		while (data_child) {
			printf("key:%s, value:%s\n", data_child->key, data_child->value);
			data_child = data_child->next;
		}
	}
	json.clear();*/
	//json.add("op", "new fish");
	//json.push("id", 1);
	//json.push("f", 1);
	cnf->destroy();
	delete cnf;

	char test[] = "ABCD";
	*(test + 1) = 0; *(test + 2) = 0; *(test + 3) = 0;

	int x = *(int*)test;
	//printf("%d\n", x);

	Players::init();
	WebSocket server(0, 1680);
	delete Redis::$class;
	delete Mysql::$class;
	delete Setting::$class;
	memory_destroy();
	system("pause");
	return 0; 
}
