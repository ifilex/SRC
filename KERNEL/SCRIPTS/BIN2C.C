/****************************************************************/
/*                                                              */
/*                                                              */
/*                            DOS-C                             */
/*                                                              */
/*      Block cache functions and device driver interface       */
/*                                                              */
/*                      Copyright (c) 1995                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/*                                                              */
/****************************************************************/

#include <stdio.h>

int main(int argc, char **argv)
{
  FILE *in, *out;
  int col;
  int c;

  if (argc < 4)
  {
    fprintf(stderr, "Uso: bin2c <archivo bin> <archivo H> <funcion>\n");
    return 1;
  }

  if ((in = fopen(argv[1], "rb")) == NULL)
  {
    fprintf(stderr, "No se puede abrir el archivo (%s).\n", argv[1]);
    return 1;
  }

  if ((out = fopen(argv[2], "wt")) == NULL)
  {
    fprintf(stderr, "No se puede abrir (%s).\n", argv[2]);
    return 1;
  }

  col = 0;

  fprintf(out, "unsigned char %s[] = {\n  ", argv[3]);

  while ((c = fgetc(in)) != EOF)
  {
    if (col)
    {
      fprintf(out, ", ");
    }
    if (col >= 8)
    {
      fprintf(out, "\n  ");
      col = 0;
    }
    fprintf(out, "0x%02X", c);
    col++;
  }

  fprintf(out, "\n};\n");
  fclose(in);
  fclose(out);

  return 0;
}
