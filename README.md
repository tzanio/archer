<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#org87abbda">1. License</a></li>
<li><a href="#orgc9d4cd6">2. Introduction</a></li>
<li><a href="#org069b511">3. Prerequisites</a></li>
<li><a href="#org674691d">4. Installation</a>
<ul>
<li><a href="#org78a3a2d">4.1. Automatic Building</a></li>
<li><a href="#orgcbf0d1a">4.2. Manual Building</a></li>
<li><a href="#orgd54219f">4.3. Stand-alone building with official LLVM OpenMP Runtime and ThreadSanitizer support</a></li>
<li><a href="#org25da2b9">4.4. Stand-alone building with LLVM OpenMP Runtime and ThreadSanitizer OMPT Support</a></li>
<li><a href="#org99304b9">4.5. Build ARCHER within Clang/LLVM</a></li>
</ul>
</li>
<li><a href="#org74896dc">5. Usage</a>
<ul>
<li><a href="#org51459d2">5.1. How to compile</a>
<ul>
<li><a href="#org0a75e90">5.1.1. Single source</a></li>
<li><a href="#org21be94b">5.1.2. Makefile</a></li>
<li><a href="#org7d7f8ce">5.1.3. Hybrid MPI-OpenMP programs</a></li>
</ul>
</li>
<li><a href="#org3ab76c1">5.2. Options</a></li>
<li><a href="#org0bbcfac">5.3. Runtime Flags</a></li>
</ul>
</li>
<li><a href="#org1eba1e4">6. Example</a></li>
<li><a href="#org538c002">7. Publications</a></li>
<li><a href="#org6429ea7">8. Contacts and Support</a></li>
<li><a href="#org36536e4">9. Sponsors</a></li>
</ul>
</div>
</div>


<a id="org87abbda"></a>

# License

Please see LICENSE for usage terms.


<a id="orgc9d4cd6"></a>

# Introduction

<img src="resources/images/archer_logo.png" hspace="5" vspace="5" height="45%" width="45%" alt="ARCHER Logo" title="ARCHER" align="right" />

**ARCHER** is a data race detector for OpenMP programs.

ARCHER combines static and dynamic techniques to identify data races
in large OpenMP applications, leading to low runtime and memory
overheads, while still offering high accuracy and precision. It builds
on open-source tools infrastructure such as LLVM, ThreadSanitizer, and
OMPT to provide portability.


<a id="org069b511"></a>

# Prerequisites

To compile ARCHER you need an host Clang/LLVM version >= 3.9, a
CMake version >= 3.4.3.

Ninja build system is preferred. For more information how to obtain
Ninja visit <https://martine.github.io/ninja>.

ARCHER has been tested with the LLVM OpenMP Runtime version >= 3.9,
and with the LLVM OpenMP Runtime with OMPT support currently under
development at <https://github.com/OpenMPToolsInterface/LLVM-openmp>
(under the branch "align-to-tr").


<a id="org674691d"></a>

# Installation

ARCHER has been developed under LLVM 3.9 (for more information visit
<http://llvm.org>).


<a id="org78a3a2d"></a>

## Automatic Building

For an automatic building script (recommended) please visit the GitHub
page <https://github.com/PRUNER/llvm_archer>.


<a id="orgcbf0d1a"></a>

## Manual Building

ARCHER comes as an LLVM tool, it can be compiled both as stand-alone
tool or within the Clang/LLVM infrastructure.

In order to obtain and build ARCHER follow the instructions below for
stand-alone or full Clang/LLVM with ARCHER support building
(instructions are based on bash shell, Clang/LLVM 3.9 version, Ninja
build system, and the LLVM OpenMP Runtime with OMPT support).

Note: Using the LLVM OpenMP Runtime version >= 3.9 may results in some
false positives during the data race detection process. Please, notice
that the full building of Clang/LLVM with ARCHER support depends on
the version of LLVM OpenMP runtime. In the configuration section there
will be two different commands depending on the type of runtime
chosen.


<a id="orgd54219f"></a>

## Stand-alone building with official LLVM OpenMP Runtime and ThreadSanitizer support

Create a folder to download and build ARCHER:

    export ARCHER_BUILD=$PWD/ArcherBuild
    mkdir $ARCHER_BUILD && cd $ARCHER_BUILD

Obtain the LLVM OpenMP Runtime:

    git clone git@github.com:llvm-mirror/openmp.git openmp

and build it with the following command:

    export OPENMP_INSTALL=$HOME/usr           # or any other install path
    cd openmp/runtime
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release \
     -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL \
     -D LIBOMP_TSAN_SUPPORT=TRUE \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install

Obtain ARCHER:

    cd $ARCHER_BUILD
    git clone git@github.com:PRUNER/archer.git archer

and build it with the following commands:

    export ARCHER_INSTALL=$HOME/usr           # or any other install path
    cd archer
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} \
     -D LIBOMP_TSAN_SUPPORT=TRUE \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install
    cd ../..


