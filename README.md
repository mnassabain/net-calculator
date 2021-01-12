# Net calculator

## About

Le but de ce projet est de réaliser un système de calcul distribué, c’est-à-dire un système permettant de déterminer des valeurs fk(x1, x2 ... xn), le calcul des fk étant effectué sur des « noeuds » spécialisés. Le système doit permettre l’ajout ou le retrait de noeuds, voire la panne d’un noeud sans provoquer de perturbation s’il y a une redondance.

L’orchestrateur est le programme central : il est le point de convergence de plusieurs flux provenant des autres noeuds de calcul. Dans ce projet il y a un noeud par opération de base (addition, soustraction, multiplication et division).

Pour plus de détails veuillez consulter les fichiers de documentation.

## Manuel

```
# Compiler tous les programmes
make
```