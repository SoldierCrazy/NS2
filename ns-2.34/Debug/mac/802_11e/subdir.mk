################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../mac/802_11e/d-tail.o \
../mac/802_11e/mac-802_11e.o \
../mac/802_11e/mac-timers_802_11e.o \
../mac/802_11e/priq.o 

CC_SRCS += \
../mac/802_11e/d-tail.cc \
../mac/802_11e/mac-802_11e.cc \
../mac/802_11e/mac-timers_802_11e.cc \
../mac/802_11e/priq.cc 

OBJS += \
./mac/802_11e/d-tail.o \
./mac/802_11e/mac-802_11e.o \
./mac/802_11e/mac-timers_802_11e.o \
./mac/802_11e/priq.o 

CC_DEPS += \
./mac/802_11e/d-tail.d \
./mac/802_11e/mac-802_11e.d \
./mac/802_11e/mac-timers_802_11e.d \
./mac/802_11e/priq.d 


# Each subdirectory must supply rules for building sources it contributes
mac/802_11e/%.o: ../mac/802_11e/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


