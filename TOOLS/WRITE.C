#include <stdio.h>
#include <dir.h>
#include <dos.h>
#include <process.h>
#include <string.h>

#define maxparaV  3
#define maxargV   3


int exist( inFname )
	char *inFname ;
{
struct	ffblk fi;

    return( findfirst(inFname, &fi,(  FA_RDONLY
                                   | FA_HIDDEN
                                   | FA_SYSTEM
                                   | FA_ARCH ) ) ) ;
}

int chkBat( inFname )
    char *inFname ;
{
    char work[80] ;
    char drive[3], dir[65], name[17], ext[8] ;

    fnsplit(inFname, drive, dir, name, ext) ;

    strcpy( work, inFname) ;
    if( 0==strcmp(ext, "" ) ) strcat(work, ".bat" ) ;
    else return(0) ;

    if( 0==strcmp(drive,"") && 0==strcmp( dir, "") ) {
        if( searchpath( work ) == NULL) return(0) ;
        else return(1) ;
    } else {
        if( exist( work ) == -1) return(0) ;
        else return(1) ;
    }
}

int genProcess( paraV )
    char *paraV[] ;
{
    int i ;
    char work[200] ;

    if( chkBat( paraV[0] ) ) {
        strcpy( work, paraV[0] ) ;
        i = 1 ;
        while( paraV[i] != NULL ) {
            strcat( work, " " ) ;
            strcat( work, paraV[i] ) ;
            i++ ;
        }
        system( work ) ;
        exit(0) ;
    }

    return( spawnvp(P_WAIT, paraV[0], paraV) ) ;
}

int main(int argc, char *argv[])
{
    char work[200], work1[200] ;
    char *paraV[maxparaV], paraVv[maxparaV][128] ;
    char *argx[maxargV] ;

    int  i, j ;

    if( argc >= maxargV ) j = maxargV ;
        else j = argc ;
    for( i = 0 ; i < j ; i++ ) argx[i] = argv[i] ;
    for( ; j < maxargV ; j++ ) argx[j] = "" ;

    fprintf(stdout, "\n") ;
    fprintf(stdout, "Writer Imager 1.2 for 2OS\n") ;
    fprintf(stdout, "GNU - GLP 2.0 - www.gnu.org\n") ;
    fprintf(stdout, "\n") ;
    fprintf(stdout, "Comando:\n") ;
    fprintf(stdout, "\n") ;
    fprintf(stdout, "write [imagen] [unidad:]\n") ;
    fprintf(stdout, "\n") ;
    fprintf(stdout, " [imagen] Archivo PAK de imagen instalatoria.\n") ;
    fprintf(stdout, " [unidad] Unidad donde se instalara. (ej a:)\n") ;
    fprintf(stdout, "\n") ;
    sprintf(work, "%s", argx[1]) ;
    if( exist(work) ) {
    } else {
        fprintf(stdout, "\n") ;
        sprintf(work, "copy %s x.exe >NUL", argx[1]) ;
        system(work) ;
        if( 0==exist("x.exe") ) {
            fprintf(stdout, "\n") ;
            fprintf(stdout, "Espere porfavor, descompactando imagen..\n") ;
            fprintf(stdout, "\n") ;
            sprintf(work, "x -e -y %s\\        >NUL", argx[2]) ;
            system(work) ;
            sprintf(work, "rm -r %s\\boot      >NUL", argx[2]) ;
            system(work) ;
            system("x -e -y boot\\*.*   >NUL") ;
            chdir( "boot" ) ;
            if( 0==exist("sys.com") ) {
                paraV[0] = "sys" ;
            	if( 0 == strcmp( paraV[0], "") ) paraV[0] = " " ;
                sprintf( paraVv[1], "%s", argx[2] ) ;
                paraV[1] = paraVv[1] ;
            	if( 0 == strcmp( paraV[1], "") ) paraV[1] = " " ;
                paraV[2] = NULL ;
                genProcess( paraV ) ;
                system("cd..") ;
                system("rm -r boot     >NUL") ;
                system("rm x.exe       >NUL") ;
                goto L_end ;
            }
        }
        fprintf(stdout, "\n") ;
        fprintf(stdout, "ATENCION!! Archivo corrupto!!\n") ;
        fprintf(stdout, "\n") ;
        fprintf(stdout, "Es probable que el formato de la imagen no sea soportada o bien que no\n") ;
        fprintf(stdout, "sea un archivo de imagen.\n") ;
        fprintf(stdout, "Los archivos de imagenes 2/OS CE son de extencion PAK y formato ARC.\n") ;
        fprintf(stdout, "\n") ;
        goto L_end ;
    }
L_end: ;
}
