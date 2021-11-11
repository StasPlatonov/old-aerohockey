################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../External/Box2D/Box2D/Dynamics/b2Body.cpp \
../External/Box2D/Box2D/Dynamics/b2ContactManager.cpp \
../External/Box2D/Box2D/Dynamics/b2Fixture.cpp \
../External/Box2D/Box2D/Dynamics/b2Island.cpp \
../External/Box2D/Box2D/Dynamics/b2World.cpp \
../External/Box2D/Box2D/Dynamics/b2WorldCallbacks.cpp 

OBJS += \
./External/Box2D/Box2D/Dynamics/b2Body.o \
./External/Box2D/Box2D/Dynamics/b2ContactManager.o \
./External/Box2D/Box2D/Dynamics/b2Fixture.o \
./External/Box2D/Box2D/Dynamics/b2Island.o \
./External/Box2D/Box2D/Dynamics/b2World.o \
./External/Box2D/Box2D/Dynamics/b2WorldCallbacks.o 

CPP_DEPS += \
./External/Box2D/Box2D/Dynamics/b2Body.d \
./External/Box2D/Box2D/Dynamics/b2ContactManager.d \
./External/Box2D/Box2D/Dynamics/b2Fixture.d \
./External/Box2D/Box2D/Dynamics/b2Island.d \
./External/Box2D/Box2D/Dynamics/b2World.d \
./External/Box2D/Box2D/Dynamics/b2WorldCallbacks.d 


# Each subdirectory must supply rules for building sources it contributes
External/Box2D/Box2D/Dynamics/%.o: ../External/Box2D/Box2D/Dynamics/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DUSE_GL=1 -I"/home/test/Projects/PlayWork" -I"/home/test/Projects/PlayWork/External/Box2D" -I"/home/test/Projects/PlayWork/External/RakNet/Source" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


