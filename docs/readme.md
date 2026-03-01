uses zephyr, follow zephyr installation guidelines, install in parent directory with all 

please comment 
dont force push, make sure code compiles before PR

>west build -b nucleo_h753zi -p always < for building pristine for dev board>
west flash

open terminal on STLINK-V3 Virtual COM port at 115200 baud for debug output. 


use clangd for linting and compiler warning
C++11 with certain extensions to C++20 no STD:: thread control blocks use zephyr kernel functions 
zephyr latest release at 4.2.0. 