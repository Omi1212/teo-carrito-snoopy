# 🤖 OumiSharp Compiler & VM

Este proyecto implementa un **compilador completo** (Source-to-Bytecode) y una **Máquina Virtual** para controlar un robot siguelíneas basado en Arduino.

El sistema permite escribir la lógica del robot en un lenguaje de alto nivel personalizado, compilarlo en una PC y ejecutarlo en el Arduino leyendo el código binario desde una tarjeta SD.

---

## 📋 Prerrequisitos

Antes de empezar, necesitas instalar las herramientas de compilación (`gcc`, `make`, `flex`, `bison`).

### 🪟 Windows (Recomendado: MSYS2)
En Windows, no uses PowerShell ni CMD. Usaremos **MSYS2**.

1.  Descarga e instala [MSYS2](https://www.msys2.org/).
2.  Abre la terminal **"MSYS2 UCRT64"**.
3.  Ejecuta el siguiente comando para instalar todo lo necesario:
    ```bash
    pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make flex bison
    ```

### 🍎 macOS
Necesitas tener instalado **Homebrew** y las herramientas de línea de comandos de Xcode.

1.  Abre la Terminal.
2.  Ejecuta:
    ```bash
    xcode-select --install
    brew install flex bison make
    ```
    *(Nota: macOS ya trae `flex` y `bison` antiguos, pero instalar las versiones nuevas con brew es más seguro).*

---

## 🛠️ 1. Construir el Compilador

El primer paso es crear el ejecutable `mi_compilador` desde el código fuente C.

1.  Abre tu terminal (MSYS2 UCRT64 en Windows / Terminal en Mac).
2.  Navega a la carpeta del proyecto:
    ```bash
    cd /ruta/a/tu/carpeta/teo-carrito-snoopy
    ```
3.  Ejecuta el comando de construcción:

    **En Windows:**
    ```bash
    mingw32-make
    ```

    **En macOS:**
    ```bash
    make
    ```

Si todo sale bien, verás un mensaje como: `--- Compilador [mi_compilador] construido exitosamente. ---`.

> **Nota:** Si hiciste cambios en el código y quieres recompilar desde cero, ejecuta `mingw32-make clean` (Win) o `make clean` (Mac) antes de construir.

---

## ⚙️ 2. Compilar tu Código de Robot

Ahora usaremos el compilador para traducir tu archivo de texto (`.txt`) a un archivo binario (`.bin`) que el robot entiende.

1.  Asegúrate de tener tu código en `siguelineas.txt`.
2.  Ejecuta:

    **En Windows:**
    ```bash
    ./mi_compilador.exe siguelineas.txt
    ```

    **En macOS:**
    ```bash
    ./mi_compilador siguelineas.txt
    ```

✅ **Resultado:** Si no hay errores, aparecerá un nuevo archivo llamado **`program.bin`** en la carpeta.

---

## 💾 3. Cargar en el Robot

### Preparar la SD
1.  Consigue una tarjeta microSD formateada en **FAT32**.
2.  Copia el archivo **`program.bin`** que acabas de generar a la raíz de la tarjeta SD.
3.  Inserta la tarjeta en el módulo SD del robot.

### Cargar el Firmware (Arduino IDE)
Esto solo se hace una vez (o si cambias los pines).

1.  Abre la carpeta `OumiVM` en el **Arduino IDE**.
2.  Asegúrate de tener las 3 pestañas abiertas:
    * `OumiVM.ino` (Código principal).
    * `vm.h` (Definiciones).
    * `vm.cpp` (Implementación del hardware).
3.  Verifica que los pines en `vm.cpp` y `OumiVM.ino` coincidan con tu conexión real:
    * **SD CS:** Pin 4
    * **Motor Izq:** ENA(6), IN1(2), IN2(3)
    * **Motor Der:** ENB(7), IN3(5), IN4(8)
    * **Sensores:** 9 y 10
4.  Conecta el Arduino por USB y dale al botón **Subir (Upload)**.

---

## 🤖 ¡A Rodar!

1.  Desconecta el USB.
2.  Enciende las baterías del robot.
3.  El robot leerá la SD automáticamente y ejecutará tu lógica.

