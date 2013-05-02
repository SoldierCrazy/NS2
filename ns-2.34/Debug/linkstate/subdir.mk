################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../linkstate/hdr-ls.o \
../linkstate/ls.o \
../linkstate/rtProtoLS.o 

CC_SRCS += \
../linkstate/hdr-ls.cc \
../linkstate/ls.cc \
../linkstate/rtProtoLS.cc 

OBJS += \
./linkstate/hdr-ls.o \
./linkstate/ls.o \
./linkstate/rtProtoLS.o 

CC_DEPS += \
./linkstate/hdr-ls.d \
./linkstate/ls.d \
./linkstate/rtProtoLS.d 


# Each subdirectory must supply rules for building sources it contributes
linkstate/%.o: ../linkstate/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


