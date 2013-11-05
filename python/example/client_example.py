#!/usr/bin/env python
#/***************************************************************************
# *   Copyright (C) 2004 by Miroslav Talasek                                *
# ***************************************************************************/

import fastrpc



client = fastrpc.ServerProxy("http://localhost:2424/RPC2",readTimeout=1000,\
                        writeTimeout=1000, connectTimeout=1000, \
                        useBinary =fastrpc.ON_SUPPORT_ON_KEEP_ALIVE)
                        
try:

    retVal1 = client.system.listMethods()
    print retVal1
    
    print client.system.methodSignature(retVal1[4])
    
    strct = client.test1(2,"hello")
    
    print strct
    
    print "\n Status is : %d "%(strct["status"])
    
except fastrpc.Fault, f:
    print f
except fastrpc.ProtocolError, e:
    print e
except fastrpc.Error, e:
    print e
