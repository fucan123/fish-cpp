class Entry {
private:
	Json*   _json;
	Player* _player;
	User*   _user;
	char*   _msg;
public:
	int now_time;
public:
	Entry(Player* player, Json* json);
	void verifyAccount();
	void inRoom();
	void leaveRoom();
	void toShot();
	void hitFish();
	void viewOnline();
	void kickUser();
	void serverInfo();
	void setSetting(); //����
	void exit();
	void other();
	void sendmsgToRoom(char* msg);
	void senderror(error_msg_code); //���ʹ�����Ϣ
	inline void sendmsg(char* msg, char fin=1, char opcode=1);
};

Entry::Entry(Player* player, Json* json) {
	_player = player;
	_user = (User*)player->user;
	_json = json;

	char* op = json->value("op");
	if (!op) {
		this->senderror(ERROR_MSG_OP);
		return;
	}
	if (0) {
		return;
	}
	if (strcmp(op, "verify account") != 0) { //��֤�ʺŲ���Ҫ�û���Ϣ
		if (!_user) { //û���û���Ϣ
			return;
		}
	}
	
	this->now_time = gettime();
	//printf("op: %s\n", op);
	//printf("op:%s, bullet id:%s\n", op, _json->value("bullet_id"));
	if (strcmp(op, "verify account") == 0) { //��֤�ʺ�
		this->verifyAccount();
	} else
	if (strcmp(op, "in room") == 0) { //���뷿��
		this->inRoom();
	} else 
	if (strcmp(op, "leave room") == 0) { //�뿪����
		if (_player->room > 0) this->leaveRoom();
	} else
	if (strcmp(op, "to shot") == 0) { //���
		if (_player->room > 0) this->toShot();
	} else
	if (strcmp(op, "hit fish") == 0) { //������
		if (_player->room > 0) this->hitFish();
	} else
	if (strcmp(op, "view online") == 0) { //�鿴�������
		if (!_user->privilege.view) { 
			this->senderror(ERROR_MSG_PRILIVEGE);
			return;
		}
		this->viewOnline();
	} else 
	if (strcmp(op, "kick user") == 0) { //���������
		if (!_user->privilege.kick) {
			this->senderror(ERROR_MSG_PRILIVEGE);
			return;
		}
		this->kickUser();
	} else 
	if (strcmp(op, "server info") == 0) { //��������Ϣ
		//printf("server info.\n");
		if (!_user->privilege.exit) {
			this->senderror(ERROR_MSG_PRILIVEGE);
			return;
		}
		this->serverInfo();
	} else
	if (strcmp(op, "set setting") == 0) { //��������Ϣ
		if (!_user->privilege.exit) {
			this->senderror(ERROR_MSG_PRILIVEGE);
			return;
		}
		this->setSetting();
	} else
	if (strcmp(op, "exit") == 0) { //��ֹ������
		//printf("p-exit.\n");
		if (!_user->privilege.exit) {
			this->senderror(ERROR_MSG_PRILIVEGE);
			return;
		}
		this->exit();
	}
	else { //��֪��
		this->other();
	}
}

void Entry::verifyAccount() {
	//��֤�ʺ�ȷ����Ϣ[statusΪ1��֤�ɹ�, statusΪ0��֤ʧ��, statusΪ-1�ʺű�����, statusΪ-2�ʺŵ�¼����]
	char* key = _json->value("key");
	if (!key) {
		this->sendmsg("{'op':'verify account', 'uid':0, 'status':0}");
	}
	else {
		_player->kick = _json->valueToInt("kick");
		_player->tmp = key;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread::verifyAccount, (LPVOID)_player, 0, NULL);
	}
}

void Entry::inRoom() {
	if (_player->room > 0 || !_player->user) {
		return;
	}
	int room_i, seat;
	rooms.findRoom(&room_i, &seat);
	if (room_i == 0) {
		this->sendmsg("{op: 'find room', room:0}");
	}
	else {
		_player->room = room_i;
		_player->seat = seat;
		_player->game_time = this->now_time;
		Room* room = rooms.getRoom(room_i - 1);
		room->players[seat - 1] = _player;

		char send_str[100];
		sprintf(send_str, "{'op':'find room', 'uid':%d, 'name':'%s', 'room':%d, 'seat':%d, 'time':%.3f}", _player->uid, _user->name, room_i, seat, getmtime());
		//printf("send: %s\n", send_str);
		this->sendmsgToRoom(send_str);
		for (int i = 0; i < 4; i++) {
			if (room->players[i] && room->players[i]->socket) {
				//printf("%d=%d\n", room->players[i]->uid, _player->uid);
				if (room->players[i]->uid != _player->uid) {
					printf("send.\n");
					User* user = (User*)(room->players[i]->user);
					if (user) {
						sprintf(send_str, "{'op':'find room', 'uid':%d, 'name':'%s', 'room':%d, 'seat':%d, 'time':%.3f}", room->players[i]->uid, user->name, room_i, room->players[i]->seat, getmtime());
						this->sendmsg(send_str);
					}
				}
			}
		}

		if (room->fishs == NULL) {
			room->create_time = getmtime();
			room->fishs = (Fishs*)_malloc(sizeof(Fishs));
			room->fishs->init();
			room->bullets = (Bullets*)_malloc(sizeof(Bullets));
			room->bullets->init();
			Fish* fish = room->fishs->create();

			char* send_str = room->fishs->fishJson(fish);
			//printf("send: %s\n", send_str);
			this->sendmsg(send_str);
			_free(send_str);

			//printf("room empty, %d, %p\n", _player->room - 1, room->fishs);
		}
	}
}

