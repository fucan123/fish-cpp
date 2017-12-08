class Thread {
public:
	static void bullet();                     //�ӵ�����
	static void fish();                       //�㴦��
	static void saveUser();                   //�����û���Ϣ
	static void verifyAccount(LPVOID player); //��֤�ʺ�
};

void Thread::bullet() {
	int max_room = rooms.getMaxRoom();
	while (true) {
		int index = 0;
		while (index < max_room) {
			Room* room = rooms.getRoom(index);
			if (0 && !room->bullets) {
				room->bullets = (Bullets*)_malloc(sizeof(Bullets));
				room->bullets->init();
			}
			if (room->bullets) {
				//room->bullets->create(168, 0, 0, 0);
				//printf("locked:%d\n", room->bullets->lock);
				Bullet* p = room->bullets->getHead();
				while (p) {
					if (room->bullets->clearing) { //�������ʱ, ��ʾ�˷�����û����, ��ִ���������
						break;
					}
					if (p->lock) { //�˶���������, ��ִ�в���
						printf("�ӵ�����(ID:%d, locked:%d).\n", p->id, p->lock);
						p = p->next;
						continue;
					}
					if (p->isdel) { //�˶����ѱ���־ɾ��
									//printf("�ӵ�ɾ��:(ID:%d, �û�:%d), len:%d\n", p->id, p->uid, room->bullets->getLength());
						p = room->bullets->destroy(p);
						//printf("�ӵ�ɾ���ɹ�.\n");
					}
					else {
						double time = getmtime();
						double ctime = time - p->create_time;
						if (ctime > 10) { //�ӵ�ʱ���ѹ���
										  //printf("�ӵ�����:(ID:%d, �û�:%d), len:%d\n", p->id, p->uid, room->bullets->getLength());
							p = room->bullets->destroy(p);
							//goto end;
						}
						else {
							p = p->next;
						}
					}
				}
			}
			index++;
		}
		Sleep(1000);
	}
}

void Thread::fish() {
	int max_room = rooms.getMaxRoom();
	while (true) {
		int index = 0;
		while (index < max_room) {
			Room* room = rooms.getRoom(index);
			if (room->fishs) {
				//printf("fish len:%d.\n", room->fishs->getLength());
				if (room->fishs->getLength() < 10) {
					//printf("create fish timer!\n");
					Fish* fish = room->fishs->create();
					if (fish != NULL) {
						//printf("create fish timer end!\n");
						u_int send_len;
						char* msg = room->fishs->fishJson(fish);
						char* _send_str_ = emsg(1, 1, msg, &send_len);
						for (int i = 0; i < 4; i++) {
							if (room->players[i] && room->players[i]->socket) {
								send(room->players[i]->socket, _send_str_, send_len, 0);
							}
						}
						_free(_send_str_);
						_free(msg);
					}
				}
				//printf("fucjjj!\n");
				Fish* p = room->fishs->getHead();
				while (p) {
					if (room->fishs->clearing) { //�������ʱ, ��ʾ�˷�����û����, ��ִ���������
						printf("fish clearing.\n");
						break;
					}
					if (p->lock) { //�˶���������, ��ִ�в���
						printf("������(ID:%d, locked:%d).\n", p->id, p->lock);
						p = p->next;
						continue;
					}
					if (p->isdel) { //�˶����ѱ���־ɾ��
									//printf("��ɾ��(ID:%d, locked:%d), ������:%d.\n", p->id, p->lock, room->fishs->getLength());
						p = room->fishs->destroy(p);
					}
					else {
						p = p->next;
					}
				}
				//printf("fish.......\n");
			}
			index++;
		}
		Sleep(1000);
	}
}

void Thread::saveUser() {
	int timeval = Setting::$class->systemToInt("game_user_save_timeval");
	if (timeval < 5) {
		timeval = 300;
	}
	while (true) {
		long now_time = gettime();
		for (int i = 0; i < Players::max_len; i++) {
			Player p = Players::players[i];
			if (p.user) {
				User* user = (User*)p.user;
				if ((now_time - user->op_time) > timeval) {
					printf("�����û���Ϣ.\n");
					user->save();
				}
			}
		}
		Sleep(10000);
	}
}

void Thread::verifyAccount(LPVOID player) {
	Player* p = (Player*)player;
	char* key = (char*)p->tmp;
	while (*key) {
		if (*key == '"' || *key == '\'') {
			sendmsg("{'op':'verify account', 'uid':0, 'status':0}", p);
			return;
		}
		key++;
	}

	User* user = (User*)_malloc(sizeof(User));
	int uid = user->checkSignKey((char*)p->tmp);
	if (uid > 0) {
		for (int i = 0; i < 1024; i++) {
			if (Players::players[i].save_uid == uid) { //��ǰ��¼�û����ڱ���, �ݲ������¼
				printf("���û����ڱ�����(%d, #%d).\n", Players::players[i].save_uid, Players::players[i].socket);
				sendmsg("{'op':'verify account', 'status':-1}", p);
				_free(user);
				return;
			}
		}
		for (int i = 0; i < 1024; i++) {
			if (Players::players[i].uid == uid) {
				if (Players::players[i].kick) { //���ѡ��������
					printf("��ұ�������(%d, #%d).\n", uid, Players::players[i].socket);
					sendmsg("{'op':'offline', 'status':1}", &Players::players[i]);
					rooms.leave(&Players::players[i]);
					Players::remove(&Players::players[i]);
				}
				else {
					//������Ҫ�û�ȷ��ǿ�Ƶ�¼����Ϣ status=-1
					sendmsg("{'op':'verify account', 'status':-2}", p);
					_free(user);
					return;
				}
				break;
			}
		}
		printf("��֤�˻��ɹ�, ID:%d\n", uid);
		user->load(); //�����û���Ϣ
		p->uid = uid;
		p->user = user;

		char send_str[100];
		sprintf(send_str, "{'op':'verify account', 'status':1, 'uid':%d, 'coin':%d, 'name':'%s'}", user->id, user->coin, user->name);
		sendmsg(send_str, p);
	}
	else {
		printf("��֤ʧ��!\n");
		_free(user);
		sendmsg("{'op':'verify account', 'uid':0, 'status':0}", p);
	}
}