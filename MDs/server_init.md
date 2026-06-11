# `Server::init()` — Documentación Detallada

## Descripción general

`Server::init()` es el **punto de entrada del servidor IRC**. Es la función más importante del sistema porque es la responsable de transformar el objeto `Server` (vacío tras el constructor) en un servidor TCP funcional, capaz de aceptar conexiones, leer datos y responder a múltiples clientes de forma simultánea y no bloqueante.

Se llama **una única vez** desde `main()` y no retorna hasta que el servidor se apaga.

```
main() ──► Server::init() ──► _serverLoop()  [bucle infinito hasta señal]
```

---

## Firma de la función

```cpp
void Server::init(const std::string &port, const std::string &password)
```

| Parámetro  | Tipo                | Descripción                                              |
|------------|---------------------|----------------------------------------------------------|
| `port`     | `const std::string` | Puerto en el que escuchará el servidor (ej: `"6667"`)    |
| `password` | `const std::string` | Contraseña que los clientes deben enviar con `PASS`      |

Ambos vienen como `std::string` porque provienen directamente de `argv[]`. El puerto se convierte a entero internamente.

---

## Paso a paso

---

### 1. Almacenamiento de configuración básica

```cpp
_port     = std::stoi(port);
_password = password;
_hostname = "localhost";
```

#### `_port`
- **Valor guardado:** el número de puerto convertido de string a entero (ej: `6667`).
- **Por qué:** el sistema operativo necesita un `int` para configurar el socket. `std::stoi` hace la conversión. Este valor se usará en `htons(_port)` al configurar `_confSocket`.

#### `_password`
- **Valor guardado:** la contraseña en texto plano tal como la recibe el servidor (ej: `"1234"`).
- **Por qué:** cada vez que un cliente se conecte y envíe el comando `PASS <password>`, el handler comparará su input con este valor. Si no coincide, la conexión se rechaza.

#### `_hostname`
- **Valor guardado:** `"localhost"` (o el resultado de `gethostname()` si se usa `_getHostname()`).
- **Por qué:** el hostname aparece en todos los mensajes IRC que el servidor envía hacia los clientes. Por ejemplo: `:localhost 001 nick :Welcome`. Identifica al servidor como emisor del mensaje.

---

### 2. Creación del socket

```cpp
_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
if (_serverSocket == -1)
{
    std::cerr << "Error creating socket" << std::endl;
    exit(EXIT_FAILURE);
}
```

#### `socket(AF_INET, SOCK_STREAM, 0)`

| Argumento     | Valor       | Significado                                      |
|---------------|-------------|--------------------------------------------------|
| `AF_INET`     | familia     | IPv4                                             |
| `SOCK_STREAM` | tipo        | TCP (orientado a conexión, fiable, ordenado)     |
| `0`           | protocolo   | El SO elige automáticamente (TCP en este caso)   |

- **Retorna:** un file descriptor (entero ≥ 0) que representa el socket, o `-1` si falla.

#### `_serverSocket`
- **Valor guardado:** el file descriptor del socket del servidor (ej: `3`, `4`...).
- **Por qué:** este fd es el "oído" del servidor. Todo pasa por él: `bind`, `listen`, `accept`. Es el fd que se monitoriza con `poll()` para detectar nuevas conexiones entrantes. Se guarda como atributo porque lo necesitan múltiples funciones (`_closeFds`, `_serverLoop`, etc.).

#### Error fatal con `exit()`
Si `socket()` falla no tiene sentido continuar — sin socket no hay servidor. Se usa `exit(EXIT_FAILURE)` en lugar de `throw` porque este error ocurre antes de que haya cualquier recurso que limpiar.

---

### 3. Reutilización de dirección — `SO_REUSEADDR`

```cpp
int opt = 1;
if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    throw std::runtime_error("Error setting socket options");
```

#### ¿Qué hace `SO_REUSEADDR`?
Cuando un servidor se cierra, el puerto puede quedar en estado `TIME_WAIT` durante ~60 segundos (comportamiento TCP). Sin esta opción, al reiniciar el servidor inmediatamente aparece el error `Address already in use`.

Con `SO_REUSEADDR = 1` el SO permite que el proceso reutilice ese puerto de inmediato.

