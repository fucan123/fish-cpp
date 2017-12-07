#ifndef BULLETS_HPP
#define BULLETS_HPP

typedef struct struct_bullet {
	int id;
	int uid;
	int x;
	int y;
	float angle;
	struct_bullet* prev;    //上一个节点
	struct_bullet* next;    //下一个节点
	double create_time;     //创建时间
	int lock;               //是否锁定
	int isdel;              //是否已删除
} Bullet;

class Bullets {
private:
	Bullet* _head;
	Bullet* _curr;
	int _id;
	int _len;
	int _create_time;
public:
	int lock;
	int clearing;
public:
	Bullets();
	void init();
	inline int getLength();
	inline Bullet* getHead();
	Bullet* getBullet(int id, int uid);
	int   getID();
	Bullet* create(int uid, int x, int y, float angle);
	Bullet* destroy(Bullet* fish);
	void  clear();
};

Bullets::Bullets() {
	init();
}

void Bullets::init() {
	_head = NULL;
	_id = 1;
	_len = 0;
	_create_time = 0;
	this->lock = 0;
	this->clearing = 0;
}

inline int Bullets::getLength() {
	return _len;
}

inline Bullet* Bullets::getHead() {
	return _head;
}

Bullet* Bullets::create(int uid, int x, int y, float angle) {
	while (this->lock) {
		printf("create lock\n");
	}
	this->lock = 1;
	int rand = rand();
	Bullet* p = (Bullet*)_malloc(sizeof(Bullet));
	p->id = getID();
	p->uid = uid;
	p->x = x;
	p->y = y;
	p->angle = angle;
	p->create_time = getmtime();
	p->lock = 0;
	p->isdel = 0;
	//printf("id: %d\n", p->id);
	if (_head == NULL) {
		p->prev = NULL;
		p->next = NULL;
		_head = p;
		_curr = _head;
	}
	else {
		p->prev = _curr;
		p->next = NULL;
		_curr->next = p;
		_curr = p;
	}
	this->_len++;
	this->lock = 0;
	//printf("the len(create):%d\n", this->_len);

	return p;
}

Bullet* Bullets::destroy(Bullet* b) {
	while (this->lock) { 
		printf("子弹locked..destroy"); 
	}
	this->lock = 1;
	//printf("删除中...!\n");

	if (this->clearing) {
		printf("clearing!\n");
		this->lock = 0;
		return NULL;
	}
		
	Bullet* next = b->next;
	if (b->prev != NULL) {
		b->prev->next = b->next;
	}
	if (b->next != NULL) {
		b->next->prev = b->prev;
	}
	if (_head == b) {
		_head = b->next;
	}
	if (_curr == b) {
		_curr = b->prev;
	}
	_free(b);

	this->_len--;
	this->lock = 0;
	//printf("the len(destory):%d\n", this->_len);

	return next;
}

Bullet* Bullets::getBullet(int id, int uid) {
	while (this->lock) {
		printf("子弹locked..get");
	}
	this->lock = 1;

	Bullet* p = _head;
	while (p) {
		if (p->id == id && p->uid == uid && p->lock == 0 && p->isdel == 0) {
			//p->lock = 1;
			goto end;
		}
		p = p->next;
	}
end:
	this->lock = 0;
	return p;
}

int Bullets::getID() {
	while (true) {
		if (_id > 100 && _id >= MAX_INT) {
			_id = 1;
		}

		Bullet* p = _head;
		if (p == NULL) {
			goto end;
		}
		while (true) {
			if (p->id == _id) {
				_id++;
				break;
			}
			p = p->next;
			if (p == NULL) {
				goto end;
			}
		}
	}
end:
	return _id;
}

void Bullets::clear() {
	printf("清除子弹!\n");
	while (this->lock);
	this->lock = 1;
	this->clearing = 1;

	Bullet* p = _head;
	while (p) {
		Bullet* tmp = p;
		p = p->next;
		_free(tmp);
	}
	_head = NULL;
	_len = 0;
	this->clearing = 0;
	this->lock = 0;
}
#endif // !Bullets_HPP