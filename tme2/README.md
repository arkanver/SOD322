Ce dossier contient les programmes et fichiers nécessaires à la réalisation du TME n°2.

Pour réaliser la partie sur pagerank :
- compiler le programme avec make -C pagerank
- créer un fichier contenant les degrés entrant et sortant des noeuds.
    degree/degree wiki_graph/directedLinks.txt results/wikiDegrees 1

- choisir un seuil de convergence pour le pagerank (ici, 1e-10)

- pour chaque valeur de alpha, générer le fichier contenant le pagerank. Attention, les fichiers de résultat doivent partager un même préfixe et s'achever respectivement par "_0_1", "_0_15", "_0_2", "_0_5" et "_0_9"
    pagerank/pagerank wiki_graph/directedLinks.txt results/wikiPageRank_0_1 0.1 1e-10 history_0_1.txt
    pagerank/pagerank wiki_graph/directedLinks.txt results/wikiPageRank_0_15 0.15 1e-10 history_0_15.txt
    pagerank/pagerank wiki_graph/directedLinks.txt results/wikiPageRank_0_2 0.2 1e-10 history_0_2.txt
    pagerank/pagerank wiki_graph/directedLinks.txt results/wikiPageRank_0_5 0.5 1e-10 history_0_5.txt
    pagerank/pagerank wiki_graph/directedLinks.txt results/wikiPageRank_0_9 0.9 1e-10 history_0_9.txt

- enfin, pour produire les graphiques et le fichier contenant les palmarès, utiliser le script python dédié.
    python pagerank_results.py results/wikiPageRank results/wikiDegrees

- il est à noter que dans les fichiers 'history', on trouve l'historique de convergence

Les différents graphiques ainsi que le palmarès sont alors enregistrés dans le dossier results.

Pour réaliser la partie sur la k-core-decomposition :
    Pour compiler le programme :
        make -C k-core-decomposition

    Pour réaliser la décomposition en k-noyaux d'un graphe :
        k-core-decomposition/k-core-decomposition path/to/graph desired/path/to/results

    Format du fichier de résultat :
    -les lignes 1 à N_SOMMET sont au format
        "ORDER_IN_DECOMP NODE CORENESS" où ORDER_IN_DECOMP est la place du sommet dans la décomposition, NODE est l'indice du sommet, commençant à 1, et CORENESS est la k-valeur du sommet.
    -la ligne N_SOMMET+1 est au format 
        "k_value K_VALUE" où K_VALUE est l'écriture en base 10 de la k-valeur du graphe
    - la ligne N_SOMMET+2 est au format
        "execution_time TMPS" où TMPS est le temps d'exécution, en secondes, de l'algorithme de décomposition en k-noyaux, qu'il convient de bien distinguer du temps d'exécution du programme.

    Les lignes sont enregistrées par ordre décroissant dans la décomposition en k-noyaux.

    Pour l'analyse du graphe de "Google Scholar", exécuter les commandes suivantes :
        degree/degree scholar_graph/scholar-graph.txt results/scholarDegrees 0
        k-core-decomposition/k-core-decomposition scholar_graph/scholar-graph.txt results/scholar-k-decomp.txt
	python detect_anomaly_in_scholar.py
        