void Entry::leaveRoom() {
	rooms.leave(_player);

	char send_str[] = "{'op':'leave room', 'status':1}";
	sendmsg(send_str);
}

void Entry::toShot() {
	if (!_user || _user->coin <= 0) {
		this->sendmsg("{'op':'to shot', 'status':0}");
	}
	else {
		_player->op_time = this->now_time; //������ʱ��
		_user->coin -= 1;

		int x = _json->valueToInt("x");
		int y = _json->valueToInt("y");
		float angle = _json->valueToFloat("angle");

		Room* room = rooms.getRoom(_player->room - 1);
		Bullet* bullet = room->bullets->create(_player->uid, x, y, angle);
		//printf("�����ӵ�(ID:%d, locked:%d)\n", bullet->id, bullet->lock);

		char send_str[200];
		sprintf(send_str, "{'op':'to shot', 'status':1, 'uid':%d, 'seat':%d, 'coin':%d, 'x':%d, 'y':%d, 'angle':%.2f, 'id':%d}",
			_player->uid, _player->seat, _user->coin, x, y, angle, bullet->id);
		this->sendmsgToRoom(send_str);
	}
}

void Entry::hitFish() {
	int bid = _json->valueToInt("bullet_id");
	Room* room = rooms.getRoom(_player->room - 1);
	Bullet* bullet = room->bullets->getBullet(bid, _player->uid);
	if (_user && bullet) {
		//printf("������\n");
		Fish* fish = room->fishs->getFish(_json->valueToInt("fish_id"));
		if (fish) {
			fish->hit_coin += 1; //���д��㻨�ѽ���ۼ�
			//printf("��������\n");
			_user->coin += fish->fish->coin;
			_user->killFish(fish->fish->no);

			char send_str_f[200];
			sprintf(send_str_f, "{op:'hit fish',uid:%d,coin:%d,bullet_id:%d,bx:%d,by:%d,fish_id:%d,fx:%d,fy:%d,fag:%.2f,die:%d,ftype:%d}",
				_player->uid, _user->coin,
				bid, _json->valueToInt("bx"), _json->valueToInt("by"), 
				fish->id, _json->valueToInt("fx"), _json->valueToInt("fy"), _json->valueToFloat("fag"), 1, fish->type);
			this->sendmsg(send_str_f);

			fish->isdel = 1;
			fish->lock = 0;
			//room->fishs->destroy(fish);
		}
		else {
			printf("����Ч(ID:%d, �û�:%d), ����:%d!\n", _json->valueToInt("fish_id"), _player->uid, room->fishs->getLength());
		}
		bullet->isdel = 1;
		bullet->lock = 0;
		//room->bullets->destroy(bullet);
		//printf("�ӵ���Ŀ:%d, ID:%d\n", room->bullets->getLength(), bullet->id);
	}
	else {
		printf("�ӵ���Ч(ID:%d, �û�:%d)!\n", bid, _player->uid);
		this->sendmsg("{op:'hit fish', status:0, msg:'no valid!'}");
	}
}

void Entry::viewOnline() {
	int now_time = gettime();
	this->sendmsg("{'op':'view online', data:[", 0, 1);
	char send_str[256];
	for (int i = 0; i < Players::max_len; i++) {
		Player p = Players::players[i];
		if (p.socket) {
			char fstr[] = "{'socket':%d, 'id':%d, 'name':'%s', 'coin':%d, 'backup_coin':%d, "\
				"'room':%d, 'seat':%d, 'hack':%d, "\
				"'in_time':%d, 'op_time':%d, 'time':%d},"\
				;
			if (p.user) {
				User* user = (User*)p.user;
				sprintf(send_str, fstr,
					p.socket, user->id, user->name, user->coin, user->backup_coin,
					p.room, p.seat, p.hack,
					p.in_time, user->op_time, now_time);
			}
			else {
				sprintf(send_str, fstr,
					p.socket, 0, "", 0, 0,
					p.room, p.seat, p.hack,
					p.in_time, 0, now_time);
			}
			this->sendmsg(send_str, 0, 0);
		}
	}
	this->sendmsg("]}", 1, 0);
}

