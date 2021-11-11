################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../External/Box2D/Box2D/Collision/Shapes/b2ChainShape.cpp \
../External/Box2D/Box2D/Collision/Shapes/b2CircleShape.cpp \
../External/Box2D/Box2D/Collision/Shapes/b2EdgeShape.cpp \
../External/Box2D/Box2D/Collision/Shapes/b2PolygonShape.cpp 

OBJS += \
./External/Box2D/Box2D/Collision/Shapes/b2ChainShape.o \
./External/Box2D/Box2D/Collision/Shapes/b2CircleShape.o \
./External/Box2D/Box2D/Collision/Shapes/b2EdgeShape.o \
./External/Box2D/Box2D/Collision/Shapes/b2PolygonShape.o 

CPP_DEPS += \
./External/Box2D/Box2D/Collision/Shapes/b2ChainShape.d \
./External/Box2D/Box2D/Collision/Shapes/b2CircleShape.d \
./External/Box2D/Box2D/Collision/Shapes/b2EdgeShape.d \
./External/Box2D/Box2D/Collision/Shapes/b2PolygonShape.d 


# Each subdirectory must supply rules for building sources it contributes
External/Box2D/Box2D/Collision/Shapes/%.o: ../External/Box2D/Box2D/Collision/Shapes/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DUSE_GL=1 -I"/home/test/Projects/PlayWork" -I"/home/test/Projects/PlayWork/External/Box2D" -I"/home/test/Projects/PlayWork/External/RakNet/Source" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


