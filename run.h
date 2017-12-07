void app_run(u_long host, u_short port);
#ifdef _WINDOWS
DWORD WINAPI the_main_test() {
	int            i;
	fd_set         fdread;
	int            ret;
	struct timeval tv = { 1, 200 };
	char msg[140];
	printf("正在监听(test)...\n");
	//printf("%d", Players::len);
	while (true) {
		//printf("while\n");
		FD_ZERO(&fdread);
		FD_SET(Players::players[0].socket, &fdread);

		tv.tv_sec = 3;
		tv.tv_usec = 3000;
		ret = select(0, &fdread, NULL, NULL, &tv);
		if (ret == 0) {
			continue;
		}
		if (FD_ISSET(Players::players[0].socket, &fdread)) {
		}
		Sleep(1);
	}
}

DWORD WINAPI the_bullet() {
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
					if (room->bullets->clearing) { //正在清除时, 表示此房间已没有人, 不执行下面操作
						break;
					}
					if (p->lock) { //此对象已锁定, 不执行操作
						printf("子弹锁定(ID:%d, locked:%d).\n", p->id, p->lock);
						p = p->next;
						continue;
					}
					if (p->isdel) { //此对象已被标志删除
									//printf("子弹删除:(ID:%d, 用户:%d), len:%d\n", p->id, p->uid, room->bullets->getLength());
						p = room->bullets->destroy(p);
						//printf("子弹删除成功.\n");
					}
					else {
						double time = getmtime();
						double ctime = time - p->create_time;
						if (ctime > 10) { //子弹时间已过期
										  //printf("子弹过期:(ID:%d, 用户:%d), len:%d\n", p->id, p->uid, room->bullets->getLength());
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

end:
	return 0;
}

DWORD WINAPI the_fish() {
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
					if (room->fishs->clearing) { //正在清除时, 表示此房间已没有人, 不执行下面操作
						printf("fish clearing.\n");
						break;
					}
					if (p->lock) { //此对象已锁定, 不执行操作
						printf("鱼锁定(ID:%d, locked:%d).\n", p->id, p->lock);
						p = p->next;
						continue;
					}
					if (p->isdel) { //此对象已被标志删除
									//printf("鱼删除(ID:%d, locked:%d), 所有鱼:%d.\n", p->id, p->lock, room->fishs->getLength());
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
	return 0;
}

DWORD WINAPI save_user() {
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
					printf("保存用户信息.\n");
					user->save();
				}
			}
		}
		Sleep(10000);
	}
}

DWORD WINAPI the_verify_account(LPVOID player) {
	Player* p = (Player*)player;
	char* key = (char*)p->tmp;
	while (*key) {
		if (*key == '"' || *key == '\'') {
			sendmsg("{'op':'verify account', 'uid':0, 'status':0}", p);
			break;
		}
		key++;
	}

	User* user = (User*)_malloc(sizeof(User));
	int uid = user->init((char*)p->tmp);
	if (uid > 0) {
		for (int i = 0; i < 1024; i++) {
			if (Players::players[i].save_uid == uid) { //当前登录用户正在保存, 暂不允许登录
				printf("此用户正在保存中(%d, #%d).\n", Players::players[i].save_uid, Players::players[i].socket);
				sendmsg("{'op':'verify account', 'status':-1}", p);
				_free(user);

				return 0;
			}
		}
		for (int i = 0; i < 1024; i++) {
			if (Players::players[i].uid == uid) {
				printf("玩家被踢下线(%d, #%d).\n", uid, Players::players[i].socket);
				rooms.leave(&Players::players[i]);

				sendmsg("{'op':'offline', 'status':1}", &Players::players[i]);
				Players::remove(&Players::players[i]);
				break;
			}
		}
		printf("验证账户成功, ID:%d\n", uid);
		p->uid = uid;
		p->user = user;

		char send_str[100];
		sprintf(send_str, "{'op':'verify account', 'status':1, 'uid':%d, 'coin':%d, 'name':'%s'}", user->id, user->coin, user->name);
		sendmsg(send_str, p);
	}
	else {
		printf("验证失败!\n");
		user->destory();
		_free(user);
		sendmsg("{'op':'verify account', 'uid':0, 'status':0}", p);
	}
	return 0;
}

