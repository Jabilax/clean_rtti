# Clean RTTI
C++ Reflection Library

Other reflection libraries make heavy use of templates or heavy use of macros and manual logging
which is not ideal.

My approach is to have a simple parser/executable that will fastly parse the code and generate
the RTTI headers for it. It won´t parse and log everything but just the stuff you would care about.
Like the variables in a struct, the enums etc.

It is early work in progress.
