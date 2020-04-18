#pragma once
#include<ntifs.h>
#include<ntstrsafe.h>
#include<ntddk.h>

#define DE_MAX_FILE_NAME_LEN (64)
#define	DE_MAX_PATH_LEN	(512)
#define IOCTLCODE(code)CTL_CODE(FILE_DEVICE_UNKNOWN,0x1000+(code),METHOD_OUT_DIRECT ,FILE_ANY_ACCESS)


//΢���е�������δ�ĵ���
PCHAR PsGetProcessImageFileName(PEPROCESS Process);	//��ȡָ�����̵Ľ�����
HANDLE PsGetProcessInheritedFromUniqueProcessId(PEPROCESS Process);	//��ȡָ�����̵ĸ�����ID
NTSTATUS PsSuspendProcess(PEPROCESS Process);	//��ָͣ������ִ��
NTSTATUS PsResumeProcess(PEPROCESS Process);	//�ָ�ָ������ִ��
PPEB PsGetProcessPeb(PEPROCESS process);	//����ָ�����̵�PEB�ṹ��

//���ú���ָ�룬ʹxpƽ̨Ҳ��ʹ��ƽ̨δ����ĺ���
typedef NTSTATUS(*pFuncXpPsSuspendProcess)(PEPROCESS Process);
typedef NTSTATUS(*pFuncXpPsResumeProcess)(PEPROCESS Process);
//�������ݽṹ��Դ��winternl.h��WinDef.h
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
						//����
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
//�����������û���ͨ�ŵ�����ṹ��
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
//�����豸���ƹ�����
typedef enum _IOCTLCODE
{
	EnumProc = IOCTLCODE(0),
	EnumThd = IOCTLCODE(1),
	EnumPMod = IOCTLCODE(2),
	SuspendProc = IOCTLCODE(3),
	ResumeProc = IOCTLCODE(4),
	TerminateProc = IOCTLCODE(5)
}MYCTLCODE;



//�����׵�ַͨ��windbgִ�С�uf �������������������ڴ��������
static pFuncXpPsSuspendProcess pXpPsSuspendProcess = (pFuncXpPsSuspendProcess)0x8411f717;	//uf nt!PsSuspendProcess
static pFuncXpPsResumeProcess pXpPsResumeProcess = (pFuncXpPsResumeProcess)0x8411f7d4;		//uf nt!PsResumeProcess
//��ʱȫ�ֵ�����
MYPROCESS glProcListEntry;
MYTHREAD glThdListEntry;
MYMODINFO glModListEntry;



/*������ʼ��*/
//��ʼ����ʱȫ�ֵ�����
VOID init();



/*ȫ�ֵ��������*/
//����(��ҳ)�ڴ�
PVOID MemAlloc(IN size_t size);
//�ͷ��ڴ�
VOID MemFree(IN PVOID p);
//ȫ�ֵ��������
VOID EmptyGlProc();
VOID EmptyGlThd();
VOID EmptyGlMod();

/*���̹��ܺ���*/
//����ö��
//���ڽ���˫����ı������Ͽ죬�����ڲ���Ӳ������м���������
VOID PsEnum1();
//��ͨ�ã��޼��������⣬��Ϊ���Ա���
VOID PsEnum2();

//������ͣ
//��������pid����ͣ��Ӧ���̵�ִ��
NTSTATUS PsSuspend(IN HANDLE hPid);

//���ָ̻�
//��������pid���ָ���Ӧ���̵�ִ��
NTSTATUS PsResume(IN HANDLE hPid);

//������ֹ
//��������pid����ֹ��Ӧ���̵�ִ��
NTSTATUS PsTerminate(IN HANDLE hPid);



/*�̹߳��ܺ���*/
//�߳�ö��
//�����߳�IDö�������̼߳����Ӧ����ID��������
VOID ThEnum1();

//���ݸ�����EPROCESS��ö�ٸý����µ������߳�ID
VOID ThEnum2(IN PEPROCESS process);
VOID ThEnum3(IN PEPROCESS process);



/*ģ�鹦�ܺ���*/
//����ģ��ö��
//�������̵�_EPROCESS�ṹָ�룬ö������װ�ص�ģ��
VOID PsEnumModule(IN PEPROCESS process);



/*��ǲ����*/
//ͨ����ǲ��������
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
//������IRP��ǲ����
NTSTATUS DispatchIOCTL(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);



/*Ӧ�ó���ͨ�ź���*/
//ö�����н���
//���ý���ö�ٺ��������н�����Ϣװ����ʱȫ�ֽ�������
//����ʱȫ�ֽ����������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
//д�������������ʱȫ�ֽ�������
NTSTATUS EnumPs(
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen);

//ö�ٸ���pid���̵������߳�
//�����߳�ö�ٺ����������߳���Ϣװ����ʱȫ�ֽ�������
//����ʱȫ���߳��������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
//д�������������ʱȫ���߳�����
NTSTATUS EnumThdByPid(
	IN HANDLE pid,
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen
);

//ö�ٸ���pid���̵�����װ��ģ��
//����ģ��ö�ٺ���������ģ����Ϣװ����ʱȫ��ģ������
//����ʱȫ��ģ���������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
//д�������������ʱȫ��ģ������
NTSTATUS EnumModByPid(
	IN HANDLE pid,
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen
);

