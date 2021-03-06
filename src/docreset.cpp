#include <pybind11/pybind11.h>
#include <iostream>

using namespace std;

#define STRINGIFY(x) #x

int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;

py::object ReplaceDoc(py::object f, const std::string& doc_string) {
  static std::vector<std::string> all_doc_strings;
  all_doc_strings.emplace_back(doc_string);
  const char* doc_str = all_doc_strings.back().c_str();
  PyObject* obj = f.ptr();
  if (PyCFunction_Check(obj)) {
    auto* f = (PyCFunctionObject*)obj;
    if (!f->m_ml->ml_doc) {
      cout << "function " << f->m_ml->ml_name << " has not a docstring yet.";
      return py::object();
    }
    f->m_ml->ml_doc = doc_str;
  } else if (PyFunction_Check(obj)) {
    auto* f = (PyFunctionObject*)obj;
    if (f->func_doc == Py_None) {
      cout << "function "
                          << PyBytes_AsString(
                                 PyUnicode_AsEncodedString(f->func_name, "utf-8", "~E~"))
                          << " has not a docstring yet.";
      return py::object();
    }
    Py_DECREF(f->func_doc);
    f->func_doc = PyUnicode_FromString(doc_str);
  } else {
    cout << "function is " << Py_TYPE(obj)->tp_name << ", not a valid function.";
    return py::object();
  }
  f.inc_ref();
  return f;
}

PYBIND11_MODULE(_docreset, m) {
    m.def("_reset_docstr", ReplaceDoc);

#ifdef VERSION_INFO
    m.attr("__version__") = STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
