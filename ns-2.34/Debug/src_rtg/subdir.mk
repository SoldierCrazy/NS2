################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src_rtg/classifier-sr.o \
../src_rtg/hdr_src.o \
../src_rtg/sragent.o 

CC_SRCS += \
../src_rtg/classifier-sr.cc \
../src_rtg/hdr_src.cc \
../src_rtg/sragent.cc 

OBJS += \
./src_rtg/classifier-sr.o \
./src_rtg/hdr_src.o \
./src_rtg/sragent.o 

CC_DEPS += \
./src_rtg/classifier-sr.d \
./src_rtg/hdr_src.d \
./src_rtg/sragent.d 


# Each subdirectory must supply rules for building sources it contributes
src_rtg/%.o: ../src_rtg/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


