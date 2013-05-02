################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../xcp/xcp-end-sys.o \
../xcp/xcp.o \
../xcp/xcpq.o 

CC_SRCS += \
../xcp/xcp-end-sys.cc \
../xcp/xcp.cc \
../xcp/xcpq.cc 

OBJS += \
./xcp/xcp-end-sys.o \
./xcp/xcp.o \
./xcp/xcpq.o 

CC_DEPS += \
./xcp/xcp-end-sys.d \
./xcp/xcp.d \
./xcp/xcpq.d 


# Each subdirectory must supply rules for building sources it contributes
xcp/%.o: ../xcp/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


