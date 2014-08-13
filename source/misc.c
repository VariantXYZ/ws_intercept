#include <windows.h>
#include "misc.h"

//***************************
//http://stackoverflow.com/questions/13026220/c-function-hook-memory-address-only
#pragma pack(1)
struct patch_t
{
    BYTE nPatchType; //OP code, 0xE9 for JMP
    DWORD dwAddress;
};
#pragma pack()

BOOL apply_patch(BYTE eType, DWORD dwAddress, const void *pTarget, DWORD *orig_size, BYTE *replaced)
{
	DWORD dwOldValue, dwTemp;
	struct patch_t pWrite =
	{
		eType,
		(DWORD)pTarget - (dwAddress + sizeof(DWORD) + sizeof(BYTE))
	};
	VirtualProtect((LPVOID)dwAddress,sizeof(DWORD),PAGE_EXECUTE_READWRITE,&dwOldValue);
	ReadProcessMemory(GetCurrentProcess(),(LPVOID)dwAddress,(LPVOID)replaced,sizeof(pWrite),(PDWORD)orig_size); //Keep track of the bytes we replaced
	BOOL bSuccess = WriteProcessMemory(GetCurrentProcess(),(LPVOID)dwAddress,&pWrite,sizeof(pWrite),NULL);
	VirtualProtect((LPVOID)dwAddress,sizeof(DWORD),dwOldValue,&dwTemp);
	
    return bSuccess;
}

inline void exec_copy(DWORD addr, BYTE *replaced, DWORD orig_size)
{
		DWORD old_val, temp;
		VirtualProtect((LPVOID)addr,sizeof(DWORD),PAGE_EXECUTE_READWRITE,&old_val);
		memcpy((void*)addr,replaced,orig_size);
		VirtualProtect((LPVOID)addr,sizeof(DWORD),old_val,&temp);
}