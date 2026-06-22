#include <TFT_eSPI.h> // Bibliothèque principale pour gérer l'affichage sur l'écran LCD/TFT
#include <SPI.h>      // Bibliothèque pour la communication série SPI avec l'écran

TFT_eSPI tft = TFT_eSPI(); // Création de l'objet "tft" pour interagir avec l'écran

// --- Configuration des Pins (Broches de l'ESP32) ---
#define BTN_ORANGE  25  // Bouton Orange connecté à la broche 25
#define BTN_VERT    26  // Bouton Vert connecté à la broche 26
#define BTN_BLEU    27  // Bouton Bleu connecté à la broche 27
#define BTN_JAUNE   32  // Bouton Jaune connecté à la broche 32
#define BUZZER_PIN  14  // Broche pour le haut-parleur/buzzer (effets sonores)


// --- Couleurs (Format RGB565 / Hexadécimal 16-bits) ---
#define MY_RED       0xF800
#define MY_BLUE      0x001F
#define MY_GREEN     0x07E0 
#define MY_YELLOW    0xFFE0
#define MY_ORANGE    0xFD20  
#define MY_SKY       0x5DFF // Couleur bleu ciel pour le fond des menus
#define MY_GREY      0x4208
#define GRID_LINE    0x2124 // Couleur sombre pour les lignes de la grille de jeu

// --- Variables de Jeu ---
// "enum" crée une liste de mots-clés représentant les différentes pages/écrans du jeu
enum Page { PRINCIPAL, NIVEAUX, PARAMETRES, COULEUR, LANGUE, JEU_NIV1, JEU_NIV2, JEU_NIV3 }; 
Page pageActuelle = PRINCIPAL; // Le jeu commence sur l'écran du menu principal

uint16_t couleurMarioActive = MY_RED; // Couleur de départ du personnage (Mario)
int indexCurseurCouleur = 0;          // Position du curseur dans le menu de sélection des couleurs
uint16_t palette[] = {0xF844, 0xFBE0, 0xF81F, 0x001F, 0x0400, 0x780F, 0xD6BA}; // Tableau contenant 7 couleurs au choix

bool estAnglais = false; // Gestion de la langue : false = Français, true = Anglais

// Variables pour le gameplay
int marioX, marioY;             // Coordonnées de Mario sur la grille (colonnes et lignes)
const int cibleX = 15, cibleY = 7; // Coordonnées fixes de la case d'arrivée (objectif)
byte grille[16][8];             // Grille de jeu de 16 colonnes par 8 lignes (0 = vide, 1 = obstacle)
int nbEnnemisActuels = 0;       // Nombre d'ennemis présents sur la carte selon le niveau
int ennemiX[11], ennemiY[11];   // Tableaux pour stocker les coordonnées de chaque ennemi (max 11)

// Prototypes de fonctions. Indiquent au compilateur que ces fonctions existent plus bas dans le code, évitant ainsi des erreurs de compilation.
void changerDePage(Page p);
void drawMenuCouleurs();
void drawGrilleJeu();

// --- GESTION DES SONS  ---
void bruitSelection() {
  tone(BUZZER_PIN, 1000, 50); // Un bip court et aigu lors d'un clic de bouton
}

void bruitVictoire() {
  tone(BUZZER_PIN, 1500, 150); delay(150); // Petite mélodie victoire
  tone(BUZZER_PIN, 2000, 150); delay(150);
  tone(BUZZER_PIN, 2500, 300);
}

void bruitDefaite() {
  tone(BUZZER_PIN, 400, 200); delay(200);  // Petite mélodie de la défaite 
  tone(BUZZER_PIN, 300, 200); delay(200);
  tone(BUZZER_PIN, 200, 400);
}

