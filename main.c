/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "sp_matrix.h"

typedef int BOOL;
enum
{
  FALSE = 0,
  TRUE = 1
};

#ifndef DBL_EPSILON
#define DBL_EPSILON 2.2204460492503131e-16
#endif

/*
 * Equals macro for double values
 * See http://www.rsdn.ru/forum/cpp/2640596.1.aspx for explanations
 */
#define EQL(x,y) ((fabs((x)-(y))<= fmax(fabs((x)),fabs((y)))*(DBL_EPSILON)) ? TRUE:FALSE)


BOOL test_sp_matrix()
{
  BOOL result = TRUE;
  sp_matrix mtx,mtx2,mtx3;
  double b[] = {1, 2, 3, 4, 3, 2, 1};
  double expected[] = {25, 34, 40, 45, 42, 16, 23};
  int size = sizeof(expected)/sizeof(double);
  int i;
  double x[] = {0,0,0,0,0,0,0};
  /*
   * Sparse matrix
   * 9  0  0  3  1  0  1
   * 0  11 2  1  0  0  2
   * 0  1  10 2  0  0  0
   * 0  0  2  9  1  0  0
   * 1  0  0  1  12 0  1
   * 0  0  0  0  0  8  0
   * 2  2  0  0  3  0  8
   */

  sp_matrix_init(&mtx,7,7,5,CRS);

  MTX(&mtx,0,0,9);MTX(&mtx,0,3,3);MTX(&mtx,0,4,1);MTX(&mtx,0,6,1);
  MTX(&mtx,1,1,11);MTX(&mtx,1,2,2);MTX(&mtx,1,3,1);MTX(&mtx,1,6,2);
  MTX(&mtx,2,1,1);MTX(&mtx,2,2,10);MTX(&mtx,2,3,2);
  MTX(&mtx,3,2,2);MTX(&mtx,3,3,9);MTX(&mtx,3,4,1);
  MTX(&mtx,4,0,1);MTX(&mtx,4,3,1);MTX(&mtx,4,4,12);MTX(&mtx,4,6,1);
  MTX(&mtx,5,5,8);
  MTX(&mtx,6,0,2);MTX(&mtx,6,1,2);MTX(&mtx,6,4,3);MTX(&mtx,6,6,8);

  sp_matrix_compress(&mtx);

  /* 1st test - matrix-vector multiplication */
  sp_matrix_mv(&mtx,b,x);
  for (i = 0; i < size; ++ i)
    result &= EQL(x[i],expected[i]);
  
  /* 2nd test - conversion btw different storage types */
  if (result)
  {
    sp_matrix_convert(&mtx,&mtx2,CCS);
    sp_matrix_mv(&mtx2,b,x);
    for (i = 0; i < size; ++ i)
    {
      result &= EQL(x[i],expected[i]);
    }
  }
  if (result)
  {
    sp_matrix_convert(&mtx2,&mtx3,CRS);
    sp_matrix_mv(&mtx3,b,x);
    for (i = 0; i < size; ++ i)
    {
      result &= EQL(x[i],expected[i]);
    }
    sp_matrix_free(&mtx2);
    sp_matrix_free(&mtx3);
  }
  
  sp_matrix_free(&mtx);
  printf("test_sp_matrix result: *%s*\n",result ? "pass" : "fail");
  return result;
}

