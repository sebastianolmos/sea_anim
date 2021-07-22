# Animación del movimiento de una superficie fluida

## Descripción
Tarea 3 realizado en el curso CC7515-1 Computación en GPU. Corresponde a una escena que simula una superficie fluida en tiempo real donde el usuario puede manipular sus parametros, ademas de agregar algun modelo 3D.

## Video
[![Alt text for your video](https://img.youtube.com/vi/0vI7StoU8IM/0.jpg)](https://youtu.be/0vI7StoU8IM)

## Librerías usadas
Se uso el lenguaje C++ con la siguientes librerias:
- [Glad](https://glad.dav1d.de/) : Libreria necesaria para cargar los punteros a funciones de OpenGL. En este proyecto se uso OpenGL 4.5
- [GLFW3](https://www.glfw.org/) : Libreria usada con OpenGl que provee una API para manejar ventanas
- [GLM](https://glm.g-truc.net/0.9.9/index.html) : Libreria con funciones matematicas utiles para el uso de aplicaciones con OpenGL
- [Stb-image](https://github.com/nothings/stb) : Libreria para poder cargar texturas
- [Assimp](http://assimp.org/): Libreria para poder cargar modelos 3D
- [Dear Imgui](https://github.com/ocornut/imgui): Libreria para poder agregar un menu configurable

## Como se instalaron las librerías
A continuación se darán los pasos con las que se pudo instalar las diferentes librerías para poder usarlas en el programa Visual Studio 2019:
- Se tiene que configurar el proyecto en VS, creando las carpetas /Libraries/include y /Libraries/lib si no se encuentran
- Seleccionar la plataforma x64 en el editor
- Ir a configuraciones del proyecto y seleccionar en Platform: All Platforms
- Ir a VC++ Directories -> Include Directories -> Edit -> new -> ... -> seleccionar la carpeta project/Libraries/include -> ok
- Ir a VC++ Directories -> Library Directories -> Edit -> new -> ... -> seleccionar la carpeta project/Libraries/lib -> ok
- Ir a Linker -> Input -> Additional Dependencies -> Edit -> poner en el campo de texto:
```
glfw3.lib
opengl32.lib
```
Luego para instalar las diferentes librerías:
- [Glad](https://glad.dav1d.de/) : Descargar la version OpenGL/GLAD (version 4.5 Core), abrir glad.zip -> ir a /include y copiar carpetas "glad" y "KHR" a la carpeta del proyecto /Libraries/include. Del mismo zip -> ir a /src y copiar el archivo "glad.c" en la carpeta raíz del proyecto.
- [GLFW3](https://www.glfw.org/) : Descargar, y compilar con Cmake en una carpeta build, ir a ../build/src/Debug y copiar el archivo "glfw3.lib" a la carpeta del proyecto Libraries/lib. Ir a ../include y copiar la carpeta "GLFW" a la carpeta del proyecto Libraries/include
- [GLM](https://glm.g-truc.net/0.9.9/index.html) : Descargar, descomprimir y copiar directorio que sea raíz de glm.h y pegarla en Libraries/include
- [Stb-image](https://github.com/nothings/stb) : Descargar header stb_image.h y copiar en Libraries/include. Luego crear archivo stb.cpp y copiarlo en la raíz del proyecto ya que es necesario su compilación. Debe contener lo siguiente:
```
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
```
- [Assimp](http://assimp.org/): Descargar y compilar con CMake, agregar config.h desde build/ en include/. Agregar archivo assimp-vc140-mt.dll en la raíz del proyecto
- [Dear Imgui](https://github.com/ocornut/imgui): Descargar los archivos y descomprimir. incorporar los archivos del directorio base y de la versión a usar (openGL3.* y glfw, headers y cpp que se encuentran en la carpeta backends) al proyecto directamente. Importante utilizar la versión que soporta Docking

Las librerías ya se encuentran incluidas en el archivo solucion del proyecto De VS 2019 (Se puede ejecutar desde aquí, seleccionando la plataforma x64), sin embargo se incluye también el archivo ejecutable en la raíz del proyecto.

## Controles:

Los controles de teclado son:
- [W] Desplazar la escena hacia el fondo
- [S] Desplazar la escena hacia la cámara
- [A] Desplazar la escena la derecha
- [D] Desplazar la escena la izquierda
- [O] Vuelve el centro de la escena al origen
- [UP] Rotar la cámara hacia arriba
- [DOWN] Rotar la cámara hacia abajo
- [LEFT] Rotar la cámara hacia la izquierda
- [RIGHT] Rotar la cámara hacia la derecha
- [SCAPE] Salir de la aplicación
- [SPACEBAR] Cambiar el modo de relleno de polígonos

Los Controles con el mouse son:
- [Click izquierdo + Movimiento del mouse] Permite rotar la escena
- [Click derecho + Movimiento del mouse] Permite desplazar la escena
- [Scroll] Permite alejar o acercar la escena

Aparte con el mouse se puede controlar el menú de la izquierda

## Como usar el menú de Imgui:
Basta con posicionar el mouse sobre el menú o las pestañas y seleccionar las diversas opciones con el click izquierdo.

------
### Docking y tamaño de ventana:
La aplicación provee de un sistema para poder arrastrar y reposicionar las ventanas que dispone, las cuales son una para la escena del océano y otra para el menú lateral. También se puede agrandar o cambiar el tamaño de la ventana de la aplicación y no afectará las proporciones de la visualización. Sin embargo se recomienda dejar la posición del menú como se encuentra por defecto.

------
### Pestañas / Botones
El menú lateral presenta los siguientes elementos
### Ship
Parámetros del modelo del barco:
- Position: Tres Slider para cada coordenada de la posición del barco
- Size: Slider para el valor de escala del barco
- Rotation: Slider para el valor de la rotación en el eje Z del barco

------

### Water Material
Parámetros para controlar los colores del océano y sus coeficientes de material
- Water Color: Seleccionador RGB del color base del oceano
- Water Ambient: Seleccionador RGB del coeficiente de material ambiente del océano
- Water Diffuse: Seleccionador RGB del coeficiente de material difuso del océano
- Water Specular: Seleccionador RGB del coeficiente de material especular del océano
- Shininess: Slider del brillo reflejado por el océano
------
### Waves
- Gravity: Slider con el valor de la fuerza de gravedad que influye en la velocidad del oleaje

Contiene parámetros para configurar cada uno de los tres oleajes. Cada pestaña de Wave contiene:
- Direction: Dos Sliders para las coordenadas x,y de la dirección de movimiento del oleaje
- Steepness: Slider para el valor de la amplitud del oleaje
- WaveLength: Slider para el largo del oleaje
------
### Displace
Contiene los parámetros para configurar cada uno de los tres efectos con texturas. Casa pestaña contiene:
- Direction: Dos Sliders para las coordenadas x,y de la dirección de movimiento de la textura
- Size: Slider del valor que escala el tamaño de la textura
- Speed: Slider de la velocidad de desplazamiento de la textura
- Strength: Slider del valor de saturación de la textura
- Texture Color: Seleccionador RGB para el color que satura la textura
- Lmt 1: Slider de la cota inferior sobre la cual se filtra el valor leído de la textura
- Lmt 2: Slider de la cota superior sobre la cual se filtra el valor leído de la textura
- Include inside?: Checkbox para seleccionar si el filtro es sobre el interior de los límites anteriores o sobre el exterior
------
### Light
Parámetros para controlar la posición de la luz y de los coeficientes de iluminación
- Cenit: Slider del ángulo cenital del sol
- Azim: Slider del ángulo azimutal del sol
- Ambient: Slider del coeficiente de iluminación ambiental sobre una luz blanca
- Diffuse: Slider del coeficiente de iluminación difusa sobre una luz blanca
- Specular: Slider del coeficiente de iluminación especular sobre una luz blanca
------
### Ship view / Global View
Botón para cambiar entre la vista de la escena global o entre la vista desde el barco

------
### Configuration 1
Botón para cambiar a la configuración de la escena con grandes olas

------
### Configuration 2
Botón para cambiar a la configuración de la escena con pequeñas olas

------
### Configuration 3
Botón para cambiar a la configuración de la escena atardecer

------
### Configuration 4
Botón para cambiar a la configuración de la escena de lava

------
