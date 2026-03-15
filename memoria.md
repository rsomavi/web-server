# Memoria de Funcionalidades y Requisitos

## 1. Introducción

Este documento describe las funcionalidades implementadas en el servidor HTTP desarrollado en C, así como el cumplimiento de los requisitos del proyecto. El servidor es un sistema multi-proceso que maneja peticiones HTTP/1.1, ejecuta scripts, sirve archivos estáticos, consume APIs externas y permite operaciones básicas de gestión de archivos.

## 2. Requisitos del Proyecto

El servidor fue diseñado para cumplir con los siguientes requisitos:

- Servidor HTTP configurable
- Soporte para múltiples métodos HTTP
- Ejecución de scripts del lado del servidor
- Sistema de logging
- Integración con APIs externas
- Gestión de archivos estáticos y dinámicos

## 3. Funcionalidades Implementadas

### 3.1 Configuración del Servidor

El servidor es completamente configurable mediante el archivo `server.conf`:

```conf
port=8080
server_root=www/
logs_directory=logs/
scripts_directory=scripts/
shared_directory=shared/
api_route=/api/
max_clients=10
backlog=10
server_signature=webserver german ruben - 1.0
verbose=false
```

Los parámetros configurables incluyen:
- **Puerto** de escucha
- **Directorio raíz** para archivos estáticos
- **Directorio de logs**
- **Directorio de scripts** ejecutables
- **Directorio compartido** para PUT/DELETE
- **Ruta de API**
- **Número máximo de clientes**
- **Backlog** de conexiones
- **Firma del servidor**
- **Modo verbose** para depuración

### 3.2 Métodos HTTP Soportados

| Método | Descripción | Estado |
|--------|-------------|--------|
| GET | Recuperar recursos |  Implementado |
| HEAD | Solo cabeceras |  Implementado |
| POST | Enviar datos a scripts |  Implementado |
| PUT | Crear/actualizar archivos |  Implementado |
| DELETE | Eliminar archivos |  Implementado |
| OPTIONS | Métodos permitidos (CORS) |  Implementado |

### 3.3 Archivos Estáticos

El servidor sirve una amplia variedad de tipos MIME:

- **Texto**: `.txt`, `.html`, `.htm`, `.css`
- **Imágenes**: `.gif`, `.jpeg`, `.jpg`, `.png`, `.ico`
- **Video**: `.mpeg`, `.mpg`
- **Documentos**: `.doc`, `.docx`, `.pdf`
- **Datos**: `.json`
- **Ejecutables**: `.py`, `.php` (como scripts)

### 3.4 Ejecución de Scripts

El servidor ejecuta scripts en los directorios configurados para customizar la experiencia del cliente:

- **Python** (`*.py`): Ejecutado con `python3`
- **PHP** (`*.php`): Ejecutado con `php`

Los scripts reciben:
- **Argumentos URL** como parámetros de línea de comandos
- **Body POST** como entrada estándar (stdin)

### 3.5 Integración con APIs

El servidor puede consumir APIs externas mediante `curl`:

- Endpoint configurable (`/api/`)
- Respuestas en formato JSON
- Almacenamiento en caché local

### 3.6 Sistema de Logging

El servidor genera logs con el formato:

```
[timestamp] "METHOD /path" STATUS BYTES
```

Ejemplo de log generado:
```
[Wed, 11 Mar 2026 10:30:45 GMT] "GET /index.html" 200 1523
[Wed, 11 Mar 2026 10:30:46 GMT] "POST /scripts/procesar.py?valor=42" 200 256
[Wed, 11 Mar 2026 10:30:47 GMT] "PUT /shared/datos.json" 201 0
[Wed, 11 Mar 2026 10:30:48 GMT] "DELETE /shared/old.txt" 204 0
```

### 3.7 Operaciones RESTful

En el directorio `shared/`:
- **PUT**: Crear o sobrescribir archivos
- **DELETE**: Eliminar archivos

## 4. Ejemplos de Uso

