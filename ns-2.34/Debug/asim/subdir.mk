################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../asim/asim.o 

CC_SRCS += \
../asim/asim.cc \
../asim/asimstd.cc 

OBJS += \
./asim/asim.o \
./asim/asimstd.o 

CC_DEPS += \
./asim/asim.d \
./asim/asimstd.d 


# Each subdirectory must supply rules for building sources it contributes
asim/%.o: ../asim/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


