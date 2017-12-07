inline double getmtime() { //获取有毫秒的时间
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

inline long gettime() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec;
}

u_long ip2long(const char* ip) {
	unsigned char a, b, c, d;
	sscanf(ip, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d);
	return ((a << 24) | (b << 16) | (c << 8) | d);
}

void long2ip(u_long ip, char buf[]) {
	int i = 0;
	u_long tmp[4] = { 0 };

	for (i = 0; i < 4; i++) {
		tmp[i] = ip & 255;
		ip = ip >> 8;
	}
	sprintf(buf, "%lu.%lu.%lu.%lu", tmp[3], tmp[2], tmp[1], tmp[0]);
}
