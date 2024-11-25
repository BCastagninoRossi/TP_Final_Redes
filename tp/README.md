# Proyecto de Redes en C

Este proyecto es un proyecto de redes en C que incluye módulos para el análisis de texto, la creación de servidores TCP y más. El proyecto ha sido reestructurado para tener una estructura profesional y bien organizada.

## Comenzando

### Prerrequisitos

- GCC (GNU Compiler Collection)
- libcurl
- libjson-c

### Instalación de Dependencias

En sistemas basados en Debian/Ubuntu, puedes instalar las dependencias necesarias con los siguientes comandos:

```sh
sudo apt-get update
sudo apt-get install gcc libcurl4-openssl-dev libjson-c-dev
```

## Estructura del Proyecto

El proyecto está organizado de la siguiente manera:

```
tp/
├── include/        # Archivos de cabecera
├── parser/         # Código fuente del parser
├── server/         # Código fuente del servidor
├── utils/          # Utilidades y scripts
├── src/            # Código fuente principal
├── Makefile        # Archivo de construcción
└── README.md       # Documentación del proyecto
```

### Construyendo el Proyecto

Para construir el proyecto, navega al directorio `tp` y ejecuta el siguiente comando:

```sh
make
```

### Ejecutando el Proyecto

Después de construir el proyecto, puedes ejecutar el servidor usando el siguiente comando:

```sh
./main
```

### Limpiando el Proyecto

Para limpiar los archivos generados después de la compilación, puedes usar el siguiente comando:

```sh
make clean
```

## Uso

El proyecto incluye un servidor TCP que escucha conexiones entrantes en el puerto 15001. Los clientes pueden enviar mensajes al servidor, que procesará los mensajes y registrará los resultados en el syslog.


### Enviando Mensajes

Desde una terminal distinta a aquella en la que se encuentra en ejecución el archivo `main` puedes usar el comando `nc` (netcat) para enviar mensajes al servidor. Aquí hay algunos ejemplos:

```sh
echo -e "usuario1\x02timestamp1\x02mensaje1\x04" | nc localhost 15001
echo -e "usuario2\x02timestamp2\x02mensaje2\x04usuario3\x02timestamp3\x02mensaje3\x04" | nc localhost 15001
echo -e "usuario4\x02timestamp4\x02mensaje4\x04usuario5\x02timestamp5\x02mensaje5\x04usuario6\x02timestamp6\x02mensaje6\x04" | nc localhost 15001
echo -e "usuario7\x02timestamp7\x02mensaje7_incomplete" | nc localhost 15001
```

También puedes enviar un archivo binario `.bin` al servidor. Para crear un archivo binario de prueba, puedes usar el archivo `make_bin.py` presente en la carpeta `utils`. Aquí hay un ejemplo de cómo usarlo:

```sh
python3 utils/make_bin.py
cat test_case_FINAL.bin | nc localhost 15001
```

Finalmente, en la carpeta `tests` hay 2 carpetas, provistas por la cátedra, que contienen código de prueba. Para utilizarlo se puede navegar hasta cada una de las carpetas mediante una terminal (distinta a aquella en la que se encuentra corriendo el servidor), compilar el archivo utilizando el comando:
```sh
make clean
```
y siguiendo las instrucciones de ejecución que se encuentran en el archivo README dentro de cada carpeta de tests.


### Visualizando SysLogs

A lo largo de la ejecución del servidor, mientras recibe mensajes, se mostrarán mensajes de confirmación y/o error en la terminal.

También, cada interacción, tanto exitosa como con errores, será registrada en el syslog.

Para visualizar las entradas relevantes en el log, podemos ejecutar el siguiente comando en Linux:

```sh
sudo tail -f /var/log/syslog
```

Donde se visualizarán las ultimas entradas en el Syslog.