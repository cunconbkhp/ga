#include "ga.h"
#include "globalp.h"

Integer _ga_lo[MAXDIM], _ga_hi[MAXDIM], _ga_work[MAXDIM];
Integer _ga_dims[MAXDIM], _ga_map[MAX_NPROC];


#ifdef USE_FAPI
#  define COPYC2F(carr, farr, n){\
   int i; for(i=0; i< (n); i++)(farr)[i]=(Integer)(carr)[i];} 
#  define COPYF2C(farr, carr, n){\
   int i; for(i=0; i< (n); i++)(carr)[i]=(int)(farr)[i];} 
#else
#  define COPYC2F(carr, farr, n){\
   int i; for(i=0; i< (n); i++)(farr)[n-i-1]=(Integer)(carr)[i];} 
#  define COPYF2C(farr, carr, n){\
   int i; for(i=0; i< (n); i++)(carr)[n-i-1]=(int)(farr)[i];} 
#define BASE_0
#endif

#define COPY(CAST,src,dst,n) {\
   int i; for(i=0; i< (n); i++)(dst)[i]=(CAST)(src)[i];} 

#ifdef BASE_0 
#  define COPYINDEX_C2F(carr, farr, n){\
   int i; for(i=0; i< (n); i++)(farr)[n-i-1]=(Integer)(carr)[i]+1;}
#  define COPYINDEX_F2C(farr, carr, n){\
   int i; for(i=0; i< (n); i++)(carr)[n-i-1]=(int)(farr)[i] -1;}
#else
#  define COPYINDEX_F2C COPYF2C
#  define COPYINDEX_C2F COPYC2F
#endif

int GA_Uses_fapi(void)
{
#ifdef USE_FAPI
return 1;
#else
return 0;
#endif
}


void GA_Initialize_ltd(size_t limit)
{
Integer lim = (Integer)limit;
     ga_initialize_ltd_(&lim);
}
    

int NGA_Create(int type, int ndim,int dims[], char *name, int chunk[])
{
    Integer *ptr, g_a; 
    logical st;
    if(ndim>MAXDIM)return 0;

    COPYC2F(dims,_ga_dims, ndim);
    if(!chunk)ptr=(Integer*)0;  
    else {
         COPYC2F(chunk,_ga_work, ndim);
         ptr = _ga_work;
    }
    st = nga_create((Integer)type, (Integer)ndim, _ga_dims, name, ptr, &g_a);
    if(st==TRUE) return (int) g_a;
    else return 0;
}


int NGA_Create_irreg(int type,int ndim,int dims[],char *name,int block[],int map[])
{
    Integer *ptr, g_a;
    logical st;
    int d, base_map=0, base_work, b;
    if(ndim>MAXDIM)return 0;

    COPYC2F(dims,_ga_dims, ndim);
    COPYC2F(block,_ga_work, ndim);

    /* copy might swap only order of dimensions for blocks in map */
#ifdef  USE_FAPI
        base_work = 0;
#else
        base_work =MAX_NPROC;
#endif

    for(d=0; d<ndim; d++){
#ifndef  USE_FAPI
        base_work -= block[d];
        if(base_work <0)GA_Error("GA C api: error in block",d);
#endif
        for(b=0; b<block[d]; b++){

            _ga_map[base_work + b] = (Integer)map[base_map +b]; /*****/
#ifdef BASE_0
            _ga_map[base_work + b]++;
#endif
        }
        base_map += block[d];

#ifdef  USE_FAPI
        base_work += block[d];
        if(base_work >MAX_NPROC)GA_Error("GA (c): error in block",base_work):
#endif
     }

#ifdef  USE_FAPI
     ptr = _ga_map;
#else
     ptr = _ga_map + base_work;
#endif

    st = nga_create_irreg(type, (Integer)ndim, _ga_dims, name, ptr, _ga_work, &g_a);

    if(st==TRUE) return (int) g_a;
    else return 0;
}

    
int GA_Duplicate(int g_a, char* array_name)
{
    logical st;
    Integer a=(Integer)g_a, b;
    st = ga_duplicate(&a, &b, array_name);
    if(st==TRUE) return (int) b;
    else return 0;
}


