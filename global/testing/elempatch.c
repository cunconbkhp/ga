/**\
File: elempatch.c
Purpose: test the interfaces:

	GA_Abs_value_patch(g_a)
	GA_Add_constant_patch(g_a, alpha)
	GA_Recip_patch_patch(g_a)
	GA_Elem_multiply_patch(g_a, alo, ahi, g_b, blo, bhi, g_c, clo, chi)
	GA_Elem_divide_patch(g_a, alo, ahi, g_b, blo, bhi, g_c, clo, chi)
	GA_Elem_maximum_patch(g_a, alo, ahi, g_b, blo, bhi, g_c, clo, ch
	GA_Elem_minimum_patch(g_a, alo, ahi, g_b, blo, bhi, g_c, clo, chi)
        
        that are for TAO/Global Array Project
 
Author:

Limin Zhang, Ph.D.
Mathematics Department
Columbia Basin College
Pasco, WA 99301

Mentor:

Jarek Nieplocha
Pacific Northwest National Laboratory

Date: Jauary 30, 2002
Revised on February 26, 2002.

\**/

#include "ga.h"
#include "macdecls.h"
#include "../src/globalp.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef MPI
#include <mpi.h>
#else
#include "sndrcv.h"
#endif
#ifndef GA_HALF_MAX_INT 
#define GA_HALF_MAX_INT ((((int)1) << ((int)(8*sizeof(int))-2)) - 1)
#endif

#ifndef GA_INFINITY_I
#define GA_INFINITY_I (GA_HALF_MAX_INT + GA_HALF_MAX_INT + 1)
/* 
  Original value below.
  Seemed too small arbitrarily.
#define GA_INFINITY_I 100000
*/
#endif

#ifndef GA_NEGATIVE_INFINITY_I
#define GA_NEGATIVE_INFINITY_I (- GA_INFINITY_I)


/* 
  Original value below. 
  Seemed too small arbitrarily.
#define GA_NEGATIVE_INFINITY_I -100000
*/
#endif

#ifndef GA_HALF_MAX_LONG
#define GA_HALF_MAX_LONG ((((long)1) << ((int)(8*sizeof(long))-2)) - 1)
#endif

#ifndef GA_INFINITY_L
#define GA_INFINITY_L (GA_HALF_MAX_LONG + GA_HALF_MAX_LONG + 1)
/* Original value was
#define GA_INFINITY_L 100000
*/
#endif

#ifndef GA_NEGATIVE_INFINITY_L
#define GA_NEGATIVE_INFINITY_L (- GA_INFINITY_L)
#endif
/* 
  Original value was:
#define GA_NEGATIVE_INFINITY_L -100000
*/

/* 
  Modified by Doug Baxter 01/24/04 to distinguish between
  Double inifinity and float infinity.
#ifndef GA_INFINITY
#define GA_INFINITY 1.0e20
#endif

#ifndef GA_NEGATIVE_INFINITY
#define GA_NEGATIVE_INFINITY -1.0e20
#endif
*/
#ifndef GA_INFINITY_F
#define GA_INFINITY_F 1.0e37
#endif
/*
  Original value below.
#define GA_INFINITY_F 1.0e20
*/
#ifndef GA_NEGATIVE_INFINITY_F
#define GA_NEGATIVE_INFINITY_F -1.0e37
#endif
/*
  Original value below.
#define GA_NEGATIVE_INFINITY_F -1.0e20
*/
#ifndef GA_INFINITY_D
#define GA_INFINITY_D 1.0e307
#endif
/*
  Original value below.
#define GA_INFINITY_D 1.0e20
*/
#ifndef GA_NEGATIVE_INFINITY_D
#define GA_NEGATIVE_INFINITY_D -1.0e307
#endif


# define THRESH 1e-5
#define MISMATCHED(x,y) ABS((x)-(y))>=THRESH

#define N 10
#define OP_ELEM_MULT 0
#define OP_ELEM_DIV 1
#define OP_ELEM_MAX 2
#define OP_ELEM_MIN 3
#define OP_ABS 4
#define OP_ADD_CONST 5
#define OP_RECIP 6
#define OP_STEP_MAX 7
#define OP_STEP_BOUND_INFO 8
#define MY_TYPE 2002

Integer _ga_lo[MAXDIM], _ga_hi[MAXDIM], _ga_work[MAXDIM];
#  define COPYINDEX_C2F(carr, farr, n){\
   int i; for(i=0; i< (n); i++)(farr)[n-i-1]=(Integer)(carr)[i]+1;}

void FATR nga_vfill_patch_(Integer *g_a, Integer *lo, Integer *hi);
void FATR nga_pnfill_patch_(Integer *g_a, Integer *lo, Integer *hi);

void NGA_Vfill_patch(int g_a, int lo[], int hi[])
{
    Integer a=(Integer)g_a;
    Integer ndim = ga_ndim_(&a);
    COPYINDEX_C2F(lo,_ga_lo, ndim);
    COPYINDEX_C2F(hi,_ga_hi, ndim);

    nga_vfill_patch_(&a, _ga_lo, _ga_hi);
}


void NGA_Pnfill_patch(int g_a, int lo[], int hi[])
{
    Integer a=(Integer)g_a;
    Integer ndim = ga_ndim_(&a);
    COPYINDEX_C2F(lo,_ga_lo, ndim);
    COPYINDEX_C2F(hi,_ga_hi, ndim);

    nga_pnfill_patch_(&a, _ga_lo, _ga_hi);
}

int
ifun (int k)
{
  int result;
  result = -k - 1;
  result = -2;
  return result;
}

int
ifun2 (int k)
{
  int result;
  result = k + 1;
  result = -3;
  return result;
}

void
fill_func (int nelem, int type, void *buf)
{
  int i;


  switch (type)
    {
    case C_FLOAT:
      for (i = 0; i < nelem; i++)
	((float *) buf)[i] = (float) ifun (i);
      break;
    case C_LONG:
      for (i = 0; i < nelem; i++)
	((long *) buf)[i] = (long) ifun (i);
      break;
    case C_DBL:
      for (i = 0; i < nelem; i++)
	((double *) buf)[i] = (double) ifun (i);
      break;
    case C_DCPL:
      for (i = 0; i < 2 * nelem; i++)
	((double *) buf)[i] = (double) ifun (i);
      break;
    case C_INT:
      for (i = 0; i < nelem; i++)
	((int *) buf)[i] = ifun (i);
      break;
    default:
      ga_error (" wrong data type ", type);

    }
}

