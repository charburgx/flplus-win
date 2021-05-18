# FL Plus Windows

This is the windows version of [FL Plus](http://flpl.us). Comes in support for x86 and x64 architectures. 

Source for the web page can be found [here](http://github.com/charburgx/flplus-web)

# Usage

For usage (docs) see the [wiki](https://github.com/charburgx/flplus-win/wiki/)

# Issues

Please make a note of any issues or feature requests on the [issues page](https://github.com/charburgx/flplus-win/issues)

# Building

flplus-win is developed in Visual Studio 2019. It requires the Windows SDK and Google Test for testing. Both can be installed through the Visual Studio installer.

Building and running should be taken care of by the Visual Studio IDE. 

# Testing

To run the tests, select the testing project (flplus-win-tests) and use Test > Run All Tests. FL Plus uses the Google Test platform for testing. Unit Tests specific to a class should be placed in a class of the name [class].test.cpp.
