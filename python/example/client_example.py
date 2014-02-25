#!/usr/bin/env python
# coding=utf-8
"""
   Copyright (C) 2004 by Miroslav Talášek
"""
import fastrpc


client = fastrpc.ServerProxy("http://localhost:2424/RPC2", readTimeout=1000,
                             writeTimeout=1000, connectTimeout=1000,
                             useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE)

try:
    result = client.system.listMethods()

    print result
    print client.system.methodSignature(result[4])

    result = client.test1(2, "hello")

    print result
    print "Status is '%d'" % result["status"]
except fastrpc.Fault as f:
    print f
except fastrpc.ProtocolError as e:
    print e
except fastrpc.Error as e:
    print e
