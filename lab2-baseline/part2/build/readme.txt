This is the folder where your executable will be built.  To create the makefile and Eclipse project, run the following command:

cmake ../src -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -DCMAKE_TOOLCHAIN_FILE=../Toolchain-rpi.cmake 