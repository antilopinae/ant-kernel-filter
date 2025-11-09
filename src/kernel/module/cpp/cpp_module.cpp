/*
* C++ component inside the Linux kernel
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include "cpp_module.h"
#include "logger.h"

/* Also needed to define NULL as simple 0. It's Ok by standard. */
#define NULL 0

class foo {
public:
    foo()
        : a(0) {
        kern_log("C++ class constructor\n");
    }

    ~foo() {
        kern_log("C++ class destructor\n");
    }

    /* "Normal" virtual function */
    // virtual void set_data(int data) {
    //     kern_log("Don't call me!\n");
    // };

    /* Pure virtual function */
    // virtual int get_data() =0;

protected:
    int a;
};

/* Class bar is inheritor of the class foo
 * Overloading implementation of the class methods
 */
class bar : foo {
public:
    /* Virtual destructor is required */
    ~bar() {
    }

    void set_data(int data) {
        kern_log(">> set_data %d\n", data);
        a = data;
    }

    int get_data() {
        return a;
    }
};

static bar *bar_instance = NULL;

/* This functions can be called from the C code */
void init_cpp_subsystem_example(void) {
    kern_log("Init C++ subsystem\n");

    bar_instance = new bar;

    if (!bar_instance) {
        kern_log("Failed to allocate bar class\n");
        return;
    }

    bar_instance->set_data(42);

    kern_log("Getting data from bar: %d\n", bar_instance->get_data());
}

void release_cpp_subsystem_example(void) {
    kern_log("Release C++ subsystem\n");

    if (bar_instance) {
        delete bar_instance;
    }
}
