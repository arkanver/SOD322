## TME 3 - Label Propagation / Algorithme de Louvain

Ce dossier contient les codes nécessaires à :
  - la génération de graphes comprenant des clusters (graphGenerator.c)
  - un algorithme de label propagation (labelpropagation.c)
  - l'algorithme de Louvain, compacté en un seul code (louvain.c), d'après https://github.com/jlguillaume/louvain (Jean-Loup Guillaume)
  
Chacun de ces codes se compile de façon classique avec gcc par exemple. Les options d'optimisation (-O3 par exemple) seront utiles.

Le dossier RandomGraphs contient des exemples de graphes générés à l'aide de graphGenerator. Pour utiliser ce code, il faut donner en entrée :
  - Le chemin vers le graphe créé
  - Le nombre de clusters
  - Le nombre de sommets par cluster
  - La probabilité de présence d'une arête entre deux sommets d'un même cluster
  - La probabilité de présence d'une arête entre deux sommets de clusters différents
  
Pour utiliser labelpropagation ou louvain, il suffit de signifier le chemin vers le graphe à traiter et le chemin vers le résultat donné.

Le dossier visualisation contient deux codes python, qui dans leur état actuel ne sont pas fait pour être utilisés en ligne de commande.
graphvis.py permet de visualiser les graphes générés aléatoirement. labelvis.py permet de visualiser les communautés détectées en disposant les 
sommets de façon circulaire. Il ne marchera qu'avec 4 communautés ou moins.

Les sous-dossiers Graphs et LabeledGraphs contiennent des exemples de visualisation à l'aide de ces deux codes Python.
