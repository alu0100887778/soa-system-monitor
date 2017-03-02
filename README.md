# System Monitor

Con la ayuda de Qt vamos a crear nuestro propio monitor del sistema. Nos permitirá examinar: procesos en ejecución, conexiones abiertas, temperatura de la CPU, hardware del sistema y todo lo que se nos ocurra.

Lo haremos para Linux porque en Windows es bastante más complejo. Por ejemplo, en Linux leer la temperatura de la CPU significa abrir y leer el contenido de una archivo en /sys/class/hwmon. Sin embargo en Windows hay que acceder a las interfaces [COM](https://en.wikipedia.org/wiki/Component_Object_Model) de [Windows Management Instrumentation](https://en.wikipedia.org/wiki/Windows_Management_Instrumentation). Para ilustrarlo se pude ver un ejemplo [aquí](https://goo.gl/4GxxoB).

En todo caso, si quieres hacer la actividad en Windows, se te propone una opción alternativa al final del documento.

## Cómo empezar

 1. Acepta la [tarea asignada de GitHub Classroom](https://classroom.github.com/assignment-invitations/db3c0db7dc53b0bce07d4d2cf3826f97). Así obtendrás tu propio repositorio como una copia de este. A este repositorio lo llamaremos `upstream`.
 2. Haz un [fork](https://guides.github.com/activities/forking/) de `upstream`. Al nuevo repositorio lo llamaremos `origin`.
 3. [Clona](http://gitref.org/creating/#clone) `origin` en tu ordenador.
 4. Trabaja en tu copia local para desarrollar tu monitor del sistema, siguiendo los pasos indicados en el siguiente apartado.
 5. Modifica README.md usando [Markdown](https://guides.github.com/features/mastering-markdown/) para:
   1. Explicar cómo compilar y probar la aplicación, incluyendo los requisitos requeridos para hacerlo.
   2. Comentar las características implementadas.
   3. Discute que técnica para manejar la concurrencia te pareció mejor: más sencilla, más flexible o más escalable.
 5. [Sube](http://gitref.org/remotes/#push) los cambios al repositorio `origin` en GitHub.
 6. Crea un [pull request](https://help.github.com/articles/creating-a-pull-request) de `origin` a `upstream` para entregar la práctica.

## Ventana principal

La ventana principal puede ser algo así:

![Ventana principal](/../master/images/main-window.png)

Es decir, puedes colocar un QTabWidget que ocupe toda la ventana, con una pestaña por tipo de información que quieras mostrar: Procesos, Red, Hardware, Sensores, etc.

Dentro de cada pestaña se colocan los controles que hagan falta para mostrar la información de la forma más adecuada.

## Sensores

Una de las pestañas de nuestro programa mostrará la información de los sensores instalados en el hardware detectado por el sistema operativo. Esta información está disponible en forma de archivos dentro de la ruta `/sys`. Ese es el lugar donde Linux monta [Sysfs](https://es.wikipedia.org/wiki/Sysfs), un sistema de archivos virtual —no son archivos reales que ocupen espacio en el disco duro— que usa Linux para publicar todo tipo de información sobre los dispositivos y los controladores conocidos.

En concreto, dentro de `/sys/class/hwmon/` hay un directorio para cada chip detectado con sensores. Y dentro de ellos hay archivos como `temp1_input`, `temp1_max` o `fan1_input` que contienen información sobre la temperatura o la velocidad de los ventiladores del dispositivo en cuestión. El archivo `name` contiene el nombre del dispositivo.

Crearemos un hilo que cada segundo —véase el método `sleep()` de `QThread` para implementar la espera—:

 1. Recorra los directorios de `/sys/class/hwmon/` y los archivos con datos de sensores: `temp*`, `fan*`, etc.   
 2. Lea los datos de cada sensor y los almacene en una cola para enviarlos al hilo principal. Por ejemplo, `QVector`, `QQueue` o lo que prefieras. Cada valor se debe guardar con una etiqueta única para saber de qué sensor estamos hablando.

Obviamente es importante evitar las condiciones de carrera. Debemos usar`QMutex` para proteger la cola y `QMutexLock` para bloquear el mutex y asegurar que se libera cuando ya no es necesario. Además el hilo principal solo debe extraer datos cuando haya algo que extraer pero ¿nos servirá `QWaitCondition` para eso?

El hilo principal extraerá los datos de los sensores cada vez que hayan datos nuevos y los actualizará en la pestaña correspondiente.

Para crear el hilo usaremos [QThread](http://doc.qt.io/qt-5/qthread.html). Solo tenemos que crear una clase y heredar de `QThread` para sobreescribir su método `run()` donde pondremos el código que se ejecutará en el hilo. Es decir, allí pondremos el código que lee los sensores.

### Notas

 * Si se trabaja localmente en Linux, [QDir](http://doc.qt.io/qt-5/qdir.html) será de gran ayuda para recorrer los directorios.
 * En [la documentación del kernel](https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface) se indica qué podemos esperar encontrarnos en /sys/class/hwmon.
 * La etiqueta se puede construir, por ejemplo, con el nombre del chip —disponible en el archivo _name_— más el nombre del sensor. Por ejemplo: _coretemp:temp2\_max_ o _acpitz-temp1\_input_.
 * Para almacenar en la cola tanto la etiqueta como el valor se puede usar una estructura propia o aprovechar las facilidades de [QPair](http://doc.qt.io/qt-5/qpair.html).
 En el apartado ["Gestión de hilos en Qt" de este artículo](https://jmtorres.webs.ull.es/me/2013/02/introduccion-al-uso-de-hilos-en-qt/) se puede ver un pequeño ejemplo de cómo usar `QThread`.  

## Lista de procesos

Ya sabemos que /proc tiene un directorio para cada proceso en ejecución en el sistema. La información de cada uno está en un directorio de nombre numérico donde dicho número es el PID del proceso en cuestión.

En esta ocasión el hilo principal creará un agrupamiento de hilos [QThreadPool](http://doc.qt.io/qt-5/qthreadpool.html) y le asignará tareas:

 * Cada tarea se asigna al agrupamiento usando [QtConcurrent](http://doc.qt.io/qt-5/qtconcurrent-index.html).
 * Las funciones de [QtConcurrent](http://doc.qt.io/qt-5/qtconcurrent-index.html) devuelven un [QFuture](http://doc.qt.io/qt-5/qfuture.html) que cuando termine la tarea contendrá el resultado de esta.
 * Con [QFutureWatcher](http://doc.qt.io/qt-5/qfuturewatcher.html) se puede vigilar la evolución de una tarea. Por ejemplo tiene una señal `finished()` que emite cuando la tarea termina y el resultado ya está disponible en el `QFuture`.
 * Con [QFutureSynchronizer](http://doc.qt.io/qt-5/qfuturesynchronizer.html) se pueden sincronizar varias tareas.

entonces: 
 
 1. Cada 5 segundos se buscan todos los directorios de procesos en /proc.
 2. Al terminar, se lanza una tarea en el agrupamiento de hilos por cada directorio encontrado. Cada tarea será la encargada de recopilar la infomación sobre el proceso: PID, línea de comandos _(cmdline)_, propietario _(status)_, número de hilos, etc.
 3. Al terminar se actualiza la lista de procesos mostrada en la pestaña correspondiente.
 
### Notas
 
 * Los detalles del contenido del directorio /proc están disponibles [aquí](https://www.kernel.org/doc/Documentation/filesystems/proc.txt)
 * Podría ser buena idea usar [QTableWidget](http://doc.qt.io/qt-5/qtablewidget.html) para tener una tabla.

## Hardware

Vamos a utilizar `QProcess` para invocar al programa `lshw`. Este programa nos proporciona un listado completo del hardware instalado en nuestro sistema:

Vamos a ejecutar `lshw` con `QProcess` y leer sus resultados de forma síncrona —véase más abajo—. Por lo tanto no podemos leer su salida estándar desde el hilo principal porque lo bloquearíamos. En su lugar usaremos hilos pero en una de las formas recomendadas por Qt: [Hilos de trabajo usando señales y slots en Qt](https://jmtorres.webs.ull.es/me/2013/02/hilos-de-trabajo-usando-senales-y-slots-en-qt/):

   1. En esta ocasión crearemos un `QThread` sin heredar del él.
   2. Crearemos una clase que tendrá un slot que hará el trabajo de crear el `QProcess` lanzar `lshw` en él y leer su salida.
   3. El objeto de dicha clase lo moveremos al nuevo hilo usando el método moveToThread() y conectaremos las señales y slots de dicho objeto con los de la ventana principal.
   
Tal y como se describe en [el artículo](https://jmtorres.webs.ull.es/me/2013/02/hilos-de-trabajo-usando-senales-y-slots-en-qt/) la ventana principal puede emitir una señal que llegará al nuevo objeto. El slot que lo reciba se ejecutará en el nuevo hilo. Es decir, que las operaciones síncronas sobre `QProcess` se ejecutarán en ese hilo. Al terminar el trabajo ese slot emitirá una señál con el resultado y este será recibido por la ventana principal en su propio slot.

Con esa informaación la ventana principal actualizará la pestaña correspondiente.

### Notas

 * Usar [QTreeView](http://doc.qt.io/qt-5/QTreeView.html#details) parece la forma más sencilla de mostrar el árbol de dispositivos. Pero para eso es necesario interpretar correctamente la salida del comando `lshw`.

 * Para poder interpretar la salida de `lshw` sería interesante que dicha salida estuviera en algún formato para el que Qt nos diera facilidades. Lo mejor es pasarle a `lshw` la opción `-json` y así toda la información la devolverá en formato [JSON](https://en.wikipedia.org/wiki/JSON). JSON se ha extendido mucho en el mundo de la web por su relación con Javascript y Qt dispone de [un módulo](http://doc.qt.io/qt-5/json.html) para interpretarlo. Con el módulo JSON será muy fácil recorrer el árbol de dispositivos y construir los elementos del `QTreeView`.

#### Ejecución de otros procesos

Ya hemos visto que parte de la información que queremos la podemos extraer ejecutando programas ya instalados en el sistema. Y sabemos que en Linux eso significa tirar de las llamadas al sistema `fork()` y `exec()`, mientras que en Windows se utiliza CreateProcess(). Por suerte Qt nos ofrece [QProcess](http://doc.qt.io/qt-5/qprocess.html) y así no tenemos que preocuparnos por las singularidades de cada plataforma.

El método `start()` de `QProcess` ejecuta el proceso que le indiquemos y conecta su entrada y su salida estándar a tuberías —sin que tengamos que hacer nada—. Esas tuberías se usan indirectamente a través de los métodos de `QProcess` para comunicarnos con el proceso en ejecución. Para nosotros el objeto `QProcess` es como un archivo. Por eso hereda de [QIODevice](http://doc.qt.io/qt-5/qiodevice.html) que es la clase base de todas las clases que podemos usar como dispositivos de E/S. `QIODevice` implementa métodos como `read()`, `write()`, `readLine()`, `readAll`, `open()`, `isOpen()`, `close()`, `putChar()`, `getChar()`, `seek()` y más. El uso de cualquier clase heredera de QIODevice es siempre muy similar. Y, obviamente, donde la librería admita un objeto QIODevice, podemos indicar un objeto QProcess o de cualquier otra clase heredera de QIODevice.

**Al leer o escribir en un objeto QProcess estaremos usando indirectamente tuberías para recibir o escribir en las salidas y entradas estándares del proceso en ejecución**.

#### Operaciones sincronas

Si llamamos directamente al sistema operativo, operaciones como `read()`, `write()` son bloqueantes o síncronas. Es decir, si no se pueden hacer en el momento el hilo se suspenden hasta que se completan.

Con Qt eso no funciona bien pues el resultado es que el bucle de mensajes se detiene y no puede procesar más peticiones. Por eso en Qt las operaciones de E/S de `QIODevice` y clases herederas son asíncronas. Es decir, se programan las operaciones y vuelven inmediatamente. Cuando la operación se ejecuta con éxito, llega un evento al bucle de mensajes que se emite como una señal y se entrega al slot correspondiente. Pero todo eso lo veremos más adelante. Por el momento usaremos `QIODevice` como si las operaciones fueran síncronas, sin programación dirigida a eventos.

Por tanto para leer de `QProcess` haremos así:

~~~.cpp
process.waitForReadyRead();
QByteArray data = process.readAll;
~~~

y para escribir:

~~~.cpp
process.write(data);
process.waitForBytesWritten();
~~~

siempre desde un hilo.

Ambas operaciones juntas se comportan como la llamada al sistema `read()` de Linux. `waitForReadyRead()` hace que el hilo se supenda hasta que hay datos para leer. Despues podemos leerlos con `read()`, `readAll()` o `readLine()`. 

De forma similar, el método `write()` de `QIODevice` —o `QProcess`— inicia la operación de escritura pero `waitForBytesWritten()` bloquea el hilo hasta que los datos se escriben. Además existen otros métodos `waitFor*` para bloquear el hilo hasta que ocurren ciertas cosas. Por ejemplo: `waitForFinished()` o `waitForStarted()`.

Por el momento usaremos estas funciones para la E/S, en lugar de eventos. El problema es que, como hemos dicho, estas funciones pueden bloquear los hilos donde se ejecutan, por lo que no podemos usarlas nunca en el hilo principal del programa.

## Monitor de sistemas remotos

Si quieres programar en Windows puedes hacer un monitor remoto para sistemas Linux. Y, obviamente, si trabajas en Linux y quieres más, puedes hacer que tu programa también pueda monitorizar otras máquinas en Linux ;)

La clave es tener un cliente ssh. En Linux ese cliente viene de serie y permite ejecutar un comando como `lshw` en remoto así:

    $ ssh usuario@miservidor.org 'lshw -json'

obviamente podríamos leer el contenido de un archivo así:

    $ ssh usuario@miservidor.org 'cat /etc/passwd'

y el de un directorio así:

    $ ssh usuario@miservidor.org 'ls /etc'
    
o hacer cosas así:

    $ ssh usuario@miservidor.org 'ls /etc/p*'

En Windows necesitamos instalar un cliente como [Putty](http://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html). Como toda buena aplicación de Windows, Putty tiene ventanas. Pero viene con un cliente de línea de comandos llamando `plink`:

    > plink usuario@miservidor.org "lshw -json"

Obviamente estos comandos se pueden lanzar con `QProcess` y leer su salida para hacer la actividad que hemos comentado. Simplemente necesitamos un servidor SSH, que puede ser uno cualquiera al que tengamos acceso o cualquier distribución de Linux ejecutándose en una máquina virtual en nuestro ordenador.

El único problema es que tanto `ssh` como `plink` van a pedir la contraseña del usuario para conectarse al servidor. Con `plink` podemos usar la opción `-pw` para indicarla:

    > plink usuario@miservidor.org -pw "password" "lshw -json"
    
pero eso no funciona con `ssh` ni es la opción recomendada por sus riesgos para la seguridad. Lo más seguro es usar la autenticación basada en clave privada:

 * [Using Plink to do Key Based Authentication from Windows](http://practical-admin.com/blog/glutton-for-punishment-using-plink-to-do-key-based-authentication/)
 * [Autentificación SSH con llave privada (private key)](https://www.linuxtotal.com.mx/index.php?cont=info_seyre_010)

## Opcional

 * ¿De verdad tienes ganas de más? ;) Si es así añade otra información que te interese. Por ejemplo:
   * El modelo de la CPU —disponible en `/proc/cpuinfo`—.
   * El número de conexiones abiertas —ejecutando `netstat`— por tipo de protocolo: TCP, UDP, UNIX, etc.
   * La dirección IP de tu equipo y los MB/s transferidos y recibidos ——con `ifconfig` y/o interpretando `/proc/net/netstat`, `/proc/net/dev/` o `/proc/net/tcp`, por ejemplo—.
   * Los usuarios con sesiones abiertas en el sistema.
   * ...
