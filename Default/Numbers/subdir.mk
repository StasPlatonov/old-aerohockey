################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Numbers/NumbersClient.cpp \
../Numbers/NumbersConsoleClient.cpp \
../Numbers/NumbersServer.cpp 

OBJS += \
./Numbers/NumbersClient.o \
./Numbers/NumbersConsoleClient.o \
./Numbers/NumbersServer.o 

CPP_DEPS += \
./Numbers/NumbersClient.d \
./Numbers/NumbersConsoleClient.d \
./Numbers/NumbersServer.d 


# Each subdirectory must supply rules for building sources it contributes
Numbers/%.o: ../Numbers/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++1y -DUSE_GL=1 -I"/home/odd/Projects/PlayWork" -I../../RakNet/Source -I../../Box2D -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


