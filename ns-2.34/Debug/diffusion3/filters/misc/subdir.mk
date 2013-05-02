################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../diffusion3/filters/misc/log.o \
../diffusion3/filters/misc/srcrt.o \
../diffusion3/filters/misc/tag.o 

CC_SRCS += \
../diffusion3/filters/misc/log.cc \
../diffusion3/filters/misc/srcrt.cc \
../diffusion3/filters/misc/tag.cc 

OBJS += \
./diffusion3/filters/misc/log.o \
./diffusion3/filters/misc/srcrt.o \
./diffusion3/filters/misc/tag.o 

CC_DEPS += \
./diffusion3/filters/misc/log.d \
./diffusion3/filters/misc/srcrt.d \
./diffusion3/filters/misc/tag.d 


# Each subdirectory must supply rules for building sources it contributes
diffusion3/filters/misc/%.o: ../diffusion3/filters/misc/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