void
fill_func2 (int nelem, int type, void *buf)
{
  /* int i,size=MA_sizeof(MT_CHAR,type,1);*/

  int i;

  switch (type)
    {
    case C_FLOAT:
      for (i = 0; i < nelem; i++)
	((float *) buf)[i] = (float) ifun2 (i);
      break;
    case C_LONG:
      for (i = 0; i < nelem; i++)
	((long *) buf)[i] = (long) ifun2 (i);
      break;
    case C_DBL:
      for (i = 0; i < nelem; i++)
	((double *) buf)[i] = (double) ifun2 (i);
      break;
    case C_DCPL:
      for (i = 0; i < 2 * nelem; i++)
	((double *) buf)[i] = (double) ifun2 (i);
      break;
    case C_INT:
      for (i = 0; i < nelem; i++)
	((int *) buf)[i] = ifun2 (i);
      break;
    default:
      ga_error (" wrong data type ", type);

    }
}

void
fill_func3 (int nelem, int type, void *buf)
/*taking the absolute of the ifun() */
{
/*int i,size=MA_sizeof(MT_CHAR,type,1);*/

  int i;

  switch (type)
    {
    case C_FLOAT:
      for (i = 0; i < nelem; i++)
	((float *) buf)[i] = (float) ABS (ifun (i));
      break;
    case C_LONG:
      for (i = 0; i < nelem; i++)
	((long *) buf)[i] = (long) ABS (ifun (i));
      break;
    case C_DBL:
      for (i = 0; i < nelem; i++)
	((double *) buf)[i] = (double) ABS (ifun (i));
      break;
    case C_DCPL:
      for (i = 0; i < 2 * nelem - 1; i = i + 2)
	{
	  ((double *) buf)[i] =
	    sqrt ((double)
		  (ifun (i) * ifun (i) + ifun (i + 1) * ifun (i + 1)));
	  ((double *) buf)[i + 1] = 0.0;
	}
      break;
    case C_INT:
      for (i = 0; i < nelem; i++)
	((int *) buf)[i] = ABS (ifun (i));
      break;
    default:
      ga_error (" wrong data type ", type);

    }
}






