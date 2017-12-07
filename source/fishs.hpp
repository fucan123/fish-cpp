#ifndef FISHS_HPP
#define FISHS_HPP

#define FISH_MOVE_LINE 1;

typedef int fish_type;
typedef int move_type;

typedef struct struct_point {
	int x;
	int y;
} Point;

typedef struct struct_area {
	int width;
	int height;
} Area;

typedef struct struct_fish {
	int    id;            //ID
	Point  point_start;   //开始坐标
	Point  point_ctr1;    //控制点1
	Point  point_ctr2;    //控制点2
	Point  point_end;     //结束坐标
	Area   area;       
	int    angle; 
	int    factor;        //因子数
	S_Setting_Fish* fish; //Setting->S_Setting_Fish
	fish_type       type; //类型
	struct_fish*    prev; //上一个节点
	struct_fish*    next; //下一个节点
	double create_time;   //创建时间
	int lock;             //是否锁定
	int isdel;            //是否已删除
} Fish;

class Fishs {
private:
	Fish* _head;      //链表开始节点
	Fish* _curr;      //链表当前节点
	int _id;          //鱼ID
	int _len;         //鱼长度
	int _create_time; //创建时间
	int _lock;        //标志是否锁定操作, 防止多线程并发(操作不耗时, 所以用自旋锁)
public:
	int clearing;     //是否在清除
public:
	Fishs();
	void init(); //初始化
	inline int getLength();        //多少只鱼
	inline Fish* getHead();        //鱼链表头
	Fish* getFish(int id);         //获取一只鱼的结构体, 会锁定此鱼, 操作完后需要手动解锁
	int   getID();                 //获取鱼的ID
	Fish* create();                //创建一只鱼
	Fish* destroy(Fish* fish);     //删掉一只鱼, 返回删除的下一个节点
	void  clear();                 //清除所有鱼
	static char* fishJson(Fish*);
};

Fishs::Fishs() {
	init();
}

void Fishs::init() {
	_head = NULL;
	_id = 1;
	_len = 0;
	_create_time = 0;
	_lock = 0;
	this->clearing = 0;
}

inline int Fishs::getLength() {
	return _len;
}

inline Fish* Fishs::getHead() {
	return _head;
}

Fish* Fishs::create() {
	while (this->_lock);
	this->_lock = 1;
	int rand = rand();
	Fish* p = (Fish*)_malloc(sizeof(Fish));
	p->id = getID();

	//默认生成从左到右
	p->point_start.x = 0 - my_random(200, 500);
	p->point_end.x = my_random(1200, 1500);
	if (my_random(0, 1)) { //调转方向
		int tmp_x = p->point_start.x;
		p->point_start.x = p->point_end.x;
		p->point_end.x = tmp_x;
	}
	p->point_start.y = my_random(0, 600);
	p->point_end.y = my_random(0, 600);

	//控制点1
	p->point_ctr1.x = my_random(0, 600);
	p->point_ctr1.y = my_random(0, 600);
	//控制点2
	p->point_ctr2.x = my_random(200, 1000);
	p->point_ctr2.y = my_random(0, 600);

	p->angle = my_random(0, 360);
	p->fish = Setting::$class->getFish(my_random(0, Setting::$class->getFishQty() - 1));
	p->type = my_random(0, 11);
	p->create_time = getmtime();
	p->lock = 0;
	p->isdel = 0;
	if (p->fish == NULL) {
		_free(p);
		this->_lock = 0;
		return NULL;
	}
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
	_len++;
	this->_lock = 0;

	return p;
}

Fish* Fishs::destroy(Fish* fish) {
	while (this->_lock);
	this->_lock = 1;

	if (this->clearing) {
		this->_lock = 0;
		return NULL;
	}

	Fish* next = fish->next;
	if (fish->prev != NULL) {
		fish->prev->next = fish->next;
	}
	if (fish->next != NULL) {
		fish->next->prev = fish->prev;
	}
	if (_head == fish) {
		_head = fish->next;
	}
	if (_curr == fish) {
		_curr = fish->prev;
	}
	_free(fish);

	this->_len--;
	this->_lock = 0;

	return next;
}

Fish* Fishs::getFish(int id) {
	while (this->_lock);
	this->_lock = 1;

	Fish* p = _head;
	while (p) {
		if (p->id == id) {
			if (p->lock == 0 && p->isdel == 0) {
				p->lock = 1;
				goto end;
			}
			else {
				printf("鱼信息(ID:%d, lock:%d, isdel:%d)\n", p->id, p->lock, p->isdel);
				p = NULL;
				goto end;
			}
			//p->lock = 1;
			//goto end;
		}
		if (p->next == NULL) {
			printf("最后鱼(ID:%d=%d, lock:%d, isdel:%d)\n", p->id, id, p->lock, p->isdel);
		}
		p = p->next;
	}
	p = _head;
	while (p) {
		printf("鱼(ID:%d=%d, lock:%d, isdel:%d)\n", p->id, id, p->lock, p->isdel);
		p = p->next;
	}
end:
	this->_lock = 0;
	return p;
}

int Fishs::getID() {
	while (true) {
		if (_id >= MAX_INT) {
			_id = 1;
		}

		Fish* p = _head;
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

void Fishs::clear() {
	printf("清除鱼!\n");
	while (this->_lock);
	this->_lock = 1;
	this->clearing = 1;

	Fish* p = _head;
	while (p) {
		Fish* tmp = p;
		p = p->next;
		_free(tmp);
	}
	_head = NULL;
	_len = 0;
	this->clearing = 0;
	this->_lock = 0;
}

char* Fishs::fishJson(Fish* p) {
	char* str = (char*)_malloc(300);
	sprintf(str, "{op:'new fish', id:%d, type:%d, cp:[{x:%d, y:%d},{x:%d, y:%d},{x:%d, y:%d},{x:%d, y:%d}], t:%.2f}",
	    p->id, p->type, p->point_start.x, p->point_start.y, p->point_ctr1.x, p->point_ctr1.y,
		p->point_ctr2.x, p->point_ctr2.y, p->point_end.x, p->point_end.y, 0);
	return str;
}
#endif // !FISHS_HPP