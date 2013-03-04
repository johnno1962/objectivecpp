## Objective-C++ Overview

Objective-C++ is a set of standalone header files which can be included in your project to
extend the syntax of Objective-C®. Started as a means to reference objects in NSArray
containers with a more succinct syntax using the [] operator, the integration of the C++
compiler with Objective-C code is so complete it has grown to include dictionary containers
and string concatenation along with regular expression operators and a few utility classes
used as a shorthand for common operations.

The header file Foundation++.h contain a number of wrapper classes defining inline functions
for certain operators effectively becoming a pre-processor for code. These header files are
typically included in your project's "<Project>-Prefix.pch" precompiled header file so they
are available to all sources in the project. No linking to an external library is required.

An overview is available at [this link](http://objectivecpp.johnholdsworth.com/intro.html).

