## Tarea Estrella

### Introducción y planteo del problema

En esta sección del proyecto, lo que se pretende es implementar un algoritmo que utilice alguna estrategia para minimizar la **cantidad de saltos** realizados por los paquetes en su envío a un destino. Es decir, se busca que se implemente una estrategia de enrutamiento (preferentemente óptima) aplicada **PARA CUALQUIER TIPO DE TOPOLOGÍA**.

Es decir, se pretende generalizar la tarea de diseño para que el algoritmo de enrutamiento pueda "reconocer" la forma de la red y, en base a ello, calcular cómo debe enviar los datos.
Además, se considera que los nodos tienen varias salidas (gates), de las cuales algunas pueden estar activas y otras no.

Como suposición y debido a la gran complejidad (y cantidad de casos que hay que tener en cuenta), se considera que **en ningún momento se produce algún tipo de cambio** (i.e., se toma como suposición que jamás se puede caer un nodo o agregar otro, que la red no muta).

El network brindado para la cátedra para esta sección es el siguiente:

![Topología del Punto Estrella](img/Topolog%C3%ADa%20del%20Punto%20Estrella.png)

A continuación, en las siguientes secciones, se va a dar más detalle acerca de lo realizado.

### Puntos importantes que se tuvieron en cuenta (y posibles mejoras)

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

### Idea de la estrategia de enrutamiento

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

### Análisis en base a las métricas obtenidas

**_PARTE DE FACU Y JUAN_**

### Conclusiones acerca de esta implementación

**_PARTE DE FACU Y JUAN_**
