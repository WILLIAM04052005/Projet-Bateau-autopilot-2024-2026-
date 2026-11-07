// =====================================================================
//  secrets.example.h — Modèle des identifiants Wi-Fi
//
//  1. Copier ce fichier en « secrets.h » dans le même dossier.
//  2. Renseigner vos valeurs.
//  secrets.h est ignoré par Git (.gitignore) : vos mots de passe ne
//  sont jamais publiés.
// =====================================================================
#pragma once

// Réseau créé par le bateau (mode Access Point) — 8 caractères minimum
#define AP_SSID     "BoatControl"
#define AP_PASSWORD "change-moi"

// Réseau existant auquel se connecter (mode Station, USE_AP_MODE = false)
#define STA_SSID     "VotreReseau"
#define STA_PASSWORD "VotreMotDePasse"
