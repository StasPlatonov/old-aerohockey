################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../External/Box2D/Box2D/Dynamics/Joints/b2DistanceJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2FrictionJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2GearJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2Joint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2MouseJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2PulleyJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2RopeJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2WeldJoint.cpp \
../External/Box2D/Box2D/Dynamics/Joints/b2WheelJoint.cpp 

OBJS += \
./External/Box2D/Box2D/Dynamics/Joints/b2DistanceJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2FrictionJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2GearJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2Joint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2MouseJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2PrismaticJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2PulleyJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2RevoluteJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2RopeJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2WeldJoint.o \
./External/Box2D/Box2D/Dynamics/Joints/b2WheelJoint.o 

CPP_DEPS += \
./External/Box2D/Box2D/Dynamics/Joints/b2DistanceJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2FrictionJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2GearJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2Joint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2MouseJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2PrismaticJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2PulleyJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2RevoluteJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2RopeJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2WeldJoint.d \
./External/Box2D/Box2D/Dynamics/Joints/b2WheelJoint.d 


# Each subdirectory must supply rules for building sources it contributes
External/Box2D/Box2D/Dynamics/Joints/%.o: ../External/Box2D/Box2D/Dynamics/Joints/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DUSE_GL=1 -I"/home/test/Projects/PlayWork" -I"/home/test/Projects/PlayWork/External/Box2D" -I"/home/test/Projects/PlayWork/External/RakNet/Source" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


