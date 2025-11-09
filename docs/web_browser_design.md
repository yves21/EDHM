# Navigateur Web pour l'OS EDHM

Ce document décrit l'architecture proposée pour intégrer un navigateur web natif à notre système d'exploitation expérimental. L'objectif est de fournir un navigateur moderne avec trois fonctionnalités clés : la gestion des favoris, un cache persistant et un mode de navigation privée.

## Vision générale

Le navigateur est conçu comme une application utilisateur reposant sur les services suivants du noyau :

- **Pile réseau** : sockets TCP/IP, résolveur DNS, gestion TLS via la bibliothèque `libtls` du système.
- **Système de fichiers virtuel** : stockage du profil, des favoris et du cache.
- **Gestionnaire de fenêtres** : rendu via le compositeur du shell graphique.
- **Services de sécurité** : sandboxing par espaces d'adresses isolés et politiques d'autorisations.

## Découpage des composants

| Composant | Rôle | Détails clés |
|-----------|------|--------------|
| `browser/core` | Boucle principale, orchestration des onglets, coordination des services partagés. | Gère le cycle événementiel, la planification des tâches réseau/rendu, le contrôle des processus d'onglets. |
| `browser/net` | Empilement réseau HTTP/HTTPS | Implémente HTTP/1.1 et HTTP/2 via la bibliothèque TLS, gestion des cookies et des en-têtes.
| `browser/render` | Moteur de rendu HTML/CSS/JS | Basé sur un moteur DOM multi-processus, pipeline de parsing, layout, painting avec GPU.
| `browser/storage` | Persistance des favoris, historique, cache disque. | Interface uniforme vers le VFS, chiffrage optionnel des données sensibles.
| `browser/ui` | Interface utilisateur | Toolkits natifs (widgets, barres d’outils, gestion des thèmes, raccourcis clavier). |
| `browser/private` | Gestion du mode privé | Fournit un contexte d’exécution éphémère sans persistance de cookies ni historique. |

## Gestion des favoris

- **Stockage** : fichier JSON (`~/.config/browser/bookmarks.json`) manipulé via une API de persistance transactionnelle.
- **Fonctionnalités** :
  - Collections hiérarchiques (dossiers, sous-dossiers) avec identifiants stables.
  - Synchronisation optionnelle via WebDAV/EDFS.
  - Import/export au format HTML standard.
- **Interface** : panneau latéral, barre de favoris, raccourci `Ctrl+D`.

## Gestion du cache

- **Cache disque** : répertoire `~/.cache/browser/` structuré par hachage d’URL.
- **Politique d’éviction** : LRU segmentée (fraîche vs. réutilisable) afin de prioriser les ressources critiques.
- **Compression** : ressources textuelles compressées avec Brotli, métadonnées dans un index binaire.
- **Validation** : prise en charge d’`ETag`, `Last-Modified` et des codes `304 Not Modified`.
- **Isolation** : chaque profil possède son propre espace de cache, les contexts privés utilisent un cache mémoire volatile.

## Mode de navigation privée

- **Contexte éphémère** : stockage des cookies, cache et historique en RAM uniquement.
- **Isolation de processus** : onglets privés exécutés dans des processus dédiés sans accès aux profils persistants.
- **Protection des fuites** : désactivation de l’écriture sur disque, purge mémoire sécurisée à la fermeture.
- **API** : un simple toggle permet d’ouvrir une « fenêtre privée » avec un thème visuel distinct.

## Identité et sécurité avancées

- **Coffre de mots de passe** : base chiffrée (`~/.local/share/browser/credentials.db`) protégée par la clé maître du profil. Les mots de passe sont remplis via un agent IPC et jamais exposés en clair à l’UI.
- **Gestion des permissions** : centre de contrôle site-par-site stockant les autorisations accordées (caméra, micro, notifications). Chaque requête déclenche une validation auprès de `secdaemon` et peut être révoquée depuis un panneau dédié.
- **Protection anti-suivi** : listes de filtrage mises à jour signées (`filters/bloklist.dat`), exécution dans un processus spécialisé qui assainit les requêtes sortantes.

## Expérience utilisateur et extensibilité

- **Moteur d’extensions léger** : manifestes déclaratifs (`manifest.json`) permettant d’ajouter des scripts de contenu, des thèmes et des panneaux UI supplémentaires. Les extensions sont isolées en sandbox et nécessitent une signature.
- **Outils développeur** : console JavaScript, inspecteur DOM et enregistreur réseau intégrés dans `browser/devtools`. Les outils communiquent via un protocole de débogage exposé par le moteur de rendu.
- **Gestion multi-profils** : sélecteur de profil au démarrage, profils stockés dans `~/.config/browser/profiles/` avec paramètres isolés.

## Résilience hors-ligne et synchronisation

- **Mode lecture hors connexion** : sauvegarde des pages marquées « à lire plus tard » dans un format autoportant (`.edread`) comprenant styles et images compressées.
- **Synchronisation** : service `syncd` optionnel capable de répliquer favoris, mots de passe et onglets via WebDAV/EDFS ou une passerelle pair-à-pair chiffrée.
- **Détection de connectivité** : observateur de réseau notifiant l’UI des transitions en ligne/hors ligne et adaptant les comportements du cache.

## Flux d’exécution d’un onglet

1. L’utilisateur saisit une URL ; `browser/core` demande à `browser/net` de résoudre et d’effectuer la requête.
2. Les octets reçus sont transmis au parser HTML, qui construit le DOM.
3. `browser/render` calcule le layout, applique les feuilles de style, et programme le rasterizer GPU.
4. Les scripts JavaScript s’exécutent dans un moteur isolé (processus `js_worker`).
5. Les ressources sont mises en cache selon la politique définie, en respectant le mode (normal/privé).
6. La vue est affichée par le compositeur et les événements utilisateur (clavier/souris) retournent à `browser/core`.

## Interactions avec le système

- **Service de notifications** : intégration avec le centre de notifications du shell pour les alertes (téléchargements, permissions).
- **Téléchargements** : s’appuient sur l’API `downloadd` qui gère la persistance, les reprises et les validations de signature.
- **Gestion des permissions** : boîte de dialogue unifiée (géolocalisation, caméra, micro) qui s’appuie sur le démon de sécurité.

## Prochaines étapes

1. Prototyper la bibliothèque `libbrowser` avec parsing HTML minimal et affichage texte.
2. Intégrer le moteur de rendu 2D existant (`libcanvas`) pour un premier rendu graphique.
3. Ajouter la persistance des favoris et un gestionnaire de cache basique.
4. Implémenter l’isolation du mode privé avant d’activer le JavaScript avancé.

Ce plan offre une base solide pour développer un navigateur web complet, cohérent avec les briques que nous avons déjà construites dans le système.
