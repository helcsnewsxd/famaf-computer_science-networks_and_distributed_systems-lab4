# Análisis e Implementación de Algoritmos de Enrutamiento de la Capa de Red en Networks simulados en Omnet++

## Tarea de Diseño

### Introducción y planteo del problema

En esta sección del proyecto, lo que se pretende es implementar un algoritmo que utilice alguna estrategia para minimizar la **cantidad de saltos** realizados por los paquetes en su envío a un destino. Es decir, se busca que se implemente una estrategia de enrutamiento (preferentemente óptima) aplicada en particular a este tipo de redes para poder comparar el rendimiento respecto al de la _Tarea de Análisis_

La red a considerar consta de **8 nodos** (no es relevante la cantidad ya que la estrategia no depende de ello) conectados con topología de _anillo_ y considerando que **en ningún momento se produce algún tipo de cambio** (i.e., se toma como suposición que jamás se puede caer un nodo o agregar otro, que la red no muta).

![Topología de Anillo](img/Topolog%C3%ADa%20de%20Anillo.png)

A continuación, en las siguientes secciones, se va a dar más detalle acerca de lo realizado.

### Estructura a considerar

Respecto a la estructura general bajo la cual se consideró óptima realizar el proyecto (no tiene nada que ver con la estrategia), tomamos óptimo en base a la naturaleza y la idea el laboratorio, separar nuestro código del siguiente modo:

![Estructura general del proyecto](img/Estructura%20general%20del%20proyecto.png)

Esto fue realizado por dos cuestiones principales:

- Comodidad
- Se investigó y es una estructura _usual_ (separar src y simulations) que tienen los proyectos en Omnet++

### Puntos importantes que se tuvieron en cuenta

Respecto a los puntos importantes que se tuvo en cuenta para la idea y la implementación de la estrategia de esta sección fueron:

- No va a depender de
	- El número de nodos. **Solo se pide que sean 3 o más** (por cómo se hizo el BFS)
	- El nombre de los nodos (se puede llamar de cualquier forma siempre que sea un entero, por ejemplo, podemos tener en la red a: "1, 6, 8, 234, 57567 y 1945765". Lo único que se pide es que entre en `int`)
- Va a depender de
	- Que la red tiene topología de anillo
		- Cada nodo solo puede ir a la izquierda o a la derecha. Luego, el paquete o bien se envía _todo a la derecha_ o bien _todo a la izquierda_
	- Que la red es inmutable. No se cambian enlaces ni nodos, sea de agregar o eliminar
		- Una vez que se hace el cálculo para cada nodo sobre cómo debe ser el envío para cada destino posible, no es necesario recalcularlo cada $X$ tiempo ya que tenemos como suposición que la red **nunca cambia**
- Idea de escalabilidad: para facilitar el desarrollo del punto estrella, varias secciones se hicieron de forma más _general_ y la idea se moldeó en base a eso. Esto se hizo para no tener que recodear todo el proyecto a la hora de hacer el punto estrella para implementar Dijkstra.
- Posibilidad de DEBUGGING mientras se realizaba el proyecto
	- El archivo `Net.cc` contiene la flag DEBUG como una constante booleana, la cual especifica si se desea que se imprima la información obtenida en el precómputo para chequear que esté bien. Por defecto está configurada en `false`.

### Idea de la estrategia de enrutamiento

En base a lo mencionado anteriormente, para la idea se han considerado las siguientes partes:

- Antes de cualquier envío de mensajes (en la inicialización de la clase `Net`), en cada CR se hace:
	- Obtención de información sobre mis vecinos
		- Cada nodo se fija quiénes son sus vecinos y en qué dirección está cada uno (si izquierda o derecha) y se guarda esta información
			- Se envía un paquete de tipo `NeighborInfo` solicitando los nombres de los vecinos. Estos lo completan y se lo devuelven al nodo original para que pueda guardar la información
	- Obtención de información sobre la red en general
		- Uso de paquetes **LSP** (_Link-State Packet_) que contienen la información del nombre del nodo y sus correspondientes vecinos
		- Inundación del paquete LSP de cada nodo a TODOS los demás de la red para que obtengan esta información
			- Los nombres de nodos son mapeados a índices para generalizar ante redes con nodos nombres que no necesariamente lo son
			- Cada nodo envía su LSP (y el de los demás que reciba por primera vez) a todas sus gates
		- Creación del grafo representativo
			- Cuando cada nodo pudo juntar TODOS los LSP correspondientes a los demás vecinos, significa que sabe con exactitud cómo es la red (en particular, siempre supimos la forma de anillo pero no la posición de los nodos ni la cantidad).
			- Se puede saber en topología de anillo cuándo se obtuvieron todos los LSP ya que esto sucede cuando la cantidad de nodos distintos recibidos por LSP (sea nodo principal como vecinos) es igual a la cantidad de archivos LSP enviados (_es trivial de ver_).
	- Cálculo del enrutamiento óptimo
		- Se considera como source el nodo en el que se está y se aplica BFS yendo a izquierda como a derecha. Si a un nodo llego primero desde una dirección, entonces para enviarle un paquete la ruta óptima será mediante la gate que tenga esa dirección (por propiedad de BFS y dado que minimizamos cantidad de saltos en topología de anillo)
		- Toda esta información del recorrido BFS y la dirección que el nodo debe tomar si se quiere enviar a cierto destino, se guarda (ya que se debe calcular una sola vez)
- Durante el envío de mensajes (paquetes de datos)
	- No se realiza actualización del grafo dada la suposición de inmutabilidad de la red
	- Para enviar o reenviar un mensaje de dato, se sabe por qué gate hacerlo en base a la información guardada en la sección de precómputo del enrutamiento óptimo

### Implementación

#### Obtener información de mis vecinos

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
- Si me llega de vuelta el paquete que le envié a mi vecino $V$, entonces actualizo los datos locales en base a los que me brindaron
- Cuando recibo toda la información que necesito (i.e., `neighborReached == cntNeighbor (= 2)`), procedo con el envío de los **LSP**

#### Obtener información de los vecinos de todos los nodos

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

#### Cálculo del enrutamiento óptimo

- Se va a considerar lo siguiente para guardar la información del enrutamiento óptimo y permitir el acceso para la parte del envío de paquete de datos
	```cpp
	virtual int getGateToSend(int nodeName);
	vector<int> gateToSend;
	```
- Esta parte se hace aplicando simplemente BFS y considerando todas las gates inicializadas en $-1$ (para diferenciar en visitados y no). No es nada tan complejo
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

#### Envío de paquetes de datos

En esta sección, si el paquete es de datos (i.e., el destino es $\geq 0$ ), entonces me fijo si lo tengo que enviar a `App` o reenviar a otro nodo.
```cpp
if (pkt->getDestination() == nodeName) // Enviar a App
	send((cMessage *)pkt, "toApp$o");
else // Reenviar a otro nodo
	send((cMessage *)pkt, "toLnk$o", getGateToSend(pkt->getDestination()));
	// getGateToSend(pkt->getDestination()) me dice por dónde debo llevar el paquete para que llegue a destino con la MÍNIMA cantidad de saltos posible
```

### Análisis en base a las métricas obtenidas

**_PARTE DE FACU Y JUAN_**

### Conclusiones acerca de esta implementación

**_PARTE DE FACU Y JUAN_**