void GA_Destroy(int g_a)
{
    logical st;
    Integer a=(Integer)g_a;
    st = ga_destroy_(&a);
    if(st==FALSE)GA_Error("GA (c) destroy failed",g_a);
}


void GA_Zero(int g_a)
{
    Integer a=(Integer)g_a;
    ga_zero_(&a);
}

     
double GA_Ddot(int g_a, int g_b)
{
    Integer a=(Integer)g_a;
    Integer b=(Integer)g_b;
    return (double)ga_ddot_(&a,&b);
}


DoubleComplex GA_Zdot(int g_a, int g_b)
{
    Integer a=(Integer)g_a;
    Integer b=(Integer)g_b;
    return ga_zdot(&a,&b);
}


void GA_Scale(int g_a, void *value)
{
    Integer a=(Integer)g_a;
    ga_scale_(&a,value);
}


void GA_Add(void *alpha, int g_a, void* beta, int g_b, int g_c)
{
    Integer a=(Integer)g_a;
    Integer b=(Integer)g_b;
    Integer c=(Integer)g_c;
    ga_add_(alpha, &a, beta, &b, &c);
}


void GA_Copy(int g_a, int g_b)
{
    Integer a=(Integer)g_a;
    Integer b=(Integer)g_b;
    ga_copy_(&a, &b);
}


void NGA_Get(int g_a, int lo[], int hi[], void* buf, int ld[])
{
    Integer a=(Integer)g_a;
    Integer ndim = ga_ndim_(&a);
    COPYINDEX_C2F(lo,_ga_lo, ndim);
    COPYINDEX_C2F(hi,_ga_hi, ndim);
    COPYC2F(ld,_ga_work, ndim-1);
    nga_get_(&a, _ga_lo, _ga_hi, buf, _ga_work);
}

void NGA_Put(int g_a, int lo[], int hi[], void* buf, int ld[])
{
    Integer a=(Integer)g_a;
    Integer ndim = ga_ndim_(&a);
    COPYINDEX_C2F(lo,_ga_lo, ndim);
    COPYINDEX_C2F(hi,_ga_hi, ndim);
    COPYC2F(ld,_ga_work, ndim-1);
    nga_put_(&a, _ga_lo, _ga_hi, buf, _ga_work);
}    


void NGA_Acc(int g_a, int lo[], int hi[], void* buf,int ld[], void* alpha)
{
    Integer a=(Integer)g_a;
    Integer ndim = ga_ndim_(&a);
    COPYINDEX_C2F(lo,_ga_lo, ndim);
    COPYINDEX_C2F(hi,_ga_hi, ndim);
    COPYC2F(ld,_ga_work, ndim-1);
    nga_acc_(&a, _ga_lo, _ga_hi, buf, _ga_work, alpha);
}    


void NGA_Distribution(int g_a, int iproc, int lo[], int hi[])
{
     Integer a=(Integer)g_a;
     Integer p=(Integer)iproc;
     Integer ndim = ga_ndim_(&a);
     nga_distribution_(&a, &p, _ga_lo, _ga_hi);
     COPYINDEX_F2C(_ga_lo,lo, ndim);
     COPYINDEX_F2C(_ga_hi,hi, ndim);
}


int GA_Compare_distr(int g_a, int g_b)
{
    logical st;
    Integer a=(Integer)g_a;
    Integer b=(Integer)g_b;
    st = ga_compare_distr_(&a,&b);
    if(st == TRUE) return 0;
    else return 1;
}


void NGA_Access(int g_a, int lo[], int hi[], void *ptr, int ld[])
{
     Integer a=(Integer)g_a;
     Integer ndim = ga_ndim_(&a);
     COPYINDEX_C2F(lo,_ga_lo,ndim);
     COPYINDEX_C2F(hi,_ga_hi,ndim);

     nga_access_ptr(&a,_ga_lo, _ga_hi, ptr, _ga_work);
     COPYF2C(_ga_work,ld, ndim-1);
}


