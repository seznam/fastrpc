#!/usr/bin/env python

import fastrpc

# Callback called prior to RPC call
# Passed arguments:
#   method name
#   arguments
def preCall(method, args):
    print
    print ":: Calling %s%s" % (method, args)
    # Pass some data to postCall()
    return dict(a=1, b="foo", c=None)


# Callback called after the RPC call
# Passed arguments (in this order):
#   method name
#   arguments
#   callback data from preCall (or None)
#   exception (or None if no error occured)
#   exception arguments (or None if no error occured)
#def postCall(*args):
def postCall(method, args, data, err, errArg):
    print "   Finished %s%s" % (method, args)
    if (data != None):
        print "   (passed data: %s)" % data
    if (err != None):
        print "   -- Error! %s: %s" % (err, errArg)


client = fastrpc.ServerProxy("http://localhost:8080/RPC2",readTimeout=1000,\
                        writeTimeout=1000, connectTimeout=1000, \
                        useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE,
                        preCall=preCall, postCall=postCall)

try:
    retVal1 = client.system.listMethods()
    print retVal1

    print client.system.methodSignature(retVal1[4])

    strct = client.skeleton.test.method()

    print strct
    print "Status is : %d "%(strct["status"])

    # Raise an error
    strct = client.skeleton.test.method("xx")

except fastrpc.Fault, f:
    print f
except fastrpc.ProtocolError, e:
    print e
except fastrpc.ResponseError,e:
    print e
