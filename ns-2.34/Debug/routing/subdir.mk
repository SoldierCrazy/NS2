################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../routing/addr-params.o \
../routing/address.o \
../routing/alloc-address.o \
../routing/route.o \
../routing/rtProtoDV.o \
../routing/rtmodule.o \
../routing/rttable.o 

CC_SRCS += \
../routing/addr-params.cc \
../routing/address.cc \
../routing/alloc-address.cc \
../routing/route.cc \
../routing/rtProtoDV.cc \
../routing/rtmodule.cc \
../routing/rttable.cc 

OBJS += \
./routing/addr-params.o \
./routing/address.o \
./routing/alloc-address.o \
./routing/route.o \
./routing/rtProtoDV.o \
./routing/rtmodule.o \
./routing/rttable.o 

CC_DEPS += \
./routing/addr-params.d \
./routing/address.d \
./routing/alloc-address.d \
./routing/route.d \
./routing/rtProtoDV.d \
./routing/rtmodule.d \
./routing/rttable.d 


# Each subdirectory must supply rules for building sources it contributes
routing/%.o: ../routing/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


