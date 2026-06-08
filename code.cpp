#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- Configuration des Pins ---
#define BTN_ORANGE  25 
#define BTN_VERT    26 
#define BTN_BLEU    27 
#define BTN_JAUNE   32 
#define BUZZER_PIN  14 // Pin du buzzer

// --- Couleurs ---
#define MY_RED       0xF800
#define MY_BLUE      0x001F
#define MY_GREEN     0x07E0 
#define MY_YELLOW    0xFFE0
#define MY_ORANGE    0xFD20  
#define MY_SKY       0x5DFF
#define MY_GREY      0x4208
#define GRID_LINE    0x2124 

// --- Variables de Jeu ---
enum Page { PRINCIPAL, NIVEAUX, PARAMETRES, COULEUR, LANGUE, JEU_NIV1, JEU_NIV2, JEU_NIV3 }; // Crée une liste de mots-clés numérotés pour identifier facilement chaque écran du jeu.
Page pageActuelle = PRINCIPAL; // Crée la variable qui mémorise l'écran actuellement affiché et démarre le jeu sur l'accueil.

uint16_t couleurMarioActive = MY_RED; 
int indexCurseurCouleur = 0;
uint16_t palette[] = {0xF844, 0xFFE0, 0xF81F, 0x001F, 0x0400, 0x780F, 0xD6BA}; // Palette de couleurs disponibles pour personnaliser Mario.

// Gestion de la langue (false = Français, true = Anglais)
bool estAnglais = false; // Crée une variable interrupteur : FAUX pour afficher le jeu en français, VRAI pour l'anglais.

int marioX, marioY;
const int cibleX = 15, cibleY = 7;
byte grille[16][8];
int nbEnnemisActuels = 0;
int ennemiX[11], ennemiY[11];

// --- SONS ---
void bruitSelection() {
  tone(BUZZER_PIN, 1000, 50); // Bip court
}

void bruitVictoire() {
  tone(BUZZER_PIN, 1500, 150); delay(150);
  tone(BUZZER_PIN, 2000, 150); delay(150);
  tone(BUZZER_PIN, 2500, 300);
}

void bruitDefaite() {
  tone(BUZZER_PIN, 400, 200); delay(200);
  tone(BUZZER_PIN, 300, 200); delay(200);
  tone(BUZZER_PIN, 200, 400);
}

// --- LOGIQUE MENU COULEUR ---
void handleMenuCouleurLogic() { // Déclare la fonction qui gère les boutons dans le sélecteur de couleurs.
  bool update = false; // Variable temporaire pour savoir s'il faut rafraîchir l'affichage du menu.
  if (digitalRead(BTN_ORANGE) == HIGH) { // Si on appuie sur le bouton Orange, déplace le curseur vers la gauche.
    if (indexCurseurCouleur > 0) { indexCurseurCouleur--; update = true; bruitSelection(); } 
    delay(200); 
  }
  if (digitalRead(BTN_JAUNE) == HIGH) { // Si on appuie sur le bouton Jaune, déplace le curseur vers la droite.
    if (indexCurseurCouleur < 6) { indexCurseurCouleur++; update = true; bruitSelection(); } 
    delay(200); 
  }
  if (digitalRead(BTN_VERT) == HIGH) { // Si on appuie sur Vert, valide la couleur choisie et retourne aux paramètres.
    couleurMarioActive = palette[indexCurseurCouleur]; 
    bruitSelection();
    changerDePage(PARAMETRES); 
    while(digitalRead(BTN_VERT) == HIGH); // Bloque l'exécution tant que le bouton reste enfoncé pour éviter les clics multiples.
  }
  if (digitalRead(BTN_BLEU) == HIGH) { // Si on appuie sur Bleu, annule les modifications et retourne aux paramètres.
    bruitSelection();
    changerDePage(PARAMETRES); 
    while(digitalRead(BTN_BLEU) == HIGH); 
  }
  if (update) drawMenuCouleurs(); // Si le curseur a bougé, redessine le menu pour mettre à jour la position du carré de sélection.
}

