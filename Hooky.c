// Hooky
// Dimitriy "ay0ks" <ay0o0ks000@gmail.com>
// See license in LICENSE

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <malloc.h>
#include <Python.h>

#define _HOOKY_AUTHOR  "Dymitr \"ay0ks\" <dimitriy@darkcat.cloud>"
#define _HOOKY_VERSION "hooky 1.0.0 (2023-11-05)"
#define _HOOKY_DOC     "Hooky internal implementation"

#define _Hooky_PyObject_Print(...) PyObject_Print(PyUnicode_FromFormat(__VA_ARGS__), stdout, Py_PRINT_RAW)
#define _Hooky_PyObject_Repr(_Object) _Hooky_PyObject_Print("%R\n", _Object)

#define _Hooky_PyTypeObject_SetSlot(_Type, _Slot, _Value) ((PyTypeObject*)_Type)->_Slot = (_Value)

static PyObject* _Hooky_Module;
static PyObject* _Hooky_Internal_PatchedTypes;

static PyObject* _Hooky_PyType_GetDict(PyTypeObject* type) {
    return type->tp_dict;
}

static inline bool _Hooky_PyUnicode_RichCompare_Eq(PyObject* left, PyObject* right) {
    return PyUnicode_RichCompare(left, right, Py_EQ) == Py_True;
}

static inline bool _Hooky_PyUnicode_RichCompare_EqString(PyObject* left, const char* right) {
    return PyUnicode_RichCompare(left, PyUnicode_FromString(right), Py_EQ) == Py_True;
}

static inline bool _Hooky_PyProperty_Check(PyObject* object) {
    if (!PyFunction_Check(object)) {
        if (PyObject_HasAttrString(object, "fget") || PyObject_HasAttrString(object, "fset") ||
            PyObject_HasAttrString(object, "fdel")) {
            return true;
        }
    }
    return false;
}

static void _Hooky_Internal_Hook(PyObject* type, PyObject* type_name, PyObject* type_dictionary, PyObject* item, PyObject* item_name) {
    // PyObject* patched_dictionary = PyDict_GetItem(_Hooky_Internal_PatchedTypes, type_name);
    // PyDict_SetItem(patched_dictionary, item_name, PyDict_GetItem(type_dictionary, item_name));
    PyDict_SetItem(type_dictionary, item_name, item);
    // PyDict_SetItem(_Hooky_Internal_PatchedTypes, type_name, patched_dictionary);
    // _Hooky_PyObject_Print("Hooking %R to %R in %R\n", item_name, item, type_name);
}

static void _Hooky_Internal_Unhook(PyObject* type, PyObject* type_name, PyObject* type_dictionary) {
    if (PyDict_Contains(_Hooky_Internal_PatchedTypes, type_name)) {
        // _Hooky_PyObject_Print("Patched dict: %R\n", _Hooky_Internal_PatchedTypes);
        PyObject* patched_dict = PyDict_GetItem(_Hooky_Internal_PatchedTypes, type_name);
        PyObject *member_name, *member;
        Py_ssize_t position = 0;
        while (PyDict_Next(patched_dict, &position, &member_name, &member)) {
            PyDict_SetItem(type_dictionary, member_name, member);
            // _Hooky_PyObject_Print("Unhooking %R back to %R in %R\n", member_name, member, type_name);
        }
    }
}

static PyObject* _Hooky_Hook(PyObject* self, PyObject* args) {
    PyObject* hook_target;
    PyObject* hook_target_name;
    PyObject* hook_function;
    PyObject* hook_function_name;

    if (!PyArg_ParseTuple(args, "OOOO", &hook_target, &hook_target_name, &hook_function, &hook_function_name)) {
        PyErr_SetString(PyExc_TypeError, "Couldn't parse the arguments");
        return NULL;
    }

    if (PyType_Check(hook_target)) {
        PyObject* hook_target_dict = _Hooky_PyType_GetDict((PyTypeObject*)hook_target);
        if (PyFunction_Check(hook_function) && !PyType_Check(hook_function)) {
            _Hooky_Internal_Hook(hook_target, hook_target_name, hook_target_dict, hook_function, hook_function_name);
        } else if (PyType_Check(hook_function)) {
            PyObject* class_dict = _Hooky_PyType_GetDict((PyTypeObject*)hook_function);
            PyObject *member_name, *member;
            Py_ssize_t position = 0;
            while (PyDict_Next(class_dict, &position, &member_name, &member)) {
                _Hooky_Internal_Hook(hook_target, hook_target_name, hook_target_dict, member, member_name);
            }
        }
    }

    Py_RETURN_NONE;
}

static PyObject* _Hooky_HookMember(PyObject* self, PyObject* args) {
    PyObject* hook_target;
    PyObject* hook_target_name;
    PyObject* hook_member;
    PyObject* hook_member_name;

    if (!PyArg_ParseTuple(args, "OOOO", &hook_target, &hook_target_name, &hook_member, &hook_member_name)) {
        PyErr_SetString(PyExc_TypeError, "Couldn't parse the arguments");
        return NULL;
    }

    if (PyType_Check(hook_target)) {
        _Hooky_Internal_Hook(hook_target, hook_target_name, _Hooky_PyType_GetDict((PyTypeObject*)hook_target), hook_member, hook_member_name);
    }

    Py_RETURN_NONE;
}

static PyObject* _Hooky_Unhook(PyObject* self, PyObject* args) {
    PyObject* hook_target;
    PyObject* hook_target_name;

    if (!PyArg_ParseTuple(args, "OO", &hook_target, &hook_target_name)) {
        PyErr_SetString(PyExc_TypeError, "Couldn't parse the arguments");
        return NULL;
    }

    if (PyType_Check(hook_target)) {
        _Hooky_Internal_Unhook(hook_target, hook_target_name, _Hooky_PyType_GetDict((PyTypeObject*)hook_target));
    }

    Py_RETURN_NONE;
}

static PyMethodDef _Hooky_Methods[] = {
    { "_hook",          _Hooky_Hook,           METH_VARARGS, ""   },
    { "_hook_member",   _Hooky_HookMember,     METH_VARARGS, ""   }, 
    { "_unhook",        _Hooky_Unhook,         METH_VARARGS, ""   },
    { NULL,             NULL,                  0,            NULL }
};

static struct PyModuleDef _Hooky_ModuleDef = {
    PyModuleDef_HEAD_INIT,
    "_hooky",
    _HOOKY_VERSION " by " _HOOKY_AUTHOR ".\n" _HOOKY_DOC,
    -1,
    _Hooky_Methods
};

PyMODINIT_FUNC PyInit__hooky(void) {
    _Hooky_Internal_PatchedTypes = PyDict_New();
    _Hooky_Module = PyModule_Create(&_Hooky_ModuleDef);

    return _Hooky_Module;
}
