% DO NOT EDIT this file manually; it is automatically
% generated from Documentation/snippets/new
% Make any changes in Documentation/snippets/new/
% and then run scripts/auxiliar/makelsr.py
%
% This file is in the public domain.
%% Note: this file works from version 2.13.36
\version "2.13.36"

\header {
%% Translation of GIT committish: fa19277d20f8ab0397c560eb0e7b814bd804ecec
  texidoces = "
Para tipografiar las barras agrupadas en la forma @code{3-4-3-2} sólo
es necesario modificar la estructura de pulsos:

"
  doctitlees = "Alteración de los finales de barra predeterminados"



%% Translation of GIT committish: 0a868be38a775ecb1ef935b079000cebbc64de40
  texidocde = "
Um Balken im 12/8-Takt als @code{3-4-3-2} zu gruppieren, muss man zuerst die
Standardwerte für die Balken im 12/8-Takt rückgängig machen und dann die neuen
Werte setzen:

"
  doctitlede = "Standard-Balkenwerte rückgängig machen"



%% Translation of GIT committish: 548076f550a2b7fb09f1260f0e5e2bb028ad396c
  texidocfr = "
Pour obtenir des ligatures en groupes de @code{3-4-3-2} croches, dans
une mesure à 12/8, il faudra préalablement annuler les réglages par
défaut relatifs à 12/8, puis ajouter nos propres règles :

"
  doctitlefr = "Annulation des règles de ligature par défaut"

  lsrtags = "rhythms"
  texidoc = "
To typeset beams grouped @code{3-4-3-2} one need only change the
beat structure:
"
  doctitle = "Reverting default beam endings"
} % begin verbatim


\relative c'' {
  \time 12/8

  % Default beaming
  a8 a a a a a a a a a a a

  % Set new values for beam endings
  \set Score.beatStructure = #'(3 4 3 2)
  a8 a a a a a a a a a a a
}
