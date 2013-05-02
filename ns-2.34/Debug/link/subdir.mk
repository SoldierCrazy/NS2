################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../link/delay.o \
../link/dynalink.o \
../link/hackloss.o 

CC_SRCS += \
../link/delay.cc \
../link/dynalink.cc \
../link/hackloss.cc 

OBJS += \
./link/delay.o \
./link/dynalink.o \
./link/hackloss.o 

CC_DEPS += \
./link/delay.d \
./link/dynalink.d \
./link/hackloss.d 


# Each subdirectory must supply rules for building sources it contributes
link/%.o: ../link/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


