#pragma once
#include<stdio.h>
#include<windows.h>
#include<winioctl.h>


#define DE_MAX_FILE_NAME_LEN (64)
#define	DE_MAX_PATH_LEN	(512)
#define IOCTLCODE(code)CTL_CODE(FILE_DEVICE_UNKNOWN,0x1000+(code),METHOD_OUT_DIRECT ,FILE_ANY_ACCESS)
#define DE_MAX_BUF_LEN (1024*1024)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)



//UNICODE_STRING定义
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWCH Buffer;
}UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

//定义用于与内核态通信的三类结构体
typedef struct _MYPROCESS {
	ULONG ulPid;
	ULONG ulPpid;
	CHAR ImageFileName[DE_MAX_FILE_NAME_LEN];
	struct _MYPROCESS *next;
}MYPROCESS, *PMYPROCESS;
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
}MYMODINFO, *PMYMODINFO;
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

//临时全局单链表
MYPROCESS glProcListEntry;
MYTHREAD glThdListEntry;
MYMODINFO glModListEntry;
//全局设备对象句柄
HANDLE hDevice;
//数据接收缓冲区指针
PCHAR glBuf;


//全局单链表清空
VOID EmptyGlProc();
VOID EmptyGlThd();
VOID EmptyGlMod();



/*驱动层通信函数*/
//打开并连接到指定的设备对象
NTSTATUS Init();

//程序结束时的执行资源释放等操作
NTSTATUS End();

//进程枚举通信函数
//向设备对象发送功能码EnumProc
//将从驱动层传出的信息整合进入临时全局进程链表
//整合完毕后根据全局链表，依次输出结点信息并销毁结点
NTSTATUS PsEnum();

//线程枚举通信函数
//向设备对象发送功能码EnumThd及指定的进程ID
//将从驱动层传出的信息整合进入临时全局线程链表
//整合完毕后根据全局链表，依次输出结点信息并销毁结点
NTSTATUS ThdEnumByPid(IN HANDLE pid);

//进程模块枚举通信函数
//向设备对象发送功能码EnumPMod及指定的进程ID
//将从驱动层传出的信息整合进入临时全局模块链表
//整合完毕后根据全局链表，依次输出结点信息并销毁结点
NTSTATUS ModEnumByPid(IN HANDLE pid);

//进程暂停通信函数
//向设备对象发送功能码SuspendProc及指定的进程ID
NTSTATUS PsSuspendByPid(IN HANDLE pid);

//进程恢复通信函数
//向设备对象发送功能码ResumeProc及指定的进程ID
NTSTATUS PsResumeByPid(IN HANDLE pid);

//进程结束通信函数
//向设备对象发送功能码TerminateProc及指定的进程ID
NTSTATUS PsTerminateByPid(IN HANDLE pid);