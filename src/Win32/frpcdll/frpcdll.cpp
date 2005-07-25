/*
 * FILE          $Id: frpcdll.cpp,v 1.1 2005-07-25 06:10:49 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Roman MArek <roman.marek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *       
 */

#include "stdafx.h"
#include "frpcdll.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		WSADATA wsaData;
		WSAStartup(0x101,&wsaData);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		WSACleanup();
		break;
	}
    return TRUE;
}
