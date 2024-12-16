################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libs/DCMotor_driver.c \
../libs/X4LIDAR_driver.c \
../libs/shell.c \
../libs/shell_drv_uart.c 

OBJS += \
./libs/DCMotor_driver.o \
./libs/X4LIDAR_driver.o \
./libs/shell.o \
./libs/shell_drv_uart.o 

C_DEPS += \
./libs/DCMotor_driver.d \
./libs/X4LIDAR_driver.d \
./libs/shell.d \
./libs/shell_drv_uart.d 


# Each subdirectory must supply rules for building sources it contributes
libs/%.o libs/%.su libs/%.cyclo: ../libs/%.c libs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"/Users/mariecaronello/Documents/2425_ESE_Projet_TagBot/CubeIDE/TagBot/libs" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-libs

clean-libs:
	-$(RM) ./libs/DCMotor_driver.cyclo ./libs/DCMotor_driver.d ./libs/DCMotor_driver.o ./libs/DCMotor_driver.su ./libs/X4LIDAR_driver.cyclo ./libs/X4LIDAR_driver.d ./libs/X4LIDAR_driver.o ./libs/X4LIDAR_driver.su ./libs/shell.cyclo ./libs/shell.d ./libs/shell.o ./libs/shell.su ./libs/shell_drv_uart.cyclo ./libs/shell_drv_uart.d ./libs/shell_drv_uart.o ./libs/shell_drv_uart.su

.PHONY: clean-libs

