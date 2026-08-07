# Sistemas-Embebidos-Tarea-6

# Descripción general del código

El código implementado gestiona la transición secuencial de varios protocolos de ahorro de energía y estados de suspensión propia de un microcontrolador ESP32. Utilizando una LED RGB, se indica visualmente el estado actual del sistema, un botón implementado como una fuente de interrupción externa y varias variables globales almacenadas en la memoria RTC a fin de conservarlos a pesar de los reinicios del sistema provocados por los modos de suspensión más profundos.

# Funcionamiento del sistema por estados

1. **Arranque y Verificación:** El sistema inicia los periféricos (Serial, LED, Botón) a la vez que incrementa la variable conteoArranques guardada en la memoria RTC, y comprueba cuál fue la causa del despertar (botón, temporizador o encendido).

2. **Estado Activo:** El sistema trabaja a máxima potencia (LED verde), ejecutando un bucle que suma un contador durante 10 segundos.

3. **Estado Modem Sleep:** Para este estado (LED azul), el Wi-fi intenta conectarse a una red para luego restringir la energía suministrada a la antena durante 6 segundos y entonces se desconecta y apaga la radio por completo.

4. **Estado Light Sleep:** El sistema entra en suspensión ligera (LED cyan) apagando parcialmente el CPU, pero conservando la memoria RAM. Despierta después de 8 segundos o si se presiona el botón, reanudando la ejecución del código exactamente donde se quedó.

5. **Estado Deep Sleep:** El sistema desactiva el CPU y la memoria RAM principal para un mayor ahorro (LED amarillo). Retiene únicamente la memoria RTC. Al despertar tras 10 segundos o por botón, el chip se reinicia completamente (vuelve al inicio del setup()).

6. **Estado Hibernación:** Al volver a encender desde el Deep Sleep, el sistema entra en hibernación (LED rojo). Aquí apaga incluso el oscilador principal y los periféricos del RTC, reduciendo el consumo al mínimo absoluto. Solo puede despertar mediante el botón físico para reiniciar el ciclo completo.
