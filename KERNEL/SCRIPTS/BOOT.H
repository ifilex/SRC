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

#ifdef __cplusplus
extern "C" {
#endif

int swap (unsigned char *program_name,
          unsigned char *command_line,
          unsigned char *exec_return,
          unsigned char *swap_fname);

#ifdef __cplusplus
}
#endif

int ems4_installed (void);
int xms_installed (void);


#define SWAP_OK         (0)         
#define SWAP_NO_SHRINK  (1)        
#define SWAP_NO_SAVE    (2)        
#define SWAP_NO_EXEC    (3)    

#define BAD_FUNC        (0x01)   
#define FILE_NOT_FOUND  (0x02)
#define ACCESS_DENIED   (0x05)  
#define NO_MEMORY       (0x08) 
#define BAD_ENVIRON     (0x0A) 
#define BAD_FORMAT      (0x0B)  

