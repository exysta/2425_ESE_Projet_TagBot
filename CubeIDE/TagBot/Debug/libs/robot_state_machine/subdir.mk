################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libs/robot_state_machine/robot_state_machine.c 

OBJS += \
./libs/robot_state_machine/robot_state_machine.o 

C_DEPS += \
./libs/robot_state_machine/robot_state_machine.d 


# Each subdirectory must supply rules for building sources it contributes
libs/robot_state_machine/%.o libs/robot_state_machine/%.su libs/robot_state_machine/%.cyclo: ../libs/robot_state_machine/%.c libs/robot_state_machine/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/exysta/Desktop/Work/ENSEA/3A/2425_ESE_Projet_TagBot/CubeIDE/TagBot/libs" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-libs-2f-robot_state_machine

clean-libs-2f-robot_state_machine:
	-$(RM) ./libs/robot_state_machine/robot_state_machine.cyclo ./libs/robot_state_machine/robot_state_machine.d ./libs/robot_state_machine/robot_state_machine.o ./libs/robot_state_machine/robot_state_machine.su

.PHONY: clean-libs-2f-robot_state_machine