int NGA_Locate(int g_a, int subscript[])
{
    logical st;
    Integer a=(Integer)g_a, owner;
    Integer ndim = ga_ndim_(&a);
    int i; 
    COPYINDEX_C2F(subscript,_ga_lo,ndim);

    st = nga_locate_(&a,_ga_lo,&owner);
    if(st == TRUE) return (int)owner;
    else return -1;
}


int NGA_Locate_region(int g_a,int lo[],int hi[],int map[],int procs[])
{
     logical st;
     Integer a=(Integer)g_a, np;
     Integer ndim = ga_ndim_(&a);
     Integer *tmap;
     int i;
     tmap = (Integer *)malloc(2*ndim * GA_Nnodes()*sizeof(Integer));
     if(!map)GA_Error("NGA_Locate_region: unable to allocate memory",g_a);
     COPYINDEX_C2F(lo,_ga_lo,ndim);
     COPYINDEX_C2F(hi,_ga_hi,ndim);

     st = nga_locate_region_(&a,_ga_lo, _ga_hi, tmap, _ga_map, &np);
     if(st==FALSE){
       free(tmap);
       return 0;
     }

     COPY(int,_ga_map,procs, np);

        /* might have to swap lo/hi when copying */

     for(i=0; i< np*2; i++){
        Integer *ptmap = tmap+i*ndim;
        int *pmap = map +i*ndim;
        COPYINDEX_F2C(ptmap, pmap, ndim);  
     }
     free(tmap);
     return (int)np;
}


void NGA_Inquire(int g_a, int *type, int *ndim, int dims[])
{
     Integer a=(Integer)g_a;
     Integer ttype, nndim;
     nga_inquire_(&a,&ttype, &nndim, _ga_dims);
     COPYF2C(_ga_dims, dims,nndim);  
     *ndim = (int)nndim;
     *type = (int)ttype;
}



char* GA_Inquire_name(int g_a)
{
     Integer a=(Integer)g_a;
     char *ptr;
     ga_inquire_name(&a, &ptr);
     return(ptr);
}

int GA_Memory_limited(void)
{
    if(ga_memory_limited_() ==TRUE) return 1;
    else return 0;
}

void NGA_Proc_topology(int g_a, int proc, int coord[])
{
     Integer a=(Integer)g_a;
     Integer p=(Integer)proc;
     Integer ndim = ga_ndim_(&a);
     nga_proc_topology_(&a, &p, _ga_work);
     COPYINDEX_F2C(_ga_work, coord,ndim);  
}


void GA_Check_handle(int g_a, char* string)
{
     Integer a=(Integer)g_a;
     ga_check_handle(&a,string);
}

int GA_Create_mutexes(int number)
{
     Integer n = (Integer)number;
     if(ga_create_mutexes_(&n) == TRUE)return 1;
     else return 0;
}

void GA_Lock(int mutex)
{
     Integer m = (Integer)mutex;
     ga_lock_(&m);
}

void GA_Unlock(int mutex)
{
     Integer m = (Integer)mutex;
     ga_unlock_(&m);
}

void GA_Brdcst(void *buf, int lenbuf, int root)
{
  Integer type=GA_TYPE_BRD;
  Integer len = (Integer)lenbuf;
  Integer orig = (Integer)root;
  ga_msg_brdcst(type, buf, len, orig);
}
   
void GA_Dgop(double x[], int n, char *op)
{
  Integer type=GA_TYPE_GOP;
  Integer len = (Integer)n;
  ga_dgop(type, x, len, op);
}
  

void GA_Igop(Integer x[], int n, char *op)
{
  Integer type=GA_TYPE_GOP;
  Integer len = (Integer)n;
  ga_igop(type, x, len, op);
}

/*********** to do *******/
/*
void GA_Print_patch(int g_a,int ilo,int ihi,int jlo,int jhi,int pretty)
void GA_Copy_patch(ta,int g_a, int ailo, int aihi, int ajlo, int ajhi,
                             int g_b, int bilo, int bihi, int bjlo,int bjhi)
void NGA_Scatter(int g_a, void *v, int* subsArray[], int n)
{
}

void NGA_Scatter(int g_a, void *v, int* subsArray[], int n)
{
}

void GA_Dgemm(char ta, char tb, int m, int n, int k,
              double alpha, int g_a, int g_b, double beta, int g_c )
*/
