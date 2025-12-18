/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../Gui/MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[16];
    char stringdata0[224];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 17), // "updatePathTimeout"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 16), // "toggleSimulation"
QT_MOC_LITERAL(4, 47, 12), // "onUavClicked"
QT_MOC_LITERAL(5, 60, 5), // "uavId"
QT_MOC_LITERAL(6, 66, 20), // "onTargetFocusToggled"
QT_MOC_LITERAL(7, 87, 8), // "targetId"
QT_MOC_LITERAL(8, 96, 7), // "focused"
QT_MOC_LITERAL(9, 104, 21), // "onTargetDoubleClicked"
QT_MOC_LITERAL(10, 126, 23), // "onTargetGuidanceToggled"
QT_MOC_LITERAL(11, 150, 7), // "guiding"
QT_MOC_LITERAL(12, 158, 17), // "switchDisplayMode"
QT_MOC_LITERAL(13, 176, 13), // "togglePPIDrag"
QT_MOC_LITERAL(14, 190, 20), // "onTableHeaderClicked"
QT_MOC_LITERAL(15, 211, 12) // "logicalIndex"

    },
    "MainWindow\0updatePathTimeout\0\0"
    "toggleSimulation\0onUavClicked\0uavId\0"
    "onTargetFocusToggled\0targetId\0focused\0"
    "onTargetDoubleClicked\0onTargetGuidanceToggled\0"
    "guiding\0switchDisplayMode\0togglePPIDrag\0"
    "onTableHeaderClicked\0logicalIndex"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x0a /* Public */,
       3,    0,   60,    2, 0x0a /* Public */,
       4,    1,   61,    2, 0x0a /* Public */,
       6,    2,   64,    2, 0x0a /* Public */,
       9,    1,   69,    2, 0x0a /* Public */,
      10,    2,   72,    2, 0x0a /* Public */,
      12,    0,   77,    2, 0x0a /* Public */,
      13,    0,   78,    2, 0x0a /* Public */,
      14,    1,   79,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    7,    8,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    7,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updatePathTimeout(); break;
        case 1: _t->toggleSimulation(); break;
        case 2: _t->onUavClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onTargetFocusToggled((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->onTargetDoubleClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onTargetGuidanceToggled((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->switchDisplayMode(); break;
        case 7: _t->togglePPIDrag(); break;
        case 8: _t->onTableHeaderClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