void Entry::kickUser() {
	int socket_id = _json->valueToInt("socket_id");
	if (socket_id <= 0) {
		this->sendmsg("{'op':'kick user', 'status':0, 'msg':'error socket!'}");
		return;
	}
	if (socket_id  == _player->socket) {
		this->sendmsg("{'op':'kick user', 'status':0, 'msg':'not kick self!'}");
		return;
	}
	for (u_int i = 0; i < Players::max_len; i++) {
		Player p = Players::players[i];
		if (p.socket == socket_id) {
			rooms.leave(&p);
			Players::remove(&p);
			this->sendmsg("{'op':'kick user', 'status':1, 'msg':'ok!'}");
			break;
		}
	}
}

void Entry::serverInfo() {
	memory_var* m_var = memory_data();
	int* m_con = memory_con();
	int m_con_len = 3; //m_conΪ��ά����, bit��ʼλΪm_con[4][0]ת��һάΪm_con[4*m_con_len], bit����λΪm_con[5*m_con_len], bitλ��������Ϊm_con[7*m_con_len]
	char* send_str = new char[512];
	char fstr[] = "{'op':'server info', "\
		"memory:{use:%u, total:%u, malloc_count:%lld, free_count: %lld, system_use: %lld, detail:"\
		"[{size:%d, use:%d, total:%d},{size:%d, use:%d, total:%d},{size:%d, use:%d, total:%d}]}, "\
		"cpu:{use: %.2f}"\
		"}";
	//{size:%d, use:%d, total:%d}, {size:%d, use:%d, total:%d}, {size:%d, use:%d, total:%d}
	sprintf(send_str, fstr, 
		m_var->use, m_var->total, m_var->malloc_count, m_var->free_count, m_var->system_use,
		m_con[0], m_con[21], m_con[15] - m_con[12], 
		m_con[1], m_con[22], m_con[16] - m_con[13], 
		m_con[2], m_con[23], m_con[17] - m_con[14],
		get_cpu_rate());
	//printf("%s(%d)\n", send_str, strlen(send_str));
	this->sendmsg(send_str);
}

void Entry::setSetting() {
	JsonValue* data = _json->getJsonValue("data");
	int error_i = 0, success_i = 0;
	if (data) {
		JsonValue* data_child = data->child;
		while (data_child) {
			printf("key:%s, value:%s\n", data_child->key, data_child->value);
			if (Setting::$class->setValue(data_child->key, data_child->value)) {
				success_i++;
			}
			else {
				error_i++;
			}
			data_child = data_child->next;
		}
	}
	char send_str[128];
	sprintf(send_str, "{'op':'set setting', 'error':%d, 'success':%d}", error_i, success_i);
	this->sendmsg(send_str);
}

void Entry::exit() {
	Setting::$class->closeing = true; //���������ڹر�...
	int num = 0;
	char send_str[256];
	for (u_int i = 0; i < Players::max_len; i++) {
		Player p = Players::players[i];
		if (p.socket) {
			num++;
			sprintf(send_str, "{'op':'exit', socket:%d, num:%d, status: 0}", p.socket, num);
			this->sendmsg(send_str);
			rooms.leave(&p);
			Players::remove(&p);
		}
	}
	sprintf(send_str, "{'op':'exit', socket:%d, num:%d, status: 1}", -1, num);
	this->sendmsg(send_str);
	::exit(0);
}

void Entry::other() {
	this->sendmsg(_json->value("msg"));
}

void Entry::sendmsgToRoom(char* msg) {
	//printf("room:%d.\n", _player->room);
	if (_player->room > 0) {
		u_int send_len;
		Room* room = rooms.getRoom(_player->room - 1);
		char* _send_str_ = emsg(1, 1, msg, &send_len);
		for (int i = 0; i < 4; i++) {
			if (room->players[i] && room->players[i]->socket) {
				send(room->players[i]->socket, _send_str_, send_len, 0);
				//printf("send msg for room.\n");
			}
		}
		_free(_send_str_);
		//printf("�ͷŷ����ַ����\n");
	}
}

inline void Entry::sendmsg(char* msg, char fin, char opcode) {
	if (!msg) {
		msg = "null";
	}
	//printf("send msg: %s\n", msg);
	u_int send_len;
	char* _send_str_ = emsg(fin, opcode, msg, &send_len);
	send(_player->socket, _send_str_, send_len, 0);
	_free(_send_str_, "send str");
	//printf("�ͷŷ����ַ����\n");
}

void Entry::senderror(error_msg_code code) {
	char send_str[64], msg[32] = {0};
	if (code == ERROR_MSG_OP) {
		int _msg[] = { 0xE68C87, 0xE4BBA4, 0xE99499, 0xE8AFAF, 0xEFBC81 }; //ָ�����utf8ʮ������
		int2utf8(msg, _msg, 5);
	}
	else if(code == ERROR_MSG_PRILIVEGE) {
		int _msg[] = { 0xE69D83, 0xE99990, 0xE4B88D, 0xE5A49F, 0xEFBC81 }; //Ȩ�޲�����utf8ʮ������
		int2utf8(msg, _msg, 5);
	}
	sprintf(send_str, "{'op':'error', 'code':%d, 'msg':'%s'}", code, msg);
	this->sendmsg(send_str);
}