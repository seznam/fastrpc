#!/usr/bin/python
""" Testcases for datime types DateTime, LocalTime, UTCTime
"""

from fastrpc import DateTime, LocalTime, UTCTime
import unittest

class GenericDatetypeTests(unittest.TestCase):
    """generic TestCase for all datetime types in frpc
    """

    datetype = RuntimeError

    @classmethod
    def setUpClass(cls):
        if cls is GenericDatetypeTests:
            raise unittest.SkipTest("Skip BaseTest tests, it's a base class")
        super(GenericDatetypeTests, cls).setUpClass()

    def _factory(self):
        return self.datetype()

    def test_compare(self):
        """ test <,>,=
        """
        self.assertEqual(self.datetype(1408967234), self.datetype(1408967234))
        self.assertTrue(self.datetype(1408967234) > self.datetype(1408967233))
        self.assertTrue(self.datetype(1408967233) < self.datetype(1408967234))
        self.assertTrue(self.datetype(1) < self.datetype(1408967234))

    def test_init(self):
        self._factory()
        self.datetype(123)
        self.datetype()

    def test_str(self):
        str(self._factory())

    def test_getattr(self):
        date = self._factory()
        self.assertTrue(hasattr(date, 'year'))
        self.assertTrue(hasattr(date, 'month'))
        self.assertTrue(hasattr(date, 'min'))
        self.assertTrue(hasattr(date, 'sec'))
        self.assertTrue(hasattr(date, 'value'))
        self.assertTrue(hasattr(date, 'timeZone'))

    def test_setattr(self):
        date = self._factory()
        setattr(date, "year", 1)
        self.assertEqual(date.year, 1)
        date.year = 3
        self.assertEqual(date.year, 3)

      
class DateTimeTests(GenericDatetypeTests):
    datetype = DateTime

    def _factory(self):
        return self.datetype("2015-07-22T13:53:01+0000")

    def test_init(self):
        self._factory()
        self.datetype(123,3)
        #super(DateTimeTests, self).test_init()

    def test_compare(self):
        #super(DateTimeTests, self).test_compare()

        date_a = self.datetype(123, 2)
        date_b = self.datetype(123, 1)

        self.assertTrue(date_a < date_b)

class LocalTimeTests(GenericDatetypeTests):
    datetype = LocalTime

class UTCTimeTests(GenericDatetypeTests):
    datetype = UTCTime

if __name__ == '__main__':
    unittest.main()
