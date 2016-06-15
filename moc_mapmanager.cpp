/****************************************************************************
** Meta object code from reading C++ file 'mapmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mapmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mapmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MapManager_t {
    QByteArrayData data[10];
    char stringdata[95];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_MapManager_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_MapManager_t qt_meta_stringdata_MapManager = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 10),
QT_MOC_LITERAL(2, 22, 0),
QT_MOC_LITERAL(3, 23, 8),
QT_MOC_LITERAL(4, 32, 5),
QT_MOC_LITERAL(5, 38, 17),
QT_MOC_LITERAL(6, 56, 8),
QT_MOC_LITERAL(7, 65, 8),
QT_MOC_LITERAL(8, 74, 4),
QT_MOC_LITERAL(9, 79, 14)
    },
    "MapManager\0pointFound\0\0distance\0angle\0"
    "robotStateChanged\0newState\0newAngle\0"
    "init\0terminateRobot\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MapManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06,
       5,    2,   39,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
       8,    0,   44,    2, 0x0a,
       9,    0,   45,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, QMetaType::QVariant, QMetaType::QVariant,    3,    4,
    QMetaType::Void, QMetaType::QVariant, QMetaType::QVariant,    6,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MapManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MapManager *_t = static_cast<MapManager *>(_o);
        switch (_id) {
        case 0: _t->pointFound((*reinterpret_cast< QVariant(*)>(_a[1])),(*reinterpret_cast< QVariant(*)>(_a[2]))); break;
        case 1: _t->robotStateChanged((*reinterpret_cast< QVariant(*)>(_a[1])),(*reinterpret_cast< QVariant(*)>(_a[2]))); break;
        case 2: _t->init(); break;
        case 3: _t->terminateRobot(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (MapManager::*_t)(QVariant , QVariant );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MapManager::pointFound)) {
                *result = 0;
            }
        }
        {
            typedef void (MapManager::*_t)(QVariant , QVariant );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MapManager::robotStateChanged)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject MapManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MapManager.data,
      qt_meta_data_MapManager,  qt_static_metacall, 0, 0}
};


const QMetaObject *MapManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MapManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MapManager.stringdata))
        return static_cast<void*>(const_cast< MapManager*>(this));
    return QObject::qt_metacast(_clname);
}

int MapManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void MapManager::pointFound(QVariant _t1, QVariant _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MapManager::robotStateChanged(QVariant _t1, QVariant _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
