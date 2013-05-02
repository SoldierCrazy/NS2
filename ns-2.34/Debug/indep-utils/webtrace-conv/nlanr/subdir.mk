################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../indep-utils/webtrace-conv/nlanr/logparse.o \
../indep-utils/webtrace-conv/nlanr/tr-stat.o 

CC_SRCS += \
../indep-utils/webtrace-conv/nlanr/logparse.cc \
../indep-utils/webtrace-conv/nlanr/tr-stat.cc 

OBJS += \
./indep-utils/webtrace-conv/nlanr/logparse.o \
./indep-utils/webtrace-conv/nlanr/tr-stat.o 

CC_DEPS += \
./indep-utils/webtrace-conv/nlanr/logparse.d \
./indep-utils/webtrace-conv/nlanr/tr-stat.d 


# Each subdirectory must supply rules for building sources it contributes
indep-utils/webtrace-conv/nlanr/%.o: ../indep-utils/webtrace-conv/nlanr/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


