#!/usr/bin/python
import fastrpc
import unittest 

class ServerProxyTest(unittest.TestCase):
    def setUp(self):
        self.host = "localhost"
        self.port = 2424
        
        self.url = "http://%s:%i/RPC2" % (self.host, self.port)
        self.client = fastrpc.ServerProxy(self.url, readTimeout=1000,\
                        writeTimeout=1000, connectTimeout=1000, \
                        useBinary =fastrpc.ON_SUPPORT_ON_KEEP_ALIVE)
 
    def test_getattr(self): 
        self.assertEqual(getattr(self.client, "host"), self.host)
        self.assertEqual(getattr(self.client, "port"), self.port)
        self.assertEqual(getattr(self.client, "url"), self.url)


if __name__ == '__main__':
        unittest.main()
