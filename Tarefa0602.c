#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Definições para o servo
#define SERVO_PIN 22
#define WRAP_VALUE 39062 // Para ~50Hz

// Definições para o LED RGB
#define LED_PIN 12

// Valores para diferentes posições do servo (em microssegundos)
#define SERVO_0_US 500    // 0 graus
#define SERVO_90_US 1470  // 90 graus
#define SERVO_180_US 2400 // 180 graus

// Função para converter microssegundos em nível PWM
uint16_t us_to_pwm(uint16_t us) {
    return (uint16_t)((float)us * (float)WRAP_VALUE / 20000.0f);
}

// Configuração do PWM para o servo
void servo_pwm_setup() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 64.0f);
    pwm_config_set_wrap(&config, WRAP_VALUE);
    pwm_init(slice_num, &config, true);
}

// Configuração do PWM para o LED
void led_pwm_setup() {
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);
    pwm_config config = pwm_get_default_config();
    // Configuramos o LED com uma frequência mais alta para evitar flickering
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, 255); // 8-bit resolução para o LED
    pwm_init(slice_num, &config, true);
}

// Função para mover o servo para uma posição específica
void set_servo_position(uint16_t us) {
    pwm_set_gpio_level(SERVO_PIN, us_to_pwm(us));
}

// Função para definir o brilho do LED
void set_led_brightness(uint8_t brightness) {
    pwm_set_gpio_level(LED_PIN, brightness);
}

// Função para movimento suave entre posições
void smooth_move(uint16_t start_us, uint16_t end_us) {
    int step = (start_us < end_us) ? 5 : -5;
    
    for (uint16_t pos = start_us; 
         (step > 0) ? (pos <= end_us) : (pos >= end_us); 
         pos += step) {
        set_servo_position(pos);
        
        // Calcula o brilho do LED baseado na posição do servo
        // Mapeia a posição do servo (500-2400) para o brilho do LED (0-255)
        uint8_t brightness = (uint8_t)(((float)(pos - SERVO_0_US) / 
                                      (float)(SERVO_180_US - SERVO_0_US)) * 255);
        set_led_brightness(brightness);
        
        sleep_ms(10);
    }
}

int main() {
    stdio_init_all();
    servo_pwm_setup();
    led_pwm_setup();
    
    while (true) {
        // Posição 180 graus
        printf("Movendo para 180 graus\n");
        set_servo_position(SERVO_180_US);
        set_led_brightness(255); // LED no máximo
        sleep_ms(5000);
        
        // Posição 90 graus
        printf("Movendo para 90 graus\n");
        set_servo_position(SERVO_90_US);
        set_led_brightness(128); // LED na metade
        sleep_ms(5000);
        
        // Posição 0 graus
        printf("Movendo para 0 graus\n");
        set_servo_position(SERVO_0_US);
        set_led_brightness(0); // LED apagado
        sleep_ms(5000);
        
        // Movimento suave de 0 a 180 graus
        printf("Iniciando movimento suave\n");
        smooth_move(SERVO_0_US, SERVO_180_US);
        sleep_ms(1000);
        smooth_move(SERVO_180_US, SERVO_0_US);
        sleep_ms(1000);
    }
}