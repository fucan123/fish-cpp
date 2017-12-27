class Players {
public:
	static u_int  max_len;       //最大容纳数
	static u_int  len;           //现有玩家数
	static Player players[1024]; //玩家数组
public:
	Players();
	static void init();
	static int find(SOCKET); //根据socket查找玩家, 返回players索引, -1不存在
	static bool add(SOCKET, u_long ip); //添加玩家
	static bool remove(Player*); //移除玩家 
};

u_int  Players::max_len = 1024;
u_int  Players::len = 0;
Player Players::players[1024];

Players::Players() {}

void Players::init() {
	memset(Players::players, 0, sizeof(Players::players));
}

int Players::find(SOCKET sk) {
	for (u_int i = 0; i < len; i++) {
		if (Players::players[i].socket == sk) {
			return i;
		}
	}
	return -1;
}

bool Players::add(SOCKET sk, u_long ip) {
	if (Players::len == 1024) {
		return false;
	}

	int index = -1;
	for (int i = 0; i < 1024; i++) {
		//printf("sval: %d\n", Player::players[i].socket);
		if (Players::players[i].socket == 0) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		return false;
	}

	//printf("index: %d\n", index);

	Player* p = &Players::players[index];
	p->index = index;
	p->socket = sk;
	p->ip = ip;
	p->in_time = gettime();

	Players::len++;
	return true;
}

bool Players::remove(Player* p) {
	if (p->closeing || !p->socket) {
		printf("closeing...\n");
		return true;
	}
	p->closeing = 1;
	closesocket(p->socket);
	if (p->user) {
		printf("user ptr:%p, socket:%d\n", p->user, p->socket);
		p->save_uid = p->uid;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread::userLeave, (LPVOID)p->user, 0, NULL);
	}
	memset(p, 0, sizeof(Player));
	Players::len--;
	return true;
}