BOOL test_triangle_solver()
{
  BOOL result = TRUE;
  int i;
  sp_matrix mtx,mtx2;
  double x[5] = {0};
  double x_expected[] = {1,2,-3,5,-7};
  double b[] = {-1, 5, -10, 40, -71};

  /*
   * |-1  0  0  0  0 |   | 1 |   |-1 |
   * | 1  2  0  0  0 |   | 2 |   | 5 |
   * |-1  0  3  0  0 | x |-3 | = |-10|
   * | 0  5  0  6  0 |   | 5 |   | 40|
   * | 0  0 -2  0 11 |   |-7 |   |-71|
   */
  sp_matrix_init(&mtx,5,5,3,CRS);
  MTX(&mtx,0,0,-1);
  MTX(&mtx,1,0,1);MTX(&mtx,1,1,2);
  MTX(&mtx,2,0,-1);MTX(&mtx,2,2,3);
  MTX(&mtx,3,1,5);MTX(&mtx,3,3,6);
  MTX(&mtx,4,2,-2);MTX(&mtx,4,4,11);
  sp_matrix_lower_solve(&mtx,5,b,x);
  for (i = 0; i < 5; ++ i)
    result &= EQL(x_expected[i],x[i]);
  
  if (result)
  {
    sp_matrix_convert(&mtx,&mtx2,CCS);
    memset(x,0,sizeof(double)*5);
    sp_matrix_lower_solve(&mtx,5,b,x);
    for (i = 0; i < 5; ++ i)
      result &= EQL(x_expected[i],x[i]);
    sp_matrix_free(&mtx2);
  }
  
  sp_matrix_free(&mtx);
  
  printf("test_triangle_solver result: *%s*\n",result ? "pass" : "fail");
  return result;
}

BOOL test_cg_solver()
{
  BOOL result = TRUE;
  sp_matrix mtx;
  double v[3] = {0}, x[3] = {0};
  int max_iter = 20000;
  double tolerance = 1e-15;

  /* matrix solver test  */

  /* Test 1: */
  /*
   * | 1 0 -2 |   | 1 |   |-5 |
   * | 0 1  0 | x | 2 | = | 2 | 
   * |-2 0  5 |   | 3 |   |13 |
   */
  memset(x,0,3);
  v[0] = -5;
  v[1] = 2;
  v[2] = 13;
  sp_matrix_init(&mtx,3,3,2,CRS);
  
  MTX(&mtx,0,0,1);MTX(&mtx,0,2,-2);
  MTX(&mtx,1,1,1);
  MTX(&mtx,2,0,-2);MTX(&mtx,2,2,5);

  sp_matrix_compress(&mtx);
  sp_matrix_solve_cg(&mtx,v,v,&max_iter,&tolerance,x);

  result = !( fabs(x[0]-1) > TOLERANCE ||
              fabs(x[1]-2) > TOLERANCE ||
              fabs(x[2]-3) > TOLERANCE);
  sp_matrix_free(&mtx);
  
  printf("test_cg_solver result: *%s*\n",result ? "pass" : "fail");
  return result;
}

