################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../olsr/OLSR.o \
../olsr/OLSR_printer.o \
../olsr/OLSR_rtable.o \
../olsr/OLSR_state.o 

CC_SRCS += \
../olsr/OLSR.cc \
../olsr/OLSR_printer.cc \
../olsr/OLSR_rtable.cc \
../olsr/OLSR_state.cc 

OBJS += \
./olsr/OLSR.o \
./olsr/OLSR_printer.o \
./olsr/OLSR_rtable.o \
./olsr/OLSR_state.o 

CC_DEPS += \
./olsr/OLSR.d \
./olsr/OLSR_printer.d \
./olsr/OLSR_rtable.d \
./olsr/OLSR_state.d 


# Each subdirectory must supply rules for building sources it contributes
olsr/%.o: ../olsr/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


