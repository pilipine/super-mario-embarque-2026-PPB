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
enum Page { PRINCIPAL, NIVEAUX, PARAMETRES, COULEUR, JEU_NIV1, JEU_NIV2, JEU_NIV3 };  //Crée une liste de mots-clés numérotés pour identifier facilement chaque écran du jeu.
Page pageActuelle = PRINCIPAL; //Crée la variable qui mémorise l'écran actuellement affiché et démarre le jeu sur l'accueil.

uint16_t couleurMarioActive = MY_RED; 
int indexCurseurCouleur = 0;
uint16_t palette[] = {0xF844, 0xFFE0, 0xF81F, 0x001F, 0x0400, 0x780F, 0xD6BA}; // palette de couleur de mes marios

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
void handleMenuCouleurLogic() {
  bool update = false;
  if (digitalRead(BTN_ORANGE) == HIGH) { 
    if (indexCurseurCouleur > 0) { indexCurseurCouleur--; update = true; bruitSelection(); } 
    delay(200); 
  }
  if (digitalRead(BTN_JAUNE) == HIGH) { 
    if (indexCurseurCouleur < 6) { indexCurseurCouleur++; update = true; bruitSelection(); } 
    delay(200); 
  }
  if (digitalRead(BTN_VERT) == HIGH) { 
    couleurMarioActive = palette[indexCurseurCouleur]; 
    bruitSelection();
    changerDePage(PARAMETRES); 
    while(digitalRead(BTN_VERT) == HIGH); 
  }
  if (digitalRead(BTN_BLEU) == HIGH) { 
    bruitSelection();
    changerDePage(PARAMETRES); 
    while(digitalRead(BTN_BLEU) == HIGH); 
  }
  if (update) drawMenuCouleurs();
}

// --- DEPLACEMENT DES ENNEMIS ---
void deplacerEnnemis() { //Déclare la fonction qui va gérer les mouvements automatiques des adversaires.
  for (int i = 0; i < nbEnnemisActuels; i++) { //Lance une boucle pour faire bouger chaque ennemi .
    int dir = random(4); // : Choisit un nombre au hasard entre 0 et 3 pour décider d'une direction au hasard (haut, bas, gauche, droite).
    int nx = ennemiX[i], ny = ennemiY[i]; //  Copie la position actuelle de l'ennemi dans des variables temporaires pour calculer son futur déplacement.
    if (dir == 0) nx++; //  Modifie la coordonnée horizontale vers la droite si le hasard a choisi la direction zéro.
    else if (dir == 1) nx--; // Modifie la coordonnée horizontale vers la gauche si le hasard a choisi la direction un.
    else if (dir == 2) ny++; // Modifie la coordonnée verticale vers le bas si le hasard a choisi la direction deux.
    else if (dir == 3) ny--; // Modifie la coordonnée verticale vers le haut si le hasard a choisi la direction trois.

    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) { // Vérifie si la case visée par l'ennemi reste dans l'écran et ne contient aucun mur.
      ennemiX[i] = nx; // Valide le déplacement et met à jour la position horizontale définitive de l'ennemi.
      ennemiY[i] = ny; // Valide le déplacement et met à jour la position verticale définitive de l'ennemi.
    }
  }
}

void finDePartie(String txt, uint16_t color) { // fin de la partie et message de fin 
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(color); // Configure la couleur du texte (vert pour gagné, rouge pour perdu).
  tft.setTextSize(4);
  tft.setCursor(50, 100); // Place le curseur d'écriture aux coordonnées x=50 et y=100 pour centrer le message à l'écran.
  tft.print(txt); // Écrit le texte de fin ("GAGNE !" ou "GAME OVER") sur l'écran.
  delay(3000);
  changerDePage(NIVEAUX); // Renvoie automatiquement le joueur vers l'écran de sélection des niveaux pour recommencer.
}

// --- VERIFICATION DES COLLISIONS ---
void verifierCollisions() {
  if (marioX == cibleX && marioY == cibleY) {
    bruitVictoire();
    finDePartie("GAGNE !", MY_GREEN);
  }
  for (int i = 0; i < nbEnnemisActuels; i++) {
    if (marioX == ennemiX[i] && marioY == ennemiY[i]) {
      bruitDefaite();
      finDePartie("GAME OVER", MY_RED);
    }
  }
}

