#include"appDef.h"

//全局单链表清空,保留头节点
VOID EmptyGlProc()
{
	PMYPROCESS pTmp = glProcListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		free(glProcListEntry.next);
		glProcListEntry.next = pTmp;
	}
}
VOID EmptyGlThd()
{
	PMYTHREAD pTmp = glThdListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		free(glThdListEntry.next);
		glThdListEntry.next = pTmp;
	}
}
VOID EmptyGlMod()
{
	PMYMODINFO pTmp = glModListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		free(glModListEntry.next->DllPath.Buffer);
		free(glModListEntry.next);
		glModListEntry.next = pTmp;
	}
}




/*驱动层通信函数*/
//打开并连接到指定的设备对象
NTSTATUS Init()
{
	hDevice = CreateFile(
		TEXT("\\\\.\\ProcToolDev"),
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Init:File Create Failed!\n");
		return STATUS_INVALID_HANDLE;
	}
	glBuf = (PCHAR)malloc(DE_MAX_BUF_LEN*sizeof(CHAR));
	if (!glBuf)
	{
		printf("Init:Memory Allocate Failed!\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	return STATUS_SUCCESS;
}

//程序结束时的执行资源释放等操作
NTSTATUS End()
{
	CloseHandle(hDevice);
	free(glBuf);
	return STATUS_SUCCESS;
}

//进程枚举通信函数
//向设备对象发送功能码EnumProc
//将从驱动层传出的信息整合进入临时全局进程链表
//整合完毕后根据全局链表，依次输出结点信息并销毁结点
NTSTATUS PsEnum()
{
	ULONG rLen = 0;
	ULONG wLen = 0;
	ULONG cpuUsage = 0;
	SIZE_T memUsed = 0;
	SIZE_T vmemUsed = 0;

	NTSTATUS status;
	PMYPROCESS pMyProcTmp = NULL;


	//向设备对象发送功能码EnumProc
	status = DeviceIoControl(
		hDevice,
		EnumProc,
		NULL,
		0,
		glBuf,
		DE_MAX_BUF_LEN * sizeof(CHAR),
		&wLen,
		NULL
	);
	if (!status)
	{
		status = GetLastError();
		printf("PsEnum():EnumPs Failed!0x%x\n", status);
		EmptyGlProc();
		return status;
	}

	printf("PsEnum():wlen %d\n", wLen);

	//将从驱动层传出的信息整合进入临时全局进程链表
	while (rLen < wLen)
	{
		pMyProcTmp = (PMYPROCESS)malloc(sizeof(MYPROCESS));
		if (!pMyProcTmp)
		{
			printf("PsEnum():Memory Insufficient!\n");
			EmptyGlProc();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		memcpy_s(
			pMyProcTmp,
			sizeof(MYPROCESS),
			glBuf + rLen,
			sizeof(MYPROCESS)
		);
		rLen += sizeof(MYPROCESS);

		pMyProcTmp->next = glProcListEntry.next;
		glProcListEntry.next = pMyProcTmp;
	}

	//整合完毕后根据全局链表，依次输出结点信息并销毁结点
	pMyProcTmp = glProcListEntry.next;
	while (pMyProcTmp)
	{
		//status = PsGetUsage(
		//	(HANDLE)pMyProcTmp->ulPid,
		//	&cpuUsage,
		//	&memUsed,
		//	&vmemUsed
		//);
		//printf("%s\n", pMyProcTmp->ImageFileName);
		//printf("pid:%5d\tppid:%5d\t%u\t%u\\%u\n\n",
		//	pMyProcTmp->ulPid,
		//	pMyProcTmp->ulPpid,
		//	cpuUsage,
		//	memUsed,
		//	vmemUsed);

		printf("%s\n", pMyProcTmp->ImageFileName);
		printf("pid:%5u\tppid:%5u\n\n",
			pMyProcTmp->ulPid,
			pMyProcTmp->ulPpid
		);

		pMyProcTmp = pMyProcTmp->next;
		free(glProcListEntry.next);
		glProcListEntry.next = pMyProcTmp;
	}

	return STATUS_SUCCESS;
}

//线程枚举通信函数
//向设备对象发送功能码EnumThd及指定的进程ID
//将从驱动层传出的信息整合进入临时全局线程链表
//整合完毕后根据全局链表，依次输出结点信息并销毁结点
NTSTATUS ThdEnumByPid(IN HANDLE pid)
{
	ULONG rLen = 0;
	ULONG wLen = 0;
	NTSTATUS status;
	PMYTHREAD pMyThdTmp = NULL;

	//向设备对象发送功能码EnumThd及指定的进程ID
	status = DeviceIoControl(
		hDevice,
		EnumThd,
		&pid,
		sizeof(HANDLE),
		glBuf,
		DE_MAX_BUF_LEN * sizeof(CHAR),
		&wLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("ThdEnumByPid():EnumThd Failed!0x%x\n", status);
		EmptyGlThd();
		return status;
	}

	//将从驱动层传出的信息整合进入临时全局线程链表
	while (rLen < wLen)
	{
		pMyThdTmp = (PMYTHREAD)malloc(sizeof(MYTHREAD));
		if (!pMyThdTmp)
		{
			printf("ThdEnumByPid():Memory Insufficient!\n");
			EmptyGlThd();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		memcpy_s(
			pMyThdTmp,
			sizeof(MYTHREAD),
			glBuf + rLen,
			sizeof(MYTHREAD)
		);
		rLen += sizeof(MYTHREAD);
		
		pMyThdTmp->next = glThdListEntry.next;
		glThdListEntry.next = pMyThdTmp;
	}

	//整合完毕后根据全局链表，依次输出结点信息并销毁结点
	pMyThdTmp = glThdListEntry.next;
	while (pMyThdTmp)
	{
		printf("Tid:%5d\tPrioty:%3d\tState:%3d\n",
			pMyThdTmp->ulTid,
			pMyThdTmp->cPrioty,
			pMyThdTmp->ucState
		);
		pMyThdTmp = pMyThdTmp->next;
		free(glThdListEntry.next);
		glThdListEntry.next = pMyThdTmp;
	}
	return STATUS_SUCCESS;
}

//进程模块枚举通信函数
//向设备对象发送功能码EnumPMod及指定的进程ID
//将从驱动层传出的信息整合进入临时全局模块链表
//整合完毕后根据全局链表，依次输出结点信息并销毁结点
NTSTATUS ModEnumByPid(IN HANDLE pid)
{
	ULONG rLen = 0;
	ULONG wLen = 0;
	PMYMODINFO pMyModInfo = NULL;
	NTSTATUS status;

	//向设备对象发送功能码EnumPMod及指定的进程ID
	status = DeviceIoControl(
		hDevice,
		EnumPMod,
		&pid,
		sizeof(HANDLE),
		glBuf,
		DE_MAX_BUF_LEN*sizeof(CHAR),
		&wLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("ModEnumByPid():EnumMod Failed!0x%x\n",status);
		EmptyGlMod();
		return status;
	}

	//将从驱动层传出的信息整合进入临时全局模块链表
	while (rLen < wLen)
	{
		pMyModInfo = (PMYMODINFO)malloc(sizeof(MYMODINFO));
		if (!pMyModInfo)
		{
			printf("ModEnumByPid():Memory Insufficient!\n");
			EmptyGlMod();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		//拷贝信息结构体
		memcpy_s(
			pMyModInfo,
			sizeof(MYMODINFO),
			glBuf + rLen,
			sizeof(MYMODINFO)
		);
		rLen += sizeof(MYMODINFO);
		pMyModInfo->DllPath.Buffer = (PWCH)malloc(sizeof(WCHAR)*DE_MAX_PATH_LEN);
		if (!pMyModInfo->DllPath.Buffer)
		{
			printf("ModEnumByPid():Memory Insufficient!\n");
			EmptyGlMod();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		//拷贝宽字符数据
		memcpy_s(
			pMyModInfo->DllPath.Buffer,
			sizeof(WCHAR)*DE_MAX_PATH_LEN,
			glBuf + rLen,
			sizeof(WCHAR)*DE_MAX_PATH_LEN
		);
		rLen += sizeof(WCHAR)*DE_MAX_PATH_LEN;

		pMyModInfo->next = glModListEntry.next;
		glModListEntry.next = pMyModInfo;
	}

	//整合完毕后根据全局链表，依次输出结点信息并销毁结点
	pMyModInfo = glModListEntry.next;
	while (pMyModInfo)
	{
		wprintf(L"%ws\n\n", pMyModInfo->DllPath.Buffer);
		wprintf(L"Base:0x%p\tSize:0x%x\n",
			pMyModInfo->ulStartAddr,
			pMyModInfo->ulSize
		);

		free(pMyModInfo->DllPath.Buffer);
		pMyModInfo = pMyModInfo->next;
		free(glModListEntry.next);
		glModListEntry.next = pMyModInfo;
	}
	printf("wLen:0x%x\trLen:0x%x\n", wLen, rLen);
	return STATUS_SUCCESS;
}

//进程暂停通信函数
//向设备对象发送功能码SuspendProc及指定的进程ID
NTSTATUS PsSuspendByPid(IN HANDLE pid)
{
	NTSTATUS status;
	ULONG retLen;
	status = DeviceIoControl(
		hDevice,
		SuspendProc,
		&pid,
		sizeof(HANDLE),
		NULL,
		0,
		&retLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("PsSuspendByPid():PsSuspend Failed!0x%x\n", status);
	}
	else
	{
		printf("pid:%d Suspend!\n", (ULONG)pid);
	}
	return status;
}

//进程恢复通信函数
//向设备对象发送功能码ResumeProc及指定的进程ID
NTSTATUS PsResumeByPid(IN HANDLE pid)
{
	ULONG retLen;
	NTSTATUS status;
	status = DeviceIoControl(
		hDevice,
		ResumeProc,
		&pid,
		sizeof(HANDLE),
		NULL,
		0,
		&retLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("PsResumeByPid():PsResume Failed!0x%x\n", status);
	}
	else
	{
		printf("pid:%d Resume!\n", (ULONG)pid);
	}
	return status;
}

//进程结束通信函数
//向设备对象发送功能码TerminateProc及指定的进程ID
NTSTATUS PsTerminateByPid(IN HANDLE pid)
{
	ULONG retLen;
	NTSTATUS status;
	status = DeviceIoControl(
		hDevice,
		TerminateProc,
		&pid,
		sizeof(HANDLE),
		NULL,
		0,
		&retLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("PsTerminateByPid():PsTerminate Failed!0x%x\n", status);
	}
	else
	{
		printf("pid:%d Terminate!\n", (ULONG)pid);
	}
	return status;
}



/*用户层数据函数*/
//CPU利用率相关函数
//时间转换
//将传入的文件时间的低位输出
static __int64 file_time_2_utc(IN const FILETIME *ftime)
{
	LARGE_INTEGER li;
	li.LowPart = ftime->dwLowDateTime;
	li.HighPart = ftime->dwHighDateTime;
	return li.QuadPart;
}

//获取cpu核数
static int get_processor_number()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (int)info.dwNumberOfProcessors;
}

//返回CPU占有率
int get_cpu_usage(HANDLE pid)
{
	//cpu数量
	static int processorNum = -1;
	//上一次的时间
	static __int64 lasTime = 0;
	static __int64 lastSystemTime = 0;

	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;
	__int64 system_time;
	__int64 time;
	__int64 system_time_delta;
	__int64 time_delta;

	int cpu = -1;

	if (processorNum == -1)
	{
		processorNum = get_processor_number();
	}

	GetSystemTimeAsFileTime(&now);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		return -1;
	}
	system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / processorNum;
	time = file_time_2_utc(&now);

	if ((lastSystemTime == 0) || (lasTime == 0))
	{
		lastSystemTime = system_time;
		lasTime = time;
		return -1;
	}

	system_time_delta = system_time - lastSystemTime;
	time_delta = time - lasTime;

	if (time_delta == 0)
		return -1;

	cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
	lastSystemTime = system_time;
	lasTime = time;
	return cpu;
}



//内存占有相关函数
//获取内存占有率
int get_memory_usage(IN HANDLE hProcess,OUT SIZE_T *pMem,OUT SIZE_T *pVMem)
{
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		if (pMem) *pMem = pmc.WorkingSetSize;
		if (pVMem) *pVMem = pmc.PagefileUsage;
		return 0;
	}
	return -1;
}

/*
typedef struct _PROCESS_MEMORY_COUNTERS {
	DWORD cb;
	DWORD PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;
typedef PROCESS_MEMORY_COUNTERS *PPROCESS_MEMORY_COUNTERS;
*/

//给定pid，获取其CPU占有率与内存占有
NTSTATUS PsGetUsage(
	IN HANDLE pid,
	OUT int *pCpuUsage,
	OUT SIZE_T *pMem,
	OUT SIZE_T *pVMem
)
{
	NTSTATUS status;
	//cpu数量
	static int processorNum = -1;
	//上一次的时间
	static __int64 lasTime = 0;
	static __int64 lastSystemTime = 0;

	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;
	__int64 system_time;
	__int64 time;
	__int64 system_time_delta;
	__int64 time_delta;

	if (processorNum == -1)
	{
		processorNum = get_processor_number();
	}

	GetSystemTimeAsFileTime(&now);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		status = GetLastError();
		printf("PsGetUsage():GetProcTimes Failed!0x%x\n", status);
		return STATUS_UNSUCCESSFUL;
	}
	system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / processorNum;
	time = file_time_2_utc(&now);

	if ((lastSystemTime == 0) || (lasTime == 0))
	{
		lastSystemTime = system_time;
		lasTime = time;
		printf("PsGetUsage():LasTime Zero!\n");
		return STATUS_UNSUCCESSFUL;
	}

	system_time_delta = system_time - lastSystemTime;
	time_delta = time - lasTime;

	if (time_delta == 0)
	{
		printf("PsGetUsage():TimeDelta Zero!\n");
		return STATUS_UNSUCCESSFUL;
	}

	lastSystemTime = system_time;
	lasTime = time;

	if (get_memory_usage(hProcess, pMem, pVMem))
	{
		printf("PsGetUsage():GetMemoryUsage Failed!\n");
		return STATUS_UNSUCCESSFUL;
	}

	*pCpuUsage = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
	return STATUS_SUCCESS;
}


//获取系统信息
void getSysInfo()
{
	SYSTEM_INFO sysInfo; //该结构体包含了当前计算机的信息：计算机的体系结构、中央处理器的类型、系统中中央处理器的数量、页面的大小以及其他信息。
	OSVERSIONINFOEX osvi;
	ULONG cpuUsage;
	SIZE_T mem;
	SIZE_T vMem;
	HANDLE pid = (HANDLE)GetCurrentProcessId();
	GetSystemInfo(&sysInfo);
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	PsGetUsage(pid, &cpuUsage, &mem, &vMem);

	if (GetVersionEx((LPOSVERSIONINFOA)&osvi))
	{
		printf("操作系统版本 :  %u.%u.%u\n", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
		printf("Service Pack :  %u.%u\n", osvi.wServicePackMajor, osvi.wServicePackMinor);
	}

	if (STATUS_SUCCESS == PsGetUsage(pid, &cpuUsage, &mem, &vMem))
	{
		printf("CPU占用率 ：%u\n", cpuUsage);
		printf("内存占用 ： 0x%x\n", mem);
		printf("虚拟内存占用 ： 0x%x\n", vMem);
	}

	printf("处理器架构 :  %u\n", sysInfo.wProcessorArchitecture);
	printf("处理器级别 :  %u\n", sysInfo.wProcessorLevel);
	printf("处理器版本 :  %u\n", sysInfo.wProcessorRevision);
	printf("处理器掩码 :  %u\n", sysInfo.dwActiveProcessorMask);
	printf("处理器数量 :  %u\n", sysInfo.dwNumberOfProcessors);
	printf("处理器类型 :  %u\n", sysInfo.dwProcessorType);
	printf("页面大小 :   %u\n", sysInfo.dwPageSize);
	printf("应用程序最小地址 : 0x%p\n", sysInfo.lpMinimumApplicationAddress);
	printf("应用程序最大地址 : 0x%p\n", sysInfo.lpMaximumApplicationAddress);
	printf("虚拟内存分配粒度 : %u\n", sysInfo.dwAllocationGranularity);
	printf("OemId :    %u\n", sysInfo.dwOemId);
	printf("wReserved :   %u\n", sysInfo.wReserved);
}

/*
typedef struct _SYSTEM_INFO {
	union {
		DWORD dwOemId;          // Obsolete field...do not use
		struct {
			WORD wProcessorArchitecture;
			WORD wReserved;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
	DWORD dwPageSize;
	LPVOID lpMinimumApplicationAddress;
	LPVOID lpMaximumApplicationAddress;
	DWORD_PTR dwActiveProcessorMask;
	DWORD dwNumberOfProcessors;
	DWORD dwProcessorType;
	DWORD dwAllocationGranularity;
	WORD wProcessorLevel;
	WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;
*/

/*
typedef struct _OSVERSIONINFOEXA {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	CHAR   szCSDVersion[ 128 ];     // Maintenance string for PSS usage
	WORD   wServicePackMajor;
	WORD   wServicePackMinor;
	WORD   wSuiteMask;
	BYTE  wProductType;
	BYTE  wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
*/