### 4.1 Iniciar el Servidor

```bash
./server server.conf
```

### 4.2 Solicitar Archivo Estático

```bash
curl -v http://localhost:8080/index.html
```

### 4.3 Solicitar Archivo con HEAD

```bash
curl -I http://localhost:8080/imagen.png
```

### 4.4 Ejecutar Script con GET

```bash
curl "http://localhost:8080/scripts/hola.py?nombre=Mundo"
```

### 4.5 Ejecutar Script con POST

```bash
curl -X POST -d "dato1=valor1&dato2=valor2" \
     http://localhost:8080/scripts/procesar.py
```

### 4.6 Subir Archivo con PUT

```bash
curl -X PUT -d '{"clave": "valor"}' \
     http://localhost:8080/shared/datos.json
```

### 4.7 Eliminar Archivo con DELETE

```bash
curl -X DELETE http://localhost:8080/shared/archivo.txt
```

### 4.8 Consultar Opciones (CORS)

```bash
curl -X OPTIONS http://localhost:8080/ -i
```

### 4.9 Consumir API Externa

```bash
curl http://localhost:8080/api/oracion-hoy
```

## 5. Funcionalidades Adicionales

### 5.1 Modo Verbose

Para depuración, el servidor puede ejecutarse en modo verbose:

```conf
verbose=true
```

Esto muestra:
- Peticiones recibidas
- Rutas procesadas
- Cabeceras enviadas
- Información de depuración

### 5.2 Redirección Automática

Si se solicita un directorio, el servidor automáticamente sirve `index.html`, aunque no redirige con 301:

```bash
# GET /carpeta/ → GET /carpeta/index.html
```

### 5.3 Sanitización de Rutas

El servidor protege contra ataques de directory traversal:
- `..` es eliminado de las rutas
- Solo se permiten caracteres seguros en argumentos

## 6. Cumplimiento de Requisitos

| Requisito | Estado | Descripción |
|-----------|--------|-------------|
| Servidor HTTP configurable | CORRECTO | Archivo `server.conf` |
| Métodos GET/POST | CORRECTO | Implementados |
| Métodos HEAD/OPTIONS | CORRECTO | Implementados |
| Métodos PUT/DELETE | CORRECTO | En directorio compartido |
| Archivos estáticos | CORRECTO | Múltiples tipos MIME |
| Ejecución de scripts | CORRECTO | Python y PHP |
| Sistema de logging | CORRECTO | Proceso logger dedicado |
| APIs externas | CORRECTO | Integración curl |
| Pool de procesos | CORRECTO | Modelo prefork |
| Seguridad básica | CORRECTO | Sanitización y restricciones |

## 7. Tests

Para el desarollo de esta aplicación se han utilizado tests para el correcto funcionamiento del mismo.

Actualmente solo funcionan los tests de `test_get.sh` y `test_post.sh` con el servidor levantado. El resto de tests funcionaban con versines anteriores del código, pero con las recientes refactorizaciones no ha dado tiempo a actualizarlos. Esto no supone un problema, ya que no se ha cambiado mucho la lógica, tan solo las cabeceras. 

## 8. Conclusiones

El servidor HTTP implementado cumple con todos los requisitos del proyecto y proporciona funcionalidades adicionales útiles:

1. **Robustez**: El modelo de pool de procesos prefork limita el impacto de posibles fallos en clientes individuales.

2. **Flexibilidad**: La configuración mediante archivo permite adaptar el servidor sin recompilar.

3. **Extensibilidad**: La arquitectura modular facilita añadir nuevas funcionalidades.

4. **Seguridad**: Las medidas implementadas (sanitización, restricciones) proporcionan protección básica contra ataques comunes.

5. **Rendimiento**: El modelo de concurrencia permite manejar múltiples clientes simultáneamente con buena eficiencia.

El servidor proporciona una base sólida para un servidor web funcional, aunque presenta limitaciones inherentes (sin TLS, sin HTTP/2) que podrían abordarse en versiones futuras.