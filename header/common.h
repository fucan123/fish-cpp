typedef int                  int32;
typedef long long            int64;
typedef unsigned char        u_char;
typedef unsigned short       u_short;
typedef unsigned int         u_int;
typedef unsigned long long   u_int64;

#define KB 1024
#define MB (1024 * 1024)
#define my_random(min_n, max_n)    (min_n + (rand() % (max_n - min_n + 1)))
#define my_randx(x, min_n, max_n)  (min_n + (x % (max_n - min_n + 1)))
#define MAX_INT   2147483647
#define MAX_UINT  4294967295

enum error_msg_code {
	ERROR_MSG_OP = 1,     //ָ�����
	ERROR_MSG_PRILIVEGE,  //û��Ȩ��
};

typedef struct string_sign {
	char* start;
	char* end;
} StrSign;

typedef struct struct_player {
	SOCKET socket;    //socket
	int    index;     //�������е�����
	int    uid;       //�û�ID
	int    coin;      //���
	int    room;      //�����
	int    seat;      //��λ
	int    hack;      //�Ƿ�����
	int    in_time;   //����ʱ��
	int    game_time; //��Ϸ��ʼʱ��
	int    op_time;   //���һ�β���ʱ��
	int    kick;      //�Ƿ�ǿ�Ƶ�¼(�ߵ�ǰ�ʺ�����)
	void*  tmp;       //��ʱ
	void*  user;      //User��
	int    save_uid;  //���ڱ�����û�ID(�����д��û�����������)
	int    closeing;  //�ر���
} Player;


#ifdef _WINDOWS
double get_cpu_rate() {
	HANDLE hProcess = ::GetCurrentProcess();
	static DWORD s_dwTickCountOld = 0;
	static LARGE_INTEGER s_lgProcessTimeOld = { 0 };
	static DWORD s_dwProcessorCoreNum = 0;
	if (!s_dwProcessorCoreNum)
	{
		SYSTEM_INFO sysInfo = { 0 };
		GetSystemInfo(&sysInfo);
		s_dwProcessorCoreNum = sysInfo.dwNumberOfProcessors;
	}
	double dbProcCpuPercent = 0;
	BOOL bRetCode = FALSE;

	FILETIME CreateTime, ExitTime, KernelTime, UserTime;
	LARGE_INTEGER lgKernelTime;
	LARGE_INTEGER lgUserTime;
	LARGE_INTEGER lgCurTime;

	bRetCode = GetProcessTimes(hProcess, &CreateTime, &ExitTime, &KernelTime, &UserTime);
	if (bRetCode)
	{
		lgKernelTime.HighPart = KernelTime.dwHighDateTime;
		lgKernelTime.LowPart = KernelTime.dwLowDateTime;
		lgUserTime.HighPart = UserTime.dwHighDateTime;
		lgUserTime.LowPart = UserTime.dwLowDateTime;
		lgCurTime.QuadPart = (lgKernelTime.QuadPart + lgUserTime.QuadPart);
		if (s_lgProcessTimeOld.QuadPart)
		{
			DWORD dwElepsedTime = ::GetTickCount() - s_dwTickCountOld;
			dbProcCpuPercent = (double)(((double)((lgCurTime.QuadPart - s_lgProcessTimeOld.QuadPart) * 100)) / dwElepsedTime) / 10000;
			dbProcCpuPercent = dbProcCpuPercent / s_dwProcessorCoreNum;
		}
		s_lgProcessTimeOld = lgCurTime;
		s_dwTickCountOld = ::GetTickCount();
	}

	return dbProcCpuPercent;
}
#else
double get_cpu_rate() {
	return 0;
}
#endif // _WINDOWS

#include "header/sha1.h"
#include "header/base64.h"
#include "header/g.h"