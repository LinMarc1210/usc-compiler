// opt01.usc
// Tests constant branch folding (true case)
//---------------------------------------------------------
// Copyright (c) 2014, Sanjay Madhav
// All rights reserved.
//
// This file is distributed under the BSD license.
// See LICENSE.TXT for details.
//---------------------------------------------------------

#include<stdio.h>

int main()
{
	int a = 20;
	int b = 55;
	int i = 0;
  int j = 0;
  int c = 0;
	while (i < 10)
	{
		a = a + 1;
    i = i + 1;
    while (j < 10)
    {
      b = b + 1;
      j = j + 1;
      while (c < 10)
      {
        c = c + 1;
        while (a > 1)
        {
          a = a - 1;
        }
      }
    }
	}

  while(a < 10)
  {
    a = a + 1;
  }
  a = 10;
	printf("%d\n", b);
	return 0;
}
