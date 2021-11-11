################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Common/Localization.cpp \
../Common/Serialization.cpp \
../Common/Thread.cpp \
../Common/Timer.cpp \
../Common/Utils.cpp \
../Common/xmlParser.cpp 

OBJS += \
./Common/Localization.o \
./Common/Serialization.o \
./Common/Thread.o \
./Common/Timer.o \
./Common/Utils.o \
./Common/xmlParser.o 

CPP_DEPS += \
./Common/Localization.d \
./Common/Serialization.d \
./Common/Thread.d \
./Common/Timer.d \
./Common/Utils.d \
./Common/xmlParser.d 


# Each subdirectory must supply rules for building sources it contributes
Common/%.o: ../Common/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++1y -DUSE_GL=1 -I"/home/odd/Projects/PlayWork" -I../../RakNet/Source -I../../Box2D -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


