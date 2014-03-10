#!/usr/bin/env python
import fastrpc


def precall(method, args):
    """
    Callback called prior to RPC call
    Passed arguments:
        method name
        arguments
    """
    print
    print ":: Calling %s%s" % (method, args)
    # Pass some data to postCall()
    return dict(a=1, b="foo", c=None)


def postcall(method, args, data, err, errArg):
    """
    Callback called after the RPC call
    Passed arguments (in this order):
        method name
        arguments
        callback data from preCall (or None)
        exception (or None if no error occured)
        exception arguments (or None if no error occured)
    """
    print "   Finished %s%s" % (method, args)
    if data is not None:
        print "   (passed data: %s)" % data
    if err is not None:
        print "   -- Error! %s: %s" % (err, errArg)


client = fastrpc.ServerProxy("http://localhost:8080/RPC2", readTimeout=1000,
                             writeTimeout=1000, connectTimeout=1000,
                             useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE,
                             preCall=precall, postCall=postcall)

try:
    result = client.system.listMethods()

    print result
    print client.system.methodSignature(result[4])

    result = client.skeleton.test.method()

    print result
    print "Status is '%d'" % result["status"]

    # Raise an error
    result = client.skeleton.test.method("xx")
except fastrpc.Fault as f:
    print f
except fastrpc.ProtocolError as e:
    print e
except fastrpc.ResponseError as e:
    print e
