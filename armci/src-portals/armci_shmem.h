#ifndef _SHMEM_H_
#define _SHMEM_H_
extern void  Set_Shmem_Limit(unsigned long shmemlimit);
extern void  Delete_All_Regions();
extern char* Create_Shared_Region(long idlist[], long size, long *offset);
extern char* Attach_Shared_Region(long idlist[], long size, long offset);
extern void Free_Shmem_Ptr(long id, long size, char* addr);
extern long armci_shmem_reg_size(int i, long id);
extern char* armci_shmem_reg_ptr(int i);

#define POST_ALLOC_CHECK(temp,size) ;

#define MAX_REGIONS 64 

#if defined(WIN32)
#define SHMIDLEN 3
#else
#define SHMIDLEN (MAX_REGIONS + 2)
#endif

#define IDLOC (SHMIDLEN - 3)

#endif
