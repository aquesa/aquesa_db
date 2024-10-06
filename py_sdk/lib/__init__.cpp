#include "__init__.h"

static int PySequence_init(PySequence *self, PyObject *args) {

  try {

    string seqError = "Exception, unable to create sequence at this moment";

    if(!PyTuple_Check(args))
      throw std::runtime_error(seqError);

    auto args_size = PyTuple_Size(args);

    if(args_size != 3)
      throw std::runtime_error("ArgsCountException: Database.sequence method takes only two arguments (provided "
         + std::to_string(args_size) + ")");

    PyObject *py_db = PyTuple_GetItem(args, 0);
    PyObject *py_name = PyTuple_GetItem(args, 1);
    PyObject *py_is_new = PyTuple_GetItem(args, 2);

    if(!PyObject_TypeCheck(py_db, &PyDatabaseType))
      throw std::runtime_error(seqError);

     auto py_seq = (PySequence*) ((PyDatabase*) py_db)->db->sequence(py_name, py_is_new);

     self->seq = py_seq->seq;

    return 0;

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    exit(-1);
  }
}

static PyObject* PySequence_insertNewBlock(PySequence *self, PyObject *args) {
  try {

    string insertError = "Exception, unable to insert the block at this moment.";

    if (!self->seq)
      throw std::runtime_error("Seq not found.");

    if (!PyTuple_Check(args))
      throw std::runtime_error("Args not tuple.");

    auto argsSize = PyTuple_Size(args);
    if(argsSize != 2)
      throw std::runtime_error("ArgsCountException: insertNewBlock takes exactly 2 arguments (provided "
        + std::to_string(argsSize) + ")");

    self->seq->insertNewBlock(PyTuple_GetItem(args, 0), PyTuple_GetItem(args, 1));

    Py_RETURN_NONE;

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    exit(-1);
  }
}

static PyObject* PySequence_getBlock(PySequence *self, PyObject *args) {
  try {

    string getError = "Exception, unable to retrieve the block at this moment";

    if(!self->seq)
      throw std::runtime_error(getError);

    if(!PyTuple_Check(args))
      throw std::runtime_error(getError);

    auto argsSize = PyTuple_Size(args);
    if(argsSize != 1)
      throw std::runtime_error("ArgsCountException: getBlock takes exactly 1 argument (provided "
        + std::to_string(argsSize) + ")");

    return self->seq->getBlock(PyTuple_GetItem(args, 0));

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    exit(-1);
  }
}

static void PySequence_dealloc(PySequence* self) {
  Py_TYPE(self)->tp_free((PyObject*) self);
}

static int PyDatabase_init(PyDatabase *self, PyObject *py_name) {
  self->db = std::make_shared<Database>(py_name);
  return 0;
}

static PyObject* PyDatabase_sequence(PyDatabase *self, PyObject *args) {

  try {

    string seqError = "Exception, unable to create sequence at this moment";

    if(!self->db)
      throw std::runtime_error(seqError);

    PyObject *seqClass = PyObject_GetAttrString(
      PyImport_Import(PyUnicode_FromString("py_aquesa_db")),
      "Sequence"
    );

    if(!seqClass || !(PyCallable_Check(seqClass)))
      throw std::runtime_error(seqError);

    if(!PyTuple_Check(args))
      throw std::runtime_error(seqError);

    auto argsCount = PyTuple_Size(args);
    if(argsCount != 2)
      throw std::runtime_error("ArgsCountException: sequence method takes exactly 2 arguments (provided "
        + std::to_string(argsCount) + ")");

    PyObject *seqInstance = PyObject_CallObject(seqClass, PyTuple_Pack(3, self, PyTuple_GetItem(args, 0), PyTuple_GetItem(args, 1)));
    if(!seqInstance)
      throw std::runtime_error(seqError);

    return seqInstance;

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    exit(-1);
  }
}

static void PyDatabase_dealloc(PyDatabase* self) {
  Py_TYPE(self)->tp_free((PyObject*) self);
}

void Sequence::insertNewBlock(PyObject *py_id, PyObject *py_bytearray) {
  try {
    string insertError = "Exception, unable to insert the block at this moment.";

    if (!PyUnicode_Check(py_id))
      throw std::runtime_error("TypeError: Database name should be valid python string.");
    auto c_id = PyUnicode_AsUTF8(py_id);
    if(!c_id)
      throw std::runtime_error(insertError);

    if(!PyByteArray_Check(py_bytearray))
      throw std::runtime_error("TypeError: Data must be valid python bytes array.");
    auto py_bytearray_size = PyByteArray_Size(py_bytearray);
    auto dataBytes = std::make_shared<std::byte[]>(4);
    std::memcpy(dataBytes.get(), PyByteArray_AsString(py_bytearray), py_bytearray_size);

    auto block = std::make_shared<ad_structures::Block>();
    strcpy(block->hash, ad_crypto_handler::CryptoHandler::generateSHA256(
    string(PyByteArray_AsString(py_bytearray))
    ).c_str());
    block->dataSize = py_bytearray_size;
    block->data = dataBytes;

    _seq->insertNewBlock(string(c_id), block);

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    exit(-1);
  }
}

