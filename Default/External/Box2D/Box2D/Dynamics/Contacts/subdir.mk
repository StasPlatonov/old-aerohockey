################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../External/Box2D/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2CircleContact.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2Contact.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2ContactSolver.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp \
../External/Box2D/Box2D/Dynamics/Contacts/b2PolygonContact.cpp 

OBJS += \
./External/Box2D/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2CircleContact.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2Contact.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2ContactSolver.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.o \
./External/Box2D/Box2D/Dynamics/Contacts/b2PolygonContact.o 

CPP_DEPS += \
./External/Box2D/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2CircleContact.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2Contact.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2ContactSolver.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.d \
./External/Box2D/Box2D/Dynamics/Contacts/b2PolygonContact.d 


# Each subdirectory must supply rules for building sources it contributes
External/Box2D/Box2D/Dynamics/Contacts/%.o: ../External/Box2D/Box2D/Dynamics/Contacts/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DUSE_GL=1 -I"/home/test/Projects/PlayWork" -I"/home/test/Projects/PlayWork/External/Box2D" -I"/home/test/Projects/PlayWork/External/RakNet/Source" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


