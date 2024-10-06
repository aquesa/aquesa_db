#pragma once

#ifndef PY_AQUESA_DB___INIT___H
#define PY_AQUESA_DB___INIT___H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <aquesa_db.h>

extern PyTypeObject PyDatabaseType, PySequenceType;

typedef struct {
  PyObject_HEAD;
  std::shared_ptr<class Database> db;
} PyDatabase;

typedef struct {
  PyObject_HEAD;
  std::shared_ptr<class Sequence> seq;
} PySequence;

class Sequence {
protected:
  std::shared_ptr<aqua_db::ad_seq> _seq;
public:
  Sequence(std::shared_ptr<aqua_db::ad_seq> seq) : _seq(seq) {};
  void insertNewBlock(PyObject*, PyObject*);
  PyObject* getBlock(PyObject*);
};

class Database {
protected:
  std::shared_ptr<aqua_db::ad_db> _db;
public:
  Database(PyObject*);
  PySequence* sequence(PyObject*, PyObject*);
};

static int PySequence_init(PySequence*, PyObject*);

static PyObject* PySequence_insertNewBlock(PySequence*, PyObject*);

static PyObject* PySequence_getBlock(PySequence*, PyObject*);

static int PyDatabase_init(PyDatabase*, PyObject*);

static PyObject* PyDatabase_sequence(PyDatabase*, PyObject*);

#endif //PY_AQUESA_DB___INIT___H
