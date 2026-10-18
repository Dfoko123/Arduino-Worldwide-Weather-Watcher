#include <Arduino.h>
#include "app_buttons.h"
#include "grove_button.h"

#define ANTI_REBOND_MS   30UL     /* un changement doit durer 30 ms pour compter */
#define APPUI_LONG_MS    5000UL   /* 5 secondes = appui long (PDF)               */

/* Tout ce qu'on retient pour UN bouton */
typedef struct {
    bool          derniere_lecture;    /* derniere valeur lue (brute, avec rebonds) */
    unsigned long dernier_changement;  /* moment ou la valeur brute a change        */
    bool          appuye;              /* etat "propre" apres anti-rebond           */
    unsigned long debut_appui;         /* moment ou l'appui a commence              */
    bool          deja_signale;        /* l'appui long a deja ete signale ?         */
    bool          appui_long;          /* "drapeau" lu par Buttons_RougeLong/VertLong */
} EtatBouton;

static EtatBouton rouge, vert;
static bool rouge_au_demarrage = false;
static bool initialise = false;

/* Prepare un bouton a partir de son etat au demarrage */
static void Bouton_Preparer(EtatBouton *b, bool lu)
{
    b->derniere_lecture   = lu;
    b->dernier_changement = millis();
    b->appuye             = lu;
    b->debut_appui        = millis();
    /* Si le bouton est DEJA appuye a l'allumage (bouton rouge = mode config),
       on ne doit pas compter cet appui comme un appui long. */
    b->deja_signale       = lu;
    b->appui_long         = false;
}

/* Initialisation automatique au premier appel */
static void Buttons_InitSiBesoin(void)
{
    if (initialise) return;
    rouge_au_demarrage = GroveButton1_IsPressed();
    Bouton_Preparer(&rouge, rouge_au_demarrage);
    Bouton_Preparer(&vert, GroveButton2_IsPressed());
    initialise = true;
}

/* Met a jour UN bouton : anti-rebond + detection de l'appui de 5 s */
static void Bouton_MettreAJour(EtatBouton *b, bool lu)
{
    unsigned long maintenant = millis();

    /* 1) La valeur brute a change : on note quand */
    if (lu != b->derniere_lecture) {
        b->derniere_lecture   = lu;
        b->dernier_changement = maintenant;
    }

    /* 2) Anti-rebond : on n'accepte le changement que s'il dure depuis 30 ms */
    if (lu != b->appuye && maintenant - b->dernier_changement >= ANTI_REBOND_MS) {
        b->appuye = lu;
        if (lu) b->debut_appui = maintenant;   /* debut d'un nouvel appui */
        b->deja_signale = false;               /* nouvel appui ou relachement */
    }

    /* 3) Appui long : appuye depuis 5 s et pas encore signale */
    if (b->appuye && !b->deja_signale && maintenant - b->debut_appui >= APPUI_LONG_MS) {
        b->appui_long   = true;
        b->deja_signale = true;   /* un seul signal par appui, meme tenu 15 s */
    }
}

void Buttons_Update(void)
{
    Buttons_InitSiBesoin();
    Bouton_MettreAJour(&rouge, GroveButton1_IsPressed());
    Bouton_MettreAJour(&vert,  GroveButton2_IsPressed());
}

bool Buttons_RougeAuDemarrage(void)
{
    Buttons_InitSiBesoin();
    return rouge_au_demarrage;
}

bool Buttons_RougeLong(void)
{
    if (rouge.appui_long) {
        rouge.appui_long = false;   /* on "consomme" le signal : true UNE seule fois */
        return true;
    }
    return false;
}

bool Buttons_VertLong(void)
{
    if (vert.appui_long) {
        vert.appui_long = false;
        return true;
    }
    return false;
}