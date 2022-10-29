# Proyecto 1 Proxy Inverso y balanceador de carga 
## Integrantes
- Samuel David Villegas Bedoya
- Julian Andrés Ramírez Jiménez 
- Julian Giraldo Perez 
## Introducción
### Definición
Un proxy inverso es un dispositivo o servidor que se sitúa generalmente entre el cliente y un servido web, y este tiene la función de interceptar las peticiones que hace al cliente, para posteriormente reenviarlas a los servidores web y finalmente devolver la respuesta al cliente. 

### Importancia
Añadir este intermediario a la comunicación Cliente-Servidor trae consigo diferentes ventajas como: 
- Mayor fluidez en la comunicación gracias a la implementación de balanceadores de carga 
- La posibilidad de aumentar la seguridad tanto para el cliente como para el servidor
- La capacidad de inspeccionar contenido posiblemente malicioso antes de llegar a su destinatario 
- El aumento del anonimato y disminución de la huella digital.

## Desarrollo
### visión general
Para este proyecto se implemento en c un proxy inverso con balanceador de carga, dicho proxy soportara peticiones HTTP/1.1, además se implemento en este proxy un mecanismo de balanceo de carga y de cache, lo que permitirá una mayor fluidez en términos de velocidad de respuesta. 
### Servidor Backend
Para implementar el proxy primero se comenzó creando un servidor backend, que es quien responderá las peticiones del cliente una vez estas pasen por el proxy, dicho servidor cuenta con varias peticiones que se pueden hacer, todo con el fin de probar la implementación del proxy.
En el caso de este proyecto en particular se instancian 3 servidores backend en los puertos 3000, 3001 y 3002.

### Sockets
Para establecer las conexiones cliente-proxy y proxy-servidor se utilizo la API sockets, dicha API ayuda a establecer una enlace  entre dos entidades, dicho enlace permitirá el intercambio de información entre las entidades conectadas.

