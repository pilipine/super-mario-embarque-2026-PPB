//MENU_PRINCIPAL + MENU_NIVEAUX + Menu_PARAMETRE
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- Configuration des Pins (Logique HIGH) ---
#define BTN_RETOUR  27 // Orange (Haut / Retour)
#define BTN_VERT    26 // Vert   (Bas / Niveaux)
#define BTN_BLEU    27 // Bleu   (Select / Paramètres)
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

enum Page { PRINCIPAL, NIVEAUX, PARAMETRES };
Page pageActuelle = PRINCIPAL;

void setup() {
  pinMode(BTN_RETOUR, INPUT);
  pinMode(BTN_VERT,   INPUT);
  pinMode(BTN_BLEU,   INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLK_PIN,    OUTPUT);
  digitalWrite(BLK_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  drawMenuPrincipal();
}

void loop() {
  
  // --- NAVIGATION DEPUIS LE MENU PRINCIPAL ---
  if (pageActuelle == PRINCIPAL) {
    // Si on appuie sur VERT (26) -> Niveaux
    if (digitalRead(BTN_VERT) == HIGH) {
      changerDePage(NIVEAUX);
    }
    // Si on appuie sur BLEU (27) -> Paramètres (ton image couleur mario)
    if (digitalRead(BTN_BLEU) == HIGH) {
      changerDePage(PARAMETRES);
    }
  }

  // --- NAVIGATION DE RETOUR (Toutes pages vers Principal) ---
  else {
    // Si on appuie sur ORANGE (27) -> Retour
    if (digitalRead(BTN_RETOUR) == HIGH) {
      changerDePage(PRINCIPAL);
    }
  }
}

// Fonction pour changer de page proprement avec son
void changerDePage(Page nouvellePage) {
  tone(BUZZER_PIN, 1000, 100);
  pageActuelle = nouvellePage;
  
  if (pageActuelle == PRINCIPAL)  drawMenuPrincipal();
  else if (pageActuelle == NIVEAUX)    drawMenuNiveaux();
  else if (pageActuelle == PARAMETRES) drawMenuParametres();
  
  // Attente que TOUS les boutons soient relâchés
  while(digitalRead(BTN_VERT) == HIGH || digitalRead(BTN_BLEU) == HIGH || digitalRead(BTN_RETOUR) == HIGH);
  delay(200);
}

// --- VISUEL 1 : MENU PRINCIPAL ---
void drawMenuPrincipal() {
  tft.fillScreen(MY_SKY);
  drawCommonUI();
  drawButton(110, MY_GREEN, "niveau");
  drawButton(155, MY_BLUE, "parametre");
}

// --- VISUEL 2 : MENU NIVEAUX ---
void drawMenuNiveaux() {
  tft.fillScreen(MY_SKY);
  drawCommonUI();
  drawButton(85,  MY_GREEN,  "niveau 1");
  drawButton(120, MY_YELLOW, "niveau 2");
  drawButton(155, MY_ORANGE, "niveau 3");
  drawButton(190, MY_BLUE,   "annuler");
}

// --- VISUEL 3 : MENU PARAMÈTRES (Image image_2acbf2) ---
void drawMenuParametres() {
  tft.fillScreen(MY_SKY);
  drawCommonUI();
  drawButton(110, MY_GREEN, "couleur mario");
  drawButton(155, MY_BLUE, "annuler");
}

// --- ELEMENTS COMMUNS (TITRE ET SOL) ---
void drawCommonUI() {
  tft.fillRect(0, 190, 320, 50, MY_GREY); 
  tft.fillRect(45, 15, 230, 65, MY_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 20); tft.print("Super");
  tft.setCursor(60, 45); tft.print("mario bros");
}

// --- DESSIN BOUTON ---
void drawButton(int y, uint16_t color, String label) {
  tft.fillRoundRect(80, y, 160, 30, 15, color);
  if (color == MY_YELLOW) tft.setTextColor(TFT_BLACK);
  else tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  int xPos = 160 - (label.length() * 6); 
  tft.setCursor(xPos, y + 8);
  tft.print(label);
}