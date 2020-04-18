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



//UNICODE_STRING����
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWCH Buffer;
}UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

//�����������ں�̬ͨ�ŵ�����ṹ��
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

//��ʱȫ�ֵ�����
MYPROCESS glProcListEntry;
MYTHREAD glThdListEntry;
MYMODINFO glModListEntry;
//ȫ���豸������
HANDLE hDevice;
//���ݽ��ջ�����ָ��
PCHAR glBuf;


//ȫ�ֵ��������
VOID EmptyGlProc();
VOID EmptyGlThd();
VOID EmptyGlMod();



/*������ͨ�ź���*/
//�򿪲����ӵ�ָ�����豸����
NTSTATUS Init();

//�������ʱ��ִ����Դ�ͷŵȲ���
NTSTATUS End();

//����ö��ͨ�ź���
//���豸�����͹�����EnumProc
//���������㴫������Ϣ���Ͻ�����ʱȫ�ֽ�������
//������Ϻ����ȫ������������������Ϣ�����ٽ��
NTSTATUS PsEnum();

//�߳�ö��ͨ�ź���
//���豸�����͹�����EnumThd��ָ���Ľ���ID
//���������㴫������Ϣ���Ͻ�����ʱȫ���߳�����
//������Ϻ����ȫ������������������Ϣ�����ٽ��
NTSTATUS ThdEnumByPid(IN HANDLE pid);

//����ģ��ö��ͨ�ź���
//���豸�����͹�����EnumPMod��ָ���Ľ���ID
//���������㴫������Ϣ���Ͻ�����ʱȫ��ģ������
//������Ϻ����ȫ������������������Ϣ�����ٽ��
NTSTATUS ModEnumByPid(IN HANDLE pid);

//������ͣͨ�ź���
//���豸�����͹�����SuspendProc��ָ���Ľ���ID
NTSTATUS PsSuspendByPid(IN HANDLE pid);

//���ָ̻�ͨ�ź���
//���豸�����͹�����ResumeProc��ָ���Ľ���ID
NTSTATUS PsResumeByPid(IN HANDLE pid);

//���̽���ͨ�ź���
//���豸�����͹�����TerminateProc��ָ���Ľ���ID
NTSTATUS PsTerminateByPid(IN HANDLE pid);