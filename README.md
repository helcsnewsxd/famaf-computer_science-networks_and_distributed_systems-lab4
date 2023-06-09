# Lab 4 INFORME

# **Análisis e Implementación de Algoritmos de Enrutamiento de la Capa de Red en Networks simulados en Omnet++**

- [Resumen](#resumen)
- [Introducción](#introducción)
- [Análisis de Casos (Etapa de Análisis)](#análisis-de-casos-etapa-de-análisis)
- [Tarea de Diseño](#tarea-de-diseño)
  - [Introducción y planteo del problema](#introducción-y-planteo-del-problema)
  - [Estructura a considerar](#estructura-a-considerar)
  - [Puntos importantes que se tuvieron en cuenta](#puntos-importantes-que-se-tuvieron-en-cuenta)
  - [Idea de la estrategia de enrutamiento](#idea-de-la-estrategia-de-enrutamiento)
  - [Implementación](#implementación)
    - [Obtener información de mis vecinos](#obtener-información-de-mis-vecinos)
    - [Obtener información de los vecinos de todos los nodos](#obtener-información-de-los-vecinos-de-todos-los-nodos)
- [Análisis de Casos (Etapa de Diseño)](#análisis-de-casos-etapa-de-diseño)
- [Conclusiones acerca de esta implementación](#conclusiones-acerca-de-esta-implementación)

# Resumen

En este laboratorio estudiamos la topología anillo y elaboramos un enrutamiento eficiente para la misma. Primero, lo que realizamos fue un análisis crudo del comportamiento de nuestra red en distintos casos particulares. Luego, mejoramos la estrategia de enrutamiento para evitar problemas de congestión y obtener una mayor eficiencia.

# Introducción

En una red de topología anillo, en donde se tienen varios nodos como actores responsables del envío/recibo de paquetes, es importante aprovechar las múltiples salidas y entradas para la comunicación de mensajes. Nuestro objetivo radica en eso, poder ingeniar un enrutamiento de paquetes de tal forma que se aprovechen al máximo las múltiples conexiones disponibles.

Como punto de partida, analizamos el comportamiento de la red cruda, sin cambios, tal cual como la dio la cátedra.

# Análisis de Casos (Etapa de Análisis)

Las métricas que nosotros vimos pertinente medir fueron las siguientes:

- Una medida para la demora de entrega de paquetes
- Una medida para la cantidad de saltos hechos por cada paquete
- Una medida para la utilización de los recursos de la red (buffers y enlaces)

## **Primer Caso**

En este caso corrimos el modelo sin cambios en su configuración, es decir, los nodos 0 y 2 generan paquetes y lo transmiten al nodo 5.

### Preguntas Importantes

1. ¿Qué métricas se obtienen?
    
    **Ocupación de Buffers en sistema**

    ![image](/img/graphs/analysis/buffers.png)

    Veamos que con la implementación actual del modelo, el nodo 2 y el nodo 0 (que producen paquetes) envían los mismos por el mismo lado, produciendo una sobreutilización de la red. Ésto hace que algunos paquetes deban esperar antes de poder enviarse (debido a la congestión).
    En particular, el buffer del nodo 0 crece significantemente, porque no sólo produce paquetes, si no que también recibe los del nodo 2, por lo que el control no es muy bueno.

    **Saltos y delay**
    ![image](/img/graphs/analysis/node5.png)

    El delay es el tiempo que tardan los paquetes en llegar al nodo 5 desde que han sido generados. Este tiempo, como vemos, ha ido creciendo significantemente a medida que corre el tiempo de la simulación. Ésto se produce  por lo que mencionamos anteriormente, la congestión de la red consecuencia de la pobre estrategia de enrutamiento, produce que muchos paquetes tengan que esperar que se envíen otros antes, por eso se llenan los buffers.

    Luego tenemos los saltos. La cantidad de saltos siempre será la misma y rondará entre 3 y 5 saltos, ya que los paquetes se envían siempre por el mismo lado (derecho). Serán 5 saltos si son paquetes generados por el nodo 2, y serán 3 saltos si son paquetes generados por el nodo 0.

2. ¿Cómo es el uso de los recursos de la red?

    Los recursos se usan de una manera muy ineficiente, los buffers se llenan debido a la congestión, y hay momentos de la simulación en donde la red en lugares esta con poco tráfico, mientras en otros lugares está tapada de paquetes. Además, la mitad de la red no se utiliza porque los paquetes se envían por un solo lado.

3. ¿Se puede mejorar?

    Si, eso es lo que haremos en la etapa de Diseño. Básicamente, buscaremos aprovechar las múltiples salidas y entradas de nuestros nodos, buscando formas de ahorrar saltos y caminos más cortos.

## **Segundo Caso**

**Eficiencia según el interArrivalTime**
(...)

Aquí todos los nodos generan paquetes hacia el nodo 5. PacketByteSize e InterArrivalTime son idénticos entre los nodos. Tenemos que buscar un tiempo de interArrivalTime en donde la red se estabilice.

En nuestra red, si el tiempo interArrivalTime es demasiado pequeño, los paquetes se enviarán rápido, provocando que la carga del tráfico aumente, generando congestión.

En cambio, si tomamos un tiempo más largo, los paquetes se enviarán más lento, haciendo que la red no se congestione mucho.

Si lo que nosotros buscamos en estabilizar la red, un tiempo como exponential(1) es muy pequeño, por lo que deberíamos tomar un tiempo mayor como exponential(n) para un n mayor a 1.

Si bien queremos no saturar la red, tampoco debemos subutilizarla. Si hacemos el interArrivalTime demasiado grande, produciremos un efecto igual de ineficiente que si el interArrivalTime fuera muy pequeño.

Luego de analizar la red cruda, proseguimos a implementar un algoritmo de enrutamiento mejorado. Ésto lo hicimos en la tarea de diseño.

# Tarea de Diseño

### Introducción y planteo del problema

En esta sección del proyecto, lo que se pretende es implementar un algoritmo que utilice alguna estrategia para minimizar la **cantidad de saltos** realizados por los paquetes en su envío a un destino. Es decir, se busca que se implemente una estrategia de enrutamiento (preferentemente óptima) aplicada en particular a este tipo de redes para poder comparar el rendimiento respecto al de la *Tarea de Análisis*

La red a considerar consta de **8 nodos** (no es relevante la cantidad ya que la estrategia no depende de ello) conectados con topología de *anillo* y considerando que **en ningún momento se produce algún tipo de cambio** (i.e., se toma como suposición que jamás se puede caer un nodo o agregar otro, que la red no muta).

![image](/img/Topolog%C3%ADa%20de%20Anillo.png)

A continuación, en las siguientes secciones, se va a dar más detalle acerca de lo realizado.

### Estructura a considerar

Respecto a la estructura general bajo la cual se consideró óptima realizar el proyecto (no tiene nada que ver con la estrategia), tomamos óptimo en base a la naturaleza y la idea el laboratorio, separar nuestro código del siguiente modo:

![image](/img/Estructura%20general%20del%20proyecto.png)

Esto fue realizado por dos cuestiones principales:

- Comodidad
- Se investigó y es una estructura *usual* (separar src y simulations) que tienen los proyectos en Omnet++

### Puntos importantes que se tuvieron en cuenta

Respecto a los puntos importantes que se tuvo en cuenta para la idea y la implementación de la estrategia de esta sección fueron:

- No va a depender de
    - El número de nodos. **Solo se pide que sean 3 o más** (por cómo se hizo el BFS)
    - El nombre de los nodos (se puede llamar de cualquier forma siempre que sea un entero, por ejemplo, podemos tener en la red a: "1, 6, 8, 234, 57567 y 1945765". Lo único que se pide es que entre en `int`)
- Va a depender de
    - Que la red tiene topología de anillo
        - Cada nodo solo puede ir a la izquierda o a la derecha. Luego, el paquete o bien se envía *todo a la derecha* o bien *todo a la izquierda*
    - Que la red es inmutable. No se cambian enlaces ni nodos, sea de agregar o eliminar
        - Una vez que se hace el cálculo para cada nodo sobre cómo debe ser el envío para cada destino posible, no es necesario recalcularlo cada X tiempo ya que tenemos como suposición que la red **nunca cambia**
- Idea de escalabilidad: para facilitar el desarrollo del punto estrella, varias secciones se hicieron de forma más *general* y la idea se moldeó en base a eso. Esto se hizo para no tener que recodear todo el proyecto a la hora de hacer el punto estrella para implementar Dijkstra.
- Posibilidad de DEBUGGING mientras se realizaba el proyecto
    - El archivo `Net.cc` contiene la flag DEBUG como una constante booleana, la cual especifica si se desea que se imprima la información obtenida en el precómputo para chequear que esté bien. Por defecto está configurada en `false`.

### Idea de la estrategia de enrutamiento

En base a lo mencionado anteriormente, para la idea se han considerado las siguientes partes:

- Antes de cualquier envío de mensajes (en la inicialización de la clase `Net`), en cada CR se hace:
    - Obtención de información sobre mis vecinos
        - Cada nodo se fija quiénes son sus vecinos y en qué dirección está cada uno (si izquierda o derecha) y se guarda esta información
            - Se envía un paquete de tipo `NeighborInfo` solicitando los nombres de los vecinos. Estos lo completan y se lo devuelven al nodo original para que pueda guardar la información
    - Obtención de información sobre la red en general
        - Uso de paquetes **LSP** (*Link-State Packet*) que contienen la información del nombre del nodo y sus correspondientes vecinos
        - Inundación del paquete LSP de cada nodo a TODOS los demás de la red para que obtengan esta información
            - Los nombres de nodos son mapeados a índices para generalizar ante redes con nodos nombres que no necesariamente lo son
            - Cada nodo envía su LSP (y el de los demás que reciba por primera vez) a todas sus gates
        - Creación del grafo representativo
            - Cuando cada nodo pudo juntar TODOS los LSP correspondientes a los demás vecinos, significa que sabe con exactitud cómo es la red (en particular, siempre supimos la forma de anillo pero no la posición de los nodos ni la cantidad).
            - Se puede saber en topología de anillo cuándo se obtuvieron todos los LSP ya que esto sucede cuando la cantidad de nodos distintos recibidos por LSP (sea nodo principal como vecinos) es igual a la cantidad de archivos LSP enviados (*es trivial de ver*).
    - Cálculo del enrutamiento óptimo
        - Se considera como source el nodo en el que se está y se aplica BFS yendo a izquierda como a derecha. Si a un nodo llego primero desde una dirección, entonces para enviarle un paquete la ruta óptima será mediante la gate que tenga esa dirección (por propiedad de BFS y dado que minimizamos cantidad de saltos en topología de anillo)
        - Toda esta información del recorrido BFS y la dirección que el nodo debe tomar si se quiere enviar a cierto destino, se guarda (ya que se debe calcular una sola vez)
- Durante el envío de mensajes (paquetes de datos)
    - No se realiza actualización del grafo dada la suposición de inmutabilidad de la red
    - Para enviar o reenviar un mensaje de dato, se sabe por qué gate hacerlo en base a la información guardada en la sección de precómputo del enrutamiento óptimo

### Implementación

### Obtener información de mis vecinos

- Vamos a usar el tipo de paquete especial `NeighborInfo` dado por
    
    ```cpp
    packet NeighborInfo extends Packet {
    	int gateIndex;
    	int neighborName;
    }
    
    ```
    
- La CR de cada nodo va a considerar las siguientes variables y funciones
    
    ```cpp
    // VARIABLES
    static const int cntNeighbor = 2; // Cantidad de vecinos (2 por ser anillo)
    int neighborReached; // Cantidad de vecinos de los que obtuve información
    int neighbor[cntNeighbor]; // Nombre del vecino al que conecto con la i-ésima gate (o -1 si todavía no tengo información)
    
    // FUNCIONES
    virtual bool isNeighborInfo(cMessage *msg); // Para diferenciar en el handleMessage. Es si el destino es igual a -1
    virtual void askNeighborInfo(); // Para generar los paquetes de pregunta y enviarlos por todas las gates para obtener toda la información de mis vecinos
    virtual void actualizeNeighborInfo(NeighborInfo *pkt); // Para actualizar la información que tengo guardada si un vecino me responde con sus datos
    
    ```
    
- La consulta a los vecinos y el envío de los paquetes por todas las gates se hace en la etapa `initialize` del objeto `Net` (es decir, como precómputo antes de cualquier envío de paquete de datos)
- Si llega un paquete de consulta de nombre de vecinos, completo con mis datos y lo devuelvo por la gate en la que lo recibí
    - Para esto se usa `pkt->getArrivalGate()->getIndex()`
- Si me llega de vuelta el paquete que le envié a mi vecino V, entonces actualizo los datos locales en base a los que me brindaron
- Cuando recibo toda la información que necesito (i.e., `neighborReached == cntNeighbor (= 2)`), procedo con el envío de los **LSP**

### Obtener información de los vecinos de todos los nodos

- Se consideran las siguientes variables y funciones para representar el grafo local
    
    ```cpp
    // VARIABLES
    map<int, int> id, idRev; // Nombre de Nodo -> Nro de Nodo en el Grafo (idRev es la inversa)
    vector<pair<int, int>> graph; // Representación del grafo. Cada nodo tiene los dos vecinos con los que se conecta
    vector<bool> LSPVis; // Verdadero si me llegó el paquete LSP donde el nodo con id igual al índice es el principal
    int cntLSPVis; // Cantidad de paquetes LSP distintos recibidos
    int cntNodesGraph; // Cantidad actual de nodos representados por el grafo local
    
    // FUNCIONES
    virtual int getID(int nodeName);
    virtual int getIDRev(int nodeName);
    
    // LSP
    virtual bool isLSPInfo(cMessage *msg); // Para saber si el paquete que llegó es de LSP. Es si el destino es -2
    virtual void sendLSP(LSP *pkt); // Para enviar el paquete LSP a todos los vecinos
    virtual void actualizeNetworkRepresentation(LSP *pkt); // Para actualizar el grafo local en base al LSP recibido
    
    ```
    
- Una vez que tengo toda la información de mis vecinos (paso anterior), creo mi paquete LSP con los datos necesarios y lo envío por todas mis gates
- Si recibo un paquete LSP, entonces me fijo si ya me llegó antes o no (con LSPVis). En caso que no, actualizo el grafo local en base a los nuevos datos y reenvío el paquete por todas mis gates para extenderlo por todo el grafo
- Cuando la cantidad de nodos del grafo es igual a la cantidad de LSP recibidos (i.e., `cntNodesGraph == cntLSPVis`), entonces tengo toda la representación de la red en mi nodo actual, por lo que paso a la última parte del precómputo

### Cálculo del enrutamiento óptimo

- Se va a considerar lo siguiente para guardar la información del enrutamiento óptimo y permitir el acceso para la parte del envío de paquete de datos
    
    ```cpp
    virtual int getGateToSend(int nodeName);
    vector<int> gateToSend;
    
    ```
    
- Esta parte se hace aplicando simplemente BFS y considerando todas las gates inicializadas en -1 (para diferenciar en visitados y no). No es nada tan complejo
    
    ```cpp
    queue<pair<int, int>> q;
    // Agrego mis vecinos con su respectiva dirección
    q.push(make_pair(graph[getID(nodeName)].first, 0));
    q.push(make_pair(graph[getID(nodeName)].second, 1));
    
    while (!q.empty()) {
    	pair<int, int> actNode = q.front();
    	q.pop();
    
    	// Pongo la información de la gate que tengo que tomar para enviar al paquete de actNode.first
    	gateToSend[actNode.first] = actNode.second;
    
    	if (actNode.second == 0) { // Estoy yendo por izquierda
    		if (gateToSend[graph[actNode.first].first] == -1) {
    			// Si no pasé por este nodo antes, lo agrego a la queue para procesar (i.e., ya le asignaría la dirección en base a quién lo obtuvo primero)
    			q.push(make_pair(graph[actNode.first].first, 0));
    		}
    	} else { // Estoy yendo por derecha
    		if (gateToSend[graph[actNode.first].second] == -1) {
    			// Si no pasé por este nodo antes, lo agrego a la queue para procesar (i.e., ya le asignaría la dirección en base a quién lo obtuvo primero)
    			q.push(make_pair(graph[actNode.first].second, 1));
    		}
    	}
    }
    
    ```
    

Si bien no es necesario aplicar el **BFS** ya que para cada nodo solo se visita un vecino, se ha hecho de esta forma por practicidad y costumbre.

### Envío de paquetes de datos

En esta sección, si el paquete es de datos (i.e., el destino es >= 0 ), entonces me fijo si lo tengo que enviar a `App` o reenviar a otro nodo.

```cpp
if (pkt->getDestination() == nodeName) // Enviar a App
	send((cMessage *)pkt, "toApp$o");
else // Reenviar a otro nodo
	send((cMessage *)pkt, "toLnk$o", getGateToSend(pkt->getDestination()));
	// getGateToSend(pkt->getDestination()) me dice por dónde debo llevar el paquete para que llegue a destino con la MÍNIMA cantidad de saltos posible

```
# Análisis de Casos (Etapa de Diseño)
El algoritmo de enrutamiento lo que hace es aprovechar los nodos no utilizados en la etapa de análisis, mandando por ellos los paquetes de forma tal que tengan que recorrer el camino más corto para llegar a 5.
En particular, en vez de tener un solo camino para enviar al nodo 5, ahora tendremos 2: uno para los paquetes generados por el nodo 2 y otro para los generados por el nodo 1. Ningún paquete del nodo 1 se solapa con los del nodo 2 y viceversa.

Con estos cambios, veamos los efectos en los buffers, saltos, delays y demás.

## **Primer Caso**

En este caso, los nodos 0 y 2 generan paquetes y lo transmiten al nodo 5.

### Preguntas Importantes

1. ¿Cuánto mejoran las métricas?

**Capacidad de los buffers**
 ![image](/img/graphs/design/case1/buffers.png)

* Nodo 1: La única actividad que tiene el buffer de este nodo es de los paquetes de información de vecinos. No se envían paquetes por aquí porque el algoritmo busca la ruta más corta, y el nodo 1 no forma parte de ella.
* Nodo 0 y Nodo 2: Vemos que el buffer aumenta aquí porque son los nodos responsables de la generación de paquetes. De todas maneras, el nodo 0 ya no se congestiona como en la etapa de Análisis porque el mismo no recibe más paquetes del nodo 2, por lo que su buffer no se satura.
* Resto de los nodos: veamos que la cantidad de paquetes en el buffer se mantiene constante. Ésto es por que el algoritmo de enrutamiento aplicado permite el envío fluido de paquetes entre los nodos.

**Saltos y Delays**
 ![image](/img/graphs/design/case1/delay_hops.png)

Veamos que el delay no es tanto comparado con la etapa de Análisis. Ahora los paquetes pueden fluir a través de la red, en especial porque el bloqueo que existía antes en el nodo 0 ya no existe.

Los saltos ahora son 3 siempre, ya que como aplicamos BFS buscamos la ruta más corta teniendo la ruta izquierda como alternativa, y como desde el nodo 2 se envían los paquetes por izquierda, evitamos pasar por el nodo 1, ahorrandonos saltos.


2. ¿Hay loops de enrutamiento?

Loops de enrutamiento no hay porque los paquetes van por una misma ruta de forma fluida y separada, dependiendo qué nodo lo generó. No se pasa por un nodo más de una vez.

**Otras métricas**

Luego tenemos otras métricas como por ejemplo:

*Paquetes de Control*
![image](/img/graphs/design/case1/packets_ctrl.png)
Aquí vemos cómo son las estadísticas relacionadas a los paquetes de NB y los LSP. Observamos que no se pierde ningún paquete NB ni los LSP

*Paquetes*
![image](/img/graphs/design/case1/packets.png)
Esta es la información sobre los paquetes enviados, recibidos y generados de los nodos.

## **Segundo Caso**

Para el caso dos, tenemos que todos los nodos generan paquetes, exceptuando al nodo 5 que es quién los recibe.

Veamos algunas de las métricas

**Capacidad de los buffers**

![image](/img/graphs/design/case2/buffers.png)

* Nodo 0 y Nodo 1: son los únicos nodos generadores que no reciben paquetes de otros nodos, por lo que el tamaño del buffer no se satura (aunque si tiene un pico, que se produce por la congestión de la red con el paso del tiempo)

* Nodo 5: solo recibe paquetes, no los guarda porque pasan a la capa de red directamente.

* Resto de los nodos: si tienen un aumento de uso en sus buffers porque no sólo deben enviar los paquetes que generan ellos mismos, si no que además deben procesar los recibidos por los nodos anteriores.

**Delays y Saltos**
![image](/img/graphs/design/case2/delay_hops.png)

Aquí el delay va aumentando progresivamente también.
Los saltos también serán de entre 0 y 4.

**Otras medidas**

*Paquetes de Control*
![image](/img/graphs/design/case2/packets_ctrl.png)
Aquí vemos cómo son las estadísticas relacionadas a los paquetes de NB y los LSP. Observamos que no se pierde ningún paquete NB ni los LSP

*Paquetes*
![image](/img/graphs/design/case2/packets.png)
Esta es la información sobre los paquetes enviados, recibidos y generados de los nodos.

## Conclusiones acerca de esta implementación

Podemos observar que esta implementación logra aprovechar la topología de anillo y sus nodos de mejor forma que al inicio. Además, implementamos el uso de paquetes de control para poder conocer los vecinos de la red, tal como se vió en el teórico. El enrutamiento de los paquetes en el primer caso mejoró bastante comparado con los primeros análisis. Se logró reducir la demora de entrega y mejorar la utilización de los recursos. 

De todas maneras, sería bueno analizar como es el comportamiento de una red mucho más grande usando esta implementación.

# Tarea Estrella

## Introducción y planteo del problema

En esta sección del proyecto, lo que se pretende es implementar un algoritmo que utilice alguna estrategia para minimizar la **cantidad de saltos** realizados por los paquetes en su envío a un destino. Es decir, se busca que se implemente una estrategia de enrutamiento (preferentemente óptima) aplicada **PARA CUALQUIER TIPO DE TOPOLOGÍA**.

Es decir, se pretende generalizar la tarea de diseño para que el algoritmo de enrutamiento pueda "reconocer" la forma de la red y, en base a ello, calcular cómo debe enviar los datos.
Además, se considera que los nodos tienen varias salidas (gates), de las cuales algunas pueden estar activas y otras no.

Como suposición y debido a la gran complejidad (y cantidad de casos que hay que tener en cuenta), se considera que **en ningún momento se produce algún tipo de cambio** (i.e., se toma como suposición que jamás se puede caer un nodo o agregar otro, que la red no muta).

El network brindado para la cátedra para esta sección es el siguiente:

![Topología del Punto Estrella](img/Topolog%C3%ADa%20del%20Punto%20Estrella.png)

A continuación, en las siguientes secciones, se va a dar más detalle acerca de lo realizado.

## Puntos importantes que se tuvieron en cuenta (y posibles mejoras)

Respecto a los puntos importantes que se tuvo en cuenta para la idea y la implementación de la estrategia de esta sección fueron:

- No va a depender de
	- El número de nodos
	- El nombre de los nodos (se puede llamar de cualquier forma siempre que sea un entero, por ejemplo, podemos tener en la red a: "1, 6, 8, 234, 57567 y 1945765". Lo único que se pide es que entre en `int`)
	- Cantidad de interfaces (puede tener la cantidad que quiera)
	- Que todas las interfaces estén usadas (no requiere eso ya que verifica cuáles se pueden usar y cuáles no)
	- La topología o estructura de la red, ya que el mismo algoritmo y estrategia la identifican
- Va a depender de
	- Que la red es inmutable. No se cambian enlaces ni nodos, sea de agregar o eliminar
		- Una vez que se hace el cálculo para cada nodo sobre cómo debe ser el envío para cada destino posible, no es necesario recalcularlo cada $X$ tiempo ya que tenemos como suposición que la red **nunca cambia**
- Idea de escalabilidad: para facilitar la mejora de esta estrategia para hacerla para redes **no inmutables**, se hizo todo en base a que solo se tuviera que cambiar el contenido de una sola función (la que actualiza el grafo local en base al LSP recibido). El resto de funciones y archivos no hacen falta modificarlos.
	- En caso que se quiera generalizar para estos casos, solo hay que tener en cuenta que
		- Una vez recibido un paquete de tipo `actualizeNetworkInformation`, se debe schedulear el siguiente para dentro de `X` tiempo siendo `X` una constante (por ejemplo, 3 segundos)
			- Una variante es, en vez de hacer _todo_ el proceso cada `X` tiempo, lo que se hace cada ese tiempo es revisar si mis vecinos siguen siendo los mismos. En caso que lo sean, entonces no le tengo que avisar a nadie porque la representación del grafo ya la tienen. Caso contrario, si cambiaron mis vecinos, debo dar el aviso con los paquetes LSP
			- Si se considera esta variante, hay que tener en cuenta que en los casos donde se agrega un **NUEVO** nodo a la red que desconoce cómo están conectados los demás, debe tener la posibilidad de, en un campo extra del LSP que manda (una variable booleana), pedir que los nodos a los que les llegue envíen su información de vecinos pero, en vez de usar la inundación, usando el mínimo camino calculado en ese momento (i.e., se envía un solo LSP directamente por la ruta mínima hacia el nuevo nodo que me pide mi información)
		- Se debe modificar **SOLO la función** `actualizeNetworkLocalInformation` para que actualice la representación local y las decisiones para llegar a los caminos mínimos en base a los paquetes LSP recibidos (y a su versión dado que se van a hacer varias actualizaciones)
			- Todos los demás archivos y las otras funciones de `Net` no son necesarios de modificar ya que esta generalización de la condición de **inmutabilidad** se puede hacer abstrayéndose del contexto (como si lo otro fuera un TAD) y sabiendo que recibimos paquetes LSP de vez en cuando con información de la red 
- Posibilidad de DEBUGGING mientras se realizaba el proyecto
	- La clase `Net` contiene funciones que imprimen información, tales como la del nodo, de los vecinos o del grafo local guardado hasta el momento

## Idea de la estrategia de enrutamiento

En base a lo mencionado anteriormente (y lo realizado en la _tarea de diseño_), para la idea se han considerado las siguientes partes:

- Antes de cualquier envío de mensajes (en la inicialización de la clase `Net`), en cada CR se schedulea el tipo de mensaje `actualizeNetworkInformation` para:
	- Obtención de información sobre mis vecinos
		- Cada nodo se fija quiénes son sus vecinos, qué gate tiene que tomar para ir a él y se guarda esta información
			- Se envía un paquete de tipo `NeighborInfo` solicitando los nombres de los vecinos. Estos lo completan y se lo devuelven al nodo original para que pueda guardar la información
			- Con `this->getParentModule()->gate("toNod$o", gateID)->isConnected()` chequeo si la gate con índice `gateID` está conectada a algún nodo (en particular, la CR "real" tiene esta información, así que no habría drama de usar esta función)
	- Obtención de información sobre la red en general
		- Uso de paquetes **LSP** (_Link-State Packet_) que contienen la información del nombre del nodo y sus correspondientes vecinos
		- Inundación del paquete LSP de cada nodo a TODOS los demás de la red para que obtengan esta información
			- Los nombres de nodos son mapeados a índices para generalizar ante redes con nodos nombres que no necesariamente lo son
			- Cada nodo envía su LSP (y el de los demás que reciba por primera vez) a todas sus gates
		- Creación del grafo representativo
			- Cuando cada nodo pudo juntar TODOS los LSP correspondientes a los demás vecinos, significa que sabe con exactitud cómo es la red.
			- No hace falta saber cuándo se obtuvieron todos los LSP ya que se puede ir actualizando el BFS en orden de llegada de estos
	- Cálculo del enrutamiento óptimo
		- Consideramos un nodo _source_ y se aplica BFS. Si a un nodo llego primero desde una dirección, entonces para enviarle un paquete la ruta óptima será mediante la gate que tenga esa dirección (por propiedad de BFS y dado que minimizamos cantidad de saltos)
			- Esto se hace para cada LSP recibido pero el source, a diferencia de la parte de diseño, va a estar dado por el nodo principal del paquete
		- Toda esta información del recorrido BFS y la dirección que el nodo debe tomar si se quiere enviar a cierto destino, se guarda (ya que se debe calcular una sola vez)
- Durante el envío de mensajes (paquetes de datos)
	- No se realiza actualización del grafo dada la suposición de **inmutabilidad** de la red
	- Para enviar o reenviar un mensaje de dato, se sabe por qué gate hacerlo en base a la información guardada en la sección de precómputo del enrutamiento óptimo
		- En caso que no sepamos, esto puede significar dos cosas
			- O bien me falta que lleguen paquetes LSP para terminar de actualizar mi grafo local
			- O bien el emisor y receptor están en componentes conexas distintas (i.e., no hay un camino que los una)

En particular, la idea _base_ es similar a la parte de diseño (ya que esa fue la idea para facilitar el desarrollo de las dos estrategias) aunque respecto a la implementación se prefirió realizarla desde cero.

- Esta decisión fue tomada para poder organizar mucho mejor el código para generalizar la estrategia
- Teniendo bien clara la idea en mente a la hora de codear (ya habiendo hecho gran parte en el diseño), la primera parte fue rápida. Se tuvo que prestar atención a la hora de hacer la actualización de la representación del network de forma local pero no es mucho más que un BFS que se va haciendo _de a partes_ (en base a la llegada de los LSP)