<a id="org25da2b9"></a>

## Stand-alone building with LLVM OpenMP Runtime and ThreadSanitizer OMPT Support

Create a folder to download and build ARCHER:

    export ARCHER_BUILD=$PWD/ArcherBuild
    mkdir $ARCHER_BUILD && cd $ARCHER_BUILD

Obtain the LLVM OpenMP Runtime with OMPT support:

    git clone -b align-to-tr git@github.com:OpenMPToolsInterface/LLVM-openmp.git openmp

and build it with the following command:

    export OPENMP_INSTALL=$HOME/usr           # or any other install path
    cd openmp/runtime
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release \
     -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL \
     -D LIBOMP_OMPT_SUPPORT=on \
     -D LIBOMP_OMPT_BLAME=on \
     -D LIBOMP_OMPT_TRACE=on \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install

Obtain ARCHER:

    cd $ARCHER_BUILD
    git clone git@github.com:PRUNER/archer.git archer

and build it with the following commands:

    export ARCHER_INSTALL=$HOME/usr           # or any other install path
    cd archer
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install
    cd ../..


<a id="org99304b9"></a>

## Build ARCHER within Clang/LLVM

Create a folder to download and build Clang/LLVM and ARCHER:

    export ARCHER_BUILD=$PWD/ArcherBuild
    mkdir $ARCHER_BUILD && cd $ARCHER_BUILD

Obtain LLVM:

    git clone git@github.com:llvm-mirror/llvm.git llvm_src
    cd llvm_src
    git checkout release_39

Obtain Clang:

    cd tools
    git clone git@github.com:llvm-mirror/clang.git clang
    cd clang
    git checkout release_39
    cd ..

Obtain ARCHER:

    cd tools
    git clone git@github.com:PRUNER/archer.git archer
    cd ..

Obtain the LLVM compiler-rt:

    cd projects
    git clone git@github.com:llvm-mirror/compiler-rt.git compiler-rt
    cd compiler-rt
    git checkout release_39
    cd ../..

Obtain LLVM libc++:

    cd projects
    git clone git@github.com:llvm-mirror/libcxx.git
    cd libcxx
    git checkout release_39
    cd ../..

Obtain LLVM libc++abi:

    cd projects
    git clone git@github.com:llvm-mirror/libcxxabi.git
    cd libcxxabi
    git checkout release_39
    cd ../..

Obtain LLVM libunwind:

    cd projects
    git clone git@github.com:llvm-mirror/libunwind.git
    cd libunwind
    git checkout release_39
    cd ../..

Obtain official LLVM OpenMP Runtime:

    cd projects
    git clone git@github.com:llvm-mirror/openmp.git openmp
    cd openmp
    git checkout release_39
    cd ../..

or obtain LLVM OpenMP Runtime with OMPT support:

    cd projects
    git clone git@github.com:OpenMPToolsInterface/LLVM-openmp.git
    cd openmp
    git checkout align-to-tr
    cd ../..

Now that we obtained the source code, the following command
will build LLVM/Clang infrastructure with ARCHER support.

First we boostrap clang:

    cd $ARCHER_BUILD
    mkdir -p llvm_bootstrap
    cd llvm_bootstrap
    CC=$(which gcc) CXX=$(which g++) cmake -G Ninja \
     -DCMAKE_BUILD_TYPE=Release \
     -DLLVM_TOOL_ARCHER_BUILD=OFF \
     -DLLVM_TARGETS_TO_BUILD=Native \
     ../llvm_src
    ninja -j8 -l8                           # or any number of available cores
    cd ..
    export LD_LIBRARY_PATH="$ARCHER_BUILD/llvm_bootstrap/lib:${LD_LIBRARY_PATH}"
    export PATH="$ARCHER_BUILD/llvm_bootstrap/bin:${PATH}"

Then, we can actually build LLVM/Clang with ARCHER support.

In case of official LLVM OpenMP Runtime run:

    export LLVM_INSTALL=$HOME/usr           # or any other install path
    mkdir llvm_build && cd llvm_build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release \
     -D CMAKE_INSTALL_PREFIX:PATH=$LLVM_INSTALL \
     -D CLANG_DEFAULT_OPENMP_RUNTIME:STRING=libomp \
     -D LLVM_ENABLE_LIBCXX=ON \
     -D LLVM_ENABLE_LIBCXXABI=ON \
     -D LIBCXXABI_USE_LLVM_UNWINDER=ON \
     -D CLANG_DEFAULT_CXX_STDLIB=libc++ \
     -D LIBOMP_TSAN_SUPPORT=TRUE \
     ../llvm_src
    ninja -j8 -l8                           # or any number of available cores
    ninja check-libarcher
    ninja install

