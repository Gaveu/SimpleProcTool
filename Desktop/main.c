#include"appDef.h"

int main()
{
	int i;
	NTSTATUS status;
	status = Init();
	printf("Init Done 0x%x\n", status);
	
	//总CPU占有率和内存占用
	status = PsGetUsage();
	printf("PsGetUsage Done 0x%x\n", status);

	//进程枚举
	//status = PsEnum();
	//printf("PsEnum Done 0x%x\n", status);

	//根据进程id枚举其线程
	//status = ThdEnumByPid((HANDLE)2320);
	//printf("ThdEnumByPid Done 0x%x\n", status);

	//根据进程id枚举其加载DLL
	//status = ModEnumByPid((HANDLE)1716);
	//printf("ModEnumByPid Done 0x%x\n", status);

	//根据进程id暂停进程
	//status = PsSuspendByPid((HANDLE)2320);
	//printf("PsSuspendByPid Done 0x%x\n", status);

	//根据进程id恢复进程
	//status = PsResumeByPid((HANDLE)2320);
	//printf("PsResumeByPid Done 0x%x\n", status);

	//根据进程id结束进程
	//status = PsTerminateByPid((HANDLE)2320);
	//printf("PsTerminateByPid Done 0x%x\n", status);

	//getSysInfo();

	status = End();
	printf("End Done 0x%x\n", status);
	return 0;
}