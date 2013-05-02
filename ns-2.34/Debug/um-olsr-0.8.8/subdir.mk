################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../um-olsr-0.8.8/OLSR.o \
../um-olsr-0.8.8/OLSR_printer.o \
../um-olsr-0.8.8/OLSR_rtable.o \
../um-olsr-0.8.8/OLSR_state.o 

CC_SRCS += \
../um-olsr-0.8.8/OLSR.cc \
../um-olsr-0.8.8/OLSR_printer.cc \
../um-olsr-0.8.8/OLSR_rtable.cc \
../um-olsr-0.8.8/OLSR_state.cc 

OBJS += \
./um-olsr-0.8.8/OLSR.o \
./um-olsr-0.8.8/OLSR_printer.o \
./um-olsr-0.8.8/OLSR_rtable.o \
./um-olsr-0.8.8/OLSR_state.o 

CC_DEPS += \
./um-olsr-0.8.8/OLSR.d \
./um-olsr-0.8.8/OLSR_printer.d \
./um-olsr-0.8.8/OLSR_rtable.d \
./um-olsr-0.8.8/OLSR_state.d 


# Each subdirectory must supply rules for building sources it contributes
um-olsr-0.8.8/%.o: ../um-olsr-0.8.8/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