int
test_fun (int type, int dim, int OP)
{
  double resultx,resulti,resultl,resultf,resultd,resulta;
  double boundminx,boundmini,boundminl,boundminf,boundmind,boundmina;
  double boundmaxx,boundmaxi,boundmaxl,boundmaxf,boundmaxd,boundmaxa;
  double wolfeminx,wolfemini,wolfeminl,wolfeminf,wolfemind,wolfemina;
  int ONE = 1, ZERO = 0;	/* useful constants */
  int g_a, g_b, g_c, g_d, g_e;
  int g_f, g_g, g_h, g_i, g_j;
  int g_k, g_l, g_m, g_n;
  int n = N;
  int me = GA_Nodeid (), nproc = GA_Nnodes ();
  int col, i, row;
  int dims[MAXDIM];
  int lo[MAXDIM], hi[MAXDIM];
  int index[MAXDIM];
  int index2[MAXDIM];
  int index3[MAXDIM];
  int needs_scaled_result;
  void *val;
  void *val3;
  int ival = -2;
  double dval = -2.0;
  float fval = -2.0;
  long lval = -2;
  DoubleComplex dcval;
  void *val2;
  void *val4;
  int ival2 = -3;
  double dval2 = -3.0;
  float fval2 = -3.0;
  long lval2 = -3;
  void *val5;
  int ival5 = 5;
  long lval5 = 5;
  double dval5 = 5.0;
  float fval5 = 5.0;
  DoubleComplex dcval2;
  DoubleComplex dcval3;
  DoubleComplex dcval4;
  DoubleComplex dcval5;
  DoubleComplex dcval6;
  DoubleComplex dcval7;
  void *vresult;
  int ivresult;
  double dvresult;
  float fvresult;
  long lvresult;
  DoubleComplex dcvresult;
  void *bvresult;
  DoubleComplex dcbvresult;
  int ok = 1;
  int result,result2, result3;
  void *max;
  Integer imax;
  float fmax;
  long lmax;
  double dmax;
  DoubleComplex dcmax;
  void *max2;
  DoubleComplex dcmax2;
  void *max3;
  DoubleComplex dcmax3;

  void *alpha, *beta;
  int ai = 1, bi = -1;
  long al = 1, bl = -1;
  float af = 1.0, bf = -1.0;
  double ad = 1.0, bd = -1.0;
  DoubleComplex adc, bdc;
  double x1, x2, x3, x4;

  adc.real = 1.0;
  adc.imag = 0.0;
  bdc.real = -1.0;
  bdc.imag = 0.0;

  boundmina = 0.0;
  boundmind = 0.0;
  boundminf = 0.0;
  boundmini = 0.0;
  boundminl = 0.0;
  boundminx = 0.0;
  wolfemina = 0.0;
  wolfemind = 0.0;
  wolfeminf = 0.0;
  wolfemini = 0.0;
  wolfeminl = 0.0;
  wolfeminx = 0.0;
  boundmaxa = 0.0;
  boundmaxd = 0.0;
  boundmaxf = 0.0;
  boundmaxi = 0.0;
  boundmaxl = 0.0;
  boundmaxx = 0.0;
  needs_scaled_result = 0;

  dcval.real = -sin (3.0);
  dcval.imag = -cos (3.0);
  dcval2.real = 2 * sin (3.0);
  dcval2.imag = 2 * cos (3.0);
  dcval3.real = dcval.real*1.0e200;
  dcval3.imag = dcval.imag*1.0e200;
  dcval4.real = dcval2.real*1.0e200;
  dcval4.imag = dcval2.imag*1.0e200;
  dcval5.real = 5.0;
  dcval5.imag = 0.0;
  dcval6.real = dcval3.imag;
  dcval6.imag = dcval3.real;
  dcval7.real = dcval4.imag;
  dcval7.imag = dcval4.real;

  for (i = 0; i < dim; i++)
    dims[i] = N;

  for (i = 0; i < dim; i++)
    {
      lo[i] = 0;
      hi[i] = N - 1;
    }
  g_a = NGA_Create (type, dim, dims, "A", NULL);
  if (!g_a)
    GA_Error ("create failed: A", n);

  g_b = GA_Duplicate (g_a, "B");
  if (!g_b)
    GA_Error ("duplicate failed: B", n);

  g_c = GA_Duplicate (g_a, "C");
  if (!g_c)
    GA_Error ("duplicate failed: C", n);

  g_d = GA_Duplicate (g_a, "D");
  if (!g_d)
    GA_Error ("duplicate failed: D", n);

  g_e = GA_Duplicate (g_a, "E");
  if (!g_e)
    GA_Error ("duplicate failed: E", n);

  g_f = GA_Duplicate (g_a, "F");
  if (!g_f)
    GA_Error ("duplicate failed: F", n);

  g_g = GA_Duplicate (g_a, "G");
  if (!g_g)
    GA_Error ("duplicate failed: G", n);

  g_h = GA_Duplicate (g_a, "H");
  if (!g_h)
    GA_Error ("duplicate failed: H", n);

  g_i = GA_Duplicate (g_a, "I");
  if (!g_i)
    GA_Error ("duplicate failed: I", n);

  g_j = GA_Duplicate (g_a, "J");
  if (!g_j)
    GA_Error ("duplicate failed: J", n);

  g_k = GA_Duplicate (g_a, "K");
  if (!g_k)
    GA_Error ("duplicate failed: K", n);

  g_l = GA_Duplicate (g_a, "L");
  if (!g_l)
    GA_Error ("duplicate failed: L", n);

  g_m = GA_Duplicate (g_a, "M");
  if (!g_m)
    GA_Error ("duplicate failed: M", n);

  g_n = GA_Duplicate (g_a, "N");
  if (!g_m)
    GA_Error ("duplicate failed: N", n);
  /*initialize  with zero */
  GA_Zero (g_a);
  GA_Zero (g_b);
  GA_Zero (g_c);
  GA_Zero (g_d);
  GA_Zero (g_e);
  GA_Zero (g_f);
  GA_Zero (g_g);
  GA_Zero (g_h);
  GA_Zero (g_i);
  GA_Zero (g_j);
  GA_Zero (g_k);
  GA_Zero (g_l);
  GA_Zero (g_m);
  GA_Zero (g_n);

  switch (type)
    {
    case C_INT:
      val = &ival;
      val2 = &ival2;
      val5 = &ival5;
      vresult = &ivresult;
      break;
    case C_DCPL:
      val = &dcval;
      val2 = &dcval2;
      val3 = &dcval3;
      val4 = &dcval4;
      val5 = &dcval5;
      vresult = &dcvresult;
      bvresult = &dcbvresult;
      break;

    case C_DBL:
      val = &dval;
      val2 = &dval2;
      val5 = &dval5;
      vresult = &dvresult;
      break;
    case C_FLOAT:
      val = &fval;
      val2 = &fval2;
      val5 = &fval5;
      vresult = &fvresult;
      break;
    case C_LONG:
      val = &lval;
      val2 = &lval2;
      val5 = &lval5;
      vresult = &lvresult;
      break;
    default:
      ga_error ("wrong data type.", type);
    }


  NGA_Fill_patch (g_a, lo, hi, val);
  NGA_Fill_patch (g_b, lo, hi, val2);
  NGA_Pnfill_patch (g_j, lo, hi);
  switch (OP)
    {
      double tmp, tmp2;
      DoubleComplex dctemp;
    case OP_ABS:
      if (me == 0)
	printf ("Testing GA_Abs_value...");
      GA_Abs_value_patch (g_a, lo, hi);
      ivresult = ABS (ival);
      dvresult = ABS (dval);
      fvresult = ABS (fval);
      lvresult = ABS (lval);
      if (ABS(dcval.real) >= ABS(dcval.imag)) {
	if (dcval.real == (double)0.0) {
	  dcvresult.real = (double)0.0;
	} else {
	  x1 = dcval.imag/dcval.real;
	  dcvresult.real = ABS(dcval.real)*sqrt(((double)1.0)+(x1*x1));
	}
      } else {
	x1 = dcval.real/dcval.imag;
	dcvresult.real = ABS(dcval.imag)*sqrt(((double)1.0)+(x1*x1));
      }
      dcvresult.imag = 0.0;
      NGA_Fill_patch (g_d, lo, hi, vresult);
      if (type == C_DCPL) {
        needs_scaled_result = 1;
	NGA_Fill_patch(g_f,lo,hi,val3);
	GA_Abs_value_patch (g_f, lo, hi);
	if (ABS(dcval3.real) >= ABS(dcval3.imag)) {
	  if (dcval3.real == (double)0.0) {
	    dcbvresult.real = (double)0.0;
	  } else {
	    x1 = dcval3.imag/dcval3.real;
	    dcbvresult.real = ABS(dcval3.real)*sqrt(((double)1.0)+(x1*x1));
	  }
	} else {
	  x1 = dcval3.real/dcval3.imag;
	  dcbvresult.real = ABS(dcval3.imag)*sqrt(((double)1.0)+(x1*x1));
	}
	dcbvresult.imag = (double)0.0;
	NGA_Fill_patch (g_i, lo, hi, bvresult);
	NGA_Fill_patch(g_k,lo,hi,&dcval6);
	GA_Abs_value_patch (g_k, lo, hi);
	if (ABS(dcval6.real) >= ABS(dcval6.imag)) {
	  if (dcval6.real == (double)0.0) {
	    dcbvresult.real = (double)0.0;
	  } else {
	    x1 = dcval6.imag/dcval6.real;
	    dcbvresult.real = ABS(dcval6.real)*sqrt(((double)1.0)+(x1*x1));
	  }
	} else {
	  x1 = dcval6.real/dcval6.imag;
	  dcbvresult.real = ABS(dcval6.imag)*sqrt(((double)1.0)+(x1*x1));
	}
	NGA_Fill_patch (g_n, lo, hi, bvresult);
      }
      break;
    case OP_ADD_CONST:
      if (me == 0)
	printf ("Testing GA_Add_const...");
      GA_Add_constant_patch (g_a, lo, hi, val2);
      ivresult = ival + ival2;
      dvresult = dval + dval2;
      fvresult = fval + fval2;
      lvresult = lval + lval2;
      dcvresult.real = dcval.real + dcval2.real;
      dcvresult.imag = dcval.imag + dcval2.imag;
      NGA_Fill_patch (g_d, lo, hi, vresult);
      break;
    case OP_RECIP:
      if (me == 0)
	printf ("Testing GA_Recip...");
      GA_Recip_patch (g_a, lo, hi);
      ivresult = ((int)1) / ival;
      dvresult = ((double)1.0) / dval;
      fvresult = ((float)1.0) / fval;
      lvresult = ((long)1) / lval;
      if (ABS(dcval.real) >= ABS(dcval.imag)) {
	if (dcval.real != (double)0.0) {
	  tmp = dcval.imag/dcval.real;
	  tmp2 = ((double)1.0)/((((double)1.0)+(tmp*tmp))*dcval.real);
	  dcvresult.real = tmp2;
	  dcvresult.imag = -tmp * tmp2;
	} else {
	  printf("Error in testing GA_Recip dcval = 0.0\n");
	}
      } else {
	tmp = dcval.real/dcval.imag;
	tmp2 = ((double)1.0)/((((double)1.0)+(tmp*tmp))*dcval.imag);
	dcvresult.real = tmp * tmp2;
	dcvresult.imag = -tmp2;
      }
      NGA_Fill_patch (g_d, lo, hi, vresult);
      if (type == C_DCPL) {
        needs_scaled_result = 1;
	NGA_Fill_patch (g_f, lo, hi, val3);
	GA_Recip_patch (g_f, lo, hi);
	if (ABS(dcval3.real) >= ABS(dcval3.imag)) {
	  if (dcval3.real == (double)0.0) {
	    printf("Error testing GA_Recip, dcval3.real = 0.0\n");
	  } else {
	    tmp = dcval3.imag/dcval3.real;
	    tmp2 = ((double)1.0)/((((double)1.0)+(tmp*tmp))*dcval3.real);
            dcbvresult.real = tmp2;
	    dcbvresult.imag = -tmp * tmp2;
	  }
	} else {
	  tmp = dcval3.real/dcval3.imag;
	  tmp2 = ((double)1.0)/((((double)1.0)+(tmp*tmp))*dcval3.imag);
	  dcbvresult.real = tmp * tmp2;
	  dcbvresult.imag = -tmp2;
	}
	NGA_Fill_patch (g_i, lo, hi, bvresult);
	NGA_Fill_patch(g_k,lo,hi,&dcval6);
	GA_Recip_patch (g_k, lo, hi);
	if (ABS(dcval6.real) >= ABS(dcval6.imag)) {
	  if (dcval6.real == (double)0.0) {
	    printf("Error testing GA_Recip, dcval6.real = 0.0\n");
	  } else {
	    tmp = dcval6.imag/dcval6.real;
	    tmp2 = ((double)1.0)/((((double)1.0)+(tmp*tmp))*dcval6.real);
            dcbvresult.real = tmp2;
	    dcbvresult.imag = -tmp * tmp2;
	  }
	} else {
	  tmp = dcval6.real/dcval6.imag;
	  tmp2 = ((double)1.0)/((((double)1.0)+(tmp*tmp))*dcval6.imag);
	  dcbvresult.real = tmp * tmp2;
	  dcbvresult.imag = -tmp2;
	}
	NGA_Fill_patch (g_n, lo, hi, bvresult);
      }
      break;
    case OP_ELEM_MULT:
      if (me == 0)
	printf ("Testing GA_Elem_multiply...");
      NGA_Fill_patch (g_b, lo, hi, val2);
      /* g_c is different from g_a or g_b*/
      GA_Elem_multiply_patch (g_a, lo, hi, g_b, lo, hi, g_c, lo, hi);

      ivresult = ival * ival2;
      dvresult = dval * dval2;
      fvresult = fval * fval2;
      lvresult = lval * lval2;
      dcvresult.real = dcval.real * dcval2.real - dcval.imag * dcval2.imag;
      dcvresult.imag = dcval.real * dcval2.imag + dcval2.real * dcval.imag;
      NGA_Fill_patch (g_d, lo, hi, vresult);
      break;
    case OP_ELEM_DIV:
      if (me == 0)
	printf ("Testing GA_Elem_divide...");
      NGA_Fill_patch (g_b, lo, hi, val2);
      GA_Elem_divide_patch (g_a, lo, hi, g_b, lo, hi, g_c, lo, hi);
      ivresult = ival / ival2;
      dvresult = dval / dval2;
      fvresult = fval / fval2;
      lvresult = lval / lval2;
      dcvresult.real = 0.0;
      dcvresult.imag = 0.0;
      if (ABS(dcval2.real) >= ABS(dcval2.imag)) {
	if (dcval2.real != (double)0.0) {
	  tmp = dcval2.imag/dcval2.real;
	  tmp2 = ((double)1.0)/(dcval2.real*(((double)1.0)+(tmp*tmp)));
	  dcvresult.real = (dcval.real + dcval.imag*tmp)*tmp2;
	  dcvresult.imag = (dcval.imag - dcval.real*tmp)*tmp2;
	} else {
	  printf("Error in testing GA_Elem_divide dcval = 0.0\n");
	} 
      } else {
	tmp = dcval2.real/dcval2.imag;
	tmp2 = 1.0/(dcval2.imag*(1.0+(tmp*tmp)));
	dcvresult.real = (dcval.real*tmp + dcval.imag)*tmp2;
	dcvresult.imag = (dcval.imag*tmp - dcval.real)*tmp2;
      }
      NGA_Fill_patch (g_d, lo, hi, vresult);
      if (type == C_DCPL) {
        needs_scaled_result = 1;
	NGA_Fill_patch (g_f, lo, hi, val3);
	NGA_Fill_patch (g_g, lo, hi, val4);
	GA_Elem_divide_patch (g_f, lo, hi, g_g, lo, hi, g_h, lo, hi);
	dcbvresult.real = (double)0.0;
	dcbvresult.imag = (double)0.0;
	if (ABS(dcval4.real) >= ABS(dcval4.imag)) {
	  if (dcval4.real != (double)0.0) {
	    tmp = dcval4.imag/dcval4.real;
	    tmp2 = ((double)1.0)/(dcval4.real*(((double)1.0)+(tmp*tmp)));
	    dcbvresult.real = (dcval3.real + dcval3.imag*tmp)*tmp2;
	    dcbvresult.imag = (dcval3.imag - dcval3.real*tmp)*tmp2;
	  } else {
	    printf("Error in testing GA_Elem_divide dcval4 = 0.0\n");
	  } 
	} else {
	  tmp = dcval4.real/dcval4.imag;
	  tmp2 = ((double)1.0)/(dcval4.imag*(((double)1.0)+(tmp*tmp)));
	  dcbvresult.real = (dcval3.real*tmp + dcval3.imag)*tmp2;
	  dcbvresult.imag = (dcval3.imag*tmp - dcval3.real)*tmp2;
	}
	NGA_Fill_patch (g_i, lo, hi, bvresult);
	NGA_Fill_patch (g_k, lo, hi, &dcval6);
	NGA_Fill_patch (g_l, lo, hi, &dcval7);
	GA_Elem_divide_patch (g_k, lo, hi, g_l, lo, hi, g_m, lo, hi);
	dcbvresult.real = (double)0.0;
	dcbvresult.imag = (double)0.0;
	if (ABS(dcval7.real) >= ABS(dcval7.imag)) {
	  if (dcval7.real != (double)0.0) {
	    tmp = dcval7.imag/dcval7.real;
	    tmp2 = ((double)1.0)/(dcval7.real*(((double)1.0)+(tmp*tmp)));
	    dcbvresult.real = (dcval6.real + dcval6.imag*tmp)*tmp2;
	    dcbvresult.imag = (dcval6.imag - dcval6.real*tmp)*tmp2;
	  } else {
	    printf("Error in testing GA_Elem_divide dcval7 = 0.0\n");
	  } 
	} else {
	  tmp = dcval7.real/dcval7.imag;
	  tmp2 = ((double)1.0)/(dcval7.imag*(((double)1.0)+(tmp*tmp)));
	  dcbvresult.real = (dcval6.real*tmp + dcval6.imag)*tmp2;
	  dcbvresult.imag = (dcval6.imag*tmp - dcval6.real)*tmp2;
	}
	NGA_Fill_patch (g_n, lo, hi, bvresult);
      }
      break;

    case OP_ELEM_MAX:
      if (me == 0)
	printf ("Testing GA_Elem_maximum...");
      /*NGA_Fill_patch (g_b, lo, hi, val2);*/
      GA_Elem_maximum_patch (g_a, lo, hi, g_b, lo, hi, g_c, lo, hi);
      ivresult = MAX (ival, ival2);
      dvresult = MAX (dval, dval2);
      fvresult = MAX (fval, fval2);
      lvresult = MAX (lval, lval2);
      tmp  = MAX(ABS(dcval.real),ABS(dcval.imag));
      tmp2 = MAX(ABS(dcval2.real),ABS(dcval2.imag));
      tmp  = MAX(tmp,tmp2);
      dcvresult.real = dcval.real;
      dcvresult.imag = dcval.imag;
      if (tmp != 0.0) {
	tmp = ((double)1.0)/tmp;
	x1 = dcval.real*tmp;
	x2 = dcval.imag*tmp;
	x3 = dcval2.real*tmp;
	x4 = dcval2.imag*tmp;
	tmp = x1*x1 + x2*x2;
	tmp2 = x3*x3 + x4*x4;
	if (tmp2 > tmp) {
	  dcvresult.real = dcval2.real;
	  dcvresult.imag = dcval2.imag;
	}
      }
      NGA_Fill_patch (g_d, lo, hi, vresult);
      if (type == C_DCPL) {
        needs_scaled_result = 1;
	NGA_Fill_patch (g_f, lo, hi, val3);
	NGA_Fill_patch (g_g, lo, hi, val4);
	GA_Elem_maximum_patch (g_f, lo, hi, g_g, lo, hi, g_h, lo, hi);
	tmp  = MAX(ABS(dcval3.real),ABS(dcval3.imag));
	tmp2 = MAX(ABS(dcval4.real),ABS(dcval4.imag));
	tmp  = MAX(tmp,tmp2);
	dcvresult.real = dcval3.real;
	dcvresult.imag = dcval3.imag;
	if (tmp != 0.0) {
	  tmp = ((double)1.0)/tmp;
	  x1 = dcval3.real*tmp;
	  x2 = dcval3.imag*tmp;
	  x3 = dcval4.real*tmp;
	  x4 = dcval4.imag*tmp;
	  tmp = x1*x1 + x2*x2;
	  tmp2 = x3*x3 + x4*x4;
	  if (tmp2 > tmp) {
	    dcvresult.real = dcval4.real;
	    dcvresult.imag = dcval4.imag;
	  }
	}
	NGA_Fill_patch (g_i, lo, hi, vresult);
	NGA_Fill_patch (g_k, lo, hi, &dcval6);
	NGA_Fill_patch (g_l, lo, hi, &dcval7);
	GA_Elem_maximum_patch (g_k, lo, hi, g_l, lo, hi, g_m, lo, hi);
	tmp  = MAX(ABS(dcval6.real),ABS(dcval6.imag));
	tmp2 = MAX(ABS(dcval7.real),ABS(dcval7.imag));
	tmp  = MAX(tmp,tmp2);
	dcvresult.real = dcval6.real;
	dcvresult.imag = dcval6.imag;
	if (tmp != 0.0) {
	  tmp = ((double)1.0)/tmp;
	  x1 = dcval6.real*tmp;
	  x2 = dcval6.imag*tmp;
	  x3 = dcval7.real*tmp;
	  x4 = dcval7.imag*tmp;
	  tmp = x1*x1 + x2*x2;
	  tmp2 = x3*x3 + x4*x4;
	  if (tmp2 > tmp) {
	    dcvresult.real = dcval7.real;
	    dcvresult.imag = dcval7.imag;
	  }
	}
	NGA_Fill_patch (g_n, lo, hi, vresult);
      }
      break;
    case OP_ELEM_MIN:
      if (me == 0)
	printf ("Testing GA_Elem_minimum...");
      NGA_Fill_patch (g_b, lo, hi, val2);
      GA_Elem_minimum_patch (g_a, lo, hi, g_b, lo, hi, g_c, lo, hi);
      ivresult = MIN (ival, ival2);
      dvresult = MIN (dval, dval2);
      fvresult = MIN (fval, fval2);
      lvresult = MIN (lval, lval2);
      tmp  = MAX(ABS(dcval.real),ABS(dcval.imag));
      tmp2 = MAX(ABS(dcval2.real),ABS(dcval2.imag));
      tmp  = MAX(tmp,tmp2);
      dcvresult.real = dcval.real;
      dcvresult.imag = dcval.imag;
      if (tmp != 0.0) {
	tmp = 1.0/tmp;
	x1 = dcval.real*tmp;
	x2 = dcval.imag*tmp;
	x3 = dcval2.real*tmp;
	x4 = dcval2.imag*tmp;
	tmp = x1*x1 + x2*x2;
	tmp2 = x3*x3 + x4*x4;
	if (tmp2 < tmp) {
	  dcvresult.real = dcval2.real;
	  dcvresult.imag = dcval2.imag;
	}
      }
      NGA_Fill_patch (g_d, lo, hi, vresult);
      if (type == C_DCPL) {
        needs_scaled_result = 1;
	NGA_Fill_patch (g_f, lo, hi, val3);
	NGA_Fill_patch (g_g, lo, hi, val4);
	GA_Elem_minimum_patch (g_f, lo, hi, g_g, lo, hi, g_h, lo, hi);
	tmp  = MAX(ABS(dcval3.real),ABS(dcval3.imag));
	tmp2 = MAX(ABS(dcval4.real),ABS(dcval4.imag));
	tmp  = MAX(tmp,tmp2);
	dcvresult.real = dcval3.real;
	dcvresult.imag = dcval3.imag;
	if (tmp != 0.0) {
	  tmp = ((double)1.0)/tmp;
	  x1 = dcval3.real*tmp;
	  x2 = dcval3.imag*tmp;
	  x3 = dcval4.real*tmp;
	  x4 = dcval4.imag*tmp;
	  tmp = x1*x1 + x2*x2;
	  tmp2 = x3*x3 + x4*x4;
	  if (tmp2 < tmp) {
	    dcvresult.real = dcval4.real;
	    dcvresult.imag = dcval4.imag;
	  }
	}
	NGA_Fill_patch (g_i, lo, hi, vresult);
	NGA_Fill_patch (g_k, lo, hi, &dcval6);
	NGA_Fill_patch (g_l, lo, hi, &dcval7);
	GA_Elem_minimum_patch (g_k, lo, hi, g_l, lo, hi, g_m, lo, hi);
	tmp  = MAX(ABS(dcval6.real),ABS(dcval6.imag));
	tmp2 = MAX(ABS(dcval7.real),ABS(dcval7.imag));
	tmp  = MAX(tmp,tmp2);
	dcvresult.real = dcval6.real;
	dcvresult.imag = dcval6.imag;
	if (tmp != 0.0) {
	  tmp = ((double)1.0)/tmp;
	  x1 = dcval6.real*tmp;
	  x2 = dcval6.imag*tmp;
	  x3 = dcval7.real*tmp;
	  x4 = dcval7.imag*tmp;
	  tmp = x1*x1 + x2*x2;
	  tmp2 = x3*x3 + x4*x4;
	  if (tmp2 < tmp) {
	    dcvresult.real = dcval7.real;
	    dcvresult.imag = dcval7.imag;
	  }
	}
	NGA_Fill_patch (g_n, lo, hi, vresult);
      }
      break;
    case OP_STEP_MAX:
      if (me == 0)
	printf ("Testing GA_Step_max...");
      if (type != C_DCPL) {
      	/*NGA_Fill_patch (g_b, lo, hi, val2);*/
      	GA_Abs_value_patch (g_b, lo, hi);
      	GA_Step_max_patch (g_b, lo, hi, g_j, lo, hi, &resultx);
	/*
	printf(" GA_Stepmax_patch type = %d, resultx = %le\n",type,resultx);
	fflush(stdout);
	*/
      	/* 
      	  It would be more robust to use GA_Elem_min_patch 
      	  here to determine the minimum g_j value, but for
      	  now we set it to -2.
      	*/
      	resulti = (double)((int)(ABS(ival2)/ABS(ival)));
      	resultd = ABS(dval2/dval);
      	resultf = (double) ((float)ABS(fval2/fval));
      	resultl = (double) ((long)(ABS(lval2)/ABS(lval)));
      }
      break;

    case OP_STEP_BOUND_INFO:
      if (me == 0)
	printf ("Testing GA_Step_bound_info...");
      if (type != C_DCPL) {
      	/*NGA_Fill_patch (g_b, lo, hi, val2);*/
      	GA_Abs_value_patch (g_b, lo, hi);
      	GA_Abs_value_patch (g_a, lo, hi);
      	/*GA_Abs_value_patch (g_j, lo, hi);*/
      	NGA_Fill_patch(g_c, lo, hi, val5);
      	GA_Step_bound_info_patch (g_b,lo,hi, g_j,lo,hi, g_a,lo,hi, g_c,lo,hi, &boundminx,&wolfeminx,&boundmaxx);
	/*
	printf(" GA_Stepmax2_patch type = %d, resultx = %le\n",type,resultx);
	fflush(stdout);
	*/
      	/* 
          This is currently hardwired. would need to change if 
          val, val2 or val5 change.
      	*/
      	wolfemini = (double)((int)(((int)1)/((int)2)));
      	wolfemind = (double)(((double)1.0)/((double)2.0));
      	wolfeminf = (double)(((float)1.0)/((float)2.0));
      	wolfeminl = (double)((long)(((long)1)/((long)2)));
	boundmini = wolfemini;
	boundmind = wolfemind;
	boundminf = wolfeminf;
	boundminl = wolfeminl;
	boundmaxi = (double)(GA_INFINITY_I);
	boundmaxd = GA_INFINITY_D;
	boundmaxf = (double)((float)GA_INFINITY_F);
	boundmaxl = (double)(GA_INFINITY_L);
      }
      break;
    
    default:
      GA_Error ("test_function: wrong operation.", OP);
    }
  switch (type)
    {
    case C_INT:
      alpha = &ai;
      beta = &bi;
      wolfemina = wolfemini-wolfeminx;
      boundmina = boundmini-boundminx;
      boundmaxa = boundmaxi-boundmaxx;
      break;
    case C_DCPL:
      alpha = &adc;
      beta = &bdc;
      break;

    case C_DBL:
      alpha = &ad;
      beta = &bd;
      wolfemina = wolfemind-wolfeminx;
      boundmina = boundmind-boundminx;
      boundmaxa = boundmaxd-boundmaxx;
      break;
    case C_FLOAT:
      alpha = &af;
      beta = &bf;
      wolfemina = wolfeminf-wolfeminx;
      boundmina = boundminf-boundminx;
      boundmaxa = boundmaxf-boundmaxx;
      break;
    case C_LONG:
      alpha = &al;
      beta = &bl;
      wolfemina = wolfeminl-wolfeminx;
      boundmina = boundminl-boundminx;
      boundmaxa = boundmaxl-boundmaxx;
      break;
    default:
      ga_error ("wrong data type.", type);
    }

  if (OP < 4) {
    /* 
      Binary operation. 
    */
    NGA_Add_patch (alpha, g_c, lo, hi, beta, g_d, lo, hi, g_e, lo, hi);
    if (needs_scaled_result == 1) {
      NGA_Add_patch (alpha, g_h, lo, hi, beta, g_i, lo, hi, g_j, lo, hi);
      NGA_Add_patch (alpha, g_m, lo, hi, beta, g_n, lo, hi, g_n, lo, hi);
    }
  }
  else {
    /*
      Unary operation.
    */
    if (OP < 7) {
      NGA_Add_patch (alpha, g_a, lo, hi, beta, g_d, lo, hi, g_e, lo, hi);
      if (needs_scaled_result == 1) {
	NGA_Add_patch (alpha, g_f, lo, hi, beta, g_i, lo, hi, g_j, lo, hi);
	NGA_Add_patch (alpha, g_k, lo, hi, beta, g_n, lo, hi, g_n, lo, hi);
      }
    }
    /* 
      Else it was a reduction operation (one of the step_max functions).
    */
  }

  switch (type)
    {
    case C_INT:
      max = &imax;
      break;
    case C_DCPL:
      max = &dcmax;
      max2 = &dcmax2;
      max3 = &dcmax3;
      break;
    case C_DBL:
      max = &dmax;
      break;
    case C_FLOAT:
      max = &fmax;
      break;
    case C_LONG:
      max = &lmax;
      break;
    default:
      ga_error ("wrong data type.", type);
    }

  /*  
    for unary and binary operators extract the maximum difference between
    computed and correct solutions.
  */
  if (OP < 7) {
    NGA_Select_elem (g_e, "max", max, index);
    if (needs_scaled_result == 1) {
      NGA_Select_elem (g_j, "max", max2, index2);
      NGA_Select_elem (g_n, "max", max3, index3);
    }
  }
  /*  NGA_Select_elem (g_e, "min", min, index);*/

  if (OP < 7) {
    /* 
      Binary or Unary operators.
    */
    switch (type)
      {
  	double r, im, tmp;
      case C_INT:
  	/*      result = (int)(imax - imin);*/
  	result = imax;
  	if (result != (int)0) result = 1;
  	break;
      case C_DCPL:
  	/*
  	r = dcmax.real - dcmin.real;
  	im = dcmax.imag - dcmin.imag;
  	*/
  	r = dcmax.real;
  	im = dcmax.imag;
  	if ((ABS(r) + ABS(im)) == (double)0.0) {
 	  result = 0;
  	} else {
 	  result = 1;
  	}
  	if (needs_scaled_result == 1) {
	  result2 = 0;
 	  r = dcmax2.real;
 	  im = dcmax2.imag;
 	  if ((ABS(r) + ABS(im)) == (double)0.0) {
 	    result2 = 0;
 	  } else {
 	    result2 = 1;
 	  }
 	  r = dcmax3.real;
 	  im = dcmax3.imag;
 	  if ((ABS(r) + ABS(im)) == (double)0.0) {
 	    result3 = 0;
 	  } else {
 	    result3 = 1;
 	  }
	  result = result | result2 | result3;
  	}
  	break;
      case C_DBL:
  	if (dmax == (double)0.0) {
 	  result = 0;
  	} else {
 	  result = 1;
  	}
  	break;
      case C_FLOAT:
  	if (fmax == (float)0.0) {
 	  result = 0;
  	} else {
 	  result = 1;
  	}
  	break;
      case C_LONG:
  	if (lmax == (long)0) {
 	  result = 0;
  	} else {
 	  result = 1;
  	}
  	break;
      default:
  	ga_error ("wrong data type.", type);
      }
  } else {
    /*
      A reduction operation, Step_max or Step_bound_info.
    */
    if (type == C_DCPL) {
      result = 0;
    } else {
      if (wolfemina == ((double)0.0)) {
	result = 0;
      } else {
	result = 1;
      }
      if (boundmina == ((double)0.0)) {
	result2 = 0;
      } else {
	result2 = 1;
      }
      if (boundmaxa == ((double)0.0)) {
	result3 = 0;
      } else {
	result3 = 1;
      }
      result = result | result2 | result3;
    }
  }
  if (me == 0)
    {
      if (MISMATCHED (result, 0))
	printf ("is not ok\n");
      else
	printf ("is ok.\n");
    }

/*
 NGA_Print_patch(g_a, lo, hi, 1);
 NGA_Print_patch(g_d, lo, hi, 1);
 NGA_Print_patch(g_e, lo, hi, 1);
*/

  GA_Destroy (g_a);
  GA_Destroy (g_b);
  GA_Destroy (g_c);
  GA_Destroy (g_d);
  GA_Destroy (g_e);
  GA_Destroy (g_f);
  GA_Destroy (g_g);
  GA_Destroy (g_h);
  GA_Destroy (g_i);
  GA_Destroy (g_j);
  GA_Destroy (g_k);
  GA_Destroy (g_l);
  GA_Destroy (g_m);
  GA_Destroy (g_n);

  return ok;
}

