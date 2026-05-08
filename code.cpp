#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- Configuration des Pins ---
#define BTN_ORANGE  25 
#define BTN_VERT    26 
#define BTN_BLEU    27 
#define BTN_JAUNE   32 
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
enum Page { PRINCIPAL, NIVEAUX, PARAMETRES, JEU_NIV1, JEU_NIV2, JEU_NIV3 };
Page pageActuelle = PRINCIPAL;

int marioX, marioY;
const int cibleX = 15, cibleY = 7;
byte grille[16][8];

int nbEnnemisActuels = 0;
int ennemiX[9], ennemiY[9];

// --- FONCTION SETUP (Celle qui manquait) ---
void setup() {
  pinMode(BTN_ORANGE, INPUT); pinMode(BTN_VERT, INPUT);
  pinMode(BTN_BLEU, INPUT); pinMode(BTN_JAUNE, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); pinMode(BLK_PIN, OUTPUT);
  digitalWrite(BLK_PIN, HIGH);
  
  tft.init(); 
  tft.setRotation(1);
  drawMenuPrincipal();
}

// --- LOGIQUE PRINCIPALE ---
void loop() {
  if (pageActuelle >= JEU_NIV1) {
    handleGameLogic();
  } else {
    handleMenuNavigation();
  }
}

void handleGameLogic() {
  int nextX = marioX, nextY = marioY;
  bool moved = false;

  if (digitalRead(BTN_ORANGE) == HIGH) { nextX--; moved = true; }
  else if (digitalRead(BTN_JAUNE) == HIGH) { nextX++; moved = true; }
  else if (digitalRead(BTN_BLEU) == HIGH) { nextY--; moved = true; }
  else if (digitalRead(BTN_VERT) == HIGH) { nextY++; moved = true; }

  if (moved) {
    if (nextX >= 0 && nextX <= 15 && nextY >= 0 && nextY <= 7 && grille[nextX][nextY] == 0) {
      marioX = nextX; marioY = nextY;
      if (pageActuelle != JEU_NIV1) deplacerEnnemis();
      drawGrilleJeu();
      verifierCollisions();
    } else {
      tone(BUZZER_PIN, 200, 50);
    }
    delay(150);
  }
}

void handleMenuNavigation() {
  if (digitalRead(BTN_VERT) == HIGH) {
    if (pageActuelle == PRINCIPAL) changerDePage(NIVEAUX);
    else if (pageActuelle == NIVEAUX) initialiserJeu(JEU_NIV1); 
    while(digitalRead(BTN_VERT) == HIGH); delay(100);
  }
  if (digitalRead(BTN_JAUNE) == HIGH && pageActuelle == NIVEAUX) {
     initialiserJeu(JEU_NIV2); 
     while(digitalRead(BTN_JAUNE) == HIGH); delay(100);
  }
  if (digitalRead(BTN_ORANGE) == HIGH && pageActuelle == NIVEAUX) {
     initialiserJeu(JEU_NIV3); 
     while(digitalRead(BTN_ORANGE) == HIGH); delay(100);
  }
  if (digitalRead(BTN_BLEU) == HIGH) {
    if (pageActuelle == PRINCIPAL) changerDePage(PARAMETRES);
    else changerDePage(PRINCIPAL); 
    while(digitalRead(BTN_BLEU) == HIGH); delay(100);
  }
}

// --- FONCTIONS DE DESSIN ---

void drawGrilleJeu() {
  tft.fillScreen(0x0841);
  tft.drawRoundRect(20, 20, 280, 200, 10, MY_BLUE);
  
  tft.fillRoundRect(30, 30, 260, 35, 5, 0x2124);
  tft.setTextColor(MY_SKY); tft.setTextSize(2);
  tft.setCursor(110, 40);
  if (pageActuelle == JEU_NIV1) tft.print("niveau 1");
  else if (pageActuelle == JEU_NIV2) tft.print("niveau 2");
  else if (pageActuelle == JEU_NIV3) tft.print("niveau 3");

  int gridX = 30, gridY = 70, cellW = 16, cellH = 17;
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 16; c++) {
      int px = gridX + (c * cellW), py = gridY + (r * cellH);
      tft.drawRect(px, py, cellW, cellH, GRID_LINE);
      if (grille[c][r] == 1) tft.fillRect(px+2, py+2, cellW-4, cellH-4, MY_GREEN);
    }
  }
  
  tft.fillRect(gridX + (cibleX*cellW)+2, gridY + (cibleY*cellH)+2, cellW-4, cellH-4, 0x07FF);
  for (int i = 0; i < nbEnnemisActuels; i++) {
    tft.fillRect(gridX + (ennemiX[i]*cellW)+2, gridY + (ennemiY[i]*cellH)+2, cellW-4, cellH-4, MY_YELLOW);
  }
  tft.fillRect(gridX + (marioX*cellW)+2, gridY + (marioY*cellH)+2, cellW-4, cellH-4, MY_RED);
}

