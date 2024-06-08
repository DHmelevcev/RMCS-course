#include <Windows.h>
#include <sysinfoapi.h>

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

// ��������� � ������� ���������� �������� ������� ������������ �������
typedef struct _MyFuncResult
{
	DWORD aL2CasheSize;
	WORD aDayOfWeek;
} MyFuncResult, *PMyFuncResult;

// �������, ������� �������� ������������ ������� cpuid
EXTERN_C __declspec(dllexport) void CPUID
(
	int CPUInfo[4],	// ����� ����������
	int EAXValue,	// �������� ������
	int ECXValue	// �������������� ��������
)
{
	//__cpuidex(CPUInfo, EAXValue, ECXValue);
	__asm {
		mov esi, CPUInfo
		mov eax, EAXValue
		mov ecx, ECXValue
		cpuid
		mov dword ptr [esi +  0], eax
		mov dword ptr [esi +  4], ebx
		mov dword ptr [esi +  8], ecx
		mov dword ptr [esi + 12], edx
	}
}

// �������, ������� ���������� ��� ������������� ����������
EXTERN_C __declspec(dllexport) void GetVendor(char CPUVendor[16])
{
	//CPUID(CPUVendor, 0x0, 0x0);
	//((int*)CPUVendor)[0] = ((int*)CPUVendor)[1];
	//((int*)CPUVendor)[1] = ((int*)CPUVendor)[3];
	//((int*)CPUVendor)[2] = ((int*)CPUVendor)[2];
	//CPUVendor[12] = '\0';
	__asm {
		mov esi, CPUVendor
		xor eax, eax
		cpuid
		mov dword ptr[esi +  0], ebx
		mov dword ptr[esi +  4], edx
		mov dword ptr[esi +  8], ecx
		mov char  ptr[esi + 12], 0
	}
}

// �������, ������� ��������� ��������� ����������
EXTERN_C __declspec(dllexport) int GetDayOfWeekAndL2CasheSize(PMyFuncResult buffer)
{
	// ���������, � ������� �������� ��������� �����
	char pTimeInfo[sizeof(SYSTEMTIME)];

	// ����� ��������� ������� ��� ��������� �������
	GetSystemTime((PSYSTEMTIME)pTimeInfo);
	buffer->aDayOfWeek = ((PSYSTEMTIME)pTimeInfo)->wDayOfWeek;

	// ���������, � ������� �������� ��������� ������������ ������
	DWORD CPUInfo[4] = { 0 };

	// ������ ������������� ����������
	GetVendor((char*)CPUInfo);
	if (strcmp((char*)CPUInfo, "GenuineIntel") == 0) { // ���� Intel
		// �������� cpuid � ����������� 4 : 2
		CPUID(CPUInfo, 0x4, 0x2);

		// ���������, ���������� �� ����� ������� ����
		DWORD aCashLevel = (CPUInfo[0] & 0xE0) >> 5;
		if (aCashLevel == 0)
			return 2;

		// ���������� ��������� ����
		DWORD nSets = CPUInfo[2] + 1;
		DWORD nLineSize = (CPUInfo[1] & 0xFFF) + 1;
		DWORD nPartitions = ((CPUInfo[1] & 0x3F'F000) >> 12) + 1;
		DWORD nWaysOfAssociativity = ((CPUInfo[1] & 0xFFC0'0000) >> 22) + 1;

		// �������� cpuid � ����������� B : 0 � 1
		CPUID(CPUInfo, 0xB, 0x0);
		DWORD nPhysicalThreadsPerCore = CPUInfo[1] & 0xFFFF;
		CPUID(CPUInfo, 0xB, 0x1);
		DWORD nPhysicalThreads = CPUInfo[1] & 0xFFFF;

		buffer->aL2CasheSize = nPhysicalThreads / nPhysicalThreadsPerCore * nSets * nLineSize * nPartitions * nWaysOfAssociativity;
	}
	else if (strcmp((char*)CPUInfo, "AuthenticAMD") == 0) { // ���� AMD
		// �������� cpuid � ���������� 0x8000'0006
		CPUID(CPUInfo, 0x8000'0006, 0x0);
		DWORD nL2CasheSizePerCoreKB = (CPUInfo[2] & 0xFFFF'0000) >> 16;

		// �������� cpuid � ���������� 0x8000'0008
		CPUID(CPUInfo, 0x8000'0008, 0x0);
		DWORD nPhysicalThreads = (CPUInfo[2] & 0xFF) + 1;

		// �������� cpuid � ���������� 1, ����� ������, ������������ �� ��������� ���������������
		CPUID(CPUInfo, 0x1, 0x0);
		DWORD nPhysicalThreadsPerCore = ((CPUInfo[3] & 0x1000'0000) >> 28) + 1;

		buffer->aL2CasheSize = nPhysicalThreads / nPhysicalThreadsPerCore * nL2CasheSizePerCoreKB * 1024;
	}
	else
		return 1;
	
	return 0;
}