Además para el uso del los sockets se utilizo la siguiente guía. 
![Socket in Computer Network - GeeksforGeeks](https://media.geeksforgeeks.org/wp-content/uploads/20200509144631/223-1.png)
### Proxy
El proxy es la sección principal de este proyecto, y con el fin de explicarlo en profundidad se dividirá esta sección en diferentes módulos. Estos Módulos son representaciones de alto nivel de las funciones del código. 
#### Obtener petición cliente
Para escuchar las peticiones del cliente se crea un socket, una vez se estén escuchando las peticiones, se utiliza un loop, dentro del cual se estará esperando a que llegue alguna petición, y de llegar se ejecutara la función  **get_data_of_client()**

Dentro de la función **get_data_of_client()** se recibe la petición del cliente mediante la función recv y es guardada en un string, luego se revisara si esta ya esta en el cache, y de ser así se comprobara su validez , esto utilizando la función **readCache()**, esta retornara 0 si se encontró un cache valido o 1 si este no es valido, dependiendo de esto se obtendrá la respuesta del cache o se le hará una petición al servidor, y posteriormente se retornara esta respuesta.


#### Cache
Para la implementación del cache se creo un archivo de texto, el cual tendrá dentro de el un registro de las peticiones realizadas, estas se guardaran en el formato "Peticion=tiempo_creacion=ubicacion_respuesta".
Donde
- Peticion: Peticion realizada por el cliente. *ej GET_-api-users_HTTP-1.1*
- Tiempo_creacion: Tiempo en segundos de la función **get_time()**. *ej1666821832*
- ubicacion_respuesta: Ubicación del archivo que contiene la respuesta a la peticion. *ej ./cache/archivos/GET_-api-users_HTTP-1.1*

De esta forma cuando se va a guardar una respuesta en el cache primero se copiara la respuesta en un archivo de texto a parte, este archivo tendrá por nombre su petición correspondiente, y luego se pondrá con el formato mostrado anteriormente una entrada en el registro del cache.

Para hacer el manejo del TTL del cache se guarda en las entradas del cache el tiempo_creacion este se obtiene con la función get_time() de la libreria time, dicha función retorna el tiempo en segundos que ha pasado desde 00:00:00 UTC, Enero 1, 1970. Finalmente para comprobar si un cache es valido, se vuelve a usar get_time() y se revisa si el tiempo_creacion -  get_time es menor a un tiempo_limite establecido, de ser así el cache es valido, de otro modo este será eliminado del registro.

#### Petición a servidor
Primero se utiliza el **Balanceador de carga** para definir con que servidor backend se conectara el proxy. Posteriormente se crea un string de longitud estática, dicho string recibirá la respuesta del servidor, la cual es obtenida mediante el método **recv()**. Una vez recibida la respuesta esta se retorna y se guarda en el cache.

En este caso se tuvo que añadir a la función **recv()** el parámetro MSG_WAITALL para que se esperara a que llegara toda la respuesta por parte del servidor. 

#### Balanceador de carga
El balanceador de carga que se implemento utiliza una variable auxiliar, la cual llevara un registro del ultimo servidor utilizado para recibir la respuesta de la petición. A la hora de hacer una nueva petición se revisara esta variable auxiliar y se aumentara en 1 para ir al próximo servidor, de no haber mas servidores se reiniciara esta variable.

#### Log
Cada mensaje que se muestre por parte del proxy será guardado en un archivo LOG a traves de la funcion **saveLog()**, esto con el propósito de poder ver con mas facilidad lo que se ha ejecutado en el servidor proxy y poder detectar fallos en caso de que estos estén presentes. 
### Despliegue 
Para este despligue es necesario crear 4 instancias de EC2, 3 que servirian para montar la app y otra para el servidor proxy


#### Servers

Se debe habilitar regla de entrada TCP en el puerto que se quiere utilizar para cada servidor donde se montará la app. En nuestro caso utilizamos 3000

Abrimos la consola y ejecutamos los siguientes comandos:
	sudo su
	apt-get update
	apt-get install nodejs -y
	apt-get install npm -y
	git clone https://github.com/Bakuraza1/cache_proxy
	cd cache_proxy/server_backend
	npm i 
	nano .env 
		- Poner PORT=3000
	npm start


Luego de haber configurado los 3 servidores pasamos al servidor proxy

#### Proxy
Se debe habilitar regla de entrada TCP en el puerto que se quiere utilizar. En nuestro caso 8080

Abrimos la consola y ejecutamos los siguientes comandos:
	sudo su
	apt-get update
	apt install build-essential
	git clone https://github.com/Bakuraza1/cache_proxy
	cd cache_proxy/server_proxy
	nano env.h 
		- Se debe poner en el la constante SERVER_HOST la ip privada del servidor proxy
		- Se debe poner en la constante SERVER_LIST se debe de poner ip:puerto de los servidores a 		utilizar separados por '/'. (Las ips deben ser las públicas)
		- Se debe poner en la constante SERVER_NUMBERS el numero de servidores a utilizar 
	gcc server.c env.h general_functions.h -o server


## Conclusiones 
En general se logro el proposito de la practica, se creo el proxy con funcionalidades de cache y balanceador de carga funcionales, y al realizar pruebas estas fueron exitosas, aun así este puede ser trabajado y mejorado mas allá de lo que se presenta aquí. 

Además de aprender mas a profundidad sobre diversos conceptos de telemática, también se logro conseguir experiencia con el lenguaje c a pesar de que la utilización de dicho lenguaje resulto ser un reto en muchos aspectos.



## Referencias 
https://www.nginx.com/resources/glossary/reverse-proxy-vs-load-balancer/
https://www.geeksforgeeks.org/socket-in-computer-network/
https://www.nginx.com/resources/glossary/round-robin-load-balancing/
https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-recv-receive-data-socket