#### ¿Por qué antes de `bind`?
`setsockopt` debe llamarse **antes** de `bind()`. Si se llama después, la opción no tiene efecto para esa sesión.

#### `opt = 1`
El valor `1` significa "activar". Se pasa como puntero a `void*` porque `setsockopt` es genérico y acepta cualquier tipo de opción.

---

### 4. Modo no bloqueante — `fcntl`

```cpp
if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == -1)
    throw std::runtime_error("Error setting non-blocking mode");
```

#### ¿Qué hace `O_NONBLOCK`?
Por defecto, las llamadas al sistema como `accept()` o `recv()` **bloquean** el proceso hasta que hay datos disponibles. Con `O_NONBLOCK`, si no hay datos listos, la llamada retorna inmediatamente con `errno = EAGAIN` en lugar de congelar el proceso.

#### ¿Por qué es esencial para un servidor IRC?
El servidor maneja múltiples clientes en un **único hilo**. Si una llamada bloqueara, ningún otro cliente podría ser atendido mientras tanto. El modo no bloqueante combinado con `poll()` es lo que permite el modelo de I/O multiplexado.

#### Flujo resultante:
```
poll() detecta actividad ──► llamada no bloqueante ──► procesa ──► vuelve a poll()
```

---

### 5. Configuración de la dirección — `sockaddr_in`

```cpp
_confSocket.sin_family      = AF_INET;
_confSocket.sin_addr.s_addr = INADDR_ANY;
_confSocket.sin_port        = htons(_port);
```

`_confSocket` es un `struct sockaddr_in` que describe en qué dirección y puerto debe escuchar el servidor.

| Campo                 | Valor        | Significado                                                           |
|-----------------------|--------------|-----------------------------------------------------------------------|
| `sin_family`          | `AF_INET`    | IPv4 (debe coincidir con el socket creado)                            |
| `sin_addr.s_addr`     | `INADDR_ANY` | Escucha en **todas** las interfaces de red de la máquina (0.0.0.0)   |
| `sin_port`            | `htons(_port)` | Puerto en **network byte order** (big-endian)                       |

#### ¿Por qué `htons()`?
Los procesadores x86 usan **little-endian** (byte menos significativo primero), pero los protocolos de red usan **big-endian**. `htons` (*host to network short*) convierte el puerto al orden correcto. Sin esta conversión el servidor escucharía en un puerto completamente diferente.

#### `INADDR_ANY`
Permite conexiones desde cualquier interfaz (localhost, red local, internet). Útil para desarrollo ya que no hay que especificar la IP de la máquina.

---

### 6. `bind` — Asociar socket a dirección

```cpp
if (bind(_serverSocket, (struct sockaddr *)&_confSocket, sizeof(_confSocket)) == -1)
    throw std::runtime_error("Error binding socket to port");
```

`bind()` asocia el socket creado con la dirección y puerto configurados en `_confSocket`. A partir de aquí, el SO sabe que las conexiones entrantes al puerto `_port` deben ir a `_serverSocket`.

El cast `(struct sockaddr *)` es necesario porque `bind` acepta la versión genérica de la estructura; `sockaddr_in` es la versión específica para IPv4.

**Errores comunes que `bind` detecta:**
- Puerto ya en uso (otro proceso lo tiene)
- Puerto < 1024 sin permisos de root
- Interfaz de red no disponible

---

### 7. `listen` — Poner el socket en modo escucha

```cpp
if (listen(_serverSocket, 10) == -1)
    throw std::runtime_error("Error listening on socket");
```

`listen()` transforma el socket de un socket activo a un socket pasivo, listo para aceptar conexiones.

| Argumento | Valor | Significado                                                                 |
|-----------|-------|-----------------------------------------------------------------------------|
| fd        | `_serverSocket` | El socket a poner en escucha                                      |
| backlog   | `10`  | Tamaño de la cola de conexiones pendientes de aceptar (antes de `accept()`) |

El backlog de `10` significa que hasta 10 clientes pueden estar esperando en cola mientras el servidor procesa otro `accept()`. Para un IRC de prácticas es más que suficiente (`SOMAXCONN` sería el máximo del sistema).

