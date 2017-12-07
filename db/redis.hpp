#include "../redis/hiredis.h"

class Redis {
private:
	redisContext* _redis;
	redisReply* _reply;
public:
	static Redis* $class;
public:
	Redis(char*, int);
	bool  set(char* key, char* value);
	bool  set(char* key, int value);
	char* get(char* key);
	int   getInt(char* key);
	inline redisContext* redis();
	static inline Redis* _this();
};

Redis* Redis::$class = 0;

Redis::Redis(char* host, int port) {
	this->_redis = redisConnect(host, port);
	this->_reply = 0;
}

bool Redis::set(char* key, char* value) {
	_reply = (redisReply *)redisCommand(_redis, "SET %s %s", key, value);
	freeReplyObject(_reply);
	return _reply ? true : false;
}

bool Redis::set(char* key, int value) {
	_reply = (redisReply *)redisCommand(_redis, "SET %s %d", key, value);
	freeReplyObject(_reply);
	return _reply ? true : false;
}

char* Redis::get(char* key) {
	char* str = NULL;
	_reply = (redisReply *)redisCommand(_redis, "GET %s", key);
	//printf("redis get len: %d\n", _reply->len);
	if (_reply && _reply->type == REDIS_REPLY_STRING && _reply->len > 0) {
		str = (char*)_malloc(_reply->len + 1);
		memcpy(str, _reply->str, _reply->len + 1);
	}

	freeReplyObject(_reply);
	return str;
}

int Redis::getInt(char* key) {
	int num = 0;
	_reply = (redisReply *)redisCommand(_redis, "GET %s", key);
	if (_reply && _reply->type == REDIS_REPLY_INTEGER && _reply->len > 0) {
		num = atoi(_reply->str);
	}

	freeReplyObject(_reply);
	return num;
}

inline redisContext* Redis::redis() {
	return _redis;
}

inline Redis* Redis::_this() {
	return Redis::$class;
}