# Traitement de texte basique pour l'OS EDHM

Ce document présente la conception d'un éditeur de texte enrichi simple mais complet pour l'environnement de bureau EDHM. L'objectif est d'offrir une application légère capable de créer, modifier et mettre en forme des documents tout en restant intégrée aux services du système (stockage, sécurité, collaboration).

## Vision générale

Le traitement de texte (`writer/`) est pensé comme une application modulaire composée des blocs suivants :

- **Core** : moteur de document, modèle de texte riche, gestion des fichiers et historique d'annulation.
- **UI** : interface graphique avec ruban minimal, zones de style, barre d'état et panneaux contextuels.
- **Services** : fonctionnalités additionnelles (collaboration, export PDF, orthographe) exposées via des modules optionnels.

## Format de document

- **Format natif** : `.edoc`, basé sur un conteneur ZIP contenant un XML de structure (paragraphes, styles) et les ressources (images, styles personnalisés).
- **Compatibilité** : import/export vers `.txt` (texte brut), `.rtf` (texte enrichi) et `.md` (Markdown) via des convertisseurs dans `writer/io/`.
- **Styles** : feuille de style embarquée décrivant la typographie, les couleurs et les listes numérotées/pucées.

## Modèle de document et fonctionnalités

- **Édition riche** : prises en charge des styles (gras, italique, souligné, surligné), alignements, retrait et interligne.
- **Structures** : titres hiérarchiques, listes, tableaux simples, blocs de citation et images ancrées.
- **Historique** : undo/redo multi-niveaux basé sur un journal différentiel (`writer/core/history.rs`).
- **Rechercher/remplacer** : moteur d'expressions régulières avec surlignage et options sensibles à la casse/accents.
- **Gestion des sections** : pages, en-têtes/pieds de page, numérotation automatique.

## Interface utilisateur

- **Barre d'outils compacte** : boutons contextuels pour les styles, menus déroulants pour les polices et tailles, raccourcis clavier (`Ctrl+B`, `Ctrl+I`, `Ctrl+U`).
- **Barre latérale** : navigation par titres, styles rapides, gestion des commentaires.
- **Barre d'état** : indicateurs de langue, mode de saisie, nombre de mots et suivi de synchronisation.
- **Mode distraction minimale** : bascule plein écran avec masquage des panneaux inutilisés.

## Gestion des fichiers

- **Autosauvegarde** : snapshots périodiques dans `~/.local/share/writer/autosave/` avec récupération après crash.
- **Versioning** : intégration avec le service `files/services/historyd` pour conserver un historique des documents.
- **Compatibilité cloud** : ouverture directe depuis les emplacements distants gérés par le gestionnaire de fichiers (WebDAV, SFTP).

## Collaboration et partage

- **Commentaires** : annotations contextualisées, réponses et résolution, stockées dans le document.
- **Coédition** : mode temps réel facultatif via le service `collabd`, avec curseurs multiples et verrouillage granulaire par paragraphe.
- **Suivi des modifications** : journal des insertions/suppressions par auteur, acceptation ou rejet par section.

## Vérification linguistique

- **Correcteur orthographique** : dictionnaires `*.dic`/`*.aff` chargés par langue, suggestions dans un menu contextuel.
- **Grammaire de base** : règles heuristiques (accords simples, ponctuation) évaluées en tâche de fond.
- **Synonymes** : mini-thésaurus pour les langues principales stocké dans `writer/data/thesaurus.db`.

## Export et impression

- **Export PDF** : conversion via `libpdf` intégrée, respect des styles et des images.
- **Aperçu avant impression** : rendu paginé fidèle avec ajustement des marges et des en-têtes.
- **Modèles** : bibliothèque de modèles (`~/.config/writer/templates/`) sélectionnables depuis l'écran d'accueil.

## Accessibilité

- **Navigation clavier complète** : focus cyclique, raccourcis pour chaque commande du ruban.
- **Lecteur d'écran** : expose l'arborescence du document via `libaccess` pour narrer paragraphes et attributs.
- **Modes contrastés** : thèmes haute visibilité et support du mode sombre système.

## Intégration avec l'OS

- **Gestionnaire de fichiers** : ouverture/sauvegarde via l'API `files/core`, support du glisser-déposer.
- **Notifications** : alertes pour autosauvegarde, coédition, conflits de version.
- **Sécurité** : sandboxing des macros (si activées) via un moteur de scripts restreint (`writer/scripts/`).

## Prochaines étapes

1. Créer un prototype en texte riche avec affichage basique et support du format `.edoc` minimal.
2. Implémenter l'historique undo/redo et la barre d'outils principale.
3. Ajouter l'export `.txt`/`.md` puis l'autosauvegarde.
4. Intégrer le correcteur orthographique et le mode coédition facultatif.

Ce plan permet de livrer un traitement de texte simple mais efficace, bien intégré aux autres composants d'EDHM.
