#ifndef AKINATOR_SKILL_H
#define AKINATOR_SKILL_H

////// do we have the architecture of Maggie or do we compile a stand alone ?
#define STAND_ALONE     1
////// are we compiling in Maggie ?
#define IN_VISMAGGIE 0

/***************************************************************************//**
 * \class CAkinator
 *
 * \brief A game where Maggie must guess which people you think about in
 * less than 20 questions
 *
 * The power of it comes from http://es.akinator.com/
 * It uses AkinatorDialog for the communication with the website.
 *
 * \author Arnaud Ramey ( arnaud.ramey@m4x.org )
 *
 * \date 19/06/2009
*******************************************************************************/


///// my imports
#include "AkinatorDialog.h"

#define LEFT_SHOULDER 10
#define RIGHT_SHOULDER 20
#define HEAD 30

///// Maggie imports
#if !STAND_ALONE
#include "habilidad.h"
#include "cevent.h"
#include "protoAD.h"
#if IN_VISMAGGIE
#include "ettsSkillWrapper.h"
#include "armBase.h"
#include "actuadorCuello.h"
#include "hab_tacto.h"      // the contact detector
#endif // IN_VISMAGGIE
#endif // STAND_ALONE

#if STAND_ALONE
class CAkinator {
#else
class CAkinator : public habAD::CHabilidad {
#endif
public:
  /** constructor */
  CAkinator(unsigned long tiempoCiclo);
  /** destructor */
  ~CAkinator();

  /////
  ///// general functions
  /////
  /** main loop of the CAkinator mode */
  void proceso(void);
  /** lauch the CAkinator mode */
  void launch(void);
  /** terminate the CAkinator mode */
  void end(void);

  //! display DEBUG infos
  bool DEBUG;

  void touch_reaction( int body_part );
  void say_last_question();

private:
#if !STAND_ALONE
  //! Gestor de eventos de la habilidad
  CEventManager controlEventos;
#endif

  /////
  ///// general fields
  /////
  AkinatorDialog aki_solver;
  void touchListener_init();
  void start_game();
  void send_input(int code);
  void analyse_solver_answer();
  string solver_rep;
  int solver_rep_type;


  /////
  ///// body
  /////
#if IN_VISMAGGIE
  CactuadorCuello neck;
  CarmBase arms;
  CHab_Tacto* contact;
#endif

  /////
  ///// voice
  /////
public:
#if IN_VISMAGGIE
  CettsSkillWrapper etts; // the voice
#endif
  /** say something */
  void say(string msg);

private:
  /** displays a text */
  inline void displayInfo(string txt) {cout << txt << endl;}

  /** displays a text in DEBUG mode */
  inline void debugInfo(string txt)         {if (DEBUG) cout << "CAkinator:" << txt << endl;}
  inline void debugInfo(string txt, double v)   {if (DEBUG) cout << "CAkinator:" << txt << " = " << v << endl;}
};


#endif
