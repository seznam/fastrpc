#!/usr/bin/python

import fastrpc
import unittest


class ExceptionTest(unittest.TestCase):

    def test_repr(self):
        self.assertEqual(repr(fastrpc.Error('message')), '<fastrpc.Error>')
        self.assertEqual(repr(fastrpc.ResponseError('message')), '<fastrpc.ResponseError: message>')
        self.assertEqual(repr(fastrpc.ProtocolError(100, 'message')), '<fastrpc.ProtocolError: 100, message>')
        self.assertEqual(repr(fastrpc.Fault(100, 'message')), '<fastrpc.Fault: 100, message>')

        # NOTE: We cannot test constructor with method since the MethodObject
        # has no Python wrapper.
        #self.assertEqual(repr(fastrpc.ResponseError('message', method)), '')
        #self.assertEqual(repr(fastrpc.ProtocolError(100, 'message', method), ''))
        #self.assertEqual(repr(fastrpc.Fault(100, 'message', method)), '')


        pass

    def test_str(self):
        self.assertEqual(str(fastrpc.Error('message')), '<fastrpc.Error>')
        self.assertEqual(str(fastrpc.ResponseError('message')), '<fastrpc.ResponseError: message>')
        self.assertEqual(str(fastrpc.ProtocolError(100, 'message')), '<fastrpc.ProtocolError: 100, message>')
        self.assertEqual(str(fastrpc.Fault(100, 'message')), '<fastrpc.Fault: 100, message>')

        # NOTE: We cannot test constructor with method since the MethodObject
        # has no Python wrapper.
        #self.assertEqual(str(fastrpc.ResponseError('message', method)), '')
        #self.assertEqual(str(fastrpc.ProtocolError(100, 'message', method), ''))
        #self.assertEqual(str(fastrpc.Fault(100, 'message', method)), '')


if __name__ == '__main__':
    unittest.main()
