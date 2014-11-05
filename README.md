GPUVerify OpenCL Kernel Interceptor
===================================

[![Build Status](https://travis-ci.org/mc-imperial/gvki.svg?branch=master)](https://travis-ci.org/mc-imperial/gvki)

This is a simple wrapper library to intercept OpenCL host code calls to collect
information needed by [GPUVerify](http://multicore.doc.ic.ac.uk/tools/GPUVerify/) so it can verify executed kernels.

It provides two ways of intercepting calls to OpenCL host functions

* A preloadable library. A shared library is built which
  can used with ``LD_PRELOAD`` (Linux) or ``DYLD_INSERT_LIBRARIES`` (OSX) to
  intercept calls of binary applications.

```
# Linux
$ LD_PRELOAD=/path/to/gvki/lib/libGVKI_preload.so ./your_program

# OSX
$ DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_INSERT_LIBRARIES=/path/to/gvki/lib/libGVKI_preload.dylib ./your_program
```

* "Macro library". For systems that do not support pre loadable libraries we also
  provide a header file that can be included in your application to rewrite all
  relevant calls to functions in our interceptor library
  (``lib/libGVKI_macro.a``) which you then must link with afterwards.

```
# Add #include "gvki_macro_header.h" into your source files
# Compile your application linking the interceptor library
$ gcc -I/path/to/gvki_src/include/ your_application.c lib/libGVKI_macro.a -o your_application
```

Building
========

You need CMake to and the OpenCL headers installed.

Linux/OSX
---------

```
$ mkdir gvki
$ git clone git://github.com/mc-imperial/gvki.git src
$ mkdir build
$ cd build
$ cmake -DENABLE_TESTING:BOOL=ON ../src/
$ make
```

Windows
-------

TODO

Testing
=======

Run the test target (``ENABLE_TESTING`` must be set to true when configuring with cmake)

```
$ make check
```

Output produced
===============

When intercepting a ``gvki-<N>`` directory is created where ``<N>``
is the next available integer. The directory contains the following

* ``log.json`` file should which contains information about logged
  executions.

* ``<entry_point>.<M>.cl`` files which are the logged OpenCL kernels
  where ``<entry_point>`` is the name of kernel and <M> is the next
  available integer.

An example invocation of GPUVerify on the logged kernels is

```
$ gpuverify --json gvki-0/log.json
```


Special environment variables
=============================

Setting various environment variables changes its behaviour

* ``GVKI_DEBUG`` setting this causes debug information to be sent to stderr during interception.
