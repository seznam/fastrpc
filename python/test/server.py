#!/usr/bin/python

import sys
import fastrpc
import unittest


class ServerTest(unittest.TestCase):
    def test_preread(self):
        self.preread_called = False

        def preread():
            self.preread_called = True

        server = fastrpc.Server(readTimeout=0, callbacks={"preRead": preread})

        try:
            server.serve(0)
        except:
            pass

        self.assertTrue(self.preread_called)

    def test_preread_raised(self):
        if sys.version_info.major >= 3:
            fault_string = "Unhandled exception in preread " \
                        "<class 'ValueError'>: something"
        else:
            fault_string = "Unhandled exception in preread " \
                        "<type 'exceptions.ValueError'>: something"

        def preread():
            raise ValueError("something")

        server = fastrpc.Server(readTimeout=0, callbacks={"preRead": preread})

        try:
            server.serve(0)
            raise Exception("error expected")
        except fastrpc.Fault as exc:
            self.assertEqual(exc.faultString, fault_string)
            pass


if __name__ == '__main__':
    unittest.main()