Tras `listen()`, el socket ya puede recibir conexiones, pero no las acepta todavía — eso lo hace `accept()` dentro del loop.

---

### 8. Registro en `poll` — `pollfd`

```cpp
struct pollfd serverPollFd;
serverPollFd.fd      = _serverSocket;
serverPollFd.events  = POLLIN;
serverPollFd.revents = 0;

_fds.push_back(serverPollFd);
```

`poll()` necesita un array de `struct pollfd` para saber qué file descriptors monitorizar y qué eventos le interesan de cada uno.

| Campo     | Valor          | Significado                                                             |
|-----------|----------------|-------------------------------------------------------------------------|
| `fd`      | `_serverSocket`| El fd a vigilar                                                         |
| `events`  | `POLLIN`       | Avisar cuando haya datos **entrantes** listos para leer                 |
| `revents` | `0`            | Campo de salida — el SO lo rellena; se inicializa a 0                   |

#### `_fds`
- **Valor guardado:** el `pollfd` del socket del servidor como **primer elemento** del vector.
- **Por qué es el primero:** en `_serverLoop()`, se comprueba `_fds[0]` para detectar nuevas conexiones. Los clientes se añaden a partir de `_fds[1]` en adelante. Esta convención mantiene el código simple.

#### ¿Por qué `POLLIN` y no `POLLOUT`?
El servidor nunca inicia comunicación espontáneamente — solo responde. Solo necesita saber cuándo **llegan** datos o conexiones. `POLLOUT` se usaría si hubiera que saber cuándo el socket está listo para escribir (no necesario aquí).

---

### 9. Registro de señales

```cpp
_addServerSignal();
```

Registra los manejadores para `SIGINT` (Ctrl+C) y `SIGQUIT` (Ctrl+\\). Internamente, el handler pone `_signal = true`, lo que hace que el loop principal termine limpiamente en lugar de ser interrumpido de forma abrupta.

Se llama **antes** de `_serverLoop()` para garantizar que el servidor puede apagarse limpiamente desde el primer instante.

---

### 10. Arranque del loop principal

```cpp
_serverLoop();
```

Última línea de `init()`. Llama al bucle principal del servidor, que **no retorna** hasta que `_signal` se pone a `true`. A partir de aquí el servidor está completamente operativo.

---

## Flujo completo resumido

```
init()
  │
  ├─► Guardar puerto, contraseña, hostname
  │
  ├─► socket()        → crear el fd del servidor
  │
  ├─► setsockopt()    → permitir reutilizar el puerto al reiniciar
  │
  ├─► fcntl()         → modo no bloqueante (necesario para poll)
  │
  ├─► sockaddr_in     → configurar IP (todas) y puerto
  │
  ├─► bind()          → asociar socket a esa IP:puerto
  │
  ├─► listen()        → empezar a aceptar conexiones (cola de 10)
  │
  ├─► pollfd          → registrar el socket en el vector de poll
  │
  ├─► _addServerSignal() → manejar Ctrl+C limpiamente
  │
  └─► _serverLoop()   → bucle infinito (aquí vive el servidor)
```

---

## Atributos modificados por esta función

| Atributo        | Tipo                      | Valor tras `init()`                              |
|-----------------|---------------------------|--------------------------------------------------|
| `_port`         | `int`                     | Puerto como entero (ej: `6667`)                  |
| `_password`     | `std::string`             | Contraseña en texto plano                        |
| `_hostname`     | `std::string`             | `"localhost"`                                    |
| `_serverSocket` | `int`                     | File descriptor del socket del servidor          |
| `_confSocket`   | `struct sockaddr_in`      | IP (`INADDR_ANY`) y puerto configurados          |
| `_fds`          | `std::vector<pollfd>`     | Un elemento: el `pollfd` del socket del servidor |

---

## Dependencias internas

| Función llamada       | Cuándo        | Propósito                                      |
|-----------------------|---------------|------------------------------------------------|
| `_addServerSignal()`  | Antes del loop | Registrar `SIGINT`/`SIGQUIT`                  |
| `_serverLoop()`       | Al final       | Bucle principal de `poll()` + dispatch         |