#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- Pins (Logique HIGH) ---
#define BTN_ORANGE  25 // Gauche
#define BTN_VERT    26 // Bas / Entrer Niveaux
#define BTN_BLEU    27 // Haut / Entrer Paramètres
#define BTN_JAUNE   32 // Droite
#define BUZZER_PIN  14
#define BLK_PIN      4

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
enum Page { PRINCIPAL, NIVEAUX, PARAMETRES, JEU_NIVEAU_1 };
Page pageActuelle = PRINCIPAL;

int marioX = 0; // Colonne (0 à 15)
int marioY = 0; // Ligne (0 à 7)

void setup() {
  pinMode(BTN_ORANGE, INPUT);
  pinMode(BTN_VERT,   INPUT);
  pinMode(BTN_BLEU,   INPUT);
  pinMode(BTN_JAUNE,  INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLK_PIN,    OUTPUT);
  digitalWrite(BLK_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  drawMenuPrincipal();
}

void loop() {
  
  if (pageActuelle == JEU_NIVEAU_1) {
    // --- LOGIQUE DE MOUVEMENT DANS LE JEU ---
    bool moved = false;

    if (digitalRead(BTN_ORANGE) == HIGH) { // GAUCHE
      if (marioX > 0) { marioX--; moved = true; }
    }
    else if (digitalRead(BTN_JAUNE) == HIGH) { // DROITE
      if (marioX < 15) { marioX++; moved = true; }
    }
    else if (digitalRead(BTN_BLEU) == HIGH) { // HAUT
      if (marioY > 0) { marioY--; moved = true; }
    }
    else if (digitalRead(BTN_VERT) == HIGH) { // BAS
      if (marioY < 7) { marioY++; moved = true; }
    }

    if (moved) {
      tone(BUZZER_PIN, 800, 20); // Petit "tic" de mouvement
      drawGrilleNiveau1(); // Redessine la grille avec la nouvelle position
      delay(150); // Vitesse de déplacement
    }
  } 
  else {
    // --- NAVIGATION DANS LES MENUS ---
    if (digitalRead(BTN_VERT) == HIGH) {
      if (pageActuelle == PRINCIPAL) changerDePage(NIVEAUX);
      else if (pageActuelle == NIVEAUX) {
        marioX = 0; marioY = 0; // Reset position
        changerDePage(JEU_NIVEAU_1);
      }
      while(digitalRead(BTN_VERT) == HIGH); delay(100);
    }

    if (digitalRead(BTN_BLEU) == HIGH) {
      if (pageActuelle == PRINCIPAL) changerDePage(PARAMETRES);
      else changerDePage(PRINCIPAL); 
      while(digitalRead(BTN_BLEU) == HIGH); delay(100);
    }
  }
}

void changerDePage(Page nouvellePage) {
  tone(BUZZER_PIN, 1000, 50);
  pageActuelle = nouvellePage;
  if (pageActuelle == PRINCIPAL)       drawMenuPrincipal();
  else if (pageActuelle == NIVEAUX)     drawMenuNiveaux();
  else if (pageActuelle == PARAMETRES)  drawMenuParametres();
  else if (pageActuelle == JEU_NIVEAU_1) drawGrilleNiveau1();
}

void drawGrilleNiveau1() {
  tft.fillScreen(0x0841); 
  tft.drawRoundRect(20, 20, 280, 200, 10, MY_SKY);
  
  // Bandeau Titre
  tft.fillRoundRect(30, 30, 260, 35, 5, 0x2124);
  tft.setTextColor(MY_SKY);
  tft.setTextSize(2);
  tft.setCursor(110, 40);
  tft.print("niveau 1");

  int gridX = 40;
  int gridY = 75;
  int cellSize = 15;

  // Dessin de la grille vide
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 16; col++) {
      tft.drawRect(gridX + (col * cellSize), gridY + (row * cellSize), cellSize, cellSize, GRID_LINE);
    }
  }

  // Carré CYAN (Arrivée fixe)
  tft.fillRect(gridX + (15 * cellSize) + 2, gridY + (7 * cellSize) + 2, cellSize - 4, cellSize - 4, 0x07FF);

  // Carré ROUGE (Mario - Position variable)
  tft.fillRect(gridX + (marioX * cellSize) + 2, gridY + (marioY * cellSize) + 2, cellSize - 4, cellSize - 4, MY_RED);
}

// --- MENUS ---
void drawMenuPrincipal() {
  tft.fillScreen(MY_SKY);
  drawCommonUI();
  drawButton(110, MY_GREEN, "niveau");
  drawButton(155, MY_BLUE, "parametre");
}

void drawMenuNiveaux() {
  tft.fillScreen(MY_SKY);
  drawCommonUI();
  drawButton(85,  MY_GREEN,  "niveau 1");
  drawButton(120, MY_YELLOW, "niveau 2");
  drawButton(155, MY_ORANGE, "niveau 3");
  drawButton(190, MY_BLUE,   "annuler");
}

void drawMenuParametres() {
  tft.fillScreen(MY_SKY);
  drawCommonUI();
  drawButton(110, MY_GREEN, "couleur mario");
  drawButton(155, MY_BLUE, "annuler");
}

void drawCommonUI() {
  tft.fillRect(0, 190, 320, 50, MY_GREY); 
  tft.fillRect(45, 15, 230, 65, MY_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 20); tft.print("Super");
  tft.setCursor(60, 45); tft.print("mario bros");
}

void drawButton(int y, uint16_t color, String label) {
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  if (color == MY_YELLOW) tft.setTextColor(TFT_BLACK);
  else tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  int xPos = 160 - (label.length() * 6); 
  tft.setCursor(xPos, y + 8);
  tft.print(label);
}