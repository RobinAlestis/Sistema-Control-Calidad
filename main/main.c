#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

// Definición de pines (ajustar según hardware real)
#define SENSOR_DIMENSION_PIN GPIO_NUM_4
#define SENSOR_WEIGHT_PIN GPIO_NUM_5
#define ACTUATOR_REJECT_PIN GPIO_NUM_18

// Umbrales para control de calidad
#define DIMENSION_MIN 95  // Dimensión mínima en mm
#define DIMENSION_MAX 105 // Dimensión máxima en mm
#define WEIGHT_MIN 500    // Peso mínimo en gramos
#define WEIGHT_MAX 600    // Peso máximo en gramos

// Probabilidad de no añadir datos nuevos (20%)
#define SKIP_PROBABILITY 20

// Tamaño del buffer cíclico
#define BUFFER_SIZE 5

// Cola para comunicación entre tareas
static QueueHandle_t quality_check_queue = NULL;

// Semáforos para protección
static SemaphoreHandle_t buffer_semaphore = NULL;
static SemaphoreHandle_t count_semaphore = NULL;

// Variables para conteo de piezas
static volatile int accepted_count = 0;
static volatile int rejected_count = 0;

// Estructura para pasar los datos
typedef struct {
    int dimension;
    int weight;
} piece_data_t;

// Buffer cíclico para almacenar datos generados
static piece_data_t test_data[BUFFER_SIZE];
static int buffer_start = 0;
static int buffer_end = 0;
static int buffer_count = 0;

// Función para generar un dato nuevo aleatorio
void generate_test_data(void *pvParameters) {
    while (1) {
        // Intentar generar datos aleatoriamente con una probabilidad de SKIP_PROBABILITY
        if ((rand() % 100) < SKIP_PROBABILITY) {
            printf("generate_test_data: No se generaron nuevos datos en este ciclo.\n");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar antes del próximo ciclo
            continue;
        }

        // Generar datos aleatorios dentro de los rangos especificados
        piece_data_t new_data = {
            .dimension = 90 + rand() % 21, // Entre 90 y 110 mm
            .weight = 480 + rand() % 141  // Entre 480 y 620 g
        };

        // Proteger acceso al buffer cíclico con un semáforo
        xSemaphoreTake(buffer_semaphore, portMAX_DELAY);
        if (buffer_count < BUFFER_SIZE) {
            // Añadir datos al buffer cíclico
            test_data[buffer_end] = new_data;
            buffer_end = (buffer_end + 1) % BUFFER_SIZE;
            buffer_count++;
            printf("generate_test_data: Dato generado - Dimensión: %d mm, Peso: %d g\n", new_data.dimension, new_data.weight);
        } else {
            printf("generate_test_data: Buffer lleno, no se añadieron nuevos datos.\n");
        }
        xSemaphoreGive(buffer_semaphore);

        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar antes del próximo ciclo
    }
}

// Tarea para leer todos los datos del buffer y enviarlos a la cola
void sensor_task(void *pvParameters) {
    while (1) {
        xSemaphoreTake(buffer_semaphore, portMAX_DELAY);
        if (buffer_count > 0) {
            printf("sensor_task: Procesando %d datos del buffer.\n", buffer_count);

            while (buffer_count > 0) {
                // Leer datos del buffer cíclico
                piece_data_t data = test_data[buffer_start];
                buffer_start = (buffer_start + 1) % BUFFER_SIZE;
                buffer_count--;

                // Enviar datos a la cola
                if (xQueueSend(quality_check_queue, &data, portMAX_DELAY) != pdPASS) {
                    printf("sensor_task: Error al enviar datos a la cola.\n");
                }
            }

            printf("sensor_task: Todos los datos procesados y enviados a la cola.\n");
        } else {
            printf("sensor_task: No hay datos en el buffer para procesar.\n");
        }
        xSemaphoreGive(buffer_semaphore);

        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar antes del próximo ciclo
    }
}

// Tarea para analizar los datos (control de calidad)
void quality_check_task(void *pvParameters) {
    piece_data_t data;
    while (1) {
        // Esperar datos de la cola (bloqueante)
        if (xQueueReceive(quality_check_queue, &data, portMAX_DELAY)) {
            printf("Evaluando - Dimensión: %d, Peso: %d (Límites: %d-%d, %d-%d)\n",
                   data.dimension, data.weight, DIMENSION_MIN, DIMENSION_MAX, WEIGHT_MIN, WEIGHT_MAX);

            // Comprobar si la pieza cumple los requisitos
            if (data.dimension >= DIMENSION_MIN && data.dimension <= DIMENSION_MAX &&
                data.weight >= WEIGHT_MIN && data.weight <= WEIGHT_MAX) {
                xSemaphoreTake(count_semaphore, portMAX_DELAY);
                accepted_count++;
                xSemaphoreGive(count_semaphore);
                printf("Pieza ACEPTADA - Dimensión: %d mm, Peso: %d g\n", data.dimension, data.weight);
            } else {
                xSemaphoreTake(count_semaphore, portMAX_DELAY);
                rejected_count++;
                xSemaphoreGive(count_semaphore);
                printf("Pieza RECHAZADA - Dimensión: %d mm, Peso: %d g\n", data.dimension, data.weight);

                // Activar actuador para rechazar la pieza
                gpio_set_level(ACTUATOR_REJECT_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(500)); // Activar actuador por 0.5 segundos
                gpio_set_level(ACTUATOR_REJECT_PIN, 0);
            }

            // Imprimir conteo de piezas buenas y malas
            printf("Piezas aceptadas: %d, Piezas rechazadas: %d\n", accepted_count, rejected_count);
        }
    }
}

// Configuración inicial del hardware
void app_main(void) {
    // Configuración del pin del actuador
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << ACTUATOR_REJECT_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Crear la cola para la comunicación
    quality_check_queue = xQueueCreate(10, sizeof(piece_data_t));
    if (quality_check_queue == NULL) {
        printf("Error: No se pudo crear la cola.\n");
        return;
    }

    // Crear semáforos
    buffer_semaphore = xSemaphoreCreateMutex();
    count_semaphore = xSemaphoreCreateMutex();
    if (buffer_semaphore == NULL || count_semaphore == NULL) {
        printf("Error: No se pudieron crear los semáforos.\n");
        return;
    }

    // Crear tareas
    xTaskCreate(generate_test_data, "generate_test_data", 2048, NULL, 5, NULL);
    xTaskCreate(sensor_task, "sensor_task", 2048, NULL, 5, NULL);
    xTaskCreate(quality_check_task, "quality_check_task", 2048, NULL, 5, NULL);

    printf("Sistema de control de calidad iniciado.\n");
}