// --- LOGIQUE MENU COULEUR ---
void handleMenuCouleurLogic() { 
  bool update = false; // Devient vrai si le joueur déplace le curseur (demande de redessiner)
  
  if (digitalRead(BTN_ORANGE) == LOW) { // Si appui sur le bouton Orange (Déplacement Gauche)
    if (indexCurseurCouleur > 0) { indexCurseurCouleur--; update = true; bruitSelection(); } 
    delay(200); // Anti-rebond (debounce) pour éviter les sauts trop rapides
  }
  if (digitalRead(BTN_JAUNE) == LOW) { // Si appui sur le bouton Jaune (Déplacement Droite)
    if (indexCurseurCouleur < 6) { indexCurseurCouleur++; update = true; bruitSelection(); } 
    delay(200); 
  }
  if (digitalRead(BTN_VERT) == LOW) { // Bouton Vert pour valider la couleur sélectionnée
    couleurMarioActive = palette[indexCurseurCouleur]; // Applique la couleur choisie à Mario
    bruitSelection();
    changerDePage(PARAMETRES); // Retour au menu des paramètres
    while(digitalRead(BTN_VERT) == LOW); // Attend que le joueur relâche le bouton
  }
  if (digitalRead(BTN_BLEU) == LOW) { // Bouton Bleu pour annuler et revenir en arrière
    bruitSelection();
    changerDePage(PARAMETRES); 
    while(digitalRead(BTN_BLEU) == LOW); 
  }
  if (update) drawMenuCouleurs(); // Si le curseur a bougé, on rafraîchit l'affichage du menu
}

// --- LOGIQUE MENU LANGUE ---
void handleMenuLangueLogic() { 
  if (digitalRead(BTN_VERT) == LOW) { // Vert = Français
    estAnglais = false; 
    bruitSelection(); changerDePage(PARAMETRES); 
    while(digitalRead(BTN_VERT) == LOW); 
  }
  if (digitalRead(BTN_ORANGE) == LOW) { // Orange = Anglais
    estAnglais = true; 
    bruitSelection(); changerDePage(PARAMETRES); 
    while(digitalRead(BTN_ORANGE) == LOW); 
  }
  if (digitalRead(BTN_BLEU) == LOW) { // Bleu = Annuler / Retour
    bruitSelection(); changerDePage(PARAMETRES);
    while(digitalRead(BTN_BLEU) == LOW); 
  }
}

// --- DEPLACEMENT DES ENNEMIS ---
void deplacerEnnemis() { 
  for (int i = 0; i < nbEnnemisActuels; i++) { // Boucle qui passe en revue chaque ennemi actif
    int dir = random(4); // Choisit une direction aléatoire (0: Droite, 1: Gauche, 2: Bas, 3: Haut)
    int nx = ennemiX[i], ny = ennemiY[i]; // Coordonnées temporaires de la future position
    
    if (dir == 0) nx++; 
    else if (dir == 1) nx--; 
    else if (dir == 2) ny++; 
    else if (dir == 3) ny--; 
    
    // Vérification de sécurité : l'ennemi reste dans l'écran (0 à 15 en X, 0 à 7 en Y) et ne marche pas sur un mur (grille == 0)
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) { 
      ennemiX[i] = nx; // Met à jour la vraie position de l'ennemi
      ennemiY[i] = ny; 
    }
  }
}

// --- ECRAN FIN DE PARTIE ---
void finDePartie(String txtFr, String txtEn, uint16_t color) { 
  tft.fillScreen(TFT_BLACK); // Efface l'écran en noir
  tft.setTextColor(color);   // Applique la couleur du texte (Vert pour victoire, Rouge pour défaite)
  tft.setTextSize(4);        // Gros texte bien visible
  tft.setCursor(50, 100);    // Centre grossièrement le texte
  tft.print(estAnglais ? txtEn : txtFr); // Affiche le bon texte selon la langue choisie
  delay(3000);               // Laisse le texte affiché pendant 3 secondes
  changerDePage(NIVEAUX);    // Renvoie le joueur à la sélection des niveaux
}

// --- VERIFICATION DES COLLISIONS ---
void verifierCollisions() { 
  // 1. Test si Mario est arrivé sur la case objectif (Victoire)
  if (marioX == cibleX && marioY == cibleY) { 
    bruitVictoire();
    finDePartie("GAGNE !", "YOU WIN !", MY_GREEN); 
  }
  // 2. Test si Mario touche un des ennemis (Défaite)
  for (int i = 0; i < nbEnnemisActuels; i++) { 
    if (marioX == ennemiX[i] && marioY == ennemiY[i]) { 
      bruitDefaite();
      finDePartie("GAME OVER", "GAME OVER", MY_RED); 
    }
  }
}

