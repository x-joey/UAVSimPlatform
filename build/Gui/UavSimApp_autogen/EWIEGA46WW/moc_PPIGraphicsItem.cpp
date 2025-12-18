/****************************************************************************
** Meta object code from reading C++ file 'PPIGraphicsItem.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../Gui/PPIGraphicsItem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PPIGraphicsItem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PPIGraphicsItem_t {
    QByteArrayData data[12];
    char stringdata0[170];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PPIGraphicsItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PPIGraphicsItem_t qt_meta_stringdata_PPIGraphicsItem = {
    {
QT_MOC_LITERAL(0, 0, 15), // "PPIGraphicsItem"
QT_MOC_LITERAL(1, 16, 13), // "targetClicked"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 8), // "targetId"
QT_MOC_LITERAL(4, 40, 19), // "targetDoubleClicked"
QT_MOC_LITERAL(5, 60, 18), // "targetFocusToggled"
QT_MOC_LITERAL(6, 79, 7), // "focused"
QT_MOC_LITERAL(7, 87, 21), // "targetGuidanceToggled"
QT_MOC_LITERAL(8, 109, 7), // "guiding"
QT_MOC_LITERAL(9, 117, 13), // "onUpdateTimer"
QT_MOC_LITERAL(10, 131, 17), // "toggleTargetFocus"
QT_MOC_LITERAL(11, 149, 20) // "toggleTargetGuidance"

    },
    "PPIGraphicsItem\0targetClicked\0\0targetId\0"
    "targetDoubleClicked\0targetFocusToggled\0"
    "focused\0targetGuidanceToggled\0guiding\0"
    "onUpdateTimer\0toggleTargetFocus\0"
    "toggleTargetGuidance"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PPIGraphicsItem[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    1,   52,    2, 0x06 /* Public */,
       5,    2,   55,    2, 0x06 /* Public */,
       7,    2,   60,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   65,    2, 0x08 /* Private */,
      10,    1,   66,    2, 0x0a /* Public */,
      11,    1,   69,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void PPIGraphicsItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PPIGraphicsItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->targetClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->targetDoubleClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->targetFocusToggled((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->targetGuidanceToggled((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->onUpdateTimer(); break;
        case 5: _t->toggleTargetFocus((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->toggleTargetGuidance((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PPIGraphicsItem::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PPIGraphicsItem::targetClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PPIGraphicsItem::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PPIGraphicsItem::targetDoubleClicked)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PPIGraphicsItem::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PPIGraphicsItem::targetFocusToggled)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PPIGraphicsItem::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PPIGraphicsItem::targetGuidanceToggled)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PPIGraphicsItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_PPIGraphicsItem.data,
    qt_meta_data_PPIGraphicsItem,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PPIGraphicsItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PPIGraphicsItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PPIGraphicsItem.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(this);
    if (!strcmp(_clname, "org.qt-project.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(this);
    return QObject::qt_metacast(_clname);
}

int PPIGraphicsItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PPIGraphicsItem::targetClicked(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PPIGraphicsItem::targetDoubleClicked(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PPIGraphicsItem::targetFocusToggled(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PPIGraphicsItem::targetGuidanceToggled(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
