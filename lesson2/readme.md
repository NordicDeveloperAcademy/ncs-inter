# Exercise 1 
In this exercise we will be using the same fundation as in exercise 2. But instead of the core dump we will use the ADDR2LINE tool to "translate" the faulting register address to a line in the code. 

The [addr2line](https://www.man7.org/linux/man-pages/man1/addr2line.1.html) is a linux tool that translate addresses or symbol+offset into a filename and line number. 

### Step 1:
Open the applicaiton Exercise 1 and build it for your targed and flash it to the board

### Step 2: 
Open a terminal to the devkit 
You should see the following screen: 
```
*** Booting nRF Connect SDK v2.5.0 ***
Lesson 2 Exersice 1 ADDR2LINE Started
Set up button at gpio@50000000 pin 11
Set up LED at gpio@50000000 pin 13
Press the button to crash the application
```

### Step 3: 
Hit the button 1 to crash the application
You should see the following message. Note, the address might differ for different boards
```
[00:00:08.966,156] <err> os: ***** USAGE FAULT *****
[00:00:08.966,186] <err> os:   Attempt to execute undefined instruction
[00:00:08.966,217] <err> os: r0/a1:  0x00000000  r1/a2:  0x00000002  r2/a3:  0x00000001
[00:00:08.966,217] <err> os: r3/a4:  0x000003e5 r12/ip:  0x00000004 r14/lr:  0x000075fb
[00:00:08.966,247] <err> os:  xpsr:  0x41000016
[00:00:08.966,247] <err> os: Faulting instruction address (r15/pc): 0x000075f0
[00:00:08.966,278] <err> os: >>> ZEPHYR FATAL ERROR 36: Unknown error on CPU 0
[00:00:08.966,308] <err> os: Fault during interrupt handling
````

### Step 4: 
Open a terminal in your project folder
Find the following application in your toolchain folder ```arm-zephyr-eabi-addr2line```
In Linux it is located within "ncs_folder/toolchains/1f9b40e71a/opt/zephyr-sdk/arm-zephyr-eabi/bin/" or ```ncs_folder\toolchains\31f4403e35\opt\zephyr-sdk\arm-zephyr-eabi\bin``` in Windows

Step 5: 
Run the following command ```ncs_folder/toolchains/1f9b40e71a/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-addr2line -e zephyr/zephyr.elf 0x000075f0``` for Linux or ```ncs_folder\toolchains\31f4403e35\opt\zephyr-sdk\arm-zephyr-eabi\bin\arm-zephyr-eabi-addr2line.exe -e zephyr/zephyr.elf 0x000075f0``` for Windows
This gives should give a output similiar to this 
```
...\Lesson2_exercise2_solution/build/../src/main.c:61
```
This means the the instruction leading to the fault is found in "main.c" and line 59. If we have a look in the example in line 59 we find the following line: 
```
	/* Dereferencing null-pointer in TrustZone-enabled
	 * builds may crash the system, so use, instead an
	 * undefined instruction to trigger a CPU fault.
	 */
	__asm__ volatile("udf #0" : : : );
```
This shows how the addr2line tool can be used to find out where an application is crashing and help with further debuging. 
The addr2line tool share similarites with the core dump in the previous example. Where as the core dump has more requierements in regardigs to storage or sending the core dump, the addr2line only needs the instruction address and the zephyr.elf. With the core dump the developer has access to read the register values at the time of the crash and the fuction calls leading up to the fatal error, the addr2line tools just gives a line and the module. 