// --- LOGIQUE DE JEU ---
void handleGameLogic() { // Déclare la fonction qui gère les actions du joueur pendant une partie.
  int nx = marioX, ny = marioY;
  bool moved = false; //Crée une variable de contrôle pour savoir si le joueur a appuyé sur un bouton de direction.

  if (digitalRead(BTN_ORANGE) == HIGH) { nx--; moved = true; } //  gauche
  else if (digitalRead(BTN_JAUNE) == HIGH) { nx++; moved = true; } // droite
  else if (digitalRead(BTN_BLEU) == HIGH) { ny--; moved = true; } // haut
  else if (digitalRead(BTN_VERT) == HIGH) { ny++; moved = true; } // bas

  if (moved) { // si bouton appuyer alors bouger
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) { //Vérifie que la nouvelle case visée ne sort pas de l'écran et qu'elle ne contient pas un mur.
      marioX = nx; marioY = ny; // Valide définitivement le déplacement en appliquant les nouvelles coordonnées à Mario.
      if (nbEnnemisActuels > 0) deplacerEnnemis(); // : Fait bouger les ennemis d'une case juste après le mouvement de Mario si le niveau en contient
      drawGrilleJeu();
      verifierCollisions();
    }
    delay(150);
  }
}

// --- AFFICHAGE ---
void drawGrilleJeu() { // (L'écran de partie)
  tft.fillScreen(0x0841);
  tft.drawRoundRect(10, 10, 300, 40, 5, MY_BLUE);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(110, 22);
  if (pageActuelle == JEU_NIV1) tft.print("niveau 1");
  else if (pageActuelle == JEU_NIV2) tft.print("niveau 2");
  else if (pageActuelle == JEU_NIV3) tft.print("niveau 3");

  tft.drawRoundRect(10, 55, 300, 175, 10, MY_BLUE);
  int gridX = 30, gridY = 70, cellW = 16, cellH = 17;
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 16; c++) {
      int px = gridX + (c * cellW), py = gridY + (r * cellH);
      tft.drawRect(px, py, cellW, cellH, GRID_LINE);
      if (grille[c][r] == 1) tft.fillRect(px+2, py+2, cellW-4, cellH-4, MY_GREEN);
    }
  }
  for (int i = 0; i < nbEnnemisActuels; i++) tft.fillRect(gridX + (ennemiX[i]*cellW)+2, gridY + (ennemiY[i]*cellH)+2, cellW-4, cellH-4, MY_YELLOW);
  tft.fillRect(gridX + (marioX*cellW)+2, gridY + (marioY*cellH)+2, cellW-4, cellH-4, couleurMarioActive);
  tft.fillRect(gridX + (cibleX*cellW)+2, gridY + (cibleY*cellH)+2, cellW-4, cellH-4, 0x07FF);
}

void drawCommonUI() { // (L'habillage des menus)
  tft.fillRect(0, 200, 320, 40, MY_GREY);
  tft.fillRect(45, 15, 230, 65, MY_RED);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(3);
  tft.setCursor(60, 20); tft.print("Super");
  tft.setCursor(60, 45); tft.print("mario bros");
}

void drawButton(int y, uint16_t color, String label) { // (Le créateur de boutons)
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(160 - (label.length()*6), y + 8); tft.print(label);
}

void drawMenuCouleurs() { // (Selecteur de couleur Mario)
  tft.fillScreen(MY_SKY); drawCommonUI();
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(65, 90); tft.print("choisis ta couleur");
  int startX = 40; int yPos = 120;
  for(int i = 0; i < 7; i++) {
    int xBox = startX + (i * 35);
    if (i == indexCurseurCouleur) tft.drawRect(xBox-3, yPos-3, 26, 26, MY_YELLOW);
    tft.fillRect(xBox, yPos, 20, 20, palette[i]);
  }
  drawButton(160, MY_GREEN, "selectionner");
  drawButton(195, MY_BLUE, "annuler");
}

void changerDePage(Page p) {
  pageActuelle = p;
  if (p == PRINCIPAL) { tft.fillScreen(MY_SKY); drawCommonUI(); drawButton(110, MY_GREEN, "niveau"); drawButton(155, MY_BLUE, "parametre"); }
  else if (p == NIVEAUX) { tft.fillScreen(MY_SKY); drawCommonUI(); drawButton(85, MY_GREEN, "niveau 1"); drawButton(120, MY_YELLOW, "niveau 2"); drawButton(155, MY_ORANGE, "niveau 3"); drawButton(190, MY_BLUE, "annuler"); }
  else if (p == PARAMETRES) { tft.fillScreen(MY_SKY); drawCommonUI(); drawButton(110, MY_GREEN, "couleur mario"); drawButton(155, MY_BLUE, "annuler"); }
  else if (p == COULEUR) drawMenuCouleurs();
  else drawGrilleJeu();
}