/*
void app_run(u_long host, u_short port) {
WSADATA     wsaData;
SOCKET      sListen, sClient;
SOCKADDR_IN local, client;
int         iaddrSize = sizeof(SOCKADDR_IN);
DWORD       dwThreadId;

WSAStartup(0x0202, &wsaData);

sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

local.sin_addr.S_un.S_addr = htonl(host);
local.sin_family = AF_INET;
local.sin_port = htons(port);
bind(sListen, (struct sockaddr *)&local, sizeof(SOCKADDR_IN));

listen(sListen, 3);

//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)the_main_test, NULL, 0, &dwThreadId);
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)the_main, NULL, 0, &dwThreadId);
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)the_bullet, NULL, 0, &dwThreadId);
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)the_fish, NULL, 0, &dwThreadId);
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)save_user, NULL, 0, &dwThreadId);

while (true) {
sClient = accept(sListen, (struct sockaddr *)&client, &iaddrSize);
printf("连接\n");
Players::add(sClient);
printf("consck: %d\n", sClient);
}
}

DWORD WINAPI the_main() {
int            i;
fd_set         fdread;
int            ret;
struct timeval tv = { 1, 200 };
char msg[140];
printf("正在监听x...\n");
//printf("%d", Players::len);
while (true) {
//Sleep(1);
//printf("while\n");
FD_ZERO(&fdread);
for (i = 0; i < 1024; i++) {
if (Players::players[i].socket) {
FD_SET(Players::players[i].socket, &fdread);
}
}

ret = select(0, &fdread, NULL, NULL, &tv);
if (ret == 0) {
continue;
}
//printf("sta:%d\n", ret);
for (i = 0; i < 1024; i++) {
if (!Players::players[i].socket) {
continue;
}
printf("sta:%d\n", ret);
if (FD_ISSET(Players::players[i].socket, &fdread)) {
Player* player = &Players::players[i];
SOCKET c_sck = Players::players[i].socket;
//printf("sta:%d\n", ret);
//continue;[[
int size = 20;
char str[20];
if (ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)) {
printf("Client socket %d closed...\n", c_sck);
rooms.leave(player);
Players::remove(player);
}
else {
char msg[1024];
msg[0] = 0;
char ch;
//int sd = send(Cache::sockets[i], "abc", 3, 0);
//printf("send2: %d\n", sd);
//int ret = recv(Cache::sockets[i], msg, 1024, MSG_WAITALL);
//int index = 0;
ret = recv(c_sck, msg, 1024 - 1, 0);
if (ret < 6) {
printf("Client socket %d closed., len: %d\n", c_sck, ret);
rooms.leave(player);
Players::remove(player);
continue;
}
while (ret > 0) {
//printf("ret1: %d, sk: %d\n", ret, Cache::sockets[i]);
msg[ret] = 0;

if (ret == 1023) {
ret = recv(c_sck, &ch, 1, 0);
}
else {
ret = 0;
}
//printf("ret2: %d\n", ret);
}
//printf("sk: %d\n", Cache::sockets[i]);

if (Players::players[i].hack == 0) {
//printf("%s", msg);
char* key = strstr(msg, "Sec-WebSocket-Key:");
//printf("p:%p\n", key);
if (key) {
char sec[50];
int j = 0;
key += 18;

while (*key != '\r' && j < 50) {
if (*key != ' ') {
sec[j] = *key;
j++;
}

key++;
}

sec[j] = 0;
//printf("sec: %s\n", sec);
char skey[100];
skey[0] = 0;
strcat(skey, sec);
strcat(skey, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
//printf("skey: %s\n", skey);
char sha1_buf[50];
sha1_hash(skey, sha1_buf);
//printf("sha1: %s\n", sha1_buf);
char base64_str[200];
base64_encode(sha1_buf, 20, base64_str);
//printf("base64: %s\n", base64_str);
char upgrade[500] = "HTTP/1.1 101 WebSocket Protocol Handshake\r\n";
strcat(upgrade, "Upgrade: websocket\r\n");
strcat(upgrade, "Connection: Upgrade\r\n");
strcat(upgrade, "WebSocket-Origin: http://127.0.0.1\r\n");
strcat(upgrade, "WebSocket-Location: ws://127.0.0.1:1680\r\n");
strcat(upgrade, "Sec-WebSocket-Accept: ");
strcat(upgrade, base64_str);
strcat(upgrade, "\r\n\r\n");
//printf("send: %s\n", upgrade);
int sd = send(c_sck, upgrade, strlen(upgrade), 0);
Players::players[i].hack = 1;
/*char* send_str = emsg(1, 1, "connect", strlen("connect"));
sd = send(c_sck, send_str, strlen(send_str), 0);
Player::remove(i);
//printf("send: %d\n", sd);
//printf("error: %d\n", GetLastError());*
}
else {

}
}
else {
if (Setting::$class->closeing) { //服务器正在关闭中...
break;
}
int code = 0;
char* t_msg = fmsg(msg, &code);
//printf("请求数据(%d): %s\n", c_sck, t_msg);
if (code == -1) {
rooms.leave(player);
Players::remove(player);
printf("刷新离开!\n");
}
if (code == 1) {
Json json;
//printf("解析JSON\n");
json.parseString(t_msg);
//printf("解析JSON完成\n");
Entry entry(&Players::players[i], &json);
//printf("清理JSON\n");
json.clear();
//printf("清理JSON完成\n");
}
if (t_msg) {
//printf("准备释放tmsg!\n");
_free(t_msg);
}
}
break;
}
}
}
}
return 0;
}
*/
#else
int the_main();
int the_bullet();
#endif // _WINDOWS
