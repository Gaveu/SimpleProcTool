#pragma once
#include<ntifs.h>
#include<ntstrsafe.h>
#include<ntddk.h>

#define DE_MAX_FILE_NAME_LEN (64)
#define	DE_MAX_PATH_LEN	(512)
#define IOCTLCODE(code)CTL_CODE(FILE_DEVICE_UNKNOWN,0x1000+(code),METHOD_OUT_DIRECT ,FILE_ANY_ACCESS)


//微软有导出，但未文档化
PCHAR PsGetProcessImageFileName(PEPROCESS Process);	//获取指定进程的进程名
HANDLE PsGetProcessInheritedFromUniqueProcessId(PEPROCESS Process);	//获取指定进程的父进程ID
NTSTATUS PsSuspendProcess(PEPROCESS Process);	//暂停指定进程执行
NTSTATUS PsResumeProcess(PEPROCESS Process);	//恢复指定进程执行
PPEB PsGetProcessPeb(PEPROCESS process);	//返回指定进程的PEB结构体

//配置函数指针，使xp平台也能使用平台未导入的函数
typedef NTSTATUS(*pFuncXpPsSuspendProcess)(PEPROCESS Process);
typedef NTSTATUS(*pFuncXpPsResumeProcess)(PEPROCESS Process);
//下列数据结构来源：winternl.h、WinDef.h
typedef unsigned char BYTE;
typedef struct _PEB_LDR_DATA {
	BYTE Reserved1[8];
	PVOID Reserved2[3];
	LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;
typedef struct _LDR_DATA_TABLE_ENTRY {
	PVOID Reserved1[2];	//InLoadOrderLinks	_LIST_ENTRY
	LIST_ENTRY InMemoryOrderLinks;
	PVOID Reserved2[2];	//InInitializationOrderLinks	_LIST_ENTRY
	PVOID DllBase;
	PVOID Reserved3[2];	//EntryPoint	Ptr32 Void
	UNICODE_STRING FullDllName;
	BYTE Reserved4[8];	//BaseDllName	UNICODE_STRING
						//……
	PVOID Reserved5[3];	
	union {
		ULONG CheckSum;
		PVOID Reserved6;
	} DUMMYUNIONNAME;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PVOID ProcessParameters;
	PVOID Reserved4[3];
	PVOID AtlThunkSListPtr;
	PVOID Reserved5;
	ULONG Reserved6;
	PVOID Reserved7;
	ULONG Reserved8;
	ULONG AtlThunkSListPtr32;
	PVOID Reserved9[45];
	BYTE Reserved10[96];
	PVOID PostProcessInitRoutine;
	BYTE Reserved11[128];
	PVOID Reserved12[1];
	ULONG SessionId;
} PEB, *PPEB;
//定义用于与用户层通信的三类结构体
typedef struct _MYPROCESS {
	ULONG ulPid;
	ULONG ulPpid;
	CHAR ImageFileName[DE_MAX_FILE_NAME_LEN];
	struct _MYPROCESS *next;
}MYPROCESS,*PMYPROCESS;
typedef struct _MYTHREAD {
	ULONG ulTid;
	CHAR cPrioty;
	UCHAR ucState;
	struct _MYTHREAD *next;
}MYTHREAD, *PMYTHREAD;
typedef struct _MYMODINFO {
	UNICODE_STRING DllPath;
	PVOID ulStartAddr;
	ULONG ulSize;
	struct _MYMODINFO *next;
}MYMODINFO,*PMYMODINFO;
//定义设备控制功能码
typedef enum _IOCTLCODE
{
	EnumProc = IOCTLCODE(0),
	EnumThd = IOCTLCODE(1),
	EnumPMod = IOCTLCODE(2),
	SuspendProc = IOCTLCODE(3),
	ResumeProc = IOCTLCODE(4),
	TerminateProc = IOCTLCODE(5)
}MYCTLCODE;



//函数首地址通过windbg执行“uf 函数符号名”，进行内存检索而得
static pFuncXpPsSuspendProcess pXpPsSuspendProcess = (pFuncXpPsSuspendProcess)0x8411f717;	//uf nt!PsSuspendProcess
static pFuncXpPsResumeProcess pXpPsResumeProcess = (pFuncXpPsResumeProcess)0x8411f7d4;		//uf nt!PsResumeProcess
//临时全局单链表
MYPROCESS glProcListEntry;
MYTHREAD glThdListEntry;
MYMODINFO glModListEntry;



/*驱动初始化*/
//初始化临时全局单链表
VOID init();



/*全局单链表操作*/
//申请(分页)内存
PVOID MemAlloc(IN size_t size);
//释放内存
VOID MemFree(IN PVOID p);
//全局单链表清空
VOID EmptyGlProc();
VOID EmptyGlThd();
VOID EmptyGlMod();

/*进程功能函数*/
//进程枚举
//基于进程双链表的遍历，较快，但由于采用硬编码而有兼容性问题
VOID PsEnum1();
//较通用，无兼容性问题，但为线性遍历
VOID PsEnum2();

//进程暂停
//给定进程pid，暂停对应进程的执行
NTSTATUS PsSuspend(IN HANDLE hPid);

//进程恢复
//给定进程pid，恢复对应进程的执行
NTSTATUS PsResume(IN HANDLE hPid);

//进程终止
//给定进程pid，终止对应进程的执行
NTSTATUS PsTerminate(IN HANDLE hPid);



/*线程功能函数*/
//线程枚举
//根据线程ID枚举所有线程及其对应进程ID、进程名
VOID ThEnum1();

//根据给定的EPROCESS，枚举该进程下的所有线程ID
VOID ThEnum2(IN PEPROCESS process);
VOID ThEnum3(IN PEPROCESS process);



/*模块功能函数*/
//进程模块枚举
//给定进程的_EPROCESS结构指针，枚举其已装载的模块
VOID PsEnumModule(IN PEPROCESS process);



/*派遣函数*/
//通用派遣函数处理
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
//功能码IRP派遣函数
NTSTATUS DispatchIOCTL(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);



/*应用程序通信函数*/
//枚举所有进程
//调用进程枚举函数，所有进程信息装入临时全局进程链表
//将临时全局进程链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
//写入完毕则销毁临时全局进程链表
NTSTATUS EnumPs(
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen);

//枚举给定pid进程的所有线程
//调用线程枚举函数，所有线程信息装入临时全局进程链表
//将临时全局线程链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
//写入完毕则销毁临时全局线程链表
NTSTATUS EnumThdByPid(
	IN HANDLE pid,
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen
);

//枚举给定pid进程的所有装载模块
//调用模块枚举函数，所有模块信息装入临时全局模块链表
//将临时全局模块链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
//写入完毕则销毁临时全局模块链表
NTSTATUS EnumModByPid(
	IN HANDLE pid,
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen
);