// --- LOGIQUE MENU LANGUE ---
void handleMenuLangueLogic() { // Déclare la fonction qui gère le choix de la langue du jeu.
  if (digitalRead(BTN_VERT) == HIGH) { // Si bouton Vert enfoncé, choisit le Français.
    estAnglais = false; // Désactive le mode anglais.
    bruitSelection();
    changerDePage(PARAMETRES); // Retourne automatiquement à l'écran précédent.
    while(digitalRead(BTN_VERT) == HIGH); // Attend le relâchement du bouton.
  }
  if (digitalRead(BTN_ORANGE) == HIGH) { // Si bouton Orange enfoncé, choisit l'Anglais.
    estAnglais = true; // Active le mode anglais.
    bruitSelection();
    changerDePage(PARAMETRES); // Retourne automatiquement à l'écran précédent.
    while(digitalRead(BTN_ORANGE) == HIGH); // Attend le relâchement du bouton.
  }
  if (digitalRead(BTN_BLEU) == HIGH) { // Si bouton Bleu enfoncé, annule et quitte le menu.
    bruitSelection();
    changerDePage(PARAMETRES);
    while(digitalRead(BTN_BLEU) == HIGH); // Attend le relâchement du bouton.
  }
}

// --- DEPLACEMENT DES ENNEMIS ---
void deplacerEnnemis() { // Déclare la fonction qui va gérer les mouvements automatiques des adversaires.
  for (int i = 0; i < nbEnnemisActuels; i++) { // Lance une boucle pour faire bouger chaque ennemi.
    int dir = random(4); // Choisit un nombre au hasard entre 0 et 3 pour décider d'une direction au hasard (haut, bas, gauche, droite).
    int nx = ennemiX[i], ny = ennemiY[i]; // Copie la position actuelle de l'ennemi dans des variables temporaires pour calculer son futur déplacement.
    if (dir == 0) nx++; // Modifie la coordonnée horizontale vers la droite si le hasard a choisi la direction zéro.
    else if (dir == 1) nx--; // Modifie la coordonnée horizontale vers la gauche si le hasard a choisi la direction un.
    else if (dir == 2) ny++; // Modifie la coordonnée verticale vers le bas si le hasard a choisi la direction deux.
    else if (dir == 3) ny--; // Modifie la coordonnée verticale vers le haut si le hasard a choisi la direction trois.
    
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) { // Vérifie si la case visée par l'ennemi reste dans l'écran et ne contient aucun mur.
      ennemiX[i] = nx; // Valide le déplacement et met à jour la position horizontale définitive de l'ennemi.
      ennemiY[i] = ny; // Valide le déplacement et met à jour la position verticale définitive de l'ennemi.
    }
  }
}

void finDePartie(String txtFr, String txtEn, uint16_t color) { // Fin de la partie et message de fin bilingue.
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(color); // Configure la couleur du texte (vert pour gagné, rouge pour perdu).
  tft.setTextSize(4);
  tft.setCursor(50, 100); // Place le curseur d'écriture aux coordonnées x=50 et y=100 pour centrer le message à l'écran.
  tft.print(estAnglais ? txtEn : txtFr); // Affiche le texte anglais ou français selon la langue active du jeu.
  delay(3000);
  changerDePage(NIVEAUX); // Renvoie automatiquement le joueur vers l'écran de sélection des niveaux pour recommencer.
}

