################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../diffusion3/lib/diffapp.o \
../diffusion3/lib/dr.o 

CC_SRCS += \
../diffusion3/lib/diffapp.cc \
../diffusion3/lib/dr.cc 

OBJS += \
./diffusion3/lib/diffapp.o \
./diffusion3/lib/dr.o 

CC_DEPS += \
./diffusion3/lib/diffapp.d \
./diffusion3/lib/dr.d 


# Each subdirectory must supply rules for building sources it contributes
diffusion3/lib/%.o: ../diffusion3/lib/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


