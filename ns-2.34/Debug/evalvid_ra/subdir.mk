################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../evalvid_ra/evalvid_ra_sink.o \
../evalvid_ra/evalvid_ra_udp.o \
../evalvid_ra/vbr_rateadapt.o 

CC_SRCS += \
../evalvid_ra/cbr_rateadapt.cc \
../evalvid_ra/evalvid_ra_sink.cc \
../evalvid_ra/evalvid_ra_udp.cc \
../evalvid_ra/poisson_rateadapt.cc \
../evalvid_ra/vbr_rateadapt.cc 

OBJS += \
./evalvid_ra/cbr_rateadapt.o \
./evalvid_ra/evalvid_ra_sink.o \
./evalvid_ra/evalvid_ra_udp.o \
./evalvid_ra/poisson_rateadapt.o \
./evalvid_ra/vbr_rateadapt.o 

CC_DEPS += \
./evalvid_ra/cbr_rateadapt.d \
./evalvid_ra/evalvid_ra_sink.d \
./evalvid_ra/evalvid_ra_udp.d \
./evalvid_ra/poisson_rateadapt.d \
./evalvid_ra/vbr_rateadapt.d 


# Each subdirectory must supply rules for building sources it contributes
evalvid_ra/%.o: ../evalvid_ra/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


