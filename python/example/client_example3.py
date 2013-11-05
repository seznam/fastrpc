#!/usr/bin/env python
#
#  Simple client initialized from configuration file
#
# Copyright (C) 2013 by Tomas Golembiovsky

import fastrpc
import ConfigParser

        
cfg = ConfigParser.RawConfigParser()
cfg.read("client.conf")


client = fastrpc.ServerProxy(cfg, "proxy")
                        
try:

    retVal1 = client.system.listMethods()
    print retVal1
    
    print client.system.methodSignature(retVal1[4])
    
except fastrpc.Fault, f:
    print f
except fastrpc.ProtocolError, e:
    print e