// --- VERIFICATION DES COLLISIONS ---
void verifierCollisions() { // Déclare la fonction qui scrute si Mario touche l'arrivée ou un ennemi.
  if (marioX == cibleX && marioY == cibleY) { // Vérifie si les coordonnées de Mario sont identiques à celles de la case d'arrivée.
    bruitVictoire();
    finDePartie("GAGNE !", "YOU WIN !", MY_GREEN); // Lance l'écran de victoire avec les textes adaptés.
  }
  for (int i = 0; i < nbEnnemisActuels; i++) { // Parcourt la liste des monstres présents pour analyser leur position.
    if (marioX == ennemiX[i] && marioY == ennemiY[i]) { // Si Mario se trouve sur la même case qu'un ennemi, il y a contact.
      bruitDefaite();
      finDePartie("GAME OVER", "GAME OVER", MY_RED); // Lance l'écran de défaite.
    }
  }
}

// --- LOGIQUE DE JEU ---
void handleGameLogic() { // Déclare la fonction qui gère les actions du joueur pendant une partie.
  int nx = marioX, ny = marioY;
  bool moved = false; // Crée une variable de contrôle pour savoir si le joueur a appuyé sur un bouton de direction.

  if (digitalRead(BTN_ORANGE) == HIGH) { nx--; moved = true; } // Déplacement vers la gauche.
  else if (digitalRead(BTN_JAUNE) == HIGH) { nx++; moved = true; } // Déplacement vers la droite.
  else if (digitalRead(BTN_BLEU) == HIGH) { ny--; moved = true; } // Déplacement vers le haut.
  else if (digitalRead(BTN_VERT) == HIGH) { ny++; moved = true; } // Déplacement vers le bas.

  if (moved) { // Si un bouton de direction a été pressé, traite le mouvement.
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) { // Vérifie que la nouvelle case visée ne sort pas de l'écran et qu'elle ne contient pas un mur.
      marioX = nx; marioY = ny; // Valide définitivement le déplacement en appliquant les nouvelles coordonnées à Mario.
      if (nbEnnemisActuels > 0) deplacerEnnemis(); // Fait bouger les ennemis d'une case juste après le mouvement de Mario si le niveau en contient.
      drawGrilleJeu(); // Rafraîchit l'affichage du labyrinthe pour voir le déplacement à l'écran.
      verifierCollisions(); // Lance immédiatement le test pour voir si on a gagné ou perdu.
    }
    delay(150);
  }
}

// --- AFFICHAGE ---
void drawGrilleJeu() { // Trace l'interface complète du niveau en cours de jeu.
  tft.fillScreen(0x0841);
  tft.drawRoundRect(10, 10, 300, 40, 5, MY_BLUE);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(110, 22);
  
  // Affiche le titre du niveau traduit selon la langue choisie
  if (pageActuelle == JEU_NIV1) tft.print(estAnglais ? "level 1" : "niveau 1");
  else if (pageActuelle == JEU_NIV2) tft.print(estAnglais ? "level 2" : "niveau 2");
  else if (pageActuelle == JEU_NIV3) tft.print(estAnglais ? "level 3" : "niveau 3");

  tft.drawRoundRect(10, 55, 300, 175, 10, MY_BLUE);
  int gridX = 30, gridY = 70, cellW = 16, cellH = 17;
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 16; c++) {
      int px = gridX + (c * cellW), py = gridY + (r * cellH);
      tft.drawRect(px, py, cellW, cellH, GRID_LINE);
      if (grille[c][r] == 1) tft.fillRect(px+2, py+2, cellW-4, cellH-4, MY_GREEN); // Dessine les blocs de murs en vert.
    }
  }
  for (int i = 0; i < nbEnnemisActuels; i++) tft.fillRect(gridX + (ennemiX[i]*cellW)+2, gridY + (ennemiY[i]*cellH)+2, cellW-4, cellH-4, MY_YELLOW); // Dessine les monstres en jaune.
  tft.fillRect(gridX + (marioX*cellW)+2, gridY + (marioY*cellH)+2, cellW-4, cellH-4, couleurMarioActive); // Dessine Mario avec sa couleur personnalisée.
  tft.fillRect(gridX + (cibleX*cellW)+2, gridY + (cibleY*cellH)+2, cellW-4, cellH-4, 0x07FF); // Dessine la case d'arrivée en bleu clair.
}

