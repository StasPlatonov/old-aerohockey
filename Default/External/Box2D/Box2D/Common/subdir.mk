################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../External/Box2D/Box2D/Common/b2BlockAllocator.cpp \
../External/Box2D/Box2D/Common/b2Draw.cpp \
../External/Box2D/Box2D/Common/b2Math.cpp \
../External/Box2D/Box2D/Common/b2Settings.cpp \
../External/Box2D/Box2D/Common/b2StackAllocator.cpp \
../External/Box2D/Box2D/Common/b2Timer.cpp 

OBJS += \
./External/Box2D/Box2D/Common/b2BlockAllocator.o \
./External/Box2D/Box2D/Common/b2Draw.o \
./External/Box2D/Box2D/Common/b2Math.o \
./External/Box2D/Box2D/Common/b2Settings.o \
./External/Box2D/Box2D/Common/b2StackAllocator.o \
./External/Box2D/Box2D/Common/b2Timer.o 

CPP_DEPS += \
./External/Box2D/Box2D/Common/b2BlockAllocator.d \
./External/Box2D/Box2D/Common/b2Draw.d \
./External/Box2D/Box2D/Common/b2Math.d \
./External/Box2D/Box2D/Common/b2Settings.d \
./External/Box2D/Box2D/Common/b2StackAllocator.d \
./External/Box2D/Box2D/Common/b2Timer.d 


# Each subdirectory must supply rules for building sources it contributes
External/Box2D/Box2D/Common/%.o: ../External/Box2D/Box2D/Common/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DUSE_GL=1 -I"/home/test/Projects/PlayWork" -I"/home/test/Projects/PlayWork/External/Box2D" -I"/home/test/Projects/PlayWork/External/RakNet/Source" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


