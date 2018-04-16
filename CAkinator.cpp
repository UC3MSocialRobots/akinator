#include "CAkinator.h"

/***************************************************************************//**
 * \file CAkinator.cpp
 *
 * \brief The implementation of CAkinator.h
 *
 * \author Arnaud Ramey ( arnaud.ramey@m4x.org )
 *
 * \date 19/06/2009
 *******************************************************************************/

#include <unistd.h>

#if !STAND_ALONE
string PATH_CAkinator = (string) AD_HOME + "/robot/src/skills/akinator/";

/** signal listener : start the acquisition */
void CAkinator_ManejadorEventoStart(void* aux, int p) {
  cout << "-> Starting the \"CAkinator\" skill." << endl;
  CAkinator* pHab = (CAkinator *)aux;
  if (pHab->verEstado() != habAD::EJECUCION)
  pHab->launch();
}

/** signal listener : stop the acquisition */
void CAkinator_ManejadorEventoStop(void* aux, int p) {
  cout << "-> Stopping the \"CAkinator\" skill." << endl;
  CAkinator* pHab = (CAkinator *)aux;
  if (pHab->verEstado() != habAD::BLOQUEADO)
  pHab->end();
}
#endif

/** constructor */
#if !STAND_ALONE
CAkinator::CAkinator(unsigned long tiempoCiclo) :
CHabilidad(tiempoCiclo) {
#else
CAkinator::CAkinator(unsigned long /*tiempoCiclo*/) {
#endif
  displayInfo("*** CAkinator created. ***");
  debugInfo("CAkinator constructor");

  DEBUG = 0; // display DEBUG infos

#if !STAND_ALONE
#if IN_VISMAGGIE
  ///// init body
  neck.init(VISMAGGIE);
  arms.init(VISMAGGIE);
  //contact.init(VISMAGGIE);
  contact = new CHab_Tacto(200000);
  contact->activa();
#endif

  ///// suscribe to the signals
  this->controlEventos.subscribe(AKINATOR_START,
      CAkinator_ManejadorEventoStart, (void*) this);
  this->controlEventos.subscribe(AKINATORE_STOP,
      CAkinator_ManejadorEventoStop, (void*) this);
#endif
}

/** destructor */
CAkinator::~CAkinator() {
  // stop the aki solver
  if (aki_solver.game_status != GAME_STATUS_NOT_RUNNING)
    aki_solver.stop_game();
#if !STAND_ALONE
  // unsuscribe to the signals
  this->controlEventos.unsubscribe(AKINATOR_START);
  this->controlEventos.unsubscribe(AKINATORE_STOP);
#endif
}

/** lauch the CAkinator mode */
void CAkinator::launch() {
  debugInfo("CAkinator::launch()");

  aki_solver.DEBUG = DEBUG;
  touchListener_init();
  start_game();
#if !STAND_ALONE
  activa();
#endif
}

/** terminate the CAkinator mode */
void CAkinator::end() {
  debugInfo("CAkinator::end()");
#if !STAND_ALONE
  this->bloquea();
#endif
  sleep(1); // wait a little bit...
}

/////// main loop                  ////////

/** main loop of the CAkinator mode */
void CAkinator::proceso() {
  //  debugInfo("");
  //  debugInfo("CAkinator::processo()");

}
/////// end of the main loop       ////////


/* watch if the body was touched */
void CAkinator_touchListener(void* /*aux*/, int /*p*/) {
#if IN_VISMAGGIE
  CAkinator* pHab = (CAkinator *)aux;

  if (p != 0) {
    // M_CABEZA
    // M_MANO_IZQ_1 2 3   M_HOMBRO_IZQ    M_COSTADO_IZQ   I_ESPALDA_IZQ
    // M_MANO_DRCH_1 2 3    M_HOMBRO_DRCH   M_COSTADO_DRCH    I_ESPALDA_DRCH

    if (p == M_HOMBRO_IZQ) {
      return pHab->touch_reaction( LEFT_SHOULDER);
      //pHab->say ( "1" );
    } else if (p == M_HOMBRO_DRCH) {
      return pHab->touch_reaction( RIGHT_SHOULDER);
      //pHab->say ( "2" );
    } else if (p == M_CABEZA) {
      return pHab->touch_reaction( HEAD);
      //pHab->say ( "3" );
    }
    /* touch the costado => repeat last question */
    else if (p == M_MANO_IZQ_1 || p == M_MANO_IZQ_2 || p == M_MANO_IZQ_3) {
      return pHab->say_last_question();
    }

    /* other part => protest */
    else {
      if (pHab->etts.canSpeakInmediatly() )
      pHab->say("No ! Tienes que tocarme los hombros o la cabeza.");
    }
  }
#endif
}

void CAkinator::touchListener_init() {
#if IN_VISMAGGIE
  controlEventos.subscribe(TOCADO, CAkinator_touchListener, (void*)this);
#endif
}

/*!
 * \brief   a routine for reacting to the touchs
 */
void CAkinator::touch_reaction(int body_part) {
  debugInfo("touch_reaction()", body_part);

  /* game not started => start */
  if (aki_solver.game_status == GAME_STATUS_NOT_RUNNING) {
    start_game();
  }

  /* game running => analyze */
  else if (aki_solver.game_status == GAME_STATUS_RUNNING) {

    /* question asked => answer yes or no */
    if (solver_rep_type == ANSWER_TYPE_NEW_QUESTION) {
      if (body_part == RIGHT_SHOULDER) {
        int choice = rand() % 2;
        if (choice == 0)
          say("Vale! ");
        if (choice == 1)
          say("Claro que si! ");
        return send_input(INPUT_TYPE_YES);
      }
      if (body_part == LEFT_SHOULDER) {
        int choice = rand() % 2;
        if (choice == 0)
          say("no!");
        if (choice == 1)
          say("vale. No lo es. ");
        return send_input(INPUT_TYPE_NO);
      }
      if (body_part == HEAD) {
        say("Aa ? No sabes ?");
        return send_input(INPUT_TYPE_DO_NOT_KNOW);
      }
    } // end of new question

    /* people suggested => answer yes or no */
    else if (solver_rep_type == ANSWER_TYPE_SUGGESTION) {
      if (body_part == RIGHT_SHOULDER) {
        say("Excelente! Correcto! He adivinado tu personaje.");
        return send_input(INPUT_TYPE_PEOPLE_YES);
      }
      if (body_part == LEFT_SHOULDER) {
        say("Mierda. Es otro personaje. ");
        return send_input(INPUT_TYPE_PEOPLE_NO);
      }
    } //  end of suggestion
  } //  end of if game running
}

/*!
 * \brief   start a game
 */
void CAkinator::start_game() {
  debugInfo("start_game()");
  say(
      "Te iré diciendo preguntas sobre el personaje y deberás irme respondiendo. Para decir que si tócame el hombro derecho. Para decir que no tócame el hombro izquierdo. Sino estás seguro tócame la cabeza. Si quieres que repita la última frase tocame la mano.    ");
  say("");
  say("");
  say("Comenzamos con las preguntas.");

  aki_solver.start_game();
  analyse_solver_answer();
}

/*!
 * \brief   send a message to the solver
 */
void CAkinator::send_input(int code) {
  debugInfo("send_input()", code);

  aki_solver.send_player_answer(code);
  analyse_solver_answer();
}

/*!
 * \brief   analyse the answer of the solver
 */
void CAkinator::analyse_solver_answer() {
  debugInfo("analyse_answer()");

  aki_solver.find_next_move();
  aki_solver.get_answer(&solver_rep_type, &solver_rep);

  if (solver_rep_type == ANSWER_TYPE_ERROR_WRONG_ARGUMENT)
    return;

  else if (solver_rep_type == ANSWER_TYPE_ERROR_WEBSITE_FAILURE)
    return;

  else if (aki_solver.game_status == GAME_STATUS_WON_BY_AKINATOR) {
    ostringstream m;
    m << "He ganado! . " << "He necessitado,  "
        << aki_solver.get_number_of_tries() << ", preguntas .";
    say(m.str());

    aki_solver.stop_game();
  }

  else if (aki_solver.game_status == GAME_STATUS_LOST_BY_AKINATOR) {
    say("He perdido.");
    aki_solver.stop_game();
  }

  else if (solver_rep_type == ANSWER_TYPE_NEW_QUESTION) {
    // say the sentence
    say_last_question();
  }

  else if (solver_rep_type == ANSWER_TYPE_SUGGESTION) {
    solver_rep = "Pienso a ! " + solver_rep;
    say_last_question();
  }
}

/*!
 * \brief   say the last question obtained from the website
 */
void CAkinator::say_last_question() {
  say(solver_rep);
}

/** say something */
void CAkinator::say(string msg) {
#if IN_VISMAGGIE
  /* HAPPYNESS_EMOTION, SADNESS_EMOTION,
   * TRANQUILITY_EMOTION, NERVOUSNESS_EMOTION */
  //if (etts.sayText( (char*) msg.c_str(), TRANQUILITY_EMOTION, DEFAULT_MODE);
  etts.sayText( (char*) msg.c_str(), TRANQUILITY_EMOTION, DEFAULT_MODE);
#endif
  string sentence;
  sentence = "Maggie says:'" + msg + "'";
  if (DEBUG)
    sentence = "\n\n" + sentence + "\n\n";
  displayInfo(sentence);
}

