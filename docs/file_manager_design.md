# Gestionnaire de fichiers pour l'OS EDHM

Ce document détaille la conception d'un gestionnaire de fichiers graphique destiné à l'environnement de bureau de notre système d'exploitation. L'objectif est de fournir une application intuitive capable de parcourir, organiser et sécuriser les données des utilisateurs tout en exploitant les services du noyau et du shell.

## Vision générale

Le gestionnaire de fichiers (`files/`) repose sur une architecture modulaire découpée en trois blocs :

- **Core** : moteur d'exploration des systèmes de fichiers, exécution des opérations (copie, déplacement, suppression, renommage), gestion des permissions.
- **UI** : interface graphique construite sur le toolkit natif (`libui`), proposant vues liste et grille, barre latérale, et barre d'adresse intelligente.
- **Services** : intégrations optionnelles (indexation, recherche plein texte, montage distant, file d'opérations). Chaque service communique via des bus IPC standardisés.

## Fonctionnalités principales

### Navigation et affichage

- Vues multiple colonnes : liste détaillée (nom, taille, type, dates) et grille d'icônes adaptative.
- Barre de navigation avec historique (précédent/suivant), fil d'Ariane et favoris d'emplacements.
- Prise en charge de plusieurs onglets et fenêtres pour la gestion parallèle.

### Opérations sur les fichiers

- Copie/déplacement via file d'opérations asynchrone (`files/core/operations_queue.rs`).
- Suppression avec corbeille virtuelle (`~/.local/share/Trash`) et purge programmée.
- Renommage en ligne avec détection de conflits et suggestions automatiques.
- Gestion avancée des permissions POSIX et ACL via un dialogue dédié.

### Recherche et indexation

- Service `files/services/indexer` qui surveille les changements via inotify/ednotify.
- Indexation des métadonnées (nom, type, tags) et intégration optionnelle de contenu (texte, PDF) via plugins.
- Barre de recherche unifiée avec filtres dynamiques (type, taille, date, tags).

### Favoris et emplacements spéciaux

- Panneau latéral configurable listant les emplacements fréquents (Documents, Téléchargements, Volumes montés).
- Synchronisation des favoris avec l'environnement du navigateur (`~/.config/user-places.json`).
- Raccourcis clavier (`Ctrl+L` pour la barre d'adresse, `Ctrl+T` pour un nouvel onglet, `Ctrl+D` pour ajouter un favori).

### Mode privé / Invité

- Sessions temporaires qui montent un espace utilisateur éphémère (overlayfs en mémoire) sans persistance.
- Nettoyage automatique de l'historique récent et des métadonnées à la fermeture.
- Indicateur visuel distinct (thème sombre + bannière « Invité »).

### Accessibilité et internationalisation

- Support complet de la navigation clavier, lecteurs d'écran via l'API d'accessibilité (`libaccess`).
- Gestion des textes RTL et des locales multiples, formats de date et numérations adaptatives.

## Interaction avec le système

- **Gestionnaire de fenêtres** : intégration avec les bureaux virtuels, drag & drop entre applications.
- **Gestion des volumes** : communication avec `vold` pour monter/démonter les périphériques externes (USB, réseau).
- **Sécurité** : toutes les opérations potentiellement destructrices passent par `secdaemon` pour validation des politiques.
- **Notifications** : retours utilisateur via le centre de notifications (copies longues, erreurs, demandes d'autorisation).

## Extensibilité

- Plugin API (`files/plugins/`) permettant d'ajouter de nouveaux panneaux (aperçu images, lecteurs audio), de nouveaux protocoles (SFTP, WebDAV) ou des actions contextuelles.
- Scripts d'automatisation via `filesctl` exposant une interface CLI pour orchestrer des tâches.

## Partage et collaboration

- **Liens sécurisés** : génération de liens temporisés (`edhm://share/<uuid>`) dont les permissions (lecture, écriture) sont validées par `secdaemon`. Les invités peuvent accéder aux ressources via le client bureau ou le navigateur.
- **Groupes de travail** : définition de groupes synchronisés avec le gestionnaire d’identités pour partager des dossiers entiers avec journalisation des accès.
- **Audit** : toutes les opérations partagées sont consignées dans `~/.local/share/files/logs/audit.jsonl` consultable depuis un panneau « Historique des partages ».

## Automatisation et flux de travail

- **Règles intelligentes** : moteur `files/services/rulesd` capable de surveiller des dossiers et déclencher des actions (renommer, déplacer, exécuter un script) selon des critères déclaratifs (`rules.yaml`).
- **File d’opérations enrichie** : priorité des tâches, quotas par utilisateur et possibilité de suspendre/reprendre des lots massifs.
- **API externe** : interface gRPC optionnelle permettant à d’autres applications d’invoquer les opérations du gestionnaire de fichiers.

## Observabilité et performances

- **Tableau de bord** : vue interne affichant les métriques de la file d’opérations (débit, latence moyenne, erreurs) et l’état de l’indexeur.
- **Profilage** : instrumentation via `files/tools/profiler` pour mesurer le temps passé dans les opérations I/O et détecter les goulots d’étranglement.
- **Alertes** : intégration avec le centre de notifications pour prévenir des volumes saturés ou d’un index obsolète.

## Bureau partagé et collaboration distante

- **Projection du bureau** : le gestionnaire de fenêtres expose un flux vidéo/audio compressé (`deskshared`) accessible via le protocole interne `rdp-lite`. Les sessions partagées utilisent la même instance graphique afin que l'utilisateur distant voie exactement l'état du bureau local.
- **Saisie synchronisée** : les événements clavier/souris du participant distant transitent par un canal chiffré IPC ⇄ `secdaemon`, puis sont réinjectés dans la file d'événements du gestionnaire de fenêtres. Un indicateur de présence (curseur coloré + avatar) distingue les actions distantes.
- **Gestion des permissions** : chaque partage déclenche une demande d'autorisation listant les droits accordés (affichage seul, contrôle, transfert de fichiers). Les accès sont limités temporellement et peuvent être révoqués instantanément depuis la barre système.
- **Transferts de fichiers intégrés** : le gestionnaire de fichiers détecte les sessions `deskshared` actives et propose un panneau « Participants » permettant de glisser-déposer des fichiers vers l'invité ; le transfert emprunte `files/services/transferd` avec chiffrement bout-en-bout et suivi de progression unifié.
- **Mode invité** : combiné avec le mode privé, il est possible d'ouvrir une session de bureau éphémère où toutes les modifications restent volatiles, pratique pour l'assistance ponctuelle.

## Flux d'opérations

1. L'utilisateur sélectionne une action (ex : copier) via l'UI.
2. `files/ui` crée une requête IPC vers `files/core` qui valide les permissions et planifie l'opération.
3. L'opération est placée dans `operations_queue` qui distribue les tâches aux workers.
4. L'état de progression est renvoyé à l'UI (barre de progression, notifications).
5. Les événements système (inotify) rafraîchissent automatiquement les vues concernées.

## Prochaines étapes

1. Prototyper l'explorateur en mode liste avec navigation basique.
2. Implémenter la file d'opérations et les dialogues de confirmation.
3. Ajouter la barre latérale des emplacements et la corbeille virtuelle.
4. Intégrer l'indexation et la recherche plein texte.
5. Définir le module `deskshared` (capture, transport, contrôle) et son intégration dans le gestionnaire de fichiers.
6. Planifier `rulesd` et le tableau de bord de supervision afin de sécuriser l’automatisation et la visibilité des opérations.

Ce plan fournit une base complète pour livrer un gestionnaire de fichiers robuste aligné avec les capacités existantes du système EDHM et le navigateur web conçu précédemment.
