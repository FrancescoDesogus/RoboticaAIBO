/****************************************************************************
** Meta object code from reading C++ file 'robotmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "robotmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'robotmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_RobotManager_t {
    QByteArrayData data[13];
    char stringdata[129];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_RobotManager_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_RobotManager_t qt_meta_stringdata_RobotManager = {
    {
QT_MOC_LITERAL(0, 0, 12),
QT_MOC_LITERAL(1, 13, 10),
QT_MOC_LITERAL(2, 24, 0),
QT_MOC_LITERAL(3, 25, 8),
QT_MOC_LITERAL(4, 34, 5),
QT_MOC_LITERAL(5, 40, 14),
QT_MOC_LITERAL(6, 55, 7),
QT_MOC_LITERAL(7, 63, 7),
QT_MOC_LITERAL(8, 71, 16),
QT_MOC_LITERAL(9, 88, 11),
QT_MOC_LITERAL(10, 100, 9),
QT_MOC_LITERAL(11, 110, 12),
QT_MOC_LITERAL(12, 123, 4)
    },
    "RobotManager\0pointFound\0\0distance\0"
    "angle\0newCameraImage\0cv::Mat\0cvImage\0"
    "victimImageFound\0victimFound\0robotTurn\0"
    "generateStep\0init\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RobotManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   49,    2, 0x06,
       5,    1,   54,    2, 0x06,
       8,    1,   57,    2, 0x06,
       9,    1,   60,    2, 0x06,
      10,    1,   63,    2, 0x06,
      11,    0,   66,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
      12,    0,   67,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, QMetaType::QVariant, QMetaType::QVariant,    3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::QVariant,    3,
    QMetaType::Void, QMetaType::QVariant,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void RobotManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RobotManager *_t = static_cast<RobotManager *>(_o);
        switch (_id) {
        case 0: _t->pointFound((*reinterpret_cast< QVariant(*)>(_a[1])),(*reinterpret_cast< QVariant(*)>(_a[2]))); break;
        case 1: _t->newCameraImage((*reinterpret_cast< cv::Mat(*)>(_a[1]))); break;
        case 2: _t->victimImageFound((*reinterpret_cast< cv::Mat(*)>(_a[1]))); break;
        case 3: _t->victimFound((*reinterpret_cast< QVariant(*)>(_a[1]))); break;
        case 4: _t->robotTurn((*reinterpret_cast< QVariant(*)>(_a[1]))); break;
        case 5: _t->generateStep(); break;
        case 6: _t->init(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (RobotManager::*_t)(QVariant , QVariant );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RobotManager::pointFound)) {
                *result = 0;
            }
        }
        {
            typedef void (RobotManager::*_t)(cv::Mat );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RobotManager::newCameraImage)) {
                *result = 1;
            }
        }
        {
            typedef void (RobotManager::*_t)(cv::Mat );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RobotManager::victimImageFound)) {
                *result = 2;
            }
        }
        {
            typedef void (RobotManager::*_t)(QVariant );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RobotManager::victimFound)) {
                *result = 3;
            }
        }
        {
            typedef void (RobotManager::*_t)(QVariant );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RobotManager::robotTurn)) {
                *result = 4;
            }
        }
        {
            typedef void (RobotManager::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RobotManager::generateStep)) {
                *result = 5;
            }
        }
    }
}

const QMetaObject RobotManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_RobotManager.data,
      qt_meta_data_RobotManager,  qt_static_metacall, 0, 0}
};


const QMetaObject *RobotManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RobotManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RobotManager.stringdata))
        return static_cast<void*>(const_cast< RobotManager*>(this));
    return QObject::qt_metacast(_clname);
}

int RobotManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void RobotManager::pointFound(QVariant _t1, QVariant _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RobotManager::newCameraImage(cv::Mat _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void RobotManager::victimImageFound(cv::Mat _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void RobotManager::victimFound(QVariant _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void RobotManager::robotTurn(QVariant _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void RobotManager::generateStep()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}
QT_END_MOC_NAMESPACE
