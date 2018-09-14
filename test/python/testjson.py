from inspect import currentframe, getframeinfo
from pathlib import Path
import sys

filename = getframeinfo(currentframe()).filename
langworthy_root = Path(filename).resolve().parent.parent.parent.parent
python_build_root = langworthy_root / ("_build/Windows/x86/Debug/tool/python/output/build/lib.win-amd64-" + sys.version[0:3])

print(python_build_root)

sys.path.append(str(python_build_root))

import _xlang
import unittest

class TestXlangJson(unittest.TestCase):

    def test_activate_JsonArray(self):
        a = _xlang.JsonArray()
        self.assertEqual(a.Size, 0)
        self.assertEqual(a.ValueType, 4)
        self.assertEqual(a.ToString(), "[]")
        self.assertEqual(a.Stringify(), "[]")


    def test_activate_JsonObject(self):
        o = _xlang.JsonObject()
        self.assertEqual(o.Size, 0)
        self.assertEqual(o.ValueType, 5)
        self.assertEqual(o.ToString(), "{}")
        self.assertEqual(o.Stringify(), "{}")

    def test_cant_activate_JsonError(self):
        with self.assertRaises(RuntimeError):
            e = _xlang.JsonError()

    def test_cant_activate_JsonValue(self):
        with self.assertRaises(RuntimeError):
            v = _xlang.JsonValue()

    def test_JsonArray_parse(self):
        a = _xlang.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        self.assertEqual(a.ValueType, 4)
        self.assertEqual(a.Size, 7)
        self.assertTrue(a.GetBooleanAt(0))
        self.assertFalse(a.GetBooleanAt(1))
        self.assertEqual(a.GetNumberAt(2), 42)
        self.assertEqual(a.GetStringAt(6), "plugh")

        a2 = a.GetArrayAt(4)
        self.assertEqual(a2.Size, 0)
        o = a.GetObjectAt(5)
        self.assertEqual(o.Size, 0)


    def test_JsonValue_boolean(self):
        t = _xlang.JsonValue.CreateBooleanValue(True)
        self.assertEqual(t.ValueType, 1)
        self.assertTrue(t.GetBoolean())

        f = _xlang.JsonValue.CreateBooleanValue(False)
        self.assertEqual(f.ValueType, 1)
        self.assertFalse(f.GetBoolean())

    def test_JsonValue_null(self):
        n = _xlang.JsonValue.CreateNullValue()
        self.assertEqual(n.ValueType, 0)

    def test_JsonValue_number(self):
        t = _xlang.JsonValue.CreateNumberValue(42)
        self.assertEqual(t.ValueType, 2)
        self.assertEqual(t.GetNumber(), 42)

    # def test_JsonArray(self):
    #     a = _xlang.JsonArray()
    #     v = _xlang.JsonValue.CreateNumberValue(3)
    #     a.InsertAt(0, _xlang.JsonValue.CreateNumberValue(3))
    #     a.InsertAt(0, _xlang.JsonValue.CreateNumberValue(2))
    #     a.InsertAt(0, _xlang.JsonValue.CreateNumberValue(1))



    def test_JsonValue_string(self):
        t = _xlang.JsonValue.CreateStringValue("Plugh")
        self.assertEqual(t.ValueType, 3)
        self.assertEqual(t.GetString(), "Plugh")

    def test_JsonValue_parse(self):
        b = _xlang.JsonValue.Parse("true")
        self.assertEqual(b.ValueType, 1)
        self.assertTrue(b.GetBoolean())

        n = _xlang.JsonValue.Parse("16")
        self.assertEqual(n.ValueType, 2)
        self.assertEqual(n.GetNumber(), 16)

        s = _xlang.JsonValue.Parse("\"plugh\"")
        self.assertEqual(s.ValueType, 3)
        self.assertEqual(s.GetString(), "plugh")

    def test_invalid_param_count_instance(self):
        a = _xlang.JsonArray()
        with self.assertRaises(RuntimeError):
            a.Append(10, 20)

    def test_invalid_param_count_static(self):
        with self.assertRaises(RuntimeError):
            _xlang.JsonArray.Parse(10, 20)

if __name__ == '__main__':
    _xlang.init_apartment()
    unittest.main()
    _xlang.uninit_apartment()
