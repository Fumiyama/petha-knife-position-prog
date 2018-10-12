#ifndef SHMEM_20150308_151200
#define SHMEM_20150308_151200

#define Shm_READ	0x00
#define	Shm_WRITE	0x01

#include	<windows.h>

#ifdef	_WIN64
	#pragma comment(lib, "ShMem/x64/ShMem.lib")
#else
	#pragma comment(lib, "ShMem/x86/ShMem.lib")
#endif

/**********************************
*	ã§óLÉÅÉÇÉäÇàµÇ§ÉNÉâÉX
*	2015/03/08	DaikiYano
**********************************/
class __declspec(dllexport)ShMem
{
public:
	ShMem();
	ShMem(DWORD Protect, DWORD Size, LPCTSTR Name);
	~ShMem();

	void *CreateShMem(DWORD Protect, DWORD Size, LPCTSTR Name);
	void ReleaseShMem(void *pAddr);
	void *GetpAddr() const;

private:
	HANDLE ghShMem;
	void	*pAddr;
};

#endif //SHMEM_20150308_151200