BOOL test_ilu()
{
  BOOL result = TRUE;
  sp_matrix mtx;
  sp_matrix_skyline m;
  sp_matrix_skyline_ilu ILU;
  double x_exact[] = {1,2,3,0,3,2,1};
  double x[7];
  double b[7];
  int i;
  /* test data for ILU decomposition test */
  double lu_diag_expected[] = {9.000000,
                               11.000000,
                               9.818182,
                               7.888889,
                               11.823161,
                               8.000000,
                               7.205303};
  double lu_lowertr_expected[] = {0.090909,
                                  0.222222,
                                  0.090909,
                                  0.185185,
                                  0.111111,
                                  0.084507,
                                  0.222222,
                                  0.181818,
                                  0.234944};
  double lu_uppertr_expected[] = {2.000000,
                                  3.000000,
                                  1.000000,
                                  1.909091,
                                  1.000000,
                                  0.777778,
                                  1.000000,
                                  2.000000,
                                  0.888889};

  memset(b,0,sizeof(b));
  memset(x,0,sizeof(x));
  
  /* Sparse matrix from Balandin
   * 9  0  0  3  1  0  1
   * 0  11 2  1  0  0  2
   * 0  1  10 2  0  0  0
   * 2  1  2  9  1  0  0
   * 1  0  0  1  12 0  1
   * 0  0  0  0  0  8  0
   * 2  2  0  0  3  0  8
   *
   * Test for
   * 1) skyline format
   * 2) ILU decomposition
   * 3) LU - solvers for ILU decomposition
   */

  sp_matrix_init(&mtx,7,7,5,CRS);

  MTX(&mtx,0,0,9);MTX(&mtx,0,3,3);MTX(&mtx,0,4,1);MTX(&mtx,0,6,1);
  MTX(&mtx,1,1,11);MTX(&mtx,1,2,2);MTX(&mtx,1,3,1);MTX(&mtx,1,6,2);
  MTX(&mtx,2,1,1);MTX(&mtx,2,2,10);MTX(&mtx,2,3,2);
  MTX(&mtx,3,0,2);MTX(&mtx,3,1,1);MTX(&mtx,3,2,2);MTX(&mtx,3,3,9);
  MTX(&mtx,3,4,1);
  MTX(&mtx,4,0,1);MTX(&mtx,4,3,1);MTX(&mtx,4,4,12);MTX(&mtx,4,6,1);
  MTX(&mtx,5,5,8);
  MTX(&mtx,6,0,2);MTX(&mtx,6,1,2);MTX(&mtx,6,4,3);MTX(&mtx,6,6,8);

  sp_matrix_compress(&mtx);
  sp_matrix_skyline_init(&m,&mtx);
  sp_matrix_skyline_ilu_copy_init(&ILU,&m);

  for (i = 0; i <  m.rows_count; ++ i)
    result &= fabs(ILU.ilu_diag[i] - lu_diag_expected[i]) < 1e-5;
  
  if (result)
  {
    for (i = 0; i <  m.tr_nonzeros; ++ i)
      result &= fabs(ILU.ilu_lowertr[i] - lu_lowertr_expected[i]) < 1e-5;
  }
  
  if (result)
  {       
    for (i = 0; i <  m.tr_nonzeros; ++ i)
      result &= fabs(ILU.ilu_uppertr[i] - lu_uppertr_expected[i]) < 1e-5;
  }

  /*
   * test for solving Lx=b
   */
  if (result)
  {
    /* prepare a right-part vector */
    sp_matrix_skyline_ilu_lower_mv(&ILU,x_exact,b);
    
    /* solve for x */
    sp_matrix_skyline_ilu_lower_solve(&ILU,b,x);
    /* test result */
    for ( i = 0; i < m.rows_count; ++ i)
      result &= EQL(x[i],x_exact[i]);
  }
  
  /*
   * test for solving Ux=b
   */
  if (result)
  {
    memset(b,0,sizeof(b));
    memset(x,0,sizeof(x));
    /* prepare a right-part vector */
    sp_matrix_skyline_ilu_upper_mv(&ILU,x_exact,b);
    
    /* solve for x */
    sp_matrix_skyline_ilu_upper_solve(&ILU,b,x);
    /* test result */
    for ( i = 0; i < m.rows_count; ++ i)
      result &= EQL(x[i],x_exact[i]);

    sp_matrix_free(&mtx);
    sp_matrix_skyline_ilu_free(&ILU);
  }
  
  printf("test_ilu result: *%s*\n",result ? "pass" : "fail");
  return result;
}

BOOL test_pcg_ilu_solver()
{
  BOOL result = TRUE;
  sp_matrix mtx;
  sp_matrix_skyline_ilu ilu;
  double v[3] = {0}, x[3] = {0};
  int max_iter = 20000;
  double tolerance = 1e-15;

  /* matrix solver test  */

  /* Test 1: */
  /*
   * | 1 0 -2 |   | 1 |   |-5 |
   * | 0 1  0 | x | 2 | = | 2 | 
   * |-2 0  5 |   | 3 |   |13 |
   */
  memset(x,0,3);
  v[0] = -5;
  v[1] = 2;
  v[2] = 13;
  sp_matrix_init(&mtx,3,3,2,CRS);
  
  MTX(&mtx,0,0,1);MTX(&mtx,0,2,-2);
  MTX(&mtx,1,1,1);
  MTX(&mtx,2,0,-2);MTX(&mtx,2,2,5);

  sp_matrix_compress(&mtx);
  sp_matrix_create_ilu(&mtx,&ilu);

  sp_matrix_solve_pcg_ilu(&mtx,&ilu,v,v,&max_iter,&tolerance,x);

  result = !( fabs(x[0]-1) > TOLERANCE ||
              fabs(x[1]-2) > TOLERANCE ||
              fabs(x[2]-3) > TOLERANCE);

  sp_matrix_skyline_ilu_free(&ilu);
  sp_matrix_free(&mtx);
  
  printf("test_pcg_ilu_solver result: *%s*\n",result ? "pass" : "fail");
  return result;
}

