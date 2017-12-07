#define _malloc(size) memory_alloc(size)
#define _free(ptr)    memory_free(ptr)

typedef struct memory_var_struct {
	void*   first_ptr;    //申请内存首地址
	void*   end_ptr;      //申请内存尾地址
	void*   use_ptr;      //内存使用情况标志首地址
	u_int   use;          //内存使用大小
	u_int   total;        //内存总大小
	u_int64 system_use;   //系统内存使用大小(malloc或new)
	u_int64 malloc_count; //累计申请内存次数
	u_int64 free_count;   //累计释放内存次数
} memory_var;

int alloc_i = 0, free_i = 0;

static int _memory_con_[8][3] = {
	{ 50, 200, 1000 }, //0 内存块大小
	{ 1, 2, 3 }, //1 内存块比例
	{ 0, 0, 0 }, //2 地址位开始位置
	{ 0, 0, 0 }, //3 地址位结束位置
	{ 0, 0, 0 }, //4 bit位开始位置
	{ 0, 0, 0 }, //5 bit位结束位置
	{ 0, 0, 0 }, //6 bit位已用位置
	{ 0, 0, 0 }, //7 bit位已用数量
};
static int m_con_len = 3;
static memory_var m_var;

bool memory_init(u_int byte) {
	//printf("m len: %d\n", m_con_len);
	u_int one_m = 0, sum_bili = 0;
	for (int i = 0; i < m_con_len; i++) {
		one_m += _memory_con_[0][i];
		sum_bili += _memory_con_[1][i];
	}
	if (byte < one_m) {
		byte = one_m;
	}
	//printf("byte:%d, sum bili: %d\n", byte, sum_bili);
	u_int size = 0, bit_size = 0, start = 0, end = 0;
	for (int i = 0; i < m_con_len; i++) {
		u_int m = byte * _memory_con_[1][i] / sum_bili; //获得每块实际的内存大小
		//printf("m1: %d, mod: %d\n", m, m % _memory_con_[0][i]);
		u_int mod = m % _memory_con_[0][i];
		if (mod != 0) { 
			m += _memory_con_[0][i] - mod; //保证内存是配置的整数倍
		}
		//printf("m2: %d\n", m);
		u_int bit = m / _memory_con_[0][i]; //获得标记内存需要使用的位数(用于标记内存是否使用, 按位计算)
		bit_size += bit;

		//printf("%d / %d: %d\n", m, _memory_con_[0][i], bit);
		//printf("bs: %d\n", m, _memory_con_[0][i], bit);

		start = end;
		end += bit;
		_memory_con_[2][i] = size;
		_memory_con_[3][i] = m + size;
		_memory_con_[4][i] = start;
		_memory_con_[5][i] = end;

		//printf("s: %d, e: %d\n", start, end);
		size += m;
		//printf("m:%d\n", m);
	}
	//printf("bytes: %d， 1M: %d\n", size, 100 * M);
	u_int bit_m = bit_size / 8;
	u_int bit_mod = bit_size % 8;
	//printf("bit size: %d， bit mod: %d\n", bit_size, 3 % 8);
	if (bit_mod != 0) {
		bit_m++;
	}
	//printf("all m:%d, bit m:  %d\n", size, bit_size);
	m_var.first_ptr = new char[size];
	m_var.use_ptr = new char[bit_m];
	m_var.end_ptr = (void*)((int)m_var.first_ptr + size - 1);
	m_var.use = 0;
	m_var.total = size;
	//printf("ptr:  %d\n", m_var.first_ptr);
	memset(m_var.first_ptr, 0, size);
	memset(m_var.use_ptr, 0, bit_m);

	int u = 0x0000000f & *(char*)m_var.use_ptr;
	//printf("char ..: %d, bit m:%d\n", u, bit_m);

	for (int i = 0; i < m_con_len; i++) {
		_memory_con_[2][i] += (u_int)m_var.first_ptr;
		_memory_con_[3][i] += (u_int)m_var.first_ptr;
		//printf("%d:%p - %p\n", i, _memory_con_[2][i], _memory_con_[3][i]);
	}

	return m_var.first_ptr && m_var.use_ptr && 1;
}

