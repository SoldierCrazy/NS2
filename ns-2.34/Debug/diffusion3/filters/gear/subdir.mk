################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../diffusion3/filters/gear/gear.o \
../diffusion3/filters/gear/gear_attr.o \
../diffusion3/filters/gear/gear_tools.o 

CC_SRCS += \
../diffusion3/filters/gear/gear.cc \
../diffusion3/filters/gear/gear_attr.cc \
../diffusion3/filters/gear/gear_tools.cc 

OBJS += \
./diffusion3/filters/gear/gear.o \
./diffusion3/filters/gear/gear_attr.o \
./diffusion3/filters/gear/gear_tools.o 

CC_DEPS += \
./diffusion3/filters/gear/gear.d \
./diffusion3/filters/gear/gear_attr.d \
./diffusion3/filters/gear/gear_tools.d 


# Each subdirectory must supply rules for building sources it contributes
diffusion3/filters/gear/%.o: ../diffusion3/filters/gear/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


