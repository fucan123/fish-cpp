bool strneq(char* str_1, char* str_2, u_int len) { //比较两个字符串是否相等, len=比较的长度
	bool eq = false;
	for (u_int i = 0; i < len; i++, str_1++, str_2++) {
		if (*str_1 != *str_2) {
			return false;
		}
	}
	return true;
}

char* fmsg(char* msg, int* code) {
	char fin = (unsigned char)*msg >> 7;
	char open_code = *msg & 0x0f;
	if (open_code == 8) {
		//printf("open code 8\n");
		*code = -1;
		return 0;
	}

	msg++;

	char mask = *msg & 0x80;
	if (mask == 0) {
		//printf("no mask\n");
		*code = -1;
		return 0;
	}

	unsigned int playod_len = *msg & 0x7f;
	if (playod_len == 0) {
		*code = 0;
		return 0;
	}

	//printf("playod len:%d\n", playod_len);
	msg++;
	if (playod_len == 126) { //网络字节, 需要自己转换
		playod_len  =  0x0000ff00 & *msg;
		playod_len |= (0x000000ff & *(msg + 1));
		//playod_len = *(unsigned short*)msg;
		msg += 2;
	}
	if (playod_len == 127) { //网络字节, 需要自己转换
		playod_len  =  0xff000000 & *(msg + 4);
		playod_len |= (0x00ff0000 & *(msg + 5));
		playod_len |= (0x0000ff00 & *(msg + 6));
		playod_len |= (0x000000ff & *(msg + 7));
		//丢弃了前4位
		msg += 8;
	}

	//printf("fin: %d, len: %d\n", fin, playod_len);
	int mask_key = *(int*)msg;
	msg += 4;

	char* nmsg = (char*)_malloc(playod_len + 1);
	char* tmp = (char*)&mask_key;
	for (u_int i = 0; i < playod_len; i++) {
		nmsg[i] = *msg ^ *(tmp + (i % 4));
		msg++;
	}
	nmsg[playod_len] = 0;

	*code = 1;
	return nmsg;
}

char* emsg(char fin, char open_code, char* msg, u_int* new_len) {
	u_int len = strlen(msg), index = 0;
	char* nmsg = (char*)_malloc(len + 12);
	nmsg[0] = (fin << 7 & 0x80) | (open_code & 0x0f);
	//index++;
	//nmsg[0] = 0x81;
	if (len < 126) {
		nmsg[1] = len | 0x0;
		index = 2;
	}
	else if (len < 0x10000) {
		nmsg[1] = 126 | 0x0;
		nmsg[2] = (len >> 8) & 0xff;
		nmsg[3] = len & 0xff;
		index = 4;
		//printf("xlen:%d, %d, %d, %d.\n", len, nmsg[1], nmsg[2], nmsg[3] & 0x0ff);
	}
	else {
		nmsg[1] = 0;
		index = 2;
		len = 0;
	}

	memcpy(nmsg + index, msg, len);
	*new_len = len + index;
	//printf("%s\n", nmsg + index);
	return nmsg;

}

char** emsg2(char fin, char open_code, char* msg, unsigned int len, int* row) {
	unsigned int index = 0;
	char** nmsg = NULL;
	//nmsg[0] = (fin << 7 & 0x80) | (open_code & 0x0f);
	int c_len = len;
	if (len <= 126) {
		*row = 1;
		nmsg[0] = (char*)_malloc(len + 2);
		nmsg[0][0] = 0x80 | (open_code & 0x0f);
	}

	memcpy(nmsg + index, msg, c_len);
	nmsg[index + c_len] = 0;
	//printf("%s\n", nmsg + index);
	return nmsg;

}

char* copy_str(char* start, char* end) {
	int len = end - start;
	if (len <= 0) {
		return NULL;
	}
	char* str = (char*)_malloc(len + 1);
	char* s = start;
	char* tmp = str;
	for (; s < end; s++, tmp++) {
		if (*s == '\\') {
			if (s + 1 < end) {
				*tmp = *++s;
			}
		}
		else {
			*tmp = *s;
		}
	}
	*tmp = 0;

	return str;
}

void sendmsg(char* msg, Player* player) {
	u_int send_len;
	char* _send_str_;
	if (!msg) {
		_send_str_ = emsg(1, 1, "null", &send_len);
	}
	else {
		_send_str_ = emsg(1, 1, msg, &send_len);
	}

	send(player->socket, _send_str_, send_len, 0);
	_free(_send_str_, "send str");
	//printf("释放发送字符完成\n");
}

int strpos(char* text, char* need, u_int start = 0, u_int end=0) { //查找字符串need在text出现的位置[0起始], start=开始搜索位置, end=结束搜索位置
	if (end > 0) {
		end += (u_int)text + 1;
	}
	int need_len = strlen(need);
	bool find = false;
	char* ptr = text + start;
	while (*ptr) {
		if (end > 0 && ((u_int)ptr + need_len) > end) {
			break;
		}
		if (strneq(ptr, need, need_len)) {
			find = true;
			break;
		}
		ptr++;
	}
	return find ? ptr - text : -1;
}