BOOL test_cholesky()
{
  BOOL result = TRUE;
  /* initial matrix */
  /* {90, 6, 4, 46, 29, 0, 26}, */
  /* {6, 127, 34, 22, 7, 0, 38}, */
  /* {4, 34, 108, 40, 2, 0, 4}, */
  /* {46, 22, 40, 96, 24, 0, 6}, */
  /* {29, 7, 2, 24, 155, 0, 37}, */
  /* {0, 0, 0, 0, 0, 64, 0}, */
  /* {26, 38, 4, 6, 37, 0, 70} */
  sp_matrix mtx;
  sp_matrix_skyline m;

  /* expected decomposition */
  double cholesky_expected[7][7] = 
    {{9.48683, 0.632456, 0.421637, 4.84883, 3.05687, 0., 2.74064},
     {0.,11.2517, 2.99807, 1.68271, 0.450304, 0., 3.22323},
     {0., 0., 9.94152,3.31043, -0.0642691, 0., -0.685914},
     {0., 0., 0., 7.66149, 1.12678,0., -1.36292},
     {0., 0., 0., 0., 12.0075, 0., 2.38705},
     {0., 0., 0.,0., 0., 8., 0.},
     {0., 0., 0., 0., 0., 0., 6.6388}};

  /* fill initial matrix */
  sp_matrix_init(&mtx,7,7,5,CRS);

/* {90, 6, 4, 46, 29, 0, 26}, */
  MTX(&mtx,0,0,90);MTX(&mtx,0,1,6);MTX(&mtx,0,2,4);MTX(&mtx,0,3,46);
  MTX(&mtx,0,4,29);MTX(&mtx,0,6,26);
/* {6, 127, 34, 22, 7, 0, 38}, */
  MTX(&mtx,1,0,6);MTX(&mtx,1,1,127);MTX(&mtx,1,2,34);MTX(&mtx,1,3,22);
  MTX(&mtx,1,4,7);MTX(&mtx,1,6,38);
/* {4, 34, 108, 40, 2, 0, 4}, */
  MTX(&mtx,2,0,4);MTX(&mtx,2,1,34);MTX(&mtx,2,2,108);MTX(&mtx,2,3,40);
  MTX(&mtx,2,4,2);MTX(&mtx,2,6,4);
/* {46, 22, 40, 96, 24, 0, 6}, */
  MTX(&mtx,3,0,46);MTX(&mtx,3,1,22);MTX(&mtx,3,2,40);MTX(&mtx,3,3,96);
  MTX(&mtx,3,4,24);MTX(&mtx,3,6,6);
/* {29, 7, 2, 24, 155, 0, 37}, */
  MTX(&mtx,4,0,29);MTX(&mtx,4,1,7);MTX(&mtx,4,2,2);MTX(&mtx,4,3,24);
  MTX(&mtx,4,4,155);MTX(&mtx,4,6,37);
/* {0, 0, 0, 0, 0, 64, 0}, */
  MTX(&mtx,5,5,64);
/* {26, 38, 4, 6, 37, 0, 70} */
  MTX(&mtx,6,0,26);MTX(&mtx,6,1,38);MTX(&mtx,6,2,4);MTX(&mtx,6,3,6);
  MTX(&mtx,6,4,37);MTX(&mtx,6,6,70);

  /* prepare initial matrix for conversion to Skyline format */
  sp_matrix_compress(&mtx);
  /* initialize skyline format from given CRS format */
  sp_matrix_skyline_init(&m,&mtx);


  /* clear matrix */
  sp_matrix_free(&mtx);
  sp_matrix_skyline_free(&m);
  
  printf("test_cholesky result: *%s*\n",result ? "pass" : "fail");
  return result;
}

int main(int argc, char *argv[])
{
  test_sp_matrix() &&
    test_triangle_solver() &&
    test_cg_solver() &&
    test_ilu() &&
    test_pcg_ilu_solver();
  
  return 0;
}
