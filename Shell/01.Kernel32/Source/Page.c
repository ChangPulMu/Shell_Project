#include "Page.h"

void kInitializePageTables(void){

	PML4TE* pstPML4TE;
	PDPTE* pstPDPTE;
	PDE* pstPDE;
	PSDE* pstPSDE;
	DWORD dwMappingAddress;
	int i;

	pstPML4TE = (PML4TE*) 0x100000;
	kSetPageEntryData( &(pstPML4TE[0]), 0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0);
	for(i=1; i < PAGE_MAXENTRYCOUNT; i++){
	
		kSetPageEntryData(&(pstPML4TE[i]),0,0,0,0);
		
	}

	pstPDPTE = (PDPTE*) 0x101000;
	for(i=0; i<64; i++){
		
		kSetPageEntryData(&(pstPDPTE[i]), 0, 0x103000 + (i * PAGE_TABLESIZE), PAGE_FLAGS_DEFAULT, 0);
		
	}
	for(i=64; i<PAGE_MAXENTRYCOUNT; i++){
		
		kSetPageEntryData(&(pstPDPTE[i]), 0, 0, 0, 0);
		
	}
	pstPDE = (PDE*) 0x103000;
	dwMappingAddress = 0;
	for(i=0; i < PAGE_MAXENTRYCOUNT * 64 ; i++){
	
		if(i == 0){
			kSetPageEntryData( &( pstPDE[ i ] ), 
				( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, 0x102000, PAGE_FLAGS_DEFAULT | 0x00, 0 );
	
		}
		else if(i == 5){
			kSetPageEntryData(&(pstPDE[ i ]), (1 * (PAGE_DEFAULTSIZE >> 20)) >> 12, 0, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
			
		}
		else{
			kSetPageEntryData( &( pstPDE[ i ] ), 
				( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0 );
		}
		dwMappingAddress += PAGE_DEFAULTSIZE;
		
	}
	
	
	pstPSDE = (PSDE*) 0x102000; 
	dwMappingAddress = 0;
	for(i=0; i<PAGE_MAXENTRYCOUNT ; i++){
		
		if( i == 511){
			kSetPageEntryData(&(pstPSDE[i]), 0, dwMappingAddress, PAGE_FLAGS_DEFAULT , 0);

		}
		else{		

			kSetPageEntryData(&(pstPSDE[i]), 0, dwMappingAddress, PAGE_FLAGS_DEFAULT , 0);
 
		}
		dwMappingAddress += 0x1000;
	}

}


void kSetPageEntryData( PTE* pstEntry, DWORD dwUpperBaseAddress,
        DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags ){

	pstEntry ->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry ->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;

}
