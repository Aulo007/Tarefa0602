#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Definições de pinos
#define SERVO_PIN 22  // Pino onde o servo está conectado
#define LED_PIN 12     // Pino onde o LED RGB está conectado

// Configuração do PWM para o servo
#define WRAP_VALUE 39062 // Define o valor de "wrap" para obter uma frequência de ~50Hz

// Definição de valores para posições do servo em microssegundos
#define SERVO_0_US 500    // 0 graus
#define SERVO_90_US 1470  // 90 graus
#define SERVO_180_US 2400 // 180 graus

// Função que converte microssegundos para um valor PWM dentro do range WRAP_VALUE
uint16_t us_to_pwm(uint16_t us) {
    return (uint16_t)((float)us * (float)WRAP_VALUE / 20000.0f);
}

// Configuração do PWM para o servo
void servo_pwm_setup() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);  // Define o pino do servo como saída PWM
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);  // Obtém o número do slice PWM do pino
    pwm_config config = pwm_get_default_config();  // Obtém a configuração padrão do PWM
    pwm_config_set_clkdiv(&config, 64.0f);  // Define o divisor de clock
    pwm_config_set_wrap(&config, WRAP_VALUE);  // Define o valor de wrap do PWM
    pwm_init(slice_num, &config, true);  // Inicializa o PWM no slice correspondente
}

// Configuração do PWM para o LED
void led_pwm_setup() {
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);  // Define o pino do LED como saída PWM
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);  // Obtém o número do slice PWM do pino
    pwm_config config = pwm_get_default_config();  // Obtém a configuração padrão do PWM
    pwm_config_set_clkdiv(&config, 1.0f);  // Define o divisor de clock para evitar flickering
    pwm_config_set_wrap(&config, 255);  // Define a resolução do PWM em 8 bits para controle do brilho
    pwm_init(slice_num, &config, true);  // Inicializa o PWM no slice correspondente
}

// Função para definir a posição do servo em microssegundos
void set_servo_position(uint16_t us) {
    pwm_set_gpio_level(SERVO_PIN, us_to_pwm(us));
}

// Função para definir o brilho do LED
void set_led_brightness(uint8_t brightness) {
    pwm_set_gpio_level(LED_PIN, brightness);
}

// Função que move suavemente o servo entre duas posições
void smooth_move(uint16_t start_us, uint16_t end_us) {
    int step = (start_us < end_us) ? 5 : -5;  // Define o incremento ou decremento da posição
    
    for (uint16_t pos = start_us; 
         (step > 0) ? (pos <= end_us) : (pos >= end_us); 
         pos += step) {
        set_servo_position(pos);  // Define a posição do servo
        
        // Calcula o brilho do LED baseado na posição do servo
        // Mapeia a posição do servo (500-2400) para o brilho do LED (0-255)
        uint8_t brightness = (uint8_t)(((float)(pos - SERVO_0_US) / 
                                      (float)(SERVO_180_US - SERVO_0_US)) * 255);
        set_led_brightness(brightness);
        
        sleep_ms(10);  // Pequeno atraso para suavizar o movimento
    }
}

// Função principal
int main() {
    stdio_init_all();  // Inicializa a comunicação serial
    servo_pwm_setup(); // Configura o PWM do servo
    led_pwm_setup();   // Configura o PWM do LED
    
    while (true) {
        // Move o servo para 180 graus e liga o LED no máximo
        printf("Movendo para 180 graus\n");
        set_servo_position(SERVO_180_US);
        set_led_brightness(255);
        sleep_ms(5000);
        
        // Move o servo para 90 graus e reduz o brilho do LED
        printf("Movendo para 90 graus\n");
        set_servo_position(SERVO_90_US);
        set_led_brightness(128);
        sleep_ms(5000);
        
        // Move o servo para 0 graus e apaga o LED
        printf("Movendo para 0 graus\n");
        set_servo_position(SERVO_0_US);
        set_led_brightness(0);
        sleep_ms(5000);
        
        // Movimento suave do servo de 0 a 180 graus
        printf("Iniciando movimento suave\n");
        smooth_move(SERVO_0_US, SERVO_180_US);
        sleep_ms(1000);
        smooth_move(SERVO_180_US, SERVO_0_US);
        sleep_ms(1000);
    }
}
