#!/usr/bin/python
import fastrpc
import unittest
import sys
if sys.version_info.major >= 3:
    import configparser
else:
    import ConfigParser as configparser
from fastrpc import ProtocolError


class ServerProxyTest(unittest.TestCase):
    attributes_names = ["host", "port", "url", "last_call", "path"]
    def setUp(self):
        self.host = "localhost"
        self.port = 2424
        self.path = "/RPC2"
        self.last_call = ""

        self.url = "http://%s:%i%s" % (self.host, self.port, self.path)

    def test_getattr(self):
        client = fastrpc.ServerProxy(
            self.url, readTimeout=1000, writeTimeout=1000, connectTimeout=1000,
            useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE, hideAttributes=False)

        for attr in self.attributes_names:
            self.assertEqual(getattr(client, attr), getattr(self, attr))

    def test_hideAttibutes(self):
        """
        If hideAttributes parametr is set to True in constructor of ServerProxy
        all serverproxy attrs should be taken as frpc call attempt (proxy.url(), proxy.path()...)
        """
        default_hide = fastrpc.ServerProxy(
            self.url, readTimeout=1, writeTimeout=1, connectTimeout=1,
            useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE)

        without_hide_false = fastrpc.ServerProxy(
            self.url, readTimeout=1, writeTimeout=1, connectTimeout=1,
            useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE, hideAttributes=False)

        with_hide = fastrpc.ServerProxy(
            self.url, readTimeout=1, writeTimeout=1, connectTimeout=1,
            useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE, hideAttributes=True)

        def try_attr(client):
            for attr in self.attributes_names:
                self.assertEqual(getattr(client, attr), getattr(self, attr))

        def try_typeerror(client, exception):
            for attr in self.attributes_names:
                self.assertRaises(exception, lambda: getattr(client, attr)())

        try_attr(without_hide_false)

        try_typeerror(default_hide, ProtocolError)
        try_typeerror(without_hide_false, TypeError)
        try_typeerror(with_hide, ProtocolError)

    def test_action(self):
        client = fastrpc.ServerProxy(self.url, readTimeout=1, writeTimeout=1, connectTimeout=1,
            useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE)

        for attr in self.attributes_names:
            self.assertEqual(client("get_" + attr), getattr(self, attr))
        
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
            except AssertionError as exc:
                self.precall_exc = exc

        def fun_postcall(method, args, data, err, err_arg):
            self.postcall_called = True
            try:
                self.assertEqual(self.method_name, method)
            except AssertionError as exc:
                self.postcall_exc = exc

        client = fastrpc.ServerProxy(
            self.url, readTimeout=1000, writeTimeout=1000, connectTimeout=1000,
            useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE, preCall=fun_precall,
            postCall=fun_postcall)

        try:
            getattr(client, self.method_name)()
        except:
            pass

        self.assertTrue(self.precall_called)
        self.assertTrue(self.postcall_called)

        if self.precall_exc:
            raise self.precall_exc

        if self.postcall_exc:
            raise self.postcall_exc

    def test_raise(self):
        client = fastrpc.ServerProxy(
            self.url, readTimeout=1000, writeTimeout=1000, connectTimeout=1000,
            useBinary=fastrpc.ON_SUPPORT_ON_KEEP_ALIVE)

        try:
            client.system.listMethods()
            self.fail("exception fastrpc.ProtocolError expected")
        except fastrpc.ProtocolError:
            pass

    def test_configparser(self):
        section = "eggplant"
        config_parser = configparser.ConfigParser()
        config_parser.add_section(section)
        config_parser.set(section, "serverUrl", self.url)

        client = fastrpc.ServerProxy(config_parser, section)

        self.assertEqual(client("get_host"), self.host)
        self.assertEqual(client("get_port"), self.port)
        self.assertEqual(client("get_url"), self.url)

    def test_dictAccess(self):
        client_hide_false = fastrpc.ServerProxy(self.url, hideAttributes=False)
        client_hide_true = fastrpc.ServerProxy(self.url, hideAttributes=True)

        self.assertEqual(client_hide_false.__dict__['host'], self.host)
        self.assertEqual(client_hide_false.__dict__['path'], self.path)
        self.assertEqual(client_hide_false.__dict__['port'], self.port)
        self.assertEqual(client_hide_false.__dict__['url'], self.url)
        if sys.version_info.major >= 3:
            self.assertIsInstance(client_hide_false.__dict__['last_call'], str)
        else:
            self.assertIsInstance(client_hide_false.__dict__['last_call'], (str, unicode))

        self.assertEqual(client_hide_true.__dict__['host'], self.host)
        self.assertEqual(client_hide_true.__dict__['path'], self.path)
        self.assertEqual(client_hide_true.__dict__['port'], self.port)
        self.assertEqual(client_hide_true.__dict__['url'], self.url)
        if sys.version_info.major >= 3:
            self.assertIsInstance(client_hide_true.__dict__['last_call'], str)
        else:
            self.assertIsInstance(client_hide_true.__dict__['last_call'], (str, unicode))

    def test_reprstr(self):
        client = fastrpc.ServerProxy(self.url)
        self.assertTrue(str(type(client.foo)).endswith(" 'Method'>"))


if __name__ == '__main__':
    unittest.main()