void* memory_alloc(int size, char* ext_str=NULL) {
	if (size < 1) {
		return NULL;
	}
	m_var.malloc_count++; //内存申请次数
	//printf("alloc i(%d)\n", alloc_i);
	int index = -1;
	for (int i = 0; i < m_con_len; i++) {
		if (size <= _memory_con_[0][i]) {
			index = i;
			break;
		}
	}
	void* ptr = 0;
	if (index == -1) {
		m_var.system_use += size;
		ptr = new char[size];
	}
	else {
		m_var.use += _memory_con_[0][index]; //内存使用数
		_memory_con_[7][index]++; //使用空间数+1

		int bit_index = _memory_con_[6][index];
		int bit_pos = bit_index + _memory_con_[4][index];
		if (bit_pos >= _memory_con_[5][index]) {
			bit_pos = _memory_con_[4][index];
			int pos = 0;
			for (int i = 0; i < index; i++) {
				//printf("b:%d,i:%d \n", _memory_con_[4][i], i);
				pos += _memory_con_[4][i];
			}
			//_memory_con_[4][index] = pos;
			//system("pause");
		}
		//printf("bit pos: %d, %d\n", bit_pos, _memory_con_[4][index]);
		int free_index = -1;
		for (int i = bit_pos; i < _memory_con_[5][index]; i++) {
			int byte_start = i >> 3; //开始字节位置  1
			int bit_start = i & 0x07;  //开始bit位位置 2
			//printf("byte start: %d\n", byte_start);
			char* use_ptr = (char*)((int)m_var.use_ptr + byte_start); //标记内存地址
			int u = 0x0000000f & *(char*)m_var.use_ptr;
			//printf("char: %d\n", u);
			if (((*use_ptr >> bit_start) & 1) == 0) {
				//printf("use ptr:%p, %d; index:%d; bit:%d, i:%d\n", use_ptr, bit_start, i, _memory_con_[4][index], index);
				free_index = i;
				*use_ptr |= 1 << bit_start;
				break;
			}
		}
		if (free_index == -1) {
			for (int i = bit_pos; i > _memory_con_[4][index]; i--) {
				int byte_start = i >> 3; //开始字节位置  1
				int bit_start = i & 0x07;  //开始bit位位置 2
				char* use_ptr = (char*)((int)m_var.use_ptr + byte_start); //标记内存地址
				//printf("use ptr:%p, %d\n", use_ptr, bit_start);
				if (((*use_ptr >> bit_start) & 1) == 0) {
					free_index = i;
					*use_ptr |= 1 << bit_start;
					break;
				}
			}
		}
		//printf("free index: %d\n", free_index);
		if (free_index == -1) {
			printf("system alloc(%s)\n", ext_str);
			ptr = new char[size];
		}
		else {
			//printf("alloc.\n");
			u_int offset_ptr = (free_index - _memory_con_[4][index])* _memory_con_[0][index];
			ptr = (void*)((int)_memory_con_[2][index] + offset_ptr); //首地址加偏移地址
			//printf("get ptr:%d\n", ptr);
			_memory_con_[6][index]++;
		}
	}

	return ptr;
}

void memory_free(void* ptr, char* ext_str=NULL) {
	m_var.free_count++; //内存释放次数
	//printf("free i(%d, %p)\n", free_i, ptr);
	if (ptr >= m_var.first_ptr && ptr <= m_var.end_ptr) {
		//printf("get free\n");
		bool ok = 0;
		int p = (int)ptr;
		for (int i = 0; i < m_con_len; i++) {
			if (p >= _memory_con_[2][i] && p < _memory_con_[3][i]) {
				ok = 1;
				m_var.use -= _memory_con_[0][i]; //内存使用数
				_memory_con_[7][i]--; //使用空间数-1
				//printf("get free\n");
				u_int start = (p - _memory_con_[2][i]) / _memory_con_[0][i]; //开始字节位置  1
				u_int bit_pos = start + _memory_con_[4][i];  //开始bit位位置 2
				//printf("free bit pos: %d, %d\n", bit_pos, _memory_con_[4][i]);
				u_int byte_start = bit_pos >> 3; //开始字节位置  1
				u_int bit_start = bit_pos & 0x07;  //开始bit位位置 2
				char* use_ptr = (char*)((int)m_var.use_ptr + byte_start); //标记内存地址
				*use_ptr &= ~(1 << bit_start);
				//printf("free use ptr:%p, %d; bit: %d, i:%d\n", use_ptr, bit_start, _memory_con_[4][i], i);
				//printf("char:%d\n", *use_ptr);
				//printf("free ptr:%p\n", ptr);
				break;
			}
		}
		if (ok == 0) {
			printf("释放内存失败!\n");
		}
	}
	else {
		printf("system free(%s)\n", ext_str);
		delete[] ptr;
	}
}

memory_var* const memory_data() {
	return &m_var;
}

int* memory_con() {
	return (int*)_memory_con_;
}

void memory_destroy() {
	if (m_var.first_ptr) delete[] m_var.first_ptr;
	if (m_var.use_ptr) delete[] m_var.use_ptr;
}