void drawCommonUI() { // Génère le bandeau titre "Super Mario Bros" commun à l'ensemble des menus.
  tft.fillRect(0, 200, 320, 40, MY_GREY);
  tft.fillRect(45, 15, 230, 65, MY_RED);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(3);
  tft.setCursor(60, 20); tft.print("Super");
  tft.setCursor(60, 45); tft.print("mario bros");
}

void drawButton(int y, uint16_t color, String label) { // Outil automatique servant à concevoir et centrer des boutons textuels.
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(160 - (label.length()*6), y + 8); tft.print(label);
}

void drawMenuCouleurs() { // Construit l'écran du choix de look pour Mario.
  tft.fillScreen(MY_SKY); drawCommonUI();
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(65, 90); tft.print(estAnglais ? "choose color" : "choisis ta couleur"); // Texte adapté à la langue.
  int startX = 40; int yPos = 120;
  for(int i = 0; i < 7; i++) {
    int xBox = startX + (i * 35);
    if (i == indexCurseurCouleur) tft.drawRect(xBox-3, yPos-3, 26, 26, MY_YELLOW); // Dessine l'encadré jaune de sélection.
    tft.fillRect(xBox, yPos, 20, 20, palette[i]);
  }
  drawButton(160, MY_GREEN, estAnglais ? "select" : "selectionner");
  drawButton(195, MY_BLUE, estAnglais ? "cancel" : "annuler");
}

void changerDePage(Page p) { // Gère la distribution et l'affichage complet des différents écrans du système.
  pageActuelle = p;
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
  else if (p == LANGUE) { // Construit le tout nouveau menu de sélection de la langue.
    tft.fillScreen(MY_SKY); drawCommonUI();
    drawButton(100, MY_GREEN, "francais");
    drawButton(135, MY_ORANGE, "english");
    drawButton(170, MY_BLUE, estAnglais ? "cancel" : "annuler");
  }
  else if (p == COULEUR) drawMenuCouleurs();
  else drawGrilleJeu();
}

// --- INITIALISATION DU JEU ---
void initialiserJeu(Page niv) { // Prépare les éléments de la carte et les variables avant le début d'une tentative.
  marioX = 0; marioY = 0; randomSeed(millis()); // Place Mario au départ (0,0) et initialise le générateur de hasard.
  if (niv == JEU_NIV1) nbEnnemisActuels = 0;
  else if (niv == JEU_NIV2) nbEnnemisActuels = 5;
  else if (niv == JEU_NIV3) nbEnnemisActuels = 11;
  
  for (int x = 0; x < 16; x++) { // Parcourt la grille du jeu colonne par colonne, de gauche à droite.
    for (int y = 0; y < 8; y++) { // Parcourt chaque ligne de la grille, de haut en bas, pour analyser chaque case.
      if (x <= 1 && y <= 1) { // Vérifie si la case actuelle se trouve dans le carré de départ 2x2 en haut à gauche.
        grille[x][y] = 0; // Force la case à être vide pour que le joueur ne commence pas la partie bloqué dans un mur.
      } else { // Indique au programme quoi faire pour toutes les autres cases situées en dehors de la zone de départ.
        grille[x][y] = (random(100) < 15) ? 1 : 0; // Donne à la case 15% de risques de devenir un mur et 85% de chances de rester libre.
      }
    }
  }
  
  grille[cibleX][cibleY] = 0; // Sécurité pour libérer l'accès sur la zone d'arrivée.
  
  for (int i = 0; i < nbEnnemisActuels; i++) { // Lance une boucle pour positionner chaque ennemi l'un après l'autre.
    ennemiX[i] = random(5, 15); // Choisit une colonne au hasard entre la 5ème et la 15ème pour placer l'ennemi sur la partie droite de l'écran.
    ennemiY[i] = random(0, 7); // Choisit une ligne au hasard entre 0 et 7 pour la hauteur de l'ennemi.
    grille[ennemiX[i]][ennemiY[i]] = 0; // Force la case choisie à être vide (valeur 0) afin de supprimer un éventuel mur pour que l'ennemi ne commence pas coincé.
  }
  
  changerDePage(niv); // Bascule instantanément sur l'écran de jeu pour lancer la partie.
}

