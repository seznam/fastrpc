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

    def test_convert_unicode(self):
        self.assertEqual(DateTime(DateTime("2016-01-02").value), DateTime("2016-01-02"))

    def test_compare(self):
        """ test <,>,=
        """
        self.assertEqual(self.datetype(1408967234), self.datetype(1408967234))
        self.assertTrue(self.datetype(1408967234) > self.datetype(1408967233))
        self.assertTrue(self.datetype(1408967233) < self.datetype(1408967234))
        self.assertTrue(self.datetype(1) < self.datetype(1408967234))

    def test_othertype_comparable(self):
        """ test ==, != against other types """
        d0 = self.datetype(0)
        d1 = self.datetype(1)

        self.assertFalse(d0 == "19700101T01:00:00+0000")
        self.assertTrue(d0  != "19700101T01:00:00+0000")

        self.assertTrue(d0 != True)
        self.assertTrue(d0 != False)
        self.assertTrue(d0 != "")
        self.assertTrue(d0 != "True")
        self.assertTrue(d1 != True)
        self.assertTrue(d1 != False)
        self.assertTrue(d1 != "")
        self.assertTrue(d1 != "True")
        self.assertFalse(d0 == True)
        self.assertFalse(d0 == False)
        self.assertFalse(d0 == "")
        self.assertFalse(d0 == "True")
        self.assertFalse(d1 == True)
        self.assertFalse(d1 == False)
        self.assertFalse(d1 == "")
        self.assertFalse(d1 == "True")

        # gt, lt ... etc throw
        excepted = False
        try:
            d1 < ""
        except:
            excepted = True
        self.assertTrue(excepted)

        # gt, lt ... etc throw
        excepted = False
        try:
            d1 > ""
        except:
            excepted = True
        self.assertTrue(excepted)

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