int
main (argc, argv)
     int argc;
     char **argv;
{
  int heap = 20000, stack = 20000;
  int me, nproc;
  int d, op;
  int ok = 1;

#ifdef MPI
  MPI_Init (&argc, &argv);	/* initialize MPI */
#else
  PBEGIN_ (argc, argv);		/* initialize TCGMSG */
#endif

  GA_Initialize ();		/* initialize GA */
  me = GA_Nodeid ();
  nproc = GA_Nnodes ();
  if (me == 0)
    {
      if (GA_Uses_fapi ())
	GA_Error ("Program runs with C array API only", 0);
      printf ("Using %ld processes\n", (long) nproc);
      fflush (stdout);
    }

  heap /= nproc;
  stack /= nproc;
  if (!MA_init (C_DBL, stack, heap))
    GA_Error ("MA_init failed", stack + heap);	/* initialize memory allocator */


  /* op = 8;*/
  for (op = 0; op < 9; op++)
    {
      /*for (d = 1; d < 2; d++)*/
      for (d = 1; d < 4; d++)
	{
	  if (me == 0) 
	    printf ("\n\ndim =%d\n\n", d);
	  if (me == 0) 
	    printf ("\ndata type: INT\t\t");
	  ok = test_fun (C_INT, d, op);
	  if (me == 0) 
	    printf ("\ndata type: double\t");
	  ok = test_fun (C_DBL, d, op);
	  if (me == 0)
	    printf ("\ndata type: float\t");
	  ok = test_fun (C_FLOAT, d, op);
	  if (me == 0)
	    printf ("\ndata type: long\t\t");
	  ok = test_fun (C_LONG, d, op);
	  if (op < 7) {
	    if (me == 0)
	      printf ("\ndata type: complex\t");
	    ok = test_fun (C_DCPL, d, op);
	  }
	}
    }

  GA_Terminate();
  
#ifdef MPI
  MPI_Finalize ();
#else
  PEND_ ();
#endif

  return 0;
}

