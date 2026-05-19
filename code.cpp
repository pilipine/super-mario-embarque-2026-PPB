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
enum Page { PRINCIPAL, NIVEAUX, PARAMETRES, COULEUR, JEU_NIV1, JEU_NIV2, JEU_NIV3 };
Page pageActuelle = PRINCIPAL;

uint16_t couleurMarioActive = MY_RED; 
int indexCurseurCouleur = 0;
uint16_t palette[] = {0xF844, 0xFFE0, 0xF81F, 0x001F, 0x0400, 0x780F, 0xD6BA};

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
void deplacerEnnemis() {
  for (int i = 0; i < nbEnnemisActuels; i++) {
    int dir = random(4); 
    int nx = ennemiX[i], ny = ennemiY[i];
    if (dir == 0) nx++; 
    else if (dir == 1) nx--; 
    else if (dir == 2) ny++; 
    else if (dir == 3) ny--;
    
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) { 
      ennemiX[i] = nx; 
      ennemiY[i] = ny; 
    }
  }
}

void finDePartie(String txt, uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(color);
  tft.setTextSize(4);
  tft.setCursor(50, 100);
  tft.print(txt);
  delay(3000);
  changerDePage(NIVEAUX);
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
void handleGameLogic() {
  int nx = marioX, ny = marioY;
  bool moved = false;

  if (digitalRead(BTN_ORANGE) == HIGH) { nx--; moved = true; }
  else if (digitalRead(BTN_JAUNE) == HIGH) { nx++; moved = true; }
  else if (digitalRead(BTN_BLEU) == HIGH) { ny--; moved = true; }
  else if (digitalRead(BTN_VERT) == HIGH) { ny++; moved = true; }

  if (moved) {
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) {
      marioX = nx; marioY = ny;
      if (nbEnnemisActuels > 0) deplacerEnnemis();
      drawGrilleJeu();
      verifierCollisions();
    }
    delay(150);
  }
}

// --- AFFICHAGE ---
void drawGrilleJeu() {
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

void drawCommonUI() {
  tft.fillRect(0, 200, 320, 40, MY_GREY);
  tft.fillRect(45, 15, 230, 65, MY_RED);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(3);
  tft.setCursor(60, 20); tft.print("Super");
  tft.setCursor(60, 45); tft.print("mario bros");
}

void drawButton(int y, uint16_t color, String label) {
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
  tft.setCursor(160 - (label.length()*6), y + 8); tft.print(label);
}

void drawMenuCouleurs() {
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
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 8; y++) {
      // Zone de sécurité 2x2 en haut à gauche pour éviter de bloquer Mario au départ
      if (x <= 1 && y <= 1) {
        grille[x][y] = 0; 
      } else {
        grille[x][y] = (random(100) < 15) ? 1 : 0;
      }
    }
  }
  
  grille[cibleX][cibleY] = 0; // Sécurité pour la zone d'arrivée
  
  // Placement des ennemis
  for (int i = 0; i < nbEnnemisActuels; i++) { 
    ennemiX[i] = random(5, 15); 
    ennemiY[i] = random(0, 7); 
    grille[ennemiX[i]][ennemiY[i]] = 0; 
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

void loop() {
  if (pageActuelle == COULEUR) handleMenuCouleurLogic();
  else if (pageActuelle >= JEU_NIV1) handleGameLogic();
  else handleMenuNavigation();
}