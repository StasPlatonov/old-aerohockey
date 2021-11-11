################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../External/Box2D/Box2D/Rope/b2Rope.cpp 

OBJS += \
./External/Box2D/Box2D/Rope/b2Rope.o 

CPP_DEPS += \
./External/Box2D/Box2D/Rope/b2Rope.d 


# Each subdirectory must supply rules for building sources it contributes
External/Box2D/Box2D/Rope/%.o: ../External/Box2D/Box2D/Rope/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DUSE_GL=1 -I"/home/test/Projects/PlayWork" -I"/home/test/Projects/PlayWork/External/Box2D" -I"/home/test/Projects/PlayWork/External/RakNet/Source" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


