class Explode {
private:
	char** _array;
	char*  _dst;
	char*  _ptrs[10][2];
	unsigned int _len;
	void copy();
public:
	Explode(char  delimiter, char* str);
	Explode(char* delimiter, char* str);
	
	inline unsigned int length();
	inline int getInt(unsigned int);
	inline char* operator[](unsigned int);

	~Explode();
};

Explode::Explode(char  delimiter, char* str) {
	this->_array = NULL;
	this->_len = 0;
	if (str != NULL && *str) {
		this->_dst = str;
		memset(this->_ptrs, 0, sizeof(char*) * 20);
		char* start = str;
		int i = 0;
		for (; *str; i++, str++) {
			if (*str == delimiter) {
				if (this->_len < 9) {
					this->_ptrs[this->_len][0] = start;
					this->_ptrs[this->_len][1] = str;
					this->_len++;
				}
				start = str + 1;
			}
		}
		if (this->_len < 9) {
			this->_ptrs[this->_len][0] = start;
			this->_ptrs[this->_len][1] = str;
			this->_len++;
		}
		this->copy();
	}
}

Explode::Explode(char* delimiter, char* str) {
	this->_array = NULL;
	this->_len = 0;
	int dlen = strlen(delimiter);
	if (str != NULL && *str && dlen > 0) {
		memset(this->_ptrs, 0, sizeof(char*) * 20);
		char* dm = delimiter;
		char* start = str;
		int i = 0;
		for (; *str; i++, str++) {
			if (*str == *dm) {
				bool find = true;
				for (; *dm; ++dm, ++str) {
					if (*str != *dm) {
						find = false;
						break;
					}
				}
				dm = delimiter;
				if (find) {
					if (this->_len < 9) {
						this->_ptrs[this->_len][0] = start;
						this->_ptrs[this->_len][1] = str - dlen;
						this->_len++;
					}
					start = str;
					str--;
				}
			}
			
		}
		if (this->_len < 9) {
			this->_ptrs[this->_len][0] = start;
			this->_ptrs[this->_len][1] = str;
			this->_len++;
		}
		this->copy();
	}
}

void Explode::copy() {
	this->_array = (char**)_malloc(sizeof(char*) * this->_len);
	for (int i = 0; i < 10; i++) {
		if (this->_ptrs[i][0] == 0) {
			break;
		}
		if (this->_ptrs[i][0] == this->_ptrs[i][1]) {
			this->_array[i] = NULL;
		}
		else {
			int len = this->_ptrs[i][1] - this->_ptrs[i][0];
			this->_array[i] = (char*)_malloc(len + 1);
			char* str = this->_ptrs[i][0];
			char* dst = this->_array[i];
			int j = 0;
			for (; str < this->_ptrs[i][1]; j++, str++, dst++) {
				*dst = *str;
			}
			*dst = 0;
		}
	}
	for (int i = 0; i < this->_len; i++) {
		printf("%d.%s\n", i, this->_array[i]);
	}
}

unsigned int inline Explode::length() {
	return this->_len;
}

int Explode::getInt(unsigned int index) {
	char* str = index >= this->_len ? NULL : this->_array[index];
	return str == NULL ? 0 : atoi(str);
}

char* Explode::operator[](unsigned int index) {
	return index >= this->_len ? NULL : this->_array[index];
}

Explode::~Explode() {
	for (int i = 0; i < this->_len; i++) {
		if (this->_array[i]) {
			_free(_array[i]);
		}
	}
}