// --- LOGIQUE DE JEU (Mouvements du Joueur) ---
void handleGameLogic() { 
  int nx = marioX, ny = marioY; // Variables temporaires pour calculer la prochaine position de Mario
  bool moved = false;           // Devient vrai si une touche directionnelle est pressée

  if (digitalRead(BTN_ORANGE) == LOW)      { nx--; moved = true; } // Gauche
  else if (digitalRead(BTN_JAUNE) == LOW)  { nx++; moved = true; } // Droite
  else if (digitalRead(BTN_BLEU) == LOW)   { ny--; moved = true; } // Haut
  else if (digitalRead(BTN_VERT) == LOW)   { ny++; moved = true; } // Bas 

  if (moved) { 
    // Vérifie si la case visée est libre (dans les limites et pas un mur)
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) { 
      marioX = nx; marioY = ny; // Mario se déplace officiellement
      if (nbEnnemisActuels > 0) deplacerEnnemis(); // Les ennemis bougent à chaque action de Mario
      drawGrilleJeu();        // Redessine l'écran de jeu mis à jour
      verifierCollisions();   // Vérifie si Mario a gagné ou s'est fait attraper
    }
    delay(150); // Petit délai pour fluidifier le déplacement et éviter de courir trop vite
  }
}

// --- AFFICHAGE DE LA GRILLE DE JEU ---
void drawGrilleJeu() { 
  tft.fillScreen(0x0841); // Couleur de fond bleu nuit
  tft.drawRoundRect(10, 10, 300, 40, 5, MY_BLUE); // Encadré du titre du niveau
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(110, 22);
  
  // Affiche le titre dynamique selon le niveau actif et la langue
  if (pageActuelle == JEU_NIV1) tft.print(estAnglais ? "level 1" : "niveau 1");
  else if (pageActuelle == JEU_NIV2) tft.print(estAnglais ? "level 2" : "niveau 2");
  else if (pageActuelle == JEU_NIV3) tft.print(estAnglais ? "level 3" : "niveau 3");

  tft.drawRoundRect(10, 55, 300, 175, 10, MY_BLUE); // Encadré principal contenant le labyrinthe
  
  int gridX = 30, gridY = 70, cellW = 16, cellH = 17; // Configuration de la taille des cases
  
  // Double boucle "for" pour dessiner toute la grille (lignes et colonnes)
  for (int r = 0; r < 8; r++) {       // Pour chaque ligne
    for (int c = 0; c < 16; c++) {    // Pour chaque colonne
      int px = gridX + (c * cellW), py = gridY + (r * cellH); // Calcul de la position en pixels sur l'écran
      tft.drawRect(px, py, cellW, cellH, GRID_LINE); // Dessine le contour de la case
      if (grille[c][r] == 1) tft.fillRect(px+2, py+2, cellW-4, cellH-4, MY_GREEN); // Dessine un mur si la grille vaut 1
    }
  }
  // Dessin des entités par-dessus la grille
  // 1. Les Ennemis (Carrés Jaunes)
  for (int i = 0; i < nbEnnemisActuels; i++) tft.fillRect(gridX + (ennemiX[i]*cellW)+2, gridY + (ennemiY[i]*cellH)+2, cellW-4, cellH-4, MY_YELLOW); 
  // 2. Mario (Carré de la couleur personnalisée par le joueur)
  tft.fillRect(gridX + (marioX*cellW)+2, gridY + (marioY*cellH)+2, cellW-4, cellH-4, couleurMarioActive); 
  // 3. L'objectif / Arrivée (Carré Cyan / Bleu clair)
  tft.fillRect(gridX + (cibleX*cellW)+2, gridY + (cibleY*cellH)+2, cellW-4, cellH-4, 0x07FF); 
}

// --- DESSIN DES INTERFACES GRAPHIQUES COMMUNES ---
void drawCommonUI() { 
  tft.fillRect(0, 200, 320, 40, MY_GREY); // Bandeau de bas de page
  tft.fillRect(45, 15, 230, 65, MY_RED);  // Rectangle rouge pour le logo de titre
  tft.setTextColor(TFT_WHITE); tft.setTextSize(3);
  tft.setCursor(60, 20); tft.print("Super");
  tft.setCursor(60, 45); tft.print("mario bros");
}

