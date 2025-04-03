#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/altcp_tls.h"
#include "example_http_client_util.h"

#define HOST "serverpico.onrender.com"
#define URL_REQUEST "/mensagem?msg="

#define SENSOR_TEMP 4  // Canal do sensor de temperatura interno
#define BUTTON_A 5      // GPIO do botão A

#define BUFFER_SIZE 512

bool is_fahrenheit = false;
bool last_button_state = true; // Estado anterior do botão

// Função para codificar URL corretamente
void urlencode(const char *input, char *output, size_t output_size) {
    char hex[] = "0123456789ABCDEF";
    size_t j = 0;
    for (size_t i = 0; input[i] && j + 3 < output_size; i++) {
        if ((input[i] >= 'A' && input[i] <= 'Z') || 
            (input[i] >= 'a' && input[i] <= 'z') || 
            (input[i] >= '0' && input[i] <= '9') || 
            input[i] == '-' || input[i] == '_' || input[i] == '.' || input[i] == '~') {
            output[j++] = input[i];
        } else {
            output[j++] = '%';
            output[j++] = hex[(input[i] >> 4) & 0xF];
            output[j++] = hex[input[i] & 0xF];
        }
    }
    output[j] = '\0';
}

// Envia os dados para o servidor
void send_data(const char* data) {
    char encoded_data[BUFFER_SIZE];
    urlencode(data, encoded_data, BUFFER_SIZE);

    char full_url[BUFFER_SIZE];
    snprintf(full_url, BUFFER_SIZE, "%s%s", URL_REQUEST, encoded_data);

    EXAMPLE_HTTP_REQUEST_T req = {0};
    req.hostname = HOST;
    req.url = full_url;
    req.tls_config = altcp_tls_create_config_client(NULL, 0);
    req.headers_fn = http_client_header_print_fn;
    req.recv_fn = http_client_receive_print_fn;

    printf("Enviando dados: %s\n", data);
    int result = http_client_request_sync(cyw43_arch_async_context(), &req);

    altcp_tls_free_config(req.tls_config);
}

// Inicializa os componentes de hardware
void init_hardware() {
    stdio_init_all();
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(SENSOR_TEMP);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A); // Habilita pull-up interno
}

// Lê a temperatura
float read_temperature() {
    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / 4095.0f;
    return 27 - (voltage - 0.706) / 0.001721;
}

// Envia a temperatura
void send_temperature() {
    float temp_c = read_temperature();
    float temp_f = temp_c * 9 / 5 + 32;
    
    char msg[BUFFER_SIZE];
    snprintf(msg, BUFFER_SIZE, "Temperatura: %.2f %s", 
             is_fahrenheit ? temp_f : temp_c,
             is_fahrenheit ? "°F" : "°C");

    send_data(msg);
}

// Envia o estado do botão
void send_button_state(bool pressed) {
    char msg[BUFFER_SIZE];
    snprintf(msg, BUFFER_SIZE, "Botão %s | Modo: %s", 
             pressed ? "pressionado" : "solto",
             is_fahrenheit ? "Fahrenheit" : "Celsius");

    send_data(msg);
}

int main() {
    init_hardware();
    sleep_ms(2000); // Tempo para inicialização

    if (cyw43_arch_init()) {
        printf("Falha ao inicializar Wi-Fi\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha na conexão Wi-Fi\n");
        return 1;
    }

    printf("Conectado! Iniciando leitura...\n");

    while (true) {
        // Verifica o estado do botão
        bool button_state = gpio_get(BUTTON_A) == 0;

        // Se houve mudança no estado do botão, alterna entre Celsius/Fahrenheit e envia o estado
        if (button_state != last_button_state) {
            if (button_state) { // Se pressionado, alterna o modo
                is_fahrenheit = !is_fahrenheit;
                printf("Botão pressionado! Agora está em %s\n", is_fahrenheit ? "Fahrenheit" : "Celsius");
            }
            send_button_state(button_state);
            last_button_state = button_state;
        }

        // Envia a temperatura a cada 1 segundo
        send_temperature();

        sleep_ms(1000);
    }

    cyw43_arch_deinit();
    return 0;
}
