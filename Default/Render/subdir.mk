################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Render/GLRender.cpp 

OBJS += \
./Render/GLRender.o 

CPP_DEPS += \
./Render/GLRender.d 


# Each subdirectory must supply rules for building sources it contributes
Render/%.o: ../Render/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++1y -DUSE_GL=1 -I"/home/odd/Projects/PlayWork" -I../../RakNet/Source -I../../Box2D -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