// Fonction utilitaire pour dessiner un bouton ovale avec du texte centré
void drawButton(int y, uint16_t color, String label) { 
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  // Calcul mathématique simple pour centrer le texte horizontalement dans le bouton
  tft.setCursor(160 - (label.length()*6), y + 8); tft.print(label);
}

// Affiche l'écran du choix des couleurs
void drawMenuCouleurs() { 
  tft.fillScreen(MY_SKY); drawCommonUI();
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(65, 90); tft.print(estAnglais ? "choose color" : "choisis ta couleur"); 
  
  int startX = 40; int yPos = 120;
  for(int i = 0; i < 7; i++) { // Dessine les 7 blocs de couleur de la palette
    int xBox = startX + (i * 35);
    if (i == indexCurseurCouleur) tft.drawRect(xBox-3, yPos-3, 26, 26, MY_YELLOW); // Encadré jaune autour de la couleur sélectionnée
    tft.fillRect(xBox, yPos, 20, 20, palette[i]);
  }
  drawButton(160, MY_GREEN, estAnglais ? "select" : "selectionner");
  drawButton(195, MY_BLUE, estAnglais ? "cancel" : "annuler");
}

// --- MACHINE A ETAT : GESTION DU CHANGEMENT DE PAGES ---
void changerDePage(Page p) { 
  pageActuelle = p; // Met à jour la variable globale d'état de la page
  
  // Selon la page demandée, on efface l'écran et on dessine les éléments correspondants
  if (p == PRINCIPAL) { 
    tft.fillScreen(MY_SKY); drawCommonUI(); 
    drawButton(110, MY_GREEN, estAnglais ? "level" : "niveau"); 
    drawButton(155, MY_BLUE, estAnglais ? "settings" : "parametre"); 
  }
  else if (p == NIVEAUX) { 
    tft.fillScreen(MY_SKY); drawCommonUI(); 
    drawButton(85, MY_GREEN, estAnglais ? "level 1" : "niveau 1"); 
    drawButton(120, MY_YELLOW, estAnglais ? "level 2" : "niveau 2"); 
    drawButton(155, MY_ORANGE, estAnglais ? "level 3" : "niveau 3"); 
    drawButton(190, MY_BLUE, estAnglais ? "cancel" : "annuler"); 
  }
  else if (p == PARAMETRES) { 
    tft.fillScreen(MY_SKY); drawCommonUI(); 
    drawButton(100, MY_GREEN, estAnglais ? "color" : "couleur"); 
    drawButton(135, MY_ORANGE, estAnglais ? "language" : "langage"); 
    drawButton(170, MY_BLUE, estAnglais ? "cancel" : "annuler"); 
  }
  else if (p == LANGUE) { 
    tft.fillScreen(MY_SKY); drawCommonUI();
    drawButton(100, MY_GREEN, "francais");
    drawButton(135, MY_ORANGE, "english");
    drawButton(170, MY_BLUE, estAnglais ? "cancel" : "annuler");
  }
  else if (p == COULEUR) drawMenuCouleurs();
  else drawGrilleJeu(); // Si c'est un état de jeu (JEU_NIV1, 2, 3), on affiche le labyrinthe
}

// --- INITIALISATION D'UNE PARTIE ---
void initialiserJeu(Page niv) { 
  marioX = 0; marioY = 0; // Réinitialise Mario en haut à gauche
  randomSeed(millis());   // Initialise le générateur de nombres aléatoires basé sur le temps écoulé
  
  // Définit le nombre d'ennemis selon la difficulté choisie
  if (niv == JEU_NIV1) nbEnnemisActuels = 0;
  else if (niv == JEU_NIV2) nbEnnemisActuels = 5;
  else if (niv == JEU_NIV3) nbEnnemisActuels = 11;
  
  // pas d'obstacle près du mario
  for (int x = 0; x < 16; x++) { 
    for (int y = 0; y < 8; y++) { 
      if (x <= 1 && y <= 1) { 
        grille[x][y] = 0; // Laisse la zone de départ de Mario vide (sécurité)
      } else { 
        grille[x][y] = (random(100) < 15) ? 1 : 0; // 15% de chances de faire apparaître un bloc d'obstacle
      }
    }
  }
  
  grille[cibleX][cibleY] = 0; // S'assure que la case d'arrivée n'est pas bloquée par un mur
  
  // Placement aléatoire des ennemis sur la partie droite du labyrinthe (X entre 5 et 15)
  for (int i = 0; i < nbEnnemisActuels; i++) { 
    ennemiX[i] = random(5, 15); 
    ennemiY[i] = random(0, 7); 
    grille[ennemiX[i]][ennemiY[i]] = 0; // Enlève un mur s'il y en avait un sous l'ennemi
  }
  
  changerDePage(niv); // Lance l'affichage du jeu
}

