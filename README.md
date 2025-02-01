# Sistema-Control-Calidad

Descripción del Proyecto

Este documento describe el desarrollo e implementación de un sistema embebido en tiempo real para el control de calidad de piezas en una línea de producción. El sistema emplea sensores para medir las dimensiones y peso de las piezas, determinando su aceptación o rechazo según criterios predefinidos. Se utiliza FreeRTOS para la gestión de tareas concurrentes, asegurando un procesamiento eficiente.

Objetivos del Sistema

Implementar un sistema embebido para la inspección automatizada de piezas.

Evaluar la calidad de cada pieza en función de sus dimensiones y peso.

Garantizar que solo las piezas que cumplen con los estándares sean aprobadas.

Optimizar la línea de producción mediante un proceso de inspección automatizado.

Requisitos del Sistema

Capacidad de Producción: 48 piezas por minuto.

Dimensiones Permitidas: 95 mm ± 10 mm.

Peso Permitido: 500 g ± 100 g.

Precisión:

Dimensiones: ±0.01 mm a ±0.1 mm.

Peso: ±0.1 g a ±1 g.

Tiempo de Procesamiento:

Máximo de 1 segundo por pieza.

Actuador de rechazo con tiempo de respuesta inferior a 500 ms.

Componentes Principales

1. Microcontrolador (ESP32 con FreeRTOS)

Coordina la ejecución de las tareas concurrentes.

Gestiona la captura y el procesamiento de datos.

2. Sensores

Sensores de Dimensiones: Barreras de luz (fotodiodos y LED).

Sensores de Peso: Celdas de carga con precisión de hasta 6000 g.

3. Actuadores

Sistema de Rechazo: Expulsión automática de piezas defectuosas.

Control de Línea: Posibilidad de detener la producción en caso de fallas repetitivas.

4. Sistema Operativo en Tiempo Real (FreeRTOS)

Administra tareas como adquisición de datos, evaluación y control de actuadores.

Manejo de concurrencia y tiempos de respuesta garantizados.

Flujo de Funcionamiento

Adquisición de Datos: Los sensores miden el peso y las dimensiones de la pieza.

Procesamiento: El microcontrolador analiza los datos en tiempo real.

Toma de Decisiones: Se verifica si la pieza cumple con los parámetros establecidos.

Acción: Si la pieza es defectuosa, se activa el actuador de rechazo.

Registro: Se almacenan datos sobre piezas aceptadas y rechazadas para análisis posterior.

Pruebas de Aceptación

Para validar el correcto funcionamiento del sistema, se realizaron las siguientes pruebas:

Validación de Sensores: Comparación de mediciones con patrones de referencia certificados.

Prueba de Rechazo: Introducción controlada de piezas fuera de especificación para verificar la respuesta del sistema.

Prueba de Rendimiento: Evaluación del sistema en condiciones de carga máxima para medir la latencia en la detección y actuación.

Conclusión

Este sistema embebido en tiempo real proporciona una solución automatizada, precisa y eficiente para el control de calidad en líneas de producción industriales. Su arquitectura modular y su capacidad de procesamiento en tiempo real permiten adaptaciones para diversos entornos de manufactura, mejorando la eficiencia operativa y la calidad del producto final.

Autor

Jesús Sedeño Perdigones

Curso: Sistemas Embebidos en Tiempo Real
