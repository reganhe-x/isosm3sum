# isosm3sum

Utilities for working with sm3sum implanted in ISO images

isosm3sum provides a way of making use of the ISO9660 application data
area to store sm3sum data about the iso. This allows you to check the
iso given nothing more than the iso itself.

<span style="color:red">Software implementation based on </span>**[isomd5sum](https://github.com/rhinstaller/isomd5sum).**

## Build

Use the following command to compile:

```bash
cmake -G 'Unix Makefiles' -B build .
make -j$(nproc) -C build
```

## Installation

Use the following command to install the compilation result into /path/to/you/dir:

```bash
make -C build DESTDIR=/path/to/you/dir
```

## Build rpm

Create the src.rpm with the following command:

```bash
make srpm -C build
```

You will see from the screen output, the path where the src.rpm file is stored:

```log
Source RPM: /path/to/isosm3sum/build/SRPMS/isosm3sum-0.9.0-1.src.rpm
```

Now you can use the rpmbuild command to build out the rpm packages:

```bash
rpmbuild --rebuild /path/to/isosm3sum/build/SRPMS/isosm3sum-0.9.0-1.src.rpm
```

If you want to execute unit tests during the build process, add the parameter "--with tests":

```bash
rpmbuild --rebuild /path/to/isosm3sum/build/SRPMS/isosm3sum-0.9.0-1.src.rpm --with tests
```
