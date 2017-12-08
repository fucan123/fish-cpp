class Cnf {
private:
	int _len;
	char** _key;
	char** _value;
public:
	Cnf() {
		_len = 0;
		_key = new char* [50];
		_value = new char*[50];
		for (int i = 0; i < 50; i++) {
			_key[i] = new char[50];
			_value[i] = new char[50];
		}
	}

	bool parse(char* file) {
		FILE* f = fopen(file, "r");
		if (!f) {
			printf("�˳���Ŀ¼�²������ļ�%s.\n", file);
			return false;
		}

		int i = 0;
		char line[100];
		while (!feof(f)) {
			if (_len == 49) {
				break;
			}
			line[0] = ';';
			fgets(line, 50, f);
			//printf("line: %s, %d\n", line, i);
			if (line[0] == ';' || line[0] == '#' || line[0] == '[' || line[0] == ' ') {
				i++;
				int len = strlen(line);
				if (line[len - 1] == '\n') {
					i = 0;
				}
				continue;
			}
			if (i == 0) {
				int x = 0;
				char* tmp = line;
				while (*tmp && *tmp != ' ' && *tmp != '=' && *tmp != ';' && *tmp != '\r' && *tmp != '\n') {
					_key[_len][x] = *tmp;
					//printf("c: %c\n", *tmp);
					tmp++;
					x++;
					if (x == 49) {
						break;
					}
				}
				_key[_len][x] = 0;

				x = 0;
				while (*tmp && (*tmp == ' ' || *tmp == '=')) tmp++;
				while (*tmp && *tmp != ' ' && *tmp != ';' && *tmp != '\r' && *tmp != '\n') {
					_value[_len][x] = *tmp;
					tmp++;
					x++;
					if (x == 49) {
						break;
					}
				}
				_value[_len][x] = 0;

				_len++;
				i++;
			}

			int len = strlen(line);
			if (line[len - 1] == '\n') {
				i = 0;
			}
		}

		return true;
	}

	bool check() {
		int status = 1;
		if (this->get("mysql_host") == 0) {
			printf("����: δ����MYSQL��������ַ; ��ʽ: mysql_host = MYSQL��������ַ\n");
			status = 0;
		}
		if (this->get("mysql_user") == 0) {
			printf("����: δ����MYSQL�������û�; ��ʽ: mysql_user = MYSQL�������û�\n");
			status = 0;
		}
		if (this->get("mysql_pword") == 0) {
			printf("����: δ����MYSQL����������; ��ʽ: mysql_pword = MYSQL����������\n");
			status = 0;
		}
		if (this->get("mysql_port") == 0) {
			printf("����: δ����MYSQL�������˿�; ��ʽ: mysql_port = MYSQL�������˿�\n");
			status = 0;
		}
		if (this->get("mysql_dbase") == 0) {
			printf("����: δ����MYSQL���ݿ�; ��ʽ: mysql_dbase = MYSQL���ݿ�\n");
			status = 0;
		}
		if (this->get("redis_host") == 0) {
			printf("����: δ����REDIS��������ַ; ��ʽ: redis_host = REDIS��������ַ\n");
			status = 0;
		}
		if (this->get("redis_port") == 0) {
			printf("����: δ����REDIS�������˿�; ��ʽ: redis_port = REDIS�������˿�\n");
			status = 0;
		}
		if (this->get("memory_size") == 0) {
			printf("����: δ�����ڴ������; ��ʽ: memory_size = ����(K����KB, M����MB)\n");
			status = 0;
		}
		return status;
	}

	char* get(char* key) {
		for (int i = 0; i < _len; i++) {
			if (strcmp(_key[i], key) == 0) {
				return _value[i];
			}
		}
		return 0;
	}

	int get_int(char* key) {
		char* val = this->get(key);
		if (val == 0) {
			return 0;
		}
		return atoi(val);
	}

	int get_memory_size() {
		char* mszie = this->get("memory_size");
		int memory_size = mszie != NULL ? atoi(mszie) : 0;
		if (strchr(mszie, 'K') != 0) {
			memory_size *= 1024;
		}
		if (strchr(mszie, 'M') != 0) {
			memory_size *= 1024 * 1024;
		}
		if (memory_size <= 0) {
			memory_size = 1 * 1024 * 1024;
		}
		return memory_size;
	}

	void destroy() {
		for (int i = 0; i < 50; i++) {
			delete[] _key[i];
			delete[] _value[i];
		}
		delete[] _key;
		delete[] _value;
	}
};