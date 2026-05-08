//MENU_PRINCIPAL + MENU_NIVEAUX
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- Configuration des Pins (selon ton code de test) ---
#define BTN_UP      27
#define BTN_DOWN    26
#define BTN_SELECT  27 
#define BUZZER_PIN  14
#define BLK_PIN      4

// --- Couleurs Mario ---
#define MY_RED       0xF800
#define MY_BLUE      0x001F
#define MY_GREEN     0x07E0 
#define MY_YELLOW    0xFFE0
#define MY_ORANGE    0xFD20
#define MY_SKY       0x5DFF
#define MY_GREY      0x4208

enum Page { MENU_PRINCIPAL, MENU_NIVEAUX };
Page pageActuelle = MENU_PRINCIPAL;

void setup() {
  // Configuration des entrées selon ton test (Logique HIGH)
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_SELECT, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLK_PIN, OUTPUT);
  digitalWrite(BLK_PIN, HIGH); // Allume le rétroéclairage

  tft.init();
  tft.setRotation(1);
  
  // Affichage initial
  drawMenuPrincipal();
}

void loop() {
  // --- Logique de passage au menu Niveaux ---
  // On utilise BTN_DOWN (26) comme tu l'as demandé pour changer de page
  if (digitalRead(BTN_DOWN) == HIGH) { 
    if (pageActuelle == MENU_PRINCIPAL) {
      tone(BUZZER_PIN, 1000, 100); // Son de transition
      pageActuelle = MENU_NIVEAUX;
      drawMenuNiveaux();
      
      // Attente du relâchement pour éviter de reboucler
      while(digitalRead(BTN_DOWN) == HIGH); 
      delay(200);
    }
  }

  // --- Logique pour revenir (Annuler) ---
  // On peut imaginer que BTN_UP (27) permet de revenir au menu principal
  if (digitalRead(BTN_UP) == HIGH) {
    if (pageActuelle == MENU_NIVEAUX) {
      tone(BUZZER_PIN, 500, 100);
      pageActuelle = MENU_PRINCIPAL;
      drawMenuPrincipal();
      
      while(digitalRead(BTN_UP) == HIGH);
      delay(200);
    }
  }
}

// --- VISUEL : MENU PRINCIPAL ---
void drawMenuPrincipal() {
  tft.fillScreen(MY_SKY);
  tft.fillRect(0, 190, 320, 50, MY_GREY);

  // Titre Mario
  tft.fillRect(45, 20, 230, 70, MY_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 30); tft.print("Super");
  tft.setCursor(60, 55); tft.print("mario bros");

  // Boutons
  drawButton(110, MY_GREEN, "niveau");
  drawButton(155, MY_BLUE, "parametre");
}

// --- VISUEL : MENU NIVEAUX ---
void drawMenuNiveaux() {
  tft.fillScreen(MY_SKY);
  tft.fillRect(0, 190, 320, 50, MY_GREY);

  // Titre
  tft.fillRect(45, 10, 230, 65, MY_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 15); tft.print("Super");
  tft.setCursor(60, 40); tft.print("mario bros");

  // Liste des niveaux
  drawButton(85,  MY_GREEN,  "niveau 1");
  drawButton(120, MY_YELLOW, "niveau 2");
  drawButton(155, MY_ORANGE, "niveau 3");
  drawButton(190, MY_BLUE,   "annuler");
}

// --- FONCTION DESSIN BOUTON ---
void drawButton(int y, uint16_t color, String label) {
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  
  if (color == MY_YELLOW) tft.setTextColor(TFT_BLACK);
  else tft.setTextColor(TFT_WHITE);
  
  tft.setTextSize(2);
  int padding = (160 - (label.length() * 12)) / 2;
  tft.setCursor(80 + padding, y + 8);
  tft.print(label);
}