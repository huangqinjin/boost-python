// Copyright David Abrahams 2002. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#include <boost/python/class.hpp>
#include <boost/python/module.hpp>
#include <boost/python/returning.hpp>
#include <boost/ref.hpp>
#include <boost/utility.hpp>

using namespace boost::python;

struct X
{
    explicit X(int x) : x(x), magic(7654321) { ++counter; }
    X(X const& rhs) : x(rhs.x), magic(7654321) { ++counter; }
    virtual ~X() { assert(magic == 7654321); magic = 6666666; x = 9999; --counter; }

    void set(int x) { assert(magic == 7654321); this->x = x; }
    int value() const { assert(magic == 7654321); return x; }
    static int count() { return counter; }
 private:
    void operator=(X const&);
 private:
    int x;
    long magic;
    static int counter;
};

struct Y : X
{
    Y(int x) : X(x) {};
};

struct abstract : X
{
    abstract(int x) : X(x) {};
    int call_f(Y const& y) { return f(y); }
    virtual int f(Y const& y) = 0;
};

struct concrete : X
{
    concrete(int x) : X(x) {};
    int call_f(Y const& y) { return f(y); }
    virtual int f(Y const& y) { set(y.value()); return y.value(); }
};

struct abstract_callback : abstract
{
    abstract_callback(PyObject* p, int x)
        : abstract(x), self(p)
    {}

    int f(Y const& y)
    {
        return returning<int>::call_method(self, "f", boost::ref(y));
    }

    PyObject* self;
};

struct concrete_callback : concrete
{
    concrete_callback(PyObject* p, int x)
        : concrete(x), self(p)
    {}

    concrete_callback(PyObject* p, concrete const& x)
        : concrete(x), self(p)
    {}

    int f(Y const& y)
    {
        return returning<int>::call_method(self, "f", boost::ref(y));
    }

    int f_impl(Y const& y)
    {
        return this->concrete::f(y);
    }
    
    PyObject* self;
};

int X::counter;

BOOST_PYTHON_MODULE_INIT(virtual_functions_ext)
{
    module("virtual_functions_ext")
        .add(
            class_<concrete, concrete_callback>("concrete")
            .def_init(args<int>())
            .def("value", &concrete::value)
            .def("set", &concrete::set)
            .def("call_f", &concrete::call_f)
            .def("f", &concrete_callback::f_impl))
        
        .add(
            class_<abstract, boost::noncopyable, boost::shared_ptr<abstract_callback>
            >("abstract")
            
            .def_init(args<int>())
            .def("value", &abstract::value)
            .def("call_f", &abstract::call_f)
            .def("set", &abstract::set))
        
        .add(
            class_<Y>("Y")
            .def_init(args<int>())
            .def("value", &Y::value)
            .def("set", &Y::set)
            )
        ;
}

#include "module_tail.cpp"