/*\ FILL IN ARRAY WITH Varying VALUEs. (from 0 to product of dims-1).
    For complex arrays make the real and imaginary parts equal.
\*/
void FATR nga_vfill_patch_(Integer *g_a, Integer *lo, Integer *hi)
{
    Integer i, j;
    Integer ndim, dims[MAXDIM], type;
    Integer loA[MAXDIM], hiA[MAXDIM], ld[MAXDIM];
    void *data_ptr;
    Integer idx, n1dim;
    Integer bvalue[MAXDIM], bunit[MAXDIM], baseld[MAXDIM];
    Integer me= ga_nodeid_();
    int local_sync_begin,local_sync_end;
   
#ifdef GA_USE_VAMPIR
    vampir_begin(NGA_VFILL_PATCH,__FILE__,__LINE__);
#endif 
    local_sync_begin = _ga_sync_begin; local_sync_end = _ga_sync_end;
    _ga_sync_begin = 1; _ga_sync_end=1; /*remove any previous masking*/
    if(local_sync_begin)ga_sync_(); 

    GA_PUSH_NAME("nga_vfill_patch");
    
    nga_inquire_internal_(g_a,  &type, &ndim, dims);

    /* get limits of VISIBLE patch */ 
    nga_distribution_(g_a, &me, loA, hiA);
    
    /*  determine subset of my local patch to access  */
    /*  Output is in loA and hiA */
    if(ngai_patch_intersect(lo, hi, loA, hiA, ndim)){

        /* get data_ptr to corner of patch */
        /* ld are leading dimensions INCLUDING ghost cells */
        nga_access_ptr(g_a, loA, hiA, &data_ptr, ld);
 
        /* number of n-element of the first dimension */
        n1dim = 1; for(i=1; i<ndim; i++) n1dim *= (hiA[i] - loA[i] + 1);
        
        /* calculate the destination indices */
        bvalue[0] = 0; bvalue[1] = 0; bunit[0] = 1; bunit[1] = 1;
        /* baseld[0] = ld[0]
         * baseld[1] = ld[0] * ld[1]
         * baseld[2] = ld[0] * ld[1] * ld[2] .....
         */
        baseld[0] = ld[0]; baseld[1] = baseld[0] *ld[1];
        for(i=2; i<ndim; i++) {
            bvalue[i] = 0;
            bunit[i] = bunit[i-1] * (hiA[i-1] - loA[i-1] + 1);
            baseld[i] = baseld[i-1] * ld[i];
        }

        switch (type){
            case C_INT:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    for(j=0; j<(hiA[0]-loA[0]+1); j++)
                        ((int *)data_ptr)[idx+j] = (int)(idx+j);
                }
                break;
            case C_DCPL:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    
                    for(j=0; j<(hiA[0]-loA[0]+1); j++) {
                        ((DoubleComplex *)data_ptr)[idx+j].real = (double)(idx+j);
                        ((DoubleComplex *)data_ptr)[idx+j].imag = (double)(idx+j);
                    }
                }
                
                break;
            case C_DBL:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    
                    for(j=0; j<(hiA[0]-loA[0]+1); j++) 
		      ((double*)data_ptr)[idx+j] = (double)(idx+j);
                }
                break;
            case C_FLOAT:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    for(j=0; j<(hiA[0]-loA[0]+1); j++)
                        ((float *)data_ptr)[idx+j] = (float)(idx+j);
                }
                break;     
            case C_LONG:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    for(j=0; j<(hiA[0]-loA[0]+1); j++)
                        ((long *)data_ptr)[idx+j] = (long)(idx+j);
                } 
                break;                          
            default: ga_error(" wrong data type ",type);
        }
        
        /* release access to the data */
        nga_release_update_(g_a, loA, hiA);
    }
    GA_POP_NAME;
    if(local_sync_end)ga_sync_();