PyObject* Sequence::getBlock(PyObject *py_id) {
  try {
    string insertError = "Exception, unable to get the block at this moment.";

    if (!PyUnicode_Check(py_id))
      throw std::runtime_error("TypeError: Database name should be valid python string.");
    auto c_id = PyUnicode_AsUTF8(py_id);
    if(!c_id)
      throw std::runtime_error(insertError);

    auto block = _seq->getBlock(string(c_id));

    auto block_tuple = PyTuple_New(4);
    if(!block_tuple)
      throw std::runtime_error(insertError);
    PyTuple_SetItem(block_tuple, 0, PyUnicode_FromString(block->prevHash));
    PyTuple_SetItem(block_tuple, 1, PyUnicode_FromString(block->hash));
    PyTuple_SetItem(block_tuple, 2, PyLong_FromLong(static_cast<long>(block->dataSize)));
    PyTuple_SetItem(block_tuple, 3, PyByteArray_FromStringAndSize(
       reinterpret_cast<char*>(block->data.get()),
       block->dataSize
    ));

    return block_tuple;

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    exit(-1);
  }
}


Database::Database(PyObject *py_name) {
  try {
    string dbError = "Exception, unable to initialize database at this moment.";

    char *c_name;

    if(!PyArg_ParseTuple(py_name, "s", &c_name))
      throw std::runtime_error(dbError);

    _db = std::make_shared<aqua_db::ad_db>(string(c_name));

  } catch(std::runtime_error &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    exit(-1);
  }
}

PySequence* Database::sequence(PyObject *py_name, PyObject *py_is_new) {

  try {
    string seqError = "Exception, unable to initialize sequence at this moment.";

    if (!PyUnicode_Check(py_name))
      throw std::runtime_error("TypeError: Sequence name should be valid python string.");
    auto c_name = PyUnicode_AsUTF8(py_name);
    if (!c_name)
      throw std::runtime_error(seqError);

    if (!PyBool_Check(py_is_new))
      throw std::runtime_error(seqError);
    auto c_is_new = (PyObject_IsTrue(py_is_new) == 1);

    auto py_seq = new PySequence();
    py_seq->seq = std::make_shared<Sequence>(_db->sequence(string(c_name), c_is_new));

    return py_seq;

  } catch(std::runtime_error &e) {
    cerr << e.what() << endl;
    exit(-1);
  }
}

static PyMethodDef PyDatabase_Methods[] = {
    {"sequence", (PyCFunction) PyDatabase_sequence, METH_VARARGS, "Create new sequence inside this database"},
    {NULL}
};

static PyMethodDef PySequence_Methods[] = {
    {"insertNewBlock", (PyCFunction) PySequence_insertNewBlock, METH_VARARGS, "Inserting a new block into this sequence"},
    {"getBlock", (PyCFunction) PySequence_getBlock, METH_VARARGS, "Retrieving a block using it's ID from this sequence"},
    {NULL}
};

PyTypeObject PyDatabaseType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "py_aquesa_database.Database",                  // tp_name
    sizeof(PyDatabase),                // tp_basicsize
    0,                                   // tp_itemsize
    (destructor) PyDatabase_dealloc,     // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    0,                                   // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    "Database class of py_aquesa_database",                    // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    PyDatabase_Methods,                      // tp_methods
    0,                                   // tp_members
    0,                                   // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    (initproc)PyDatabase_init,           // tp_init (constructor)
    0,                                   // tp_alloc
    PyType_GenericNew,                  // tp_new
};

PyTypeObject PySequenceType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "py_aquesa_database.Sequence",        // tp_name
  sizeof(PySequence),                  // tp_basicsize
  0,                                   // tp_itemsize
  (destructor) PySequence_dealloc,     // tp_dealloc
  0,                                   // tp_print
  0,                                   // tp_getattr
  0,                                   // tp_setattr
  0,                                   // tp_compare
  0,                                   // tp_repr
  0,                                   // tp_as_number
  0,                                   // tp_as_sequence
  0,                                   // tp_as_mapping
  0,                                   // tp_hash
  0,                                   // tp_call
  0,                                   // tp_str
  0,                                   // tp_getattro
  0,                                   // tp_setattro
  0,                                   // tp_as_buffer
  Py_TPFLAGS_DEFAULT,                 // tp_flags
  "Sequence class of py_aquesa_database",                    // tp_doc
  0,                                   // tp_traverse
  0,                                   // tp_clear
  0,                                   // tp_richcompare
  0,                                   // tp_weaklistoffset
  0,                                   // tp_iter
  0,                                   // tp_iternext
  PySequence_Methods,                  // tp_methods
  0,                                   // tp_members
  0,                                   // tp_getset
  0,                                   // tp_base
  0,                                   // tp_dict
  0,                                   // tp_descr_get
  0,                                   // tp_descr_set
  0,                                   // tp_dictoffset
  (initproc)PySequence_init,           // tp_init (constructor)
  0,                                   // tp_alloc
  PyType_GenericNew,                  // tp_new
};

static struct PyModuleDef py_aquesa_db_module = {
    PyModuleDef_HEAD_INIT,
    "py_aquesa_db",
    "Official python module to interact with Aquesa DB",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit_py_aquesa_db(void) {

  PyObject *m;

  if(PyType_Ready(&PyDatabaseType) < 0)
    return NULL;

  if(PyType_Ready(&PySequenceType) < 0)
    return NULL;

  m = PyModule_Create(&py_aquesa_db_module);

  if(!m)
    return NULL;

  Py_INCREF(&PyDatabaseType);
  Py_INCREF(&PySequenceType);

  PyModule_AddObject(m, "Database", (PyObject*)&PyDatabaseType);
  PyModule_AddObject(m, "Sequence", (PyObject*)&PySequenceType);

  return m;
}