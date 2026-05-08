#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- Pins ---
#define BTN_ORANGE  25 // Gauche
#define BTN_VERT    26 // Bas / Entrer Niveaux
#define BTN_BLEU    27 // Haut / Entrer Paramètres / Retour
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

int marioX = 0;
int marioY = 0;
const int cibleX = 15;
const int cibleY = 7;

// Grille de 16x8 pour stocker les obstacles (0 = vide, 1 = obstacle)
byte grille[16][8];

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
    int nextX = marioX;
    int nextY = marioY;
    bool moveAttempt = false;

    if (digitalRead(BTN_ORANGE) == HIGH) { nextX--; moveAttempt = true; }
    else if (digitalRead(BTN_JAUNE) == HIGH) { nextX++; moveAttempt = true; }
    else if (digitalRead(BTN_BLEU) == HIGH) { nextY--; moveAttempt = true; }
    else if (digitalRead(BTN_VERT) == HIGH) { nextY++; moveAttempt = true; }

    if (moveAttempt) {
      // Vérification des limites et des obstacles (grille == 1)
      if (nextX >= 0 && nextX <= 15 && nextY >= 0 && nextY <= 7) {
        if (grille[nextX][nextY] == 0) {
          marioX = nextX;
          marioY = nextY;
          tone(BUZZER_PIN, 800, 20);
          drawGrilleNiveau1();
        } else {
          tone(BUZZER_PIN, 200, 50); // Son d'erreur (mur)
        }
      }
      
      // Vérification Victoire
      if (marioX == cibleX && marioY == cibleY) {
        ecranGagne();
      }
      delay(150);
    }
  } 
  else {
    // Navigation Menus
    if (digitalRead(BTN_VERT) == HIGH) {
      if (pageActuelle == PRINCIPAL) changerDePage(NIVEAUX);
      else if (pageActuelle == NIVEAUX) {
        genererNiveau(); // Génère les obstacles
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

// Génère des obstacles aléatoires
void genererNiveau() {
  marioX = 0; marioY = 0;
  randomSeed(millis()); // Pour avoir de l'aléatoire différent à chaque fois
  
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 8; y++) {
      // 20% de chance d'avoir un obstacle
      if (random(100) < 20) grille[x][y] = 1; 
      else grille[x][y] = 0;
    }
  }
  // Sécurité : Pas d'obstacle sur le départ ni l'arrivée
  grille[0][0] = 0;
  grille[cibleX][cibleY] = 0;
}

void drawGrilleNiveau1() {
  tft.fillScreen(0x0841); // Fond bleu très sombre

  // 1. Dessin du cadre extérieur bleu (le contour)
  // On utilise les dimensions de ton image : 280x200
  tft.drawRoundRect(20, 20, 280, 200, 10, MY_SKY);
  
  // 2. Bandeau titre "niveau 1"
  tft.fillRoundRect(30, 30, 260, 35, 5, 0x2124);
  tft.setTextColor(MY_SKY);
  tft.setTextSize(2);
  tft.setCursor(110, 40);
  tft.print("niveau 1");

  // 3. Paramètres de la grille pour qu'elle touche les bords
  // On commence à x=30 et y=70 pour remplir l'espace sous le titre
  int gridX = 30; 
  int gridY = 70;
  int cellWidth = 16;  // Légèrement plus large pour remplir les 260px
  int cellHeight = 17; // Légèrement plus haut pour remplir les 140px restants

  // 4. Dessin du quadrillage
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 16; col++) {
      int xPos = gridX + (col * cellWidth);
      int yPos = gridY + (row * cellHeight);
      
      // On dessine les bordures des cases
      tft.drawRect(xPos, yPos, cellWidth, cellHeight, GRID_LINE);
      
      // Dessin des obstacles verts (grille == 1)
      if (grille[col][row] == 1) {
        tft.fillRect(xPos + 2, yPos + 2, cellWidth - 4, cellHeight - 4, MY_GREEN);
      }
    }
  }

  // 5. Cible (Bleu Cyan) à la dernière case
  tft.fillRect(gridX + (cibleX * cellWidth) + 2, gridY + (cibleY * cellHeight) + 2, cellWidth - 4, cellHeight - 4, 0x07FF);
  
  // 6. Mario (Rouge) à sa position actuelle
  tft.fillRect(gridX + (marioX * cellWidth) + 2, gridY + (marioY * cellHeight) + 2, cellWidth - 4, cellHeight - 4, MY_RED);
}

void ecranGagne() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(MY_YELLOW);
  tft.setTextSize(4);
  tft.setCursor(80, 100);
  tft.print("GAGNE !");
  
  // Petite mélodie de victoire
  tone(BUZZER_PIN, 1000, 100); delay(150);
  tone(BUZZER_PIN, 1200, 100); delay(150);
  tone(BUZZER_PIN, 1500, 300);
  
  delay(2000);
  changerDePage(NIVEAUX);
}

// --- Fonctions Menus (Inchangées) ---
void changerDePage(Page nouvellePage) {
  pageActuelle = nouvellePage;
  if (pageActuelle == PRINCIPAL) drawMenuPrincipal();
  else if (pageActuelle == NIVEAUX) drawMenuNiveaux();
  else if (pageActuelle == PARAMETRES) drawMenuParametres();
  else if (pageActuelle == JEU_NIVEAU_1) drawGrilleNiveau1();
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