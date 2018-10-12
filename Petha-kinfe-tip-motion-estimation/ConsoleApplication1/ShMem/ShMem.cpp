#define SHMEM_CPP
#include "ShMem.h"

BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	return TRUE;
}

ShMem::ShMem(){}
ShMem::ShMem(DWORD Protect, DWORD Size, LPCTSTR Name)
{
	pAddr = CreateShMem(Protect, Size, Name);
}

ShMem::~ShMem()
{
	ReleaseShMem(pAddr);
}

/**********************************
*	���L�������[�̍쐬���s��
*	2015/03/08	DaikiYano
**********************************/
void	*ShMem::CreateShMem(DWORD Access, DWORD Size, LPCTSTR Name)
{
	void *pAddr = NULL;

	ghShMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, Size, Name);

	if (ghShMem != NULL) {

		switch (Access)
		{
		case Shm_READ:
			Access = FILE_MAP_READ;
			break;
		case Shm_WRITE:
			Access = FILE_MAP_WRITE;
			break;
		}

		pAddr = (void *)MapViewOfFile(ghShMem, Access, 0, 0, Size);

	}

	return	pAddr;

}

/**********************************
*	���L�������[�̉�����s��
*	2015/03/08	DaikiYano
**********************************/
void	ShMem::ReleaseShMem(void *pAddr)
{
	if (ghShMem != NULL)
	{
		if (pAddr != NULL)
			UnmapViewOfFile(pAddr);
		CloseHandle(ghShMem);
		ghShMem = NULL;
	}

}

/**********************************
*	���L���������擾����
*	2015/03/08	DaikiYano
**********************************/
void	*ShMem::GetpAddr() const
{
	return	pAddr;
}
