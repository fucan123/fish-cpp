#ifndef THREAD_CLASS_H
#define THREAD_CLASS_H

class Thread {
public:
	static void bullet();                     //子弹处理
	static void fish();                       //鱼处理
	static void userSave();                   //保存用户信息
	static void userLeave(LPVOID user);       //用户离开的操作
	static void verifyAccount(LPVOID player); //验证帐号
};

#endif 