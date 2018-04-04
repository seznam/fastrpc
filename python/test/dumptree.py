from fastrpc import dumpTree
import unittest

class DumpTreeTest(unittest.TestCase):

    def test_hide_sensitive_names(self):
        value = {'extremely-long-key-name': 'sensitive-content'}
        level = 3
        names = {'extremely-long-key-name': 1}
        pos = 0

        self.assertEqual('{"extremely-...": -hidden-}', dumpTree(value, level, names, pos))

    def test_hide_sensitive_pos(self):
        value = ['one', 'two', 'three', 'four']
        level = 3
        names = {'extremely-long-key-name': 1}
        pos = int('1010', 2)

        self.assertEqual('("one", -hidden-, "three", -hidden-)', dumpTree(value, level, names, pos))


    def test_dict_key_types(self):
        level = 3
        names = {}
        pos = 0

        self.assertEqual('{true: "first"}', dumpTree({True: 'first'}, level, names, pos))
        self.assertEqual('{false: "second"}', dumpTree({False: 'second'}, level, names, pos))
        self.assertEqual('{3: "third"}', dumpTree({3: 'third'}, level, names, pos))
        self.assertEqual('{4.2: "fourth"}', dumpTree({4.2: 'fourth'}, level, names, pos))
        self.assertEqual('{"5": "fifth"}', dumpTree({'5': 'fifth'}, level, names, pos))
        test_hash_object = TestHashObject('sixth')
        self.assertEqual('{'+str(test_hash_object)+': "sixth"}', dumpTree({test_hash_object: 'sixth'}, level, names, pos))

class TestHashObject(object):
    def __init__(self, name):
        self.name = name

    def __hash__(self):
        return hash((self.name))

if __name__ == '__main__':
    unittest.main()
