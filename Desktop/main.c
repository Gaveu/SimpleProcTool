#include"appDef.h"

int main()
{
	int i;
	NTSTATUS status;
	status = Init();
	printf("Init Done 0x%x\n", status);
	
	//��CPUռ���ʺ��ڴ�ռ��
	status = PsGetUsage();
	printf("PsGetUsage Done 0x%x\n", status);

	//����ö��
	//status = PsEnum();
	//printf("PsEnum Done 0x%x\n", status);

	//���ݽ���idö�����߳�
	//status = ThdEnumByPid((HANDLE)2320);
	//printf("ThdEnumByPid Done 0x%x\n", status);

	//���ݽ���idö�������DLL
	//status = ModEnumByPid((HANDLE)1716);
	//printf("ModEnumByPid Done 0x%x\n", status);

	//���ݽ���id��ͣ����
	//status = PsSuspendByPid((HANDLE)2320);
	//printf("PsSuspendByPid Done 0x%x\n", status);

	//���ݽ���id�ָ�����
	//status = PsResumeByPid((HANDLE)2320);
	//printf("PsResumeByPid Done 0x%x\n", status);

	//���ݽ���id��������
	//status = PsTerminateByPid((HANDLE)2320);
	//printf("PsTerminateByPid Done 0x%x\n", status);

	//getSysInfo();

	status = End();
	printf("End Done 0x%x\n", status);
	return 0;
}