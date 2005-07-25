/*
 * FILE          $Id: frpcplatform.h,v 1.2 2005-07-25 06:10:47 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              ROman Marek <roman.marek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *       
 */

#ifndef FRPCPLATFORM_H_
#define FRPCPLATFORM_H_

#include "frpcsocket.h"

#ifndef FRPC_DLLEXPORT

#ifdef _WINDLL

#define FRPC_DLLEXPORT __declspec(dllexport)
#pragma warning(disable: 4275)
#pragma warning(disable: 4251) 

#else //_WINDLL

#define FRPC_DLLEXPORT

#endif //_WINDLL

#endif //FRPC_DLLEXPORT

#endif // FRPCPLATFORM_H_
