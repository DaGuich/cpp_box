[![Build Status](https://travis-ci.org/lefticus/cpp_box.svg?branch=master)](https://travis-ci.org/lefticus/cpp_box)
[![codecov](https://codecov.io/gh/lefticus/cpp_box/branch/master/graph/badge.svg)](https://codecov.io/gh/lefticus/cpp_box)
[![CircleCI master](https://img.shields.io/circleci/project/github/lefticus/cpp_box/master.svg)](https://circleci.com/gh/lefticus/cpp_box)
[![CodeFactor](https://www.codefactor.io/repository/github/lefticus/cpp_box/badge)](https://www.codefactor.io/repository/github/lefticus/cpp_box)

# cpp_box

It implements a partial ARMv4 architecture in software.

For code this can accept, use gcc or clang in `-march=armv4` mode. Almost all builds of clang but default support `--target=armv4-linux` regardless of your host platform.

## Getting Started
### Prerequisites

To compile the code, it requires at least C++17 to work. This includes std::filesystem which is not until GCC 8.

In this project Conan is used for package management. Conan is a portable package manager for C/C++ libraries.
It is used to cache all the dependencies needed to build into local directories, without needing to install system packages.

For installation methods of Conan, visit this [link](https://docs.conan.io/en/latest/installation.html). The step-by-step instructions to enable you to build with conan is provided in Conan's website. But in case you would like to see a live demo you can check out [this video](https://youtu.be/9cCQHJ-cNHY).

To build the project with conan, you will also need to have CMake with minimum version of 3.8 to support `cxx_std_17`.

### Building
To set the compiler and the compiler version in Conan, edit the `~/.conan/profiles/default` file if necessary.

Then in the project directory:

```
mkdir build && cd build
```

Let the Conan cache all the necessary packages by using this command:

```
conan install ..
```

This command will read the `conanfile.txt` and install the required packages.

After successfully passing all the steps, issue the `cmake` command to build the project automagically.

```
cmake ..
```

## Running the tests
After successfully finishing build process, run test to see if everything is work.
To do so, simply go to `build/bin` folder and run the tests.


```
$ ./constexpr_tests
All tests passed (47 assertions in 21 test cases)
```

```
$ ./relaxed_constexpr_tests
All tests passed (47 assertions in 21 test cases)
```

## Built With

* [Conan](https://conan.io/) - The C/C++ Package Manager
* [CMake](https://cmake.org/) - Cross-platform build system
* [gcc](https://gcc.gnu.org/) - The GNU Compiler Collection

## About Me

My name is Jason Turner, I'm a C++ programmer, trainer and speaker, available for code reviews and on-site training events.

 * If you are interested in my services for training and contracting: http://emptycrate.com/idocpp
 * Also check out my YouTube channel, C++ Weekly: https://www.youtube.com/c/JasonTurner-lefticus
 * I'm co-host of CppCast: http://cppcast.com

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