#ifdef GA_USE_VAMPIR
    vampir_end(NGA_VFILL_PATCH,__FILE__,__LINE__);
#endif 
}

/*\ FILL IN ARRAY WITH Varying positive and negative VALUEs. 
    (from -2 to 1).
    For complex arrays make the real and imaginary parts equal.
\*/
void FATR nga_pnfill_patch_(Integer *g_a, Integer *lo, Integer *hi)
{
    Integer i, j;
    Integer ndim, dims[MAXDIM], type;
    Integer loA[MAXDIM], hiA[MAXDIM], ld[MAXDIM];
    void *data_ptr;
    Integer idx, n1dim;
    Integer bvalue[MAXDIM], bunit[MAXDIM], baseld[MAXDIM];
    Integer me= ga_nodeid_();
    int local_sync_begin,local_sync_end;
   
#ifdef GA_USE_VAMPIR
    vampir_begin(NGA_PNFILL_PATCH,__FILE__,__LINE__);
#endif 
    local_sync_begin = _ga_sync_begin; local_sync_end = _ga_sync_end;
    _ga_sync_begin = 1; _ga_sync_end=1; /*remove any previous masking*/
    if(local_sync_begin)ga_sync_(); 

    GA_PUSH_NAME("nga_pnfill_patch");
    
    nga_inquire_internal_(g_a,  &type, &ndim, dims);

    /* get limits of VISIBLE patch */ 
    nga_distribution_(g_a, &me, loA, hiA);
    
    /*  determine subset of my local patch to access  */
    /*  Output is in loA and hiA */
    if(ngai_patch_intersect(lo, hi, loA, hiA, ndim)){

        /* get data_ptr to corner of patch */
        /* ld are leading dimensions INCLUDING ghost cells */
        nga_access_ptr(g_a, loA, hiA, &data_ptr, ld);
 
        /* number of n-element of the first dimension */
        n1dim = 1; for(i=1; i<ndim; i++) n1dim *= (hiA[i] - loA[i] + 1);
        
        /* calculate the destination indices */
        bvalue[0] = 0; bvalue[1] = 0; bunit[0] = 1; bunit[1] = 1;
        /* baseld[0] = ld[0]
         * baseld[1] = ld[0] * ld[1]
         * baseld[2] = ld[0] * ld[1] * ld[2] .....
         */
        baseld[0] = ld[0]; baseld[1] = baseld[0] *ld[1];
        for(i=2; i<ndim; i++) {
            bvalue[i] = 0;
            bunit[i] = bunit[i-1] * (hiA[i-1] - loA[i-1] + 1);
            baseld[i] = baseld[i-1] * ld[i];
        }

        switch (type){
            case C_INT:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    for(j=0; j<(hiA[0]-loA[0]+1); j++)
                        ((int *)data_ptr)[idx+j] = (int)(((idx+j)&3)-2);
                }
                break;
            case C_DCPL:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    
                    for(j=0; j<(hiA[0]-loA[0]+1); j++) {
                        ((DoubleComplex *)data_ptr)[idx+j].real = (double)(((idx+j)&3)-2);
                        ((DoubleComplex *)data_ptr)[idx+j].imag = (double)(((idx+j)&3)-2);
                    }
                }
                
                break;
            case C_DBL:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    
                    for(j=0; j<(hiA[0]-loA[0]+1); j++) 
		      ((double*)data_ptr)[idx+j] = (double)(((idx+j)&3)-2);
                }
                break;
            case C_FLOAT:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    for(j=0; j<(hiA[0]-loA[0]+1); j++)
                        ((float *)data_ptr)[idx+j] = (float)(((idx+j)&3)-2);
                }
                break;     
            case C_LONG:
                for(i=0; i<n1dim; i++) {
                    idx = 0;
                    for(j=1; j<ndim; j++) {
                        idx += bvalue[j] * baseld[j-1];
                        if(((i+1) % bunit[j]) == 0) bvalue[j]++;
                        if(bvalue[j] > (hiA[j]-loA[j])) bvalue[j] = 0;
                    }
                    for(j=0; j<(hiA[0]-loA[0]+1); j++)
                        ((long *)data_ptr)[idx+j] = (long)(((idx+j)&3)-2);
                } 
                break;                          
            default: ga_error(" wrong data type ",type);
        }
        
        /* release access to the data */
        nga_release_update_(g_a, loA, hiA);
    }
    GA_POP_NAME;
    if(local_sync_end)ga_sync_();
#ifdef GA_USE_VAMPIR
    vampir_end(NGA_PNFILL_PATCH,__FILE__,__LINE__);
#endif 
}