// --- INITIALISATION DU JEU (AVEC ZONE DE SECURITE) ---
void initialiserJeu(Page niv) {
  marioX = 0; marioY = 0; randomSeed(millis());
  if (niv == JEU_NIV1) nbEnnemisActuels = 0;
  else if (niv == JEU_NIV2) nbEnnemisActuels = 5;
  else if (niv == JEU_NIV3) nbEnnemisActuels = 11;
  
  // Génération de la grille
  for (int x = 0; x < 16; x++) { //Parcourt la grille du jeu colonne par colonne, de gauche à droite.
    for (int y = 0; y < 8; y++) { // Parcourt chaque ligne de la grille, de haut en bas, pour analyser chaque case.
      // Zone de sécurité 2x2 en haut à gauche pour éviter de bloquer Mario au départ
      if (x <= 1 && y <= 1) { //Vérifie si la case actuelle se trouve dans le carré de départ en haut à gauche.
        grille[x][y] = 0; //Force la case à être vide pour que le joueur ne commence pas la partie bloqué dans un mur.
      } else { // Indique au programme quoi faire pour toutes les autres cases situées en dehors de la zone de départ.
        grille[x][y] = (random(100) < 15) ? 1 : 0; // Donne à la case 15% de risques de devenir un mur et 85% de chances de rester libre.
      }
    }
  }
  
  grille[cibleX][cibleY] = 0; // Sécurité pour la zone d'arrivée
  
  // Placement des ennemis
  for (int i = 0; i < nbEnnemisActuels; i++) { // Lance une boucle pour positionner chaque ennemi l'un après l'autre,
    ennemiX[i] = random(5, 15); // Choisit une colonne au hasard entre la 5ème et la 15ème pour placer l'ennemi sur la partie droite de l'écran
    ennemiY[i] = random(0, 7); // Choisit une ligne au hasard entre la 0 (tout en haut) et la 7 (tout en bas) pour la hauteur de l'ennemi.
    grille[ennemiX[i]][ennemiY[i]] = 0; //Force la case choisie à être vide (valeur 0) afin de supprimer un éventuel mur pour que l'ennemi ne commence pas la partie coincé.
  }
  
  changerDePage(niv);
}

// --- NAVIGATION GENERALE ---
void handleMenuNavigation() {
  if (digitalRead(BTN_VERT) == HIGH) {
    bruitSelection();
    if (pageActuelle == PRINCIPAL) changerDePage(NIVEAUX);
    else if (pageActuelle == PARAMETRES) changerDePage(COULEUR);
    else if (pageActuelle == NIVEAUX) initialiserJeu(JEU_NIV1);
    while(digitalRead(BTN_VERT) == HIGH); delay(100);
  }
  if (digitalRead(BTN_BLEU) == HIGH) {
    bruitSelection();
    if (pageActuelle == PRINCIPAL) changerDePage(PARAMETRES);
    else changerDePage(PRINCIPAL);
    while(digitalRead(BTN_BLEU) == HIGH); delay(100);
  }
  if (pageActuelle == NIVEAUX) {
    if (digitalRead(BTN_JAUNE) == HIGH) { bruitSelection(); initialiserJeu(JEU_NIV2); }
    if (digitalRead(BTN_ORANGE) == HIGH) { bruitSelection(); initialiserJeu(JEU_NIV3); }
  }
}

void drawMenuPrincipal() { changerDePage(PRINCIPAL); }

// --- SETUP & LOOP ---
void setup() {
  pinMode(BTN_ORANGE, INPUT); pinMode(BTN_VERT, INPUT);
  pinMode(BTN_BLEU, INPUT); pinMode(BTN_JAUNE, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  tft.init(); 
  tft.setRotation(1);
  drawMenuPrincipal();
}

void loop() { // Démarre la fonction principale d'Arduino qui s'exécute en continu et en boucle tant que le jeu est allumé.
  if (pageActuelle == COULEUR) handleMenuCouleurLogic(); // Active les commandes de sélection des couleurs si le joueur se trouve sur cet écran spécifique.
  else if (pageActuelle >= JEU_NIV1) handleGameLogic(); Active les règles de la partie (mouvements, collisions) si le joueur est en train de jouer à un niveau.
  else handleMenuNavigation(); Gère les boutons pour naviguer dans les menus classiques si aucune partie n'est lancée.
}