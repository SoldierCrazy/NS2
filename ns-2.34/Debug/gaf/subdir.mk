################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../gaf/gaf.o 

CC_SRCS += \
../gaf/gaf.cc 

OBJS += \
./gaf/gaf.o 

CC_DEPS += \
./gaf/gaf.d 


# Each subdirectory must supply rules for building sources it contributes
gaf/%.o: ../gaf/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


