#ifndef THREAD_CLASS_H
#define THREAD_CLASS_H

class Thread {
public:
	static void bullet();                     //�ӵ�����
	static void fish();                       //�㴦��
	static void userSave();                   //�����û���Ϣ
	static void userLeave(LPVOID user);       //�û��뿪�Ĳ���
	static void verifyAccount(LPVOID player); //��֤�ʺ�
};

#endif 