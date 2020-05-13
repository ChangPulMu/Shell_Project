

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define BYTESOFSECTOR  512

int AdjustInSectorSize( int iFd, int iSourceSize );
void WriteKernelInformation( int iTargetFd, int iTotalKernelSectorCount, int iKernel32SectorCount );
int CopyFile( int iSourceFd, int iTargetFd );
void GetHash(int iFd,int size);

int main(int argc, char* argv[])
{
    int iSourceFd;
    int iTargetFd;
    int iBootLoaderSize;
    int iKernel32SectorCount;
    int iKernel64SectorCount;
    int iSourceSize;
    
    if( argc < 4 )
    {
        fprintf( stderr, "[ERROR] ImageMaker.exe BootLoader.bin Kernel32.bin Kernel64.bin\n" );
        exit( -1 );
    }
    
   
    if( ( iTargetFd = open( "Disk.img", O_RDWR | O_CREAT |  O_TRUNC
           , S_IREAD | S_IWRITE ) ) == -1 )
    {
        fprintf( stderr , "[ERROR] Disk.img open fail.\n" );
        exit( -1 );
    }

    printf( "[INFO] Copy boot loader to image file\n" );
    if( ( iSourceFd = open( argv[ 1 ], O_RDONLY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 1 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    
    iBootLoaderSize = AdjustInSectorSize( iTargetFd , iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
            argv[ 1 ], iSourceSize, iBootLoaderSize );


    printf( "[INFO] Copy protected mode kernel to image file\n" );
    if( ( iSourceFd = open( argv[ 2 ], O_RDONLY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 2 ] );
        exit( -1 );
    }

	  
	
    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );

    iKernel32SectorCount = AdjustInSectorSize( iTargetFd, iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
                argv[ 2 ], iSourceSize, iKernel32SectorCount );

    printf( "[INFO] Copy 32e mode kernel to image file\n" );
    if( ( iSourceFd = open( argv[ 3 ], O_RDONLY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 3 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );

    iKernel64SectorCount = AdjustInSectorSize( iTargetFd, iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
                argv[ 3 ], iSourceSize, iKernel64SectorCount );
  
    
  
    
    printf( "[INFO] Start to write kernel information\n" );    
 
    WriteKernelInformation( iTargetFd, iKernel32SectorCount + iKernel64SectorCount, iKernel32SectorCount );
    printf( "[INFO] Image file create complete\n" );
    	
    close( iTargetFd );
	
    iTargetFd = 0;
    if( ( iTargetFd = open( "Disk.img", O_RDWR
           , S_IREAD | S_IWRITE ) ) == -1 )
    {
        fprintf( stderr , "[ERROR] Disk.img open fail.\n" );
        exit( -1 );
    }
    
    GetHash( iTargetFd ,iKernel32SectorCount );      
 
    return 0;
}
void GetHash(int iFd,int size){
	int hashSize = (size+1)*BYTESOFSECTOR;	
	char hBuffer[ hashSize ];
	char hash [4] = {0x00,0x00,0x00,0x00};	
	int i ;
	long iPosition;
	
    	printf( "[INFO] hashSize [%d]\n", 
        hashSize );
	
	read( iFd , hBuffer , sizeof( hBuffer ) );
	
	
	for( i = 516; i< hashSize; i = i+4 ){
			hash[0] = hash[0] ^ hBuffer[i];
			hash[1] = hash[1] ^ hBuffer[i+1];
			hash[2] = hash[2] ^ hBuffer[i+2];
			hash[3] = hash[3] ^ hBuffer[i+3];
			printf( "[INFO] hash [%d]\n", 
        		hBuffer[i] );
		
		}

    	
	

	printf( "[INFO] hash [%02x][%02x][%02x][%02x]\n", 
        hash[0],hash[1],hash[2],hash[3] );
	
	iPosition = lseek( iFd, 512 , SEEK_SET);
	if( iPosition == -1 )
    	{
    	    fprintf( stderr, "lseek fail. Return value = %d, errno = %d, %d\n", 
    	        (long int)iPosition, (long int)errno, (long int)SEEK_SET );
    	    exit( -1 );
    	}
	

	
	write( iFd , &hash , 4);

    	printf( "[INFO] Hash except Kernel32 \n");

}

int AdjustInSectorSize( int iFd, int iSourceSize )
{
    int i;
    int iAdjustSizeToSector;
    char cCh;
    int iSectorCount;

    iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
    cCh = 0x00;
    
    if( iAdjustSizeToSector != 0 )
    {
        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        printf( "[INFO] File size [%lu] and fill [%u] byte\n", iSourceSize, 
            iAdjustSizeToSector );
        for( i = 0 ; i < iAdjustSizeToSector ; i++ )
        {
            write( iFd , &cCh , 1 );
        }
    }
    else
    {
        printf( "[INFO] File size is aligned 512 byte\n" );
    }

    iSectorCount = ( iSourceSize + iAdjustSizeToSector ) / BYTESOFSECTOR;
    return iSectorCount;
}

void WriteKernelInformation( int iTargetFd, int iTotalKernelSectorCount, int iKernel32SectorCount  )
{
    unsigned short usData;
    long lPosition;
   
    lPosition = lseek( iTargetFd, 5, SEEK_SET );
    if( lPosition == -1 )
    {
        fprintf( stderr, "lseek fail. Return value = %d, errno = %d, %d\n", 
            (long int)lPosition, (long int)errno, (long int)SEEK_SET );
        exit( -1 );
    }

    usData = ( unsigned short ) iTotalKernelSectorCount;
    write( iTargetFd, &usData, 2 );
    usData = ( unsigned short ) iKernel32SectorCount;
    write( iTargetFd, &usData, 2 );	

    printf( "[INFO] Total sector count except boot loader [%d]\n", 
        iTotalKernelSectorCount );
    printf( "[INFO] protectes kernel count except boot loader [%d]\n", 
        iKernel32SectorCount );
}


int CopyFile( int iSourceFd, int iTargetFd )
{
    int iSourceFileSize;
    int iRead;
    int iWrite;
    char vcBuffer[ BYTESOFSECTOR ];
    int i;
	
    iSourceFileSize = 0;
    while( 1 )
    {
        iRead   = read( iSourceFd, vcBuffer, sizeof( vcBuffer ) );
        iWrite  = write( iTargetFd, vcBuffer, iRead );

        if( iRead != iWrite )
        {
            fprintf( stderr, "[ERROR] iRead != iWrite.. \n" );
            exit(-1);
        }
        iSourceFileSize += iRead;
	
       
	if( iRead != sizeof( vcBuffer ) )
        {
            break;
        }
    }
	
	
    return iSourceFileSize;
} 