void initialiserJeu(Page niv) {
  marioX = 0; marioY = 0;
  randomSeed(millis());
  nbEnnemisActuels = (niv == JEU_NIV1) ? 0 : (niv == JEU_NIV2 ? 5 : 9);

  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 8; y++) {
      grille[x][y] = (random(100) < 18) ? 1 : 0;
    }
  }
  grille[0][0] = 0; grille[cibleX][cibleY] = 0;

  for (int i = 0; i < nbEnnemisActuels; i++) {
    ennemiX[i] = random(5, 15); ennemiY[i] = random(0, 7);
    grille[ennemiX[i]][ennemiY[i]] = 0;
  }
  changerDePage(niv);
}

void deplacerEnnemis() {
  for (int i = 0; i < nbEnnemisActuels; i++) {
    int dir = random(4);
    int nx = ennemiX[i], ny = ennemiY[i];
    if (dir == 0) nx++; else if (dir == 1) nx--;
    else if (dir == 2) ny++; else if (dir == 3) ny--;
    if (nx >= 0 && nx <= 15 && ny >= 0 && ny <= 7 && grille[nx][ny] == 0) {
      ennemiX[i] = nx; ennemiY[i] = ny;
    }
  }
}

void verifierCollisions() {
  if (marioX == cibleX && marioY == cibleY) finDePartie("GAGNE !", MY_GREEN);
  for (int i = 0; i < nbEnnemisActuels; i++) {
    if (marioX == ennemiX[i] && marioY == ennemiY[i]) finDePartie("GAME OVER", MY_RED);
  }
}

void finDePartie(String txt, uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(color); tft.setTextSize(4);
  tft.setCursor(50, 100); tft.print(txt);
  delay(2000);
  changerDePage(NIVEAUX);
}

void changerDePage(Page p) {
  pageActuelle = p;
  if (p == PRINCIPAL) drawMenuPrincipal();
  else if (p == NIVEAUX) drawMenuNiveaux();
  else if (p == PARAMETRES) drawMenuParametres();
  else drawGrilleJeu();
}

void drawCommonUI() {
  tft.fillRect(0, 190, 320, 50, MY_GREY);
  tft.fillRect(45, 15, 230, 65, MY_RED);
  tft.setTextColor(TFT_WHITE); tft.setTextSize(3);
  tft.setCursor(60, 20); tft.print("Super");
  tft.setCursor(60, 45); tft.print("mario bros");
}

void drawButton(int y, uint16_t color, String label) {
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  tft.setTextColor(color == MY_YELLOW ? TFT_BLACK : TFT_WHITE);
  tft.setTextSize(2);
  int xPos = 160 - (label.length() * 6); 
  tft.setCursor(xPos, y + 8); tft.print(label);
}

void drawMenuPrincipal() {
  tft.fillScreen(MY_SKY); drawCommonUI();
  drawButton(110, MY_GREEN, "niveau");
  drawButton(155, MY_BLUE, "parametre");
}

void drawMenuNiveaux() {
  tft.fillScreen(MY_SKY); drawCommonUI();
  drawButton(85, MY_GREEN, "niveau 1");
  drawButton(120, MY_YELLOW, "niveau 2");
  drawButton(155, MY_ORANGE, "niveau 3");
  drawButton(190, MY_BLUE, "annuler");
}

void drawMenuParametres() {
  tft.fillScreen(MY_SKY); drawCommonUI();
  drawButton(110, MY_GREEN, "couleur mario");
  drawButton(155, MY_BLUE, "annuler");
}