Otherwise, in case of LLVM OpenMP Runtime with OMPT support run:

    export LLVM_INSTALL=$HOME/usr           # or any other install path
    mkdir llvm_build && cd llvm_build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release \
     -D CMAKE_INSTALL_PREFIX:PATH=$LLVM_INSTALL \
     -D CLANG_DEFAULT_OPENMP_RUNTIME:STRING=libomp \
     -D LLVM_ENABLE_LIBCXX=ON \
     -D LLVM_ENABLE_LIBCXXABI=ON \
     -D LIBCXXABI_USE_LLVM_UNWINDER=ON \
     -D CLANG_DEFAULT_CXX_STDLIB=libc++ \
     -D LIBOMP_OMPT_SUPPORT=on \
     -D LIBOMP_OMPT_BLAME=on \
     -D LIBOMP_OMPT_TRACE=on \
     ../llvm_src
    ninja -j8 -l8                           # or any number of available cores
    ninja check-libarcher
    ninja install

Once the installation completes, you need to setup your environement
to allow ARCHER to work correctly.

Please set the following path variables:

    export PATH=${LLVM_INSTALL}/bin:${PATH}"
    export LD_LIBRARY_PATH=${LLVM_INSTALL}/lib:${LD_LIBRARY_PATH}"

To make the environment permanent add the previous lines or
equivalents to your shell start-up script such as "~/.bashrc".


<a id="org74896dc"></a>

# Usage


<a id="org51459d2"></a>

## How to compile

ARCHER provides a command to compile your programs with Clang/LLVM
OpenMP and hide all the mechanisms necessary to detect data races
automatically in your OpenMP programs.

The ARCHER compile command is called *clang-archer*, and this can be
used as a drop-in replacement of your compiler command (e.g., clang,
gcc, etc.).

The following are some of the examples of how one can integrate
*clang-archer* into his/her build system.

If you are using ARCHER and the LLVM OpenMP Runtime with OMPT support,
it is necessary to link your executable against the ARCHER runtime
library *libarcher.so*. (In the example below the runtime library will
be shown in square brackets).


<a id="org0a75e90"></a>

### Single source

    clang-archer example.c -o example [ -L/path/to/archer/runtime/library -larcher ]


<a id="org21be94b"></a>

### Makefile

In your Makefile, set the following variables:

    CC=clang-archer
    [ LD_FLAGS=-L/path/to/archer/runtime/library -larcher ]


<a id="org7d7f8ce"></a>

### Hybrid MPI-OpenMP programs

In your Makefile, set the following variables:

    CC = mpicc -cc=clang-archer
    [ LD_FLAGS=-L/path/to/archer/runtime/library -larcher ]


<a id="org3ab76c1"></a>

## Options

The command *clang-archer* works as a compiler wrapper, all the
options available for clang are also available for *clang-archer*.


<a id="org0bbcfac"></a>

## Runtime Flags

Runtime flags are passed via **ARCHER&#95;OPTIONS** environment variable,
separate flags are separated with spaces, e.g.:

    ARCHER_OPTIONS="flush_shadow=1" ./myprogram

<table border="2" cellspacing="0" cellpadding="6" rules="groups" frame="hsides">


<colgroup>
<col  class="org-left" />

<col  class="org-right" />

<col  class="org-left" />

<col  class="org-left" />
</colgroup>
<thead>
<tr>
<th scope="col" class="org-left">Flag Name</th>
<th scope="col" class="org-right">Default value</th>
<th scope="col" class="org-left">Clang/LLVM Version</th>
<th scope="col" class="org-left">Description</th>
</tr>
</thead>

<tbody>
<tr>
<td class="org-left">flush&#95;shadow</td>
<td class="org-right">0</td>
<td class="org-left">>= 4.0</td>
<td class="org-left">Flush shadow memory at the end of an outer OpenMP parallel region. Experiments show a memory overhead reduction of ~30%, and an increase of the runtime overhead of ~10%. This flag is useful for large OpenMP applications that normally requires a large amount of memory and may cause an "out of memory" exception when checked by ARCHER.</td>
</tr>
</tbody>
</table>


<a id="org1eba1e4"></a>

# Example

Let us take the program below and follow the steps to compile and
check the program for data races.

Suppose our program is called *myprogram.c*:

     1  #include <stdio.h>
     2
     3  #define N 1000
     4
     5  int main (int argc, char **argv)
     6  {
     7    int a[N];
     8
     9  #pragma omp parallel for
    10    for (int i = 0; i < N - 1; i++) {
    11      a[i] = a[i + 1];
    12    }
    13  }

In case we installed ARCHER with the official LLVM OpenMP runtime and
ThreadSanitizer support we compile the program as follow:

    clang-archer myprogram.c -o myprogram

otherwise, if we installed ARCHER with the LLVM OpenMP runtime and
ThreadSanitizer OMPT support our compile command will look like:

    clang-archer myprogram.c -o myprogram -larcher

Now we can run the program with the following commands:

    export OMP_NUM_THREADS=2
    ./myprogram

ARCHER will output a report in case it finds data races. In our case
the report will look as follow:

    ==================
    WARNING: ThreadSanitizer: data race (pid=13641)
      Read of size 4 at 0x7fff79a01170 by main thread:
        #0 .omp_outlined. myprogram.c:11:12 (myprogram+0x00000049b5a2)
        #1 __kmp_invoke_microtask <null> (libomp.so+0x000000077842)
        #2 __libc_start_main /build/glibc-t3gR2i/glibc-2.23/csu/../csu/libc-start.c:291 (libc.so.6+0x00000002082f)

      Previous write of size 4 at 0x7fff79a01170 by thread T1:
        #0 .omp_outlined. myprogram.c:11:10 (myprogram+0x00000049b5d6)
        #1 __kmp_invoke_microtask <null> (libomp.so+0x000000077842)

      Location is stack of main thread.

      Thread T1 (tid=13643, running) created by main thread at:
        #0 pthread_create tsan_interceptors.cc:902:3 (myprogram+0x00000043db75)
        #1 __kmp_create_worker <null> (libomp.so+0x00000006c364)
        #2 __libc_start_main /build/glibc-t3gR2i/glibc-2.23/csu/../csu/libc-start.c:291 (libc.so.6+0x00000002082f)

    SUMMARY: ThreadSanitizer: data race myprogram.c:11:12 in .omp_outlined.
    ==================
    ThreadSanitizer: reported 1 warnings


<a id="org538c002"></a>

# Publications

-   Atzeni, S., G. Gopalakrishnan, Z. Rakamaric, D. H. Ahn, I. Laguna,
    M. Schulz, G. L. Lee, J. Protze, and M. S. Müller. 2016. “ARCHER:
    Effectively Spotting Data Races in Large Openmp Applications.” In
    2016 IEEE International Parallel and Distributed Processing
    Symposium (IPDPS), 53–62. <10.1109/IPDPS.2016.68>
-   Protze, Joachim, Simone Atzeni, Dong H. Ahn, Martin Schulz, Ganesh
    Gopalakrishnan, Matthias S. Müller, Ignacio Laguna, Zvonimir
    Rakamarić, and Greg L. Lee. 2014. “Towards Providing Low-Overhead
    Data Race Detection for Large Openmp Applications.” In
    Proceedings of the 2014 LLVM Compiler Infrastructure in
    HPC, 40–47. LLVM-HPC ’14. Piscataway, NJ, USA: IEEE
    Press. <http://dl.acm.org/citation.cfm?id=2688369>


<a id="org6429ea7"></a>

# Contacts and Support

-   [Google group](https://groups.google.com/forum/#!forum/archer-pruner)
-   [Slack Channel](https://pruner.slack.com/shared_invite/MTIzNzExNzg4ODgxLTE0ODM3MzE2NTctNmRjNmM0NDYwNA)
-   E-Mail Contacts:

    <ul style="list-style-type:circle"> <li> <a href="mailto:simone@cs.utah.edu?Subject=[archer-dev]%20" target="_top">Simone Atzeni</a> </li> <li> <a href="mailto:protze@itc.rwth-aachen.de?Subject=[archer-dev]%20" target="_top">Joachim Protze</a> </li> </ul>


<a id="org36536e4"></a>

# Sponsors

<img src="resources/images/uofu_logo.png" hspace="15" vspace="5" height="23%" width="23%" alt="UofU Logo" title="University of Utah" style="float:left" /> <img src="resources/images/llnl_logo.png" hspace="70" vspace="5" height="30%" width="30%" alt="LLNL Logo" title="Lawrence Livermore National Laboratory" style="float:center" /> <img src="resources/images/rwthaachen_logo.png" hspace="15" vspace="5" height="23%" width="23%" alt="RWTH AACHEN Logo" title="RWTH AACHEN University" style="float:left" />
