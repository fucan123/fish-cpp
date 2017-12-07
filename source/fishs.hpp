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
	Point  point_start;   //��ʼ����
	Point  point_ctr1;    //���Ƶ�1
	Point  point_ctr2;    //���Ƶ�2
	Point  point_end;     //��������
	Area   area;       
	int    angle; 
	int    factor;        //������
	S_Setting_Fish* fish; //Setting->S_Setting_Fish
	fish_type       type; //����
	struct_fish*    prev; //��һ���ڵ�
	struct_fish*    next; //��һ���ڵ�
	double create_time;   //����ʱ��
	int lock;             //�Ƿ�����
	int isdel;            //�Ƿ���ɾ��
} Fish;

class Fishs {
private:
	Fish* _head;      //����ʼ�ڵ�
	Fish* _curr;      //����ǰ�ڵ�
	int _id;          //��ID
	int _len;         //�㳤��
	int _create_time; //����ʱ��
	int _lock;        //��־�Ƿ���������, ��ֹ���̲߳���(��������ʱ, ������������)
public:
	int clearing;     //�Ƿ������
public:
	Fishs();
	void init(); //��ʼ��
	inline int getLength();        //����ֻ��
	inline Fish* getHead();        //������ͷ
	Fish* getFish(int id);         //��ȡһֻ��Ľṹ��, ����������, ���������Ҫ�ֶ�����
	int   getID();                 //��ȡ���ID
	Fish* create();                //����һֻ��
	Fish* destroy(Fish* fish);     //ɾ��һֻ��, ����ɾ������һ���ڵ�
	void  clear();                 //���������
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

	//Ĭ�����ɴ�����
	p->point_start.x = 0 - my_random(200, 500);
	p->point_end.x = my_random(1200, 1500);
	if (my_random(0, 1)) { //��ת����
		int tmp_x = p->point_start.x;
		p->point_start.x = p->point_end.x;
		p->point_end.x = tmp_x;
	}
	p->point_start.y = my_random(0, 600);
	p->point_end.y = my_random(0, 600);

	//���Ƶ�1
	p->point_ctr1.x = my_random(0, 600);
	p->point_ctr1.y = my_random(0, 600);
	//���Ƶ�2
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
				printf("����Ϣ(ID:%d, lock:%d, isdel:%d)\n", p->id, p->lock, p->isdel);
				p = NULL;
				goto end;
			}
			//p->lock = 1;
			//goto end;
		}
		if (p->next == NULL) {
			printf("�����(ID:%d=%d, lock:%d, isdel:%d)\n", p->id, id, p->lock, p->isdel);
		}
		p = p->next;
	}
	p = _head;
	while (p) {
		printf("��(ID:%d=%d, lock:%d, isdel:%d)\n", p->id, id, p->lock, p->isdel);
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
	printf("�����!\n");
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