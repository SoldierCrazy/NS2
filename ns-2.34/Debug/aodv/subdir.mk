################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../aodv/aodv.o \
../aodv/aodv_logs.o \
../aodv/aodv_rqueue.o \
../aodv/aodv_rtable.o 

CC_SRCS += \
../aodv/aodv.cc \
../aodv/aodv_logs.cc \
../aodv/aodv_rqueue.cc \
../aodv/aodv_rtable.cc 

OBJS += \
./aodv/aodv.o \
./aodv/aodv_logs.o \
./aodv/aodv_rqueue.o \
./aodv/aodv_rtable.o 

CC_DEPS += \
./aodv/aodv.d \
./aodv/aodv_logs.d \
./aodv/aodv_rqueue.d \
./aodv/aodv_rtable.d 


# Each subdirectory must supply rules for building sources it contributes
aodv/%.o: ../aodv/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


