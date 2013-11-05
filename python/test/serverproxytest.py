#!/usr/bin/python
import fastrpc
import unittest 
import ConfigParser

class ServerProxyTest(unittest.TestCase):
    def setUp(self):
        self.host = "localhost"
        self.port = 2424
        
        self.url = "http://%s:%i/RPC2" % (self.host, self.port)
 
    def test_getattr(self): 
        client = fastrpc.ServerProxy(self.url, readTimeout=1000,\
                        writeTimeout=1000, connectTimeout=1000, \
                        useBinary =fastrpc.ON_SUPPORT_ON_KEEP_ALIVE)
        self.assertEqual(getattr(client, "host"), self.host)
        self.assertEqual(getattr(client, "port"), self.port)
        self.assertEqual(getattr(client, "url"), self.url)

    def test_hooks(self):
        self.method_name = "get_something"
        self.precall_called = False
        self.postcall_called = False
        self.precall_exc = None
        self.postcall_exc = None

        def fun_precall(method, args):
            self.precall_called = True
            try: 
                self.assertEqual(self.method_name, method)
            except:
                self.precall_exc = exc

        def fun_postcall(method, args, data, err, errArg):
            self.postcall_called = True
            try: 
                self.assertEqual(self.method_name, method)
            except Exception, exc:
                self.postcall_exc = exc


        client = fastrpc.ServerProxy(self.url, readTimeout=1000,\
                        writeTimeout=1000, connectTimeout=1000, \
                        useBinary =fastrpc.ON_SUPPORT_ON_KEEP_ALIVE, preCall=fun_precall, postCall=fun_postcall)

        try:
            getattr(client,self.method_name)()
        except:
            pass

        self.assertTrue(self.precall_called)
        self.assertTrue(self.postcall_called)

        if self.precall_exc:
            raise self.precall_exc

        if self.postcall_exc:
            raise self.postcall_exc

    def test_raise(self):
        client = fastrpc.ServerProxy(self.url, readTimeout=1000,\
                        writeTimeout=1000, connectTimeout=1000, \
                        useBinary =fastrpc.ON_SUPPORT_ON_KEEP_ALIVE,)

        try:
            client.system.listMethods()
            raise Exception("exception fastrpc.ProtocolError expected")
        except fastrpc.ProtocolError:
            pass

    def test_configparser(self):
        section = "baklazan"
        config_parser = ConfigParser.ConfigParser()
        config_parser.add_section(section)
        config_parser.set(section, "serverUrl", self.url)

        client = fastrpc.ServerProxy(config_parser, section)

        self.assertEqual(getattr(client, "host"), self.host)
        self.assertEqual(getattr(client, "port"), self.port)
        self.assertEqual(getattr(client, "url"), self.url)



if __name__ == '__main__':
        unittest.main()
