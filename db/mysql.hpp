#include "mysql.h"

class Mysql {
private:
	MYSQL _mysql;
	u_int _errno;
	int _lock;
	int _tmp_lock;
public:
	static Mysql* $class;
public:
	Mysql(const char* host, const char* user, const char* pword, const char* dbase, int port = 3306);
	inline MYSQL* getConn();
	inline void setLock();
	inline int getLock();
	inline void freeLock();
	int   setChar(char* ch);
	char* getItem(char* sql);
	int   getItemToInt(char* sql);
	char* getOrItem(char* sql, int num=2);
	MYSQL_ROW getRow(char* sql);
	inline MYSQL* mysql();
	inline u_int error();
	static inline Mysql* _this();
};

Mysql* Mysql::$class = 0;

Mysql::Mysql(const char* host, const char* user, const char* pword, const char* dbase, int port) {
	mysql_init(&_mysql);
	char reconnect = 1;
	mysql_options(&_mysql, MYSQL_OPT_RECONNECT, &reconnect);
	mysql_real_connect(&_mysql, host, user, pword, dbase, port, NULL, 0);
	_errno = mysql_errno(&_mysql);
	_lock = 0;
	_tmp_lock = 0;
}

MYSQL* Mysql::getConn() {
	return &this->_mysql;
}

void Mysql::setLock() {
	this->_tmp_lock = 1;
}

int Mysql::getLock() {
	return this->_lock;
}

void Mysql::freeLock() {
	this->_lock = 0;
}

int Mysql::setChar(char* ch) {
	if (!ch) ch = "utf8";
	char sql[32];
	sprintf(sql, "SET NAMES '%s'", ch);
	return mysql_real_query(&this->_mysql, sql, strlen(sql));
}

char* Mysql::getItem(char* sql) {
	MYSQL_ROW row = this->getRow(sql);
	if (row == NULL) {
		return NULL;
	}
	int len = strlen(row[0]);
	if (len == 0) {
		return NULL;
	}

	char* value = (char*)_malloc(len + 1);
	memcpy(value, row[0], len + 1);
	return value;
}

int Mysql::getItemToInt(char* sql) {
	char* value = this->getItem(sql);
	if (value) {
		int num = atoi(value);
		_free(value);
		return num;
	}
	else {
		return 0;
	}
}

char* Mysql::getOrItem(char* sql, int field_num) {
	MYSQL_ROW row = this->getRow(sql);
	if (row == NULL) {
		return NULL;
	}

	int len = 0, index = 0;
	for (int i = 0; i < field_num; i++) {
		len = strlen(row[i]);
		if (len > 0) {
			index = i;
			break;
		}
	}
	if (len == 0) {
		return NULL;
	}

	char* value = (char*)_malloc(len + 1);
	memcpy(value, row[index], len + 1);
	return value;
}

MYSQL_ROW Mysql::getRow(char* sql) {
	int error = mysql_real_query(&this->_mysql, sql, strlen(sql));
	if (error) {
		printf("Mysql::getRow error(%d, %s, %s)!\n", error, mysql_error(&this->_mysql), sql);
		return NULL;
	}

	MYSQL_RES* result = mysql_store_result(&this->_mysql);
	if (result == NULL) {
		printf("Mysql::getRow result null!\n");
		return NULL;
	}
	while (this->_lock);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	this->_lock = this->_tmp_lock;
	return row;
}

inline u_int Mysql::error() {
	return _errno;
}

inline MYSQL* Mysql::mysql() {
	return &_mysql;
}

inline Mysql* Mysql::_this() {
	return Mysql::$class;
}