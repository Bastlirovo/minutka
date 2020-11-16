// Minutka v1

// knihovny
#include "LedControl.h"
#include <TimeLib.h>
#include <DS1307RTC.h>

// nastavení čísel propojovacích pinů
#define MAX_DIN  16
#define MAX_CLK  15
#define MAX_LOAD 11
#define MAX_MODULES 1

#define TLC_1 6
#define TLC_2 8
#define TLC_3 9
#define TLC_4 10

// definice pro práci se stavem zařízení
#define HODINY  0
#define ODPOCET 1

// objekt displeje "lc" z knihovny LedControl
LedControl lc = LedControl(MAX_DIN, MAX_CLK, MAX_LOAD, MAX_MODULES);

// proměnné
bool blik = 0;
int pocitadlo = 0;
int pocitadlo_pomlk = 0;
bool stav_zarizeni = HODINY;
unsigned long cas_posledni_dvojtecky = 0;
unsigned long cas_poslednich_tlacitek = 0;
int pauza_mezi_dvojteckami = 1000;
int pauza_mezi_tlacitky    = 500;

void vypis_rtc_cas () {
  // načtení časových údajů z RTC obvodu
  tmElements_t tm;

  // v případě platných dat se vypíše čas na displej
  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    lc.setDigit(0, 1, (tm.Hour / 10),   false);
    lc.setDigit(0, 2, (tm.Hour % 10),   false);
    lc.setDigit(0, 3, (tm.Minute / 10), false);
    lc.setDigit(0, 4, (tm.Minute % 10), false);
  }
  // Vypsaní chybové hlášky při nenastaveném RTC
  else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    //    delay(9000);
  }
}

void vypis_cislo(int vstup) {
  // výpis času
  lc.setDigit(0, 1, ((vstup / 60) / 10), false);
  lc.setDigit(0, 2, ((vstup / 60) % 10), false);
  lc.setDigit(0, 3, ((vstup % 60) / 10), false);
  lc.setDigit(0, 4, ((vstup % 60) % 10), false);
}

void nacti_tlacitka() {
  int pocitadlo_stare = pocitadlo;
  bool stav = digitalRead(TLC_1);
  if (stav == LOW) {
    pocitadlo += 600; // přičtení 10 minut
  }
  stav = digitalRead(TLC_2);
  if (stav == LOW) {
    pocitadlo += 60; // přičtení   1 minuty
  }
  stav = digitalRead(TLC_3);
  if (stav == LOW) {
    pocitadlo += 10; // přičtení  10 sekund
  }
  stav = digitalRead(TLC_4);
  if (stav == LOW) {
    pocitadlo += 5; // přičtení    5 sekundy
  }
  // kontrola stisknutí libov. tlačítka
  if ((pocitadlo_stare != pocitadlo) && (stav_zarizeni != ODPOCET)) {
    stav_zarizeni = ODPOCET;
  }
}

void blik_dvojtecky() {
  if (blik == 1) {
    lc.setRow(0, 0, 24);
  }
  else {
    lc.setRow(0, 0, 0);
  }
  blik = !blik;
}

void setup() {
  // nastavení tlačítek jako vstupů
  pinMode(TLC_1, INPUT_PULLUP);
  pinMode(TLC_2, INPUT_PULLUP);
  pinMode(TLC_3, INPUT_PULLUP);
  pinMode(TLC_4, INPUT_PULLUP);

  /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
  */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 2);
  /* and clear the display */
  lc.clearDisplay(0);

}

void loop() {
  // vytvoření proměnné cas a uložení
  // aktuálního času od zapnutí Arduina
  // v sekundách
  long cas = millis() / 1000;

  if (stav_zarizeni == ODPOCET) {
    if ((pocitadlo - cas) > 0) {
      vypis_cislo(pocitadlo - cas);
    }
    else {
      // výpis pomlček
      lc.setRow(0, 1, B00000001);
      lc.setRow(0, 2, B00000001);
      lc.setRow(0, 3, B00000001);
      lc.setRow(0, 4, B00000001);
      delay(500);
      pocitadlo = millis() / 1000;
      pocitadlo_pomlk++;
      // po 5 sekundách pomlk přepneme na hodiny
      if (pocitadlo_pomlk == 10) {
        pocitadlo_pomlk = 0;
        stav_zarizeni = HODINY;
      }
    }
  }
  else if (stav_zarizeni == HODINY) {
    vypis_rtc_cas();
  }
  // načtení stavu tlačítek
  if ((millis() - cas_poslednich_tlacitek) > pauza_mezi_tlacitky) {
    nacti_tlacitka();
    cas_poslednich_tlacitek = millis();
  }

  // vyřešení blikání dvojtečkou,
  // každé druhé volání loop()
  if ((millis() - cas_posledni_dvojtecky) > pauza_mezi_dvojteckami) {
    blik_dvojtecky();
    cas_posledni_dvojtecky = millis();
  }

}
