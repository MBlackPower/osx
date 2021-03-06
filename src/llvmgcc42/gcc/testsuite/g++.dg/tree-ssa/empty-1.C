/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-optimized" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */

struct S {};
S bar (const S &a)
{
  S s;
  s = a;
  return s;
}

/* Test whether memcpy call has been optimized out.  */
/* { dg-final { scan-tree-dump-times "memcpy" 0 "optimized"} } */
/* { dg-final { cleanup-tree-dump "optimized" } } */
