#! /usr/bin/env python
# coding=utf-8
from datetime import date, datetime, time
from io import BytesIO
from sys import maxint
from unittest.case import TestCase
from unittest.main import TestProgram

from fastrpc import dumps as _dumps
from fastrpc import DateTime, LocalTime, UTCTime, Boolean, Binary

dumps = lambda parameters=(), method=None: \
    _dumps(parameters, method, useBinary=True, protocolVersionMajor=2, protocolVersionMinor=1)
result = lambda suffix: "\xca\x11\x02\x01{}".format(suffix)  # FastRPC magic


class DumpsTestCase(TestCase):
    def assertDumps(self, parameters=(), method=None, suffix=""):
        self.assertEqual(dumps(parameters, method), result(suffix))

    def assertResponse(self, result, suffix=""):
        self.assertDumps((result, ), suffix="p{}".format(suffix))

    def assertCall(self, parameters, method, suffix=""):
        self.assertDumps(parameters, method, suffix="h{}".format(suffix))

    def test_first_argument_bad_values(self):
        self.assertRaises(TypeError, dumps, set())
        self.assertRaises(TypeError, dumps, frozenset())
        self.assertRaises(TypeError, dumps, dict())
        self.assertRaises(TypeError, dumps, 1)
        self.assertRaises(TypeError, dumps, True)
        self.assertRaises(TypeError, dumps, "")
        self.assertRaises(TypeError, dumps, None)

    def test_empty_response(self):
        self.assertDumps((), suffix="p")
        self.assertDumps([], suffix="p")

    def test_empty_request(self):
        self.assertRaises(TypeError, self.assertDumps, parameters=(), method="")
        self.assertCall((), "a", "\x01a")
        self.assertCall((), "ф", "\x02\xd1\x84")
        self.assertCall((), "a.b", "\x03a.b")
        self.assertCall((), "屁.眼", "\x07\xe5\xb1\x81.\xe7\x9c\xbc")
        self.assertCall((), "a" * 255, "\xff" + "a" * 255)
        self.assertRaises(TypeError, self.assertDumps, parameters=(), method="a" * 256)
        self.assertCall([], "b", "\x01b")

    def test_boolean(self):
        self.assertResponse(True, "\x11")
        self.assertResponse(False, "\x10")
        self.assertResponse(Boolean(1), "\x11")
        self.assertResponse(Boolean(0), "\x10")

    def test_integer(self):
        self.assertResponse(0, "8\x00")
        self.assertResponse(1, "8\x01")
        self.assertResponse(1L, "8\x01")

        self.assertResponse(maxint, "?\xff\xff\xff\xff\xff\xff\xff\x7f")
        self.assertResponse(-maxint, "G\xff\xff\xff\xff\xff\xff\xff\x7f")

        self.assertRaises(OverflowError, self.assertResponse, 18446744073709551616)
        self.assertResponse(18446744073709551615, "@\x01")  # Biggest supported int
        self.assertResponse(1844674407370955161, "?\x99\x99\x99\x99\x99\x99\x99\x19")
        self.assertResponse(184467440737095516, "?\\\x8f\xc2\xf5(\\\x8f\x02")
        self.assertResponse(18446744073709551, ">\xef\xa7\xc6K7\x89A")
        self.assertResponse(1844674407370955, ">\xcb\x10\xc7\xba\xb8\x8d\x06")
        self.assertResponse(184467440737095, "=G\x1bG\xac\xc5\xa7")
        self.assertResponse(18446744073709, "=\xed\xb5\xa0\xf7\xc6\x10")
        self.assertResponse(1844674407370, "=\xca\xab)\x7f\xad\x01")
        self.assertResponse(184467440737, "<a\xc4\x1d\xf3*")
        self.assertResponse(18446744073, "<\t\xfa\x82K\x04")
        self.assertResponse(1844674407, ";g\x7f\xf3m")
        self.assertResponse(184467440, ";\xf0\xbf\xfe\n")
        self.assertResponse(18446744, ";\x98y\x19\x01")
        self.assertResponse(1844674, ":\xc2%\x1c")
        self.assertResponse(184467, ":\x93\xd0\x02")
        self.assertResponse(18446, "9\x0eH")
        self.assertResponse(1844, "94\x07")
        self.assertResponse(184, "8\xb8")
        self.assertResponse(18, "8\x12")
        self.assertResponse(1, "8\x01")

        self.assertRaises(OverflowError, self.assertResponse, -9223372036854775809)
        # Smallest supported int ---v
        self.assertResponse(-9223372036854775808, "G\x00\x00\x00\x00\x00\x00\x00\x80")
        self.assertResponse(-922337203685477580, "G\xcc\xcc\xcc\xcc\xcc\xcc\xcc\x0c")
        self.assertResponse(-92233720368547758, "G\xaeG\xe1z\x14\xaeG\x01")
        self.assertResponse(-9223372036854775, "F\xf7S\xe3\xa5\x9b\xc4 ")
        self.assertResponse(-922337203685477, "Fe\x88c]\xdcF\x03")
        self.assertResponse(-92233720368547, "E\xa3\x8d#\xd6\xe2S")
        self.assertResponse(-9223372036854, "E\xf6Z\xd0{c\x08")
        self.assertResponse(-922337203685, "D\xe5\xd5\x94\xbf\xd6")
        self.assertResponse(-92233720368, "D0\xe2\x8ey\x15")
        self.assertResponse(-9223372036, "D\x04}\xc1%\x02")
        self.assertResponse(-922337203, "C\xb3\xbf\xf96")
        self.assertResponse(-92233720, "C\xf8_\x7f\x05")
        self.assertResponse(-9223372, "B\xcc\xbc\x8c")
        self.assertResponse(-922337, "B\xe1\x12\x0e")
        self.assertResponse(-92233, "BIh\x01")
        self.assertResponse(-9223, "A\x07$")
        self.assertResponse(-922, "A\x9a\x03")
        self.assertResponse(-92, "@\\")
        self.assertResponse(-9, "@\t")

    def test_float(self):
        self.assertResponse(1., "\x18\x00\x00\x00\x00\x00\x00\xf0?")
        self.assertResponse(-1., "\x18\x00\x00\x00\x00\x00\x00\xf0\xbf")
        self.assertResponse(0., "\x18\x00\x00\x00\x00\x00\x00\x00\x00")
        self.assertResponse(.1, "\x18\x9a\x99\x99\x99\x99\x99\xb9?")
        self.assertResponse(-.1, "\x18\x9a\x99\x99\x99\x99\x99\xb9\xbf")

        self.assertResponse(float("inf"), "\x18\x00\x00\x00\x00\x00\x00\xf0\x7f")
        self.assertResponse(float("-inf"), "\x18\x00\x00\x00\x00\x00\x00\xf0\xff")

        self.assertResponse(123456789.0, "\x18\x00\x00\x00T4o\x9dA")
        self.assertResponse(-0.123456789, "\x18_c97\xdd\x9a\xbf\xbf")

        self.assertResponse(1e-7, "\x18H\xaf\xbc\x9a\xf2\xd7z>")
        self.assertResponse(1e-8, "\x18:\x8c0\xe2\x8eyE>")
        self.assertResponse(1e-13, "\x18\x82vIh\xc2%<=")
        self.assertResponse(1e-14, "\x18\x9b+\xa1\x86\x9b\x84\x06=")
        self.assertResponse(1e-323, "\x18\x02\x00\x00\x00\x00\x00\x00\x00")  # Smallest not zero

        # Actually those tests show nothing. Python can't normally process so big numbers :)
        self.assertResponse(18446744073709551615e18446744073709551615,
                            "\x18\x00\x00\x00\x00\x00\x00\xf0\x7f")
        self.assertResponse(18446744073709551616e18446744073709551616,
                            "\x18\x00\x00\x00\x00\x00\x00\xf0\x7f")

    def test_string(self):
        self.assertResponse("", " \x00")
        self.assertResponse("a", " \x01a")
        self.assertResponse("a" * 255, " \xff" + "a" * 255)
        self.assertResponse("a" * 1024, "!\x00\x04" + "a" * 1024)  # 1Kb
        self.assertResponse("a" * 1024 ** 2, '"\x00\x00\x10' + "a" * 1024 ** 2)  # 1Mb
        # self.assertResponse("a" * 1024 ** 3, "#\x00\x00\x00@" + "a" * 1024 ** 3)  # 1Gb (slow)

        self.assertResponse("abcdefghijklmnopqrstuvwxyz", " \x1aabcdefghijklmnopqrstuvwxyz")
        self.assertResponse("aábcčdďeéěfghchiíjklmnňoópqrřsštťuúůvwxyýzž",
                            " :a\xc3\xa1bc\xc4\x8dd\xc4\x8fe\xc3\xa9\xc4\x9bfghchi\xc3\xadjklmn\xc5"
                            "\x88o\xc3\xb3pqr\xc5\x99s\xc5\xa1t\xc5\xa5u\xc3\xba\xc5\xafvwxy\xc3"
                            "\xbdz\xc5\xbe")
        self.assertResponse("абвгдеёжзийклмнопрстуфхцчшщъыьэюя",
                            " B\xd0\xb0\xd0\xb1\xd0\xb2\xd0\xb3\xd0\xb4\xd0\xb5\xd1\x91\xd0\xb6\xd0"
                            "\xb7\xd0\xb8\xd0\xb9\xd0\xba\xd0\xbb\xd0\xbc\xd0\xbd\xd0\xbe\xd0\xbf"
                            "\xd1\x80\xd1\x81\xd1\x82\xd1\x83\xd1\x84\xd1\x85\xd1\x86\xd1\x87\xd1"
                            "\x88\xd1\x89\xd1\x8a\xd1\x8b\xd1\x8c\xd1\x8d\xd1\x8e\xd1\x8f")

        # Whole ASCII
        self.assertResponse("".join(chr(i) for i in range(256)),
                            "!\x00\x01\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f"
                            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f !\"#$"
                            "%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefgh"
                            "ijklmnopqrstuvwxyz{|}~\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a"
                            "\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b"
                            "\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac"
                            "\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd"
                            "\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce"
                            "\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
                            "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0"
                            "\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff")

        # Czech and slovak pangrams
        self.assertResponse(u"Příliš žluťoučký kůň úpěl ďábelské ódy.",
                            " 6P\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k"
                            "\xc5\xaf\xc5\x88 \xc3\xbap\xc4\x9bl \xc4\x8f\xc3\xa1belsk\xc3\xa9 \xc3"
                            "\xb3dy.")
        self.assertResponse(u"Kŕdeľ ďatľov učí koňa žrať kôru.",
                            " *K\xc5\x95de\xc4\xbe \xc4\x8fat\xc4\xbeov u\xc4\x8d\xc3\xad ko\xc5"
                            "\x88a \xc5\xbera\xc5\xa5 k\xc3\xb4ru.")

        # Right-to-left
        self.assertResponse(u"صِف خَلقَ خَودِ كَمِثلِ الشَمسِ إِذ بَزَغَت — "
                            u"يَحظى الضَجيعُ بِها نَجلاءَ مِعطارِ",
                            " \x97\xd8\xb5\xd9\x90\xd9\x81 \xd8\xae\xd9\x8e\xd9\x84\xd9\x82\xd9\x8e"
                            " \xd8\xae\xd9\x8e\xd9\x88\xd8\xaf\xd9\x90 \xd9\x83\xd9\x8e\xd9\x85\xd9"
                            "\x90\xd8\xab\xd9\x84\xd9\x90 \xd8\xa7\xd9\x84\xd8\xb4\xd9\x8e\xd9\x85"
                            "\xd8\xb3\xd9\x90 \xd8\xa5\xd9\x90\xd8\xb0 \xd8\xa8\xd9\x8e\xd8\xb2\xd9"
                            "\x8e\xd8\xba\xd9\x8e\xd8\xaa \xe2\x80\x94 \xd9\x8a\xd9\x8e\xd8\xad\xd8"
                            "\xb8\xd9\x89 \xd8\xa7\xd9\x84\xd8\xb6\xd9\x8e\xd8\xac\xd9\x8a\xd8\xb9"
                            "\xd9\x8f \xd8\xa8\xd9\x90\xd9\x87\xd8\xa7 \xd9\x86\xd9\x8e\xd8\xac\xd9"
                            "\x84\xd8\xa7\xd8\xa1\xd9\x8e \xd9\x85\xd9\x90\xd8\xb9\xd8\xb7\xd8\xa7"
                            "\xd8\xb1\xd9\x90")

        self.assertResponse(u"色は匂へど 散りぬるを 我が世誰ぞ 常ならむ 有為の奥山 "
                            u"今日越えて 浅き夢見じ 酔ひもせず（ん）",
                            " \x85\xe8\x89\xb2\xe3\x81\xaf\xe5\x8c\x82\xe3\x81\xb8\xe3\x81\xa9 \xe6"
                            "\x95\xa3\xe3\x82\x8a\xe3\x81\xac\xe3\x82\x8b\xe3\x82\x92 \xe6\x88\x91"
                            "\xe3\x81\x8c\xe4\xb8\x96\xe8\xaa\xb0\xe3\x81\x9e \xe5\xb8\xb8\xe3\x81"
                            "\xaa\xe3\x82\x89\xe3\x82\x80 \xe6\x9c\x89\xe7\x82\xba\xe3\x81\xae\xe5"
                            "\xa5\xa5\xe5\xb1\xb1 \xe4\xbb\x8a\xe6\x97\xa5\xe8\xb6\x8a\xe3\x81\x88"
                            "\xe3\x81\xa6 \xe6\xb5\x85\xe3\x81\x8d\xe5\xa4\xa2\xe8\xa6\x8b\xe3\x81"
                            "\x98 \xe9\x85\x94\xe3\x81\xb2\xe3\x82\x82\xe3\x81\x9b\xe3\x81\x9a\xef"
                            "\xbc\x88\xe3\x82\x93\xef\xbc\x89")

        # Some Unicode
        self.assertResponse(u"(╯°□°）╯︵ ┻━┻",
                            " \x1e(\xe2\x95\xaf\xc2\xb0\xe2\x96\xa1\xc2\xb0\xef\xbc\x89\xe2\x95\xaf"
                            "\xef\xb8\xb5 \xe2\x94\xbb\xe2\x94\x81\xe2\x94\xbb")
        self.assertResponse(u"•*´¨`*•.¸¸.•*´¨`*•.¸¸.•*´¨`*•.¸¸.•*´¨`*•.¸¸.•",
                            " O\xe2\x80\xa2*\xc2\xb4\xc2\xa8`*\xe2\x80\xa2.\xc2\xb8\xc2\xb8.\xe2"
                            "\x80\xa2*\xc2\xb4\xc2\xa8`*\xe2\x80\xa2.\xc2\xb8\xc2\xb8.\xe2\x80\xa2*"
                            "\xc2\xb4\xc2\xa8`*\xe2\x80\xa2.\xc2\xb8\xc2\xb8.\xe2\x80\xa2*\xc2\xb4"
                            "\xc2\xa8`*\xe2\x80\xa2.\xc2\xb8\xc2\xb8.\xe2\x80\xa2")

    def test_none(self):
        self.assertResponse(None, "`")

    def test_date(self):
        self.assertResponse(datetime(1985, 1, 2, 3, 4, 5, 6), '(\x00\x15\xb98\x1c+\x88!"0')
        self.assertRaises(TypeError, self.assertResponse, date(1985, 7, 8))
        self.assertRaises(TypeError, self.assertResponse, time(9, 10, 11))

        self.assertResponse(DateTime("1985-01-02 03:04:05"), '(\x00\x15\xb98\x1c+\x88!"0')
        self.assertResponse(LocalTime(473483045), '(\xfc%\xc78\x1c+\x08""0')
        self.assertResponse(UTCTime(473483045), '(\x00%\xc78\x1c+\x08""0')

        self.assertResponse(DateTime(0), "(\xfc\x00\x00\x00\x00\x04\x80\x10B.")
        self.assertResponse(DateTime(1234567890), "(\xfc\xd2\x02\x96I\xf6>\xe0$3")

    def test_binary(self):
        self.assertResponse(Binary(), "0\x00")
        self.assertResponse(Binary("abcd"), "0\x04abcd")
        self.assertResponse(Binary("абвг"), "0\x08\xd0\xb0\xd0\xb1\xd0\xb2\xd0\xb3")
        self.assertResponse(Binary(b"\x01\x02"), "0\x02\x01\x02")
        self.assertResponse(Binary(r"\x01\x02"), "0\x08\\x01\\x02")
        self.assertRaises(TypeError, self.assertResponse, bytearray("абв"))
        self.assertResponse(bytes("абв"), " \x06\xd0\xb0\xd0\xb1\xd0\xb2")  # Not a binary
        self.assertRaises(TypeError, self.assertResponse, BytesIO("абв"))
        # Not a binary as well --v
        self.assertResponse(BytesIO("абв").getvalue(), " \x06\xd0\xb0\xd0\xb1\xd0\xb2")

        # Length tests
        self.assertResponse(Binary("a" * 1024), "1\x00\x04" + "a" * 1024)
        self.assertResponse(Binary("a" * 1024 ** 2), "2\x00\x00\x10" + "a" * 1024 ** 2)
        # Too slow --v
        # self.assertResponse(Binary("a" * 1024 ** 3), "3\x00\x00\x00@" + "a" * 1024 ** 3)

    def test_list_like(self):
        self.assertResponse([], "X\x00")
        self.assertResponse((), "X\x00")

        self.assertRaises(TypeError, self.assertResponse, set())
        self.assertRaises(TypeError, self.assertResponse, frozenset())
        self.assertRaises(TypeError, self.assertResponse, xrange(5))  # Iterators don't work as well

        # Small random tests
        self.assertResponse([1, 2, 3, 4, 5], "X\x058\x018\x028\x038\x048\x05")
        self.assertResponse([True, False, None], "X\x03\x11\x10`")
        self.assertResponse([-.1, .0, 1.],
                            "X\x03\x18\x9a\x99\x99\x99\x99\x99\xb9\xbf\x18\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x18\x00\x00\x00\x00\x00\x00\xf0?")
        self.assertResponse(["a", "bc", "def"], "X\x03 \x01a \x02bc \x03def")

        # Length tests
        self.assertResponse([1] * 1024, "Y\x00\x04" + "8\x01" * 1024)
        self.assertResponse([1] * 1024 ** 2, "Z\x00\x00\x10" + "8\x01" * 1024 ** 2)

        # List in list
        self.assertResponse([[1], [2, 3], [4, 5, 6]],
                            "X\x03X\x018\x01X\x028\x028\x03X\x038\x048\x058\x06")
        self.assertResponse([[[[]], []], [[]], []], "X\x03X\x02X\x01X\x00X\x00X\x01X\x00X\x00")

    def test_dictionary(self):
        self.assertResponse({}, "P\x00")

        self.assertResponse({"a": 1, "b": 2}, "P\x02\x01a8\x01\x01b8\x02")
        self.assertResponse({"ю": 1, "я": 2}, "P\x02\x02\xd1\x8f8\x02\x02\xd1\x8e8\x01")
        self.assertRaises(TypeError, self.assertResponse, {1: "a", 2: "b"})
        self.assertRaises(TypeError, self.assertResponse, {u"эюя": None})

        self.assertResponse({
            "a": False, "b": True, "c": None, "d": 1, "e": -.1, "f": "xyz", "g": u"эюя",
            "h": [1, 2, 3], "i": LocalTime(1234567890)
        }, "P\t\x01a\x10\x01c`\x01b\x11\x01e\x18\x9a\x99\x99\x99\x99\x99\xb9\xbf\x01d8\x01\x01g "
           "\x06\xd1\x8d\xd1\x8e\xd1\x8f\x01f \x03xyz\x01i(\xfc\xd2\x02\x96I\xf6>\xe0$3\x01hX\x038"
           "\x018\x028\x03")

        # Length tests
        self.assertResponse({chr(i): i for i in range(256)},
                            "Q\x00\x01\x01\x008\x00\x01\x838\x83\x01\x048\x04\x01\x878\x87\x01\x088"
                            "\x08\x01\x8b8\x8b\x01\x0c8\x0c\x01\x8f8\x8f\x01\x108\x10\x01\x938\x93"
                            "\x01\x148\x14\x01\x978\x97\x01\x188\x18\x01\x9b8\x9b\x01\x1c8\x1c\x01"
                            "\x9f8\x9f\x01 8 \x01\xa38\xa3\x01$8$\x01\xa78\xa7\x01(8(\x01\xab8\xab"
                            "\x01,8,\x01\xaf8\xaf\x01080\x01\xb38\xb3\x01484\x01\xb78\xb7\x01888"
                            "\x01\xbb8\xbb\x01<8<\x01\xbf8\xbf\x01@8@\x01\xc38\xc3\x01D8D\x01\xc78"
                            "\xc7\x01H8H\x01\xcb8\xcb\x01L8L\x01\xcf8\xcf\x01P8P\x01\xd38\xd3\x01T8"
                            "T\x01\xd78\xd7\x01X8X\x01\xdb8\xdb\x01\\8\\\x01\xdf8\xdf\x01`8`\x01"
                            "\xe38\xe3\x01d8d\x01\xe78\xe7\x01h8h\x01\xeb8\xeb\x01l8l\x01\xef8\xef"
                            "\x01p8p\x01\xf38\xf3\x01t8t\x01\xf78\xf7\x01x8x\x01\xfb8\xfb\x01|8|"
                            "\x01\xff8\xff\x01\x808\x80\x01\x038\x03\x01\x848\x84\x01\x078\x07\x01"
                            "\x888\x88\x01\x0b8\x0b\x01\x8c8\x8c\x01\x0f8\x0f\x01\x908\x90\x01\x138"
                            "\x13\x01\x948\x94\x01\x178\x17\x01\x988\x98\x01\x1b8\x1b\x01\x9c8\x9c"
                            "\x01\x1f8\x1f\x01\xa08\xa0\x01#8#\x01\xa48\xa4\x01\'8\'\x01\xa88\xa8"
                            "\x01+8+\x01\xac8\xac\x01/8/\x01\xb08\xb0\x01383\x01\xb48\xb4\x01787"
                            "\x01\xb88\xb8\x01;8;\x01\xbc8\xbc\x01?8?\x01\xc08\xc0\x01C8C\x01\xc48"
                            "\xc4\x01G8G\x01\xc88\xc8\x01K8K\x01\xcc8\xcc\x01O8O\x01\xd08\xd0\x01S8"
                            "S\x01\xd48\xd4\x01W8W\x01\xd88\xd8\x01[8[\x01\xdc8\xdc\x01_8_\x01\xe08"
                            "\xe0\x01c8c\x01\xe48\xe4\x01g8g\x01\xe88\xe8\x01k8k\x01\xec8\xec\x01o8"
                            "o\x01\xf08\xf0\x01s8s\x01\xf48\xf4\x01w8w\x01\xf88\xf8\x01{8{\x01\xfc8"
                            "\xfc\x01\x7f8\x7f\x01\x818\x81\x01\x028\x02\x01\x858\x85\x01\x068\x06"
                            "\x01\x898\x89\x01\n8\n\x01\x8d8\x8d\x01\x0e8\x0e\x01\x918\x91\x01\x12"
                            "8\x12\x01\x958\x95\x01\x168\x16\x01\x998\x99\x01\x1a8\x1a\x01\x9d8\x9d"
                            "\x01\x1e8\x1e\x01\xa18\xa1\x01\"8\"\x01\xa58\xa5\x01&8&\x01\xa98\xa9"
                            "\x01*8*\x01\xad8\xad\x01.8.\x01\xb18\xb1\x01282\x01\xb58\xb5\x01686"
                            "\x01\xb98\xb9\x01:8:\x01\xbd8\xbd\x01>8>\x01\xc18\xc1\x01B8B\x01\xc58"
                            "\xc5\x01F8F\x01\xc98\xc9\x01J8J\x01\xcd8\xcd\x01N8N\x01\xd18\xd1\x01R8"
                            "R\x01\xd58\xd5\x01V8V\x01\xd98\xd9\x01Z8Z\x01\xdd8\xdd\x01^8^\x01\xe18"
                            "\xe1\x01b8b\x01\xe58\xe5\x01f8f\x01\xe98\xe9\x01j8j\x01\xed8\xed\x01n8"
                            "n\x01\xf18\xf1\x01r8r\x01\xf58\xf5\x01v8v\x01\xf98\xf9\x01z8z\x01\xfd8"
                            "\xfd\x01~8~\x01\x018\x01\x01\x828\x82\x01\x058\x05\x01\x868\x86\x01\t8"
                            "\t\x01\x8a8\x8a\x01\r8\r\x01\x8e8\x8e\x01\x118\x11\x01\x928\x92\x01"
                            "\x158\x15\x01\x968\x96\x01\x198\x19\x01\x9a8\x9a\x01\x1d8\x1d\x01\x9e8"
                            "\x9e\x01!8!\x01\xa28\xa2\x01%8%\x01\xa68\xa6\x01)8)\x01\xaa8\xaa\x01-8"
                            "-\x01\xae8\xae\x01181\x01\xb28\xb2\x01585\x01\xb68\xb6\x01989\x01\xba8"
                            "\xba\x01=8=\x01\xbe8\xbe\x01A8A\x01\xc28\xc2\x01E8E\x01\xc68\xc6\x01I8"
                            "I\x01\xca8\xca\x01M8M\x01\xce8\xce\x01Q8Q\x01\xd28\xd2\x01U8U\x01\xd68"
                            "\xd6\x01Y8Y\x01\xda8\xda\x01]8]\x01\xde8\xde\x01a8a\x01\xe28\xe2\x01e8"
                            "e\x01\xe68\xe6\x01i8i\x01\xea8\xea\x01m8m\x01\xee8\xee\x01q8q\x01\xf28"
                            "\xf2\x01u8u\x01\xf68\xf6\x01y8y\x01\xfa8\xfa\x01}8}\x01\xfe8\xfe")

        try:
            # I can't write whole output here (it's really big).
            dumps(({str(i): None for i in range(256)}, ))
            dumps(({str(i): None for i in range(1024)}, ))
            # dumps(({str(i): None for i in range(1024 ** 2)}, ))  # Too slow
        except Exception as ex:
            self.failureException("Should not raise anything: {!r}".format(ex))

        self.assertRaises(TypeError, self.assertResponse, {"": None})
        self.assertResponse({"a" * 255: None}, "P\x01\xff" + ("a" * 255) + "`")
        self.assertRaises(TypeError, self.assertResponse, {"a" * 256: None})

        # Dict in dict
        self.assertResponse({"a": {"b": {"c": {}}, "d": {}}, "e": {"f": {}}, "g": {}},
                            "P\x03\x01aP\x02\x01bP\x01\x01cP\x00\x01dP\x00\x01eP\x01\x01fP\x00\x01g"
                            "P\x00")

if __name__ == "__main__":
    TestProgram()
