#!/usr/bin/python

import fastrpc
import unittest

class ServerTest(unittest.TestCase): 

    def test_preRead(self):
        self.preRead_called = False

        def preRead():
            self.preRead_called = True

        server = fastrpc.Server(readTimeout=0, callbacks={"preRead": preRead})

        try:
            server.serve(0)
        except:
            pass

        self.assertTrue(self.preRead_called)


    def test_preRead_raised(self):
        faultString = "Unhandled exception in preread <type 'exceptions.ValueError'>: something"
        def preRead():
            raise ValueError("something")

        server = fastrpc.Server(readTimeout=0, callbacks={"preRead": preRead})
        
        try:
            server.serve(0)
            raise Exception("error expected")
        except fastrpc.Fault, exc:
            self.assertEqual(exc.faultString, faultString)
            pass



if __name__ == '__main__':
        unittest.main()
