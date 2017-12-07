typedef struct struct_room {
	Fishs*   fishs;
	Bullets* bullets;
	Player*  players[4];
	double   create_time; //创建时间
} Room;

class Rooms {
private:
	Room* _rooms;
	int   _max_room;
	int   _curr;
public:
	Rooms();
	~Rooms();
	bool setRooms(int max_room);
	void findRoom(int* room, int* seat);
	void leave(Player* player);
	inline Room* getRoom(int index);
	inline bool isEmpty(Room *room);
	inline int  getMaxRoom();
};

Rooms::Rooms() :_curr(0) {};
Rooms::~Rooms() { delete[] _rooms; };

bool Rooms::setRooms(int max_room) {
	_max_room = max_room;
	_rooms = new Room[max_room];
	memset(_rooms, 0, max_room * sizeof(Room));
	return _rooms != NULL;
}

void Rooms::findRoom(int* room, int* seat) {
	for (int i = _curr; i < _max_room; i++) {
		*room = i + 1;
		Room r = _rooms[_curr];
		for (int j = 0; j < 4; j++) {
			if (r.players[j] == NULL) {
				*seat = j + 1;
				if (j == 3) {
					_curr++;
				}
				return;
			}
		}
	}
	for (int i = _curr; i >= 0; i--) {
		*room = i + 1;
		Room r = _rooms[_curr];
		for (int j = 0; j < 4; j++) {
			printf("p: %d\n", r.players[j]);
			if (r.players[j] == NULL) {
				*seat = j + 1;
				if (j == 3) {
					_curr++;
				}
				return;
			}
		}
	}
	*room = 0;
	*seat = 0;
}

inline Room* Rooms::getRoom(int index) {
	return &_rooms[index];
}

inline bool Rooms::isEmpty(Room* room) {
	for (int i = 0; i < 4; i++) {
		if (room->players[i] != NULL) {
			return false;
		}
	}
	return true;
}

inline int Rooms::getMaxRoom() {
	return _max_room;
}

void Rooms::leave(Player* player) {
	if (player->room == 0) {
		return;
	}

	Room* room = this->getRoom(player->room - 1);
	room->players[player->seat - 1] = NULL;

	if (this->isEmpty(room)) {
		//printf("room empty, %d, %p\n", _player->room - 1, room->fishs);
		if (room->fishs != NULL) {
			room->fishs->clear();
			_free(room->fishs);
			room->fishs = NULL;
			printf("clear fishs\n");
		}
		if (room->bullets != NULL) {
			room->bullets->clear();
			_free(room->bullets);
			room->bullets = NULL;
			printf("clear bullets\n");
		}
	}
	player->room = 0;
	player->seat = 0;
}

Rooms rooms;