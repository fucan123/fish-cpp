template <typename T>
struct S_Link {
	T value;
	S_Link* prev;
	S_Link* next;
};

template<typename T>
class Link {
private:
	S_Link<T>*  _link;
	S_Link<T>** _end;
	int _total;
public:
	Link();
	void init();
	int total();
	S_Link<T>* getLink() const;
	S_Link<T>* add(T value) const;
};

template <typename T> 
Link<typename T>::Link() {
	this->init();
}

template <typename T> void
Link<typename T>::init() {
	this->_link = NULL;
	this->_end = &this->_link;
}

template <typename T> int
Link<typename T>::total() {
	return this->_total;
}

template <typename T> 
inline S_Link<T>*
Link<typename T>::getLink() const {
	return this->_link;
}

template <typename T> S_Link<T>*
Link<typename T>::add(T value) const {
	S_Link<T>* link = _malloc(sizeof(S_Link<T>));
	link->prev = *(this->_end);
	link->next = NULL;
	*(this->_end) = link;
	this->_end = &(link->next);
	this->_total++;
	return link;
}