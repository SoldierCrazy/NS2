################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../diffusion3/filter_core/filter_core.o \
../diffusion3/filter_core/iolog.o \
../diffusion3/filter_core/iostats.o 

CC_SRCS += \
../diffusion3/filter_core/filter_core.cc \
../diffusion3/filter_core/iolog.cc \
../diffusion3/filter_core/iostats.cc 

OBJS += \
./diffusion3/filter_core/filter_core.o \
./diffusion3/filter_core/iolog.o \
./diffusion3/filter_core/iostats.o 

CC_DEPS += \
./diffusion3/filter_core/filter_core.d \
./diffusion3/filter_core/iolog.d \
./diffusion3/filter_core/iostats.d 


# Each subdirectory must supply rules for building sources it contributes
diffusion3/filter_core/%.o: ../diffusion3/filter_core/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


