
#include "source/entry.hpp"

class WebSocket {
public:
	Player* player;
public:
	WebSocket(u_long host, u_short port);
	static void run(LPVOID websocket); //运行
	inline void onconnect(SOCKET, SOCKADDR_IN*); //连接操作
	inline void onopen(); //打开处理
	inline void onmessage(char* msg); //消息处理
	inline void onclose(); //关闭处理
	inline bool onread();  //set read前进行的检查
	void hack(char* header);
};

WebSocket::WebSocket(u_long host, u_short port) {
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
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WebSocket::run, this, 0, &dwThreadId); 
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread::bullet, NULL, 0, &dwThreadId); 
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread::fish, NULL, 0, &dwThreadId); 
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread::userSave, NULL, 0, &dwThreadId);
	
	while (true) {
		sClient = accept(sListen, (struct sockaddr *)&client, &iaddrSize);
		
		printf("连接\n");
		
		printf("consck: %d\n", sClient);
		this->onconnect(sClient, &client);
	}
}

void WebSocket::run(LPVOID websocket) {
	WebSocket* $this = (WebSocket*)websocket;
	int            i;
	fd_set         fdread;
	int            ret;
	struct timeval tv = { 1, 200 };
	char msg[140];
	printf("正在监听...\n");
	//printf("%d", Players::len);
	while (true) {
		//Sleep(1);
		//printf("while\n");
		FD_ZERO(&fdread);
		for (i = 0; i < 1024; i++) {
			Player* p = &Players::players[i];
			if (p->socket) {
				$this->player = p;
				if ($this->onread()) {
					FD_SET(p->socket, &fdread);
				}
				else { //断开连接
					$this->onclose();
				}
			}
		}

		ret = select(0, &fdread, NULL, NULL, &tv);
		if (ret == 0) {
			continue;
		}
		for (i = 0; i < 1024; i++) {
			if (!Players::players[i].socket) {
				continue;
			}
			if (FD_ISSET(Players::players[i].socket, &fdread)) {
				$this->player = &Players::players[i];
				int size = 20;
				char str[20];
				if (ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)) {
					$this->onclose();
				}
				else {
					char msg[1024];
					msg[0] = 0;
					char ch;
					ret = recv($this->player->socket, msg, 1024 - 1, 0);
					if (ret < 6) {
						$this->onclose();
						continue;
					}
					while (ret > 0) {
						//printf("ret1: %d, sk: %d\n", ret, Cache::sockets[i]);
						msg[ret] = 0;
						if (ret == 1023) {
							ret = recv($this->player->socket, &ch, 1, 0);
						}
						else {
							ret = 0;
						}
						//printf("ret2: %d\n", ret);
					}
					//printf("sk: %d\n", Cache::sockets[i]);
					if ($this->player->hack == 0) { //没有握手
						$this->hack(msg);
					}
					else {
						$this->onmessage(msg); //出来消息
					}
					break;
				}
			}
		}
	}
}

void WebSocket::onconnect(SOCKET socket, SOCKADDR_IN* client) {
	int now_time = gettime(), count = 1;
	int start_time = now_time;
	u_long ip = client->sin_addr.S_un.S_addr;
	char key[32];
	sprintf(key, "ip_limit_%ld", ip);
	char* result = Redis::$class->get(key);
	if (result) {
		Json json;
		json.parseString(result);
		start_time = json.valueToInt("start_time");
		count = json.valueToInt("count") + 1;
		json.clear();
		_free(result);
		if (count > 10000) {
			closesocket(socket);
			goto end;
		}
	}
	Players::add(socket);
end:
	char value[128];
	sprintf(value, "{'start_time':%d,'end_time':%d,'count':%d}", start_time, now_time, count);
	//printf("%s\n", value);
	Redis::$class->set(key, value);
}

void WebSocket::onopen() {
}

void WebSocket::onmessage(char* msg) {
	if (!Setting::$class->closeing) { //服务器不在关闭中...
		int code = 0;
		char* t_msg = fmsg(msg, &code);
		if (code == -1) {
			this->onclose();
			printf("刷新离开!\n");
		}
		if (code == 1) {
			Json json;
			json.parseString(t_msg);
			Entry entry(this->player, &json);
			json.clear();
		}
		if (t_msg) {
			_free(t_msg);
		}
	}
}

void WebSocket::onclose() {
	printf("关闭\n");
	rooms.leave(this->player);
	Players::remove(this->player);
}

void WebSocket::hack(char* header) {
	char* key = strstr(header, "Sec-WebSocket-Key:");
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
		int sd = send(this->player->socket, upgrade, strlen(upgrade), 0);
		this->player->hack = 1;
	}
}

bool WebSocket::onread() {
	int now_time = gettime();
	S_Setting_Value* sval = Setting::$class->values();
	//验证帐号允许的最大超时时间
	if (sval->no_verify_account_timout > 0 && (now_time - this->player->in_time) > sval->no_verify_account_timout && !this->player->uid) {
		return false;
	}
	//还未进行游戏允许的最大时间
	if (sval->no_game_timeout > 0 && (now_time - this->player->in_time) > sval->no_game_timeout && !this->player->room) {
		return false;
	}
	//最大允许的操作间隔时间
	if (sval->no_op_timeout > 0 && (now_time - this->player->op_time) > sval->no_op_timeout) {
		return false;
	}
	return true;
}