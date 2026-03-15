#!/bin/bash

# Configuración: puerto del servidor y número de clientes simultáneos
PORT=8080
NUM_CLIENTS=400

echo "Lanzando $NUM_CLIENTS conexiones simultáneas al puerto $PORT..."

for i in $(seq 1 $NUM_CLIENTS); do
    # Usamos netcat (nc) para conectar. 
    # 'nc' es como telnet pero mejor para scripts [1].
    # El comando (sleep 1) mantiene la conexión abierta un segundo para forzar la concurrencia.
    (sleep 1) | nc localhost $PORT & 
done

# Esperamos a que todos los procesos terminen [3]
wait

echo "Test finalizado. Si el pool funciona, deberías haber visto las respuestas en la terminal del servidor."
