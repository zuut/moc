# Sample C++ Test App

This requires moc being installed. 

## Building

If MOC isn't installed in one of the standard system
directories, the CMAKE_INSTALL_PREFIX below should be 
set to the directory where MOC was installed. 

To build
```
mkdir cmake-build-debug
cd cmake-build-debug
cmake .. -DCMAKE_INSTALL_PREFIX=../../../installed/share
cmake --build . 
```

## Running
This app does nothing. Its merely to demostrate the
cmake function for running the moc compiler to generate
source files based on the test_app.moc input file and
the test_app.tpl template.
