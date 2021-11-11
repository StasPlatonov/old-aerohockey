################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../External/Box2D/Box2D/Collision/b2BroadPhase.cpp \
../External/Box2D/Box2D/Collision/b2CollideCircle.cpp \
../External/Box2D/Box2D/Collision/b2CollideEdge.cpp \
../External/Box2D/Box2D/Collision/b2CollidePolygon.cpp \
../External/Box2D/Box2D/Collision/b2Collision.cpp \
../External/Box2D/Box2D/Collision/b2Distance.cpp \
../External/Box2D/Box2D/Collision/b2DynamicTree.cpp \
../External/Box2D/Box2D/Collision/b2TimeOfImpact.cpp 

OBJS += \
./External/Box2D/Box2D/Collision/b2BroadPhase.o \
./External/Box2D/Box2D/Collision/b2CollideCircle.o \
./External/Box2D/Box2D/Collision/b2CollideEdge.o \
./External/Box2D/Box2D/Collision/b2CollidePolygon.o \
./External/Box2D/Box2D/Collision/b2Collision.o \
./External/Box2D/Box2D/Collision/b2Distance.o \
./External/Box2D/Box2D/Collision/b2DynamicTree.o \
./External/Box2D/Box2D/Collision/b2TimeOfImpact.o 

CPP_DEPS += \
./External/Box2D/Box2D/Collision/b2BroadPhase.d \
./External/Box2D/Box2D/Collision/b2CollideCircle.d \
./External/Box2D/Box2D/Collision/b2CollideEdge.d \
./External/Box2D/Box2D/Collision/b2CollidePolygon.d \
./External/Box2D/Box2D/Collision/b2Collision.d \
./External/Box2D/Box2D/Collision/b2Distance.d \
./External/Box2D/Box2D/Collision/b2DynamicTree.d \
./External/Box2D/Box2D/Collision/b2TimeOfImpact.d 


# Each subdirectory must supply rules for building sources it contributes
External/Box2D/Box2D/Collision/%.o: ../External/Box2D/Box2D/Collision/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DUSE_GL=1 -I"/home/test/Projects/PlayWork" -I"/home/test/Projects/PlayWork/External/Box2D" -I"/home/test/Projects/PlayWork/External/RakNet/Source" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


