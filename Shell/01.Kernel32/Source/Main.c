
#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"


void kPrintString( int iX, int iY, const char* pcString );

BOOL kInitializeKernel164Area( void );
void kCopyKernel64ImageTo2Mbyte( void );

void Main( void )
{
	DWORD i;
	kPrintString( 0, 5, "C Language Kernel Started~!!!" );
	

	
	kInitializeKernel164Area();
	kPrintString(0, 7, "IA_32e Kernel Area Initialization Complete!!");
	
	kInitializePageTables();
	kPrintString(0, 8, "IA_32e page table Initialization Complete!!");
	
	kCopyKernel64ImageTo2Mbyte();
	kPrintString(0, 9, "IA_32e copy in 130MB Complete!!");
	
	kSwitchAndExecute64bitKernel();
	
	

    	while( 1 ) ;

}

void kPrintString( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
    int i;
    
   
    pstScreen += ( iY * 80 ) + iX;
    
  
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
    }
}


BOOL kInitializeKernel164Area( void )
{	
	DWORD* pdwCurrentAdd;
	pdwCurrentAdd = (DWORD*) 0x100000;
	
	while((DWORD)pdwCurrentAdd < 0x600000){
		
		*pdwCurrentAdd =0x00;
		
		if( *pdwCurrentAdd != 0){
			return FALSE;
		}
	
		pdwCurrentAdd++;
	}
	return TRUE;	
}
void kCopyKernel64ImageTo2Mbyte( void ){
	WORD wKernel32SectorCount, wTotalKernelSectorCount;
	DWORD* pdwSourceAdd, * pdwDestinationAdd;
	int i;
	
	wTotalKernelSectorCount = *( (WORD*) 0x7C05 );
	wKernel32SectorCount = *( (WORD*) 0x7C07 );
	
	pdwSourceAdd = (DWORD*) (0x10000 + (wKernel32SectorCount * 512));
	pdwDestinationAdd = (DWORD*) 0x200000;
	
	for(i=0; i< 512*(wTotalKernelSectorCount - wKernel32SectorCount ) / 4; i++){
		*pdwDestinationAdd = *pdwSourceAdd;
		pdwSourceAdd++;
		pdwDestinationAdd++;
	
	}
	


}
 