// --- NAVIGATION DES MENUS VIA LES BOUTONS ---
void handleMenuNavigation() { 
  if (digitalRead(BTN_VERT) == LOW) { // Actions du Bouton Vert dans les menus
    bruitSelection();
    if (pageActuelle == PRINCIPAL) changerDePage(NIVEAUX); 
    else if (pageActuelle == PARAMETRES) changerDePage(COULEUR); 
    else if (pageActuelle == NIVEAUX) initialiserJeu(JEU_NIV1); 
    while(digitalRead(BTN_VERT) == LOW); delay(50);
  }
  if (digitalRead(BTN_ORANGE) == LOW) { // Actions du Bouton Orange dans les menus
    if (pageActuelle == PARAMETRES) { 
      bruitSelection(); changerDePage(LANGUE);
      while(digitalRead(BTN_ORANGE) == LOW); delay(50);
    }
    else if (pageActuelle == NIVEAUX) { 
      bruitSelection(); initialiserJeu(JEU_NIV3); 
      while(digitalRead(BTN_ORANGE) == LOW); delay(50);
    }
  }
  if (digitalRead(BTN_BLEU) == LOW) { // Actions du Bouton Bleu (Bouton d'annulation universel)
    bruitSelection();
    if (pageActuelle == PRINCIPAL) changerDePage(PARAMETRES); 
    else changerDePage(PRINCIPAL); // Retourne à l'accueil depuis n'importe quel sous-menu
    while(digitalRead(BTN_BLEU) == LOW); delay(50);
  }
  if (pageActuelle == NIVEAUX) { // Action exclusive au menu Niveaux pour le Bouton Jaune
    if (digitalRead(BTN_JAUNE) == LOW) { 
      bruitSelection(); initialiserJeu(JEU_NIV2); 
      while(digitalRead(BTN_JAUNE) == LOW); delay(50);
    }
  }
}

void drawMenuPrincipal() { changerDePage(PRINCIPAL); }

// --- CONFIGURATION INITIALE DE L'ESP32 ---
void setup() {
  // Configuration de toutes les pins des boutons en entrées avec résistance de Pull-up interne
  // (Cela signifie que la pin lit "HIGH" par défaut, et "LOW" quand on appuie sur le bouton)
  pinMode(BTN_ORANGE, INPUT_PULLUP); 
  pinMode(BTN_VERT, INPUT_PULLUP);
  pinMode(BTN_BLEU, INPUT_PULLUP); 
  pinMode(BTN_JAUNE, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP); 
  
  pinMode(BUZZER_PIN, OUTPUT); // Le buzzer est configuré en Sortie pour pouvoir émettre des fréquences
  
  tft.init();         // Initialisation matérielle de l'écran TFT
  tft.setRotation(1); // Aligne l'écran en mode Paysage (horizontal)
  drawMenuPrincipal(); // Affiche le premier écran du jeu
}

// --- BOUCLE PRINCIPALE (S'exécute en continu à l'infini) ---
void loop() {
  // Lecture constante de la position de l'interrupteur
  bool etatInterrupteur = (digitalRead(SWITCH_PIN) == LOW); 

  // Système d'aiguillage automatique (Machine à état) : 
  // Exécute la bonne logique de code selon l'écran où se trouve le joueur
  if (pageActuelle == COULEUR) handleMenuCouleurLogic(); 
  else if (pageActuelle == LANGUE) handleMenuLangueLogic(); 
  else if (pageActuelle >= JEU_NIV1) handleGameLogic(); // Si pageActuelle vaut JEU_NIV1, NIV2 ou NIV3
  else handleMenuNavigation(); // Pour les menus de navigation basiques (Principal, Niveaux, Paramètres)
}