// --- NAVIGATION GENERALE ---
void handleMenuNavigation() { // Centralise la gestion des touches pour se déplacer à travers l'arborescence des menus.
  if (digitalRead(BTN_VERT) == HIGH) {
    bruitSelection();
    if (pageActuelle == PRINCIPAL) changerDePage(NIVEAUX); // Si on est à l'accueil, Vert ouvre le menu des Niveaux.
    else if (pageActuelle == PARAMETRES) changerDePage(COULEUR); // Si on est dans les Paramètres, Vert ouvre le menu Couleur.
    else if (pageActuelle == NIVEAUX) initialiserJeu(JEU_NIV1); // Si on est dans les Niveaux, Vert lance le Niveau 1.
    while(digitalRead(BTN_VERT) == HIGH); delay(100);
  }
  if (digitalRead(BTN_ORANGE) == HIGH) {
    if (pageActuelle == PARAMETRES) { // Dans les Paramètres, le bouton Orange ouvre la gestion de la Langue.
      bruitSelection();
      changerDePage(LANGUE);
      while(digitalRead(BTN_ORANGE) == HIGH); delay(100);
    }
    else if (pageActuelle == NIVEAUX) { // Dans le menu de sélection de Niveaux, Orange démarre directement le Niveau 3.
      bruitSelection(); 
      initialiserJeu(JEU_NIV3); 
    }
  }
  if (digitalRead(BTN_BLEU) == HIGH) {
    bruitSelection();
    if (pageActuelle == PRINCIPAL) changerDePage(PARAMETRES); // Depuis l'accueil, Bleu donne accès aux Paramètres.
    else changerDePage(PRINCIPAL); // Depuis n'importe quel autre menu secondaire, Bleu fait office de bouton "Retour".
    while(digitalRead(BTN_BLEU) == HIGH); delay(100);
  }
  if (pageActuelle == NIVEAUX) {
    if (digitalRead(BTN_JAUNE) == HIGH) { bruitSelection(); initialiserJeu(JEU_NIV2); } // Dans le menu des Niveaux, Jaune lance le Niveau 2.
  }
}

void drawMenuPrincipal() { changerDePage(PRINCIPAL); }

// --- SETUP & LOOP ---
void setup() {
pinMode(BTN_ORANGE, INPUT_PULLUP); pinMode(BTN_VERT, INPUT_PULLUP);
  pinMode(BTN_BLEU, INPUT_PULLUP); pinMode(BTN_JAUNE, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  tft.init(); 
  tft.setRotation(1);
  drawMenuPrincipal(); // Génère le tout premier affichage du jeu lors de la mise sous tension.
}

void loop() { // Démarre la fonction principale d'Arduino qui s'exécute en continu et en boucle tant que le jeu est allumé.
  if (pageActuelle == COULEUR) handleMenuCouleurLogic(); // Active les commandes de sélection des couleurs si le joueur se trouve sur cet écran spécifique.
  else if (pageActuelle == LANGUE) handleMenuLangueLogic(); // Oriente les contrôles vers la logique de choix de langue si l'écran de Langue est actif.
  else if (pageActuelle >= JEU_NIV1) handleGameLogic(); // Active les règles de la partie (mouvements, collisions) si le joueur est en train de jouer à un niveau.
  else handleMenuNavigation(); // Gère les boutons pour naviguer dans les menus classiques si aucune partie n'est lancée.
}