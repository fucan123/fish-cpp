class Players {
public:
	static u_int  max_len;       //���������
	static u_int  len;           //���������
	static Player players[1024]; //�������
public:
	Players();
	static void init();
	static int find(SOCKET); //����socket�������, ����players����, -1������
	static bool add(SOCKET, u_long ip); //������
	static bool remove(Player*); //�Ƴ���� 
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
