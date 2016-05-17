from fastrpc import Boolean
import unittest

class BooleanTest(unittest.TestCase):

    def test_comparable(self):
        self.assertTrue(Boolean(True) == Boolean(True))
        self.assertTrue(Boolean(False) == Boolean(False))
        self.assertTrue(Boolean(True) != Boolean(False))
        self.assertTrue(Boolean(False) != Boolean(True))

        self.assertTrue(Boolean(True) >= Boolean(True))
        self.assertTrue(Boolean(True) >= Boolean(False))
        self.assertTrue(Boolean(False) < Boolean(True))
        self.assertTrue(Boolean(True) > Boolean(False))
        self.assertTrue(Boolean(False) <= Boolean(True))
        self.assertTrue(Boolean(True) <= Boolean(True))
        self.assertTrue(Boolean(False) <= Boolean(False))


    def test_other_comparable(self):
        self.assertTrue(Boolean(True) == True)
        self.assertTrue(Boolean(False) == False)
        self.assertTrue(Boolean(True) != False)
        self.assertTrue(Boolean(False) != True)

        self.assertTrue(True == Boolean(True))
        self.assertTrue(False == Boolean(False))
        self.assertTrue(True != Boolean(False))
        self.assertTrue(False != Boolean(True))

        self.assertTrue(Boolean(True) != None)
        self.assertTrue(Boolean(True) != "")

        cond_pass = False
        if Boolean(True):
            cond_pass = True
        self.assertTrue(cond_pass)

        cond_pass = True
        if Boolean(False):
            cond_pass = False
        self.assertTrue(cond_pass)

        # gt, lt ... etc throw
        excepted = False
        try:
            Boolean(False) > "asdfas"
        except TypeError:
            excepted = True
        self.assertTrue(excepted)

if __name__ == '__main__':
    unittest.main()
