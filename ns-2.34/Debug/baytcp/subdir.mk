################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../baytcp/ftpc.o \
../baytcp/ftps.o \
../baytcp/tcp-full-bay.o 

CC_SRCS += \
../baytcp/ftpc.cc \
../baytcp/ftps.cc \
../baytcp/tcp-full-bay.cc 

OBJS += \
./baytcp/ftpc.o \
./baytcp/ftps.o \
./baytcp/tcp-full-bay.o 

CC_DEPS += \
./baytcp/ftpc.d \
./baytcp/ftps.d \
./baytcp/tcp-full-bay.d 


# Each subdirectory must supply rules for building sources it contributes
baytcp/%.o: ../baytcp/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


