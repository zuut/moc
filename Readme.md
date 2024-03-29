# Markedup Object Compiler
The documentation for MOC is found here [Intro](doc/Intro.md) .
This project contains:

    - moc - the source code for the marked-up object compiler
    - uf - a pre standard library C++ utility library used by moc
    - doc - documentation see [Intro](doc/Intro.md).
    - test_app - A sample test application illustrating using the moc cmake function to generate source files for the project. This project does nothing other than the generator being run.

# Building

## Required Packages

MOC requires bison and flex to be installed on your system prior to
building the executable.

### Ubuntu

The script ubuntuSetup will install the needed packages on an Ubuntu
system while the build.sh script will run cmake and then build the app.

```
   $ cd $PROJECT_ROOT/moc
   $ ./ubuntuSetup.sh
   $ ./build.sh -j {num-processors}
   
```

# Testing

The uf library has unit tests in each file that may be run as a standalone
test.

To execute the built moc app against some existing templates, do the following

```
   $ cd $PROJECT_ROOT/moc
   # to execute the tests
   $ ./test.sh 
```
