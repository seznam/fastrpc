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

if __name__ == '__main__':
    unittest.main()
