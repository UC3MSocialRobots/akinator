#include "CAkinator.h"

/***************************************************************************//**
 * \file test_AkinatorDialog.cpp
 *
 * \brief Some tests for the class AkinatorDialog
 *
 * \author Arnaud Ramey ( arnaud.ramey@m4x.org )
 *
 * \date 19/06/2009
 *******************************************************************************/

#include <string.h>         // for strlen
void test_solver() {
  string rep;
  int rep_type;
  int player_choice;

  AkinatorDialog ak;
  ak.DEBUG = 1;
  //	ak.ping();
  //	return;

  ak.start_game();

  while (ak.game_status == GAME_STATUS_RUNNING) {
    ak.find_next_move();
    if (ak.game_status == GAME_STATUS_LOST_BY_AKINATOR)
      break;

    ak.get_answer(&rep_type, &rep);

    if (rep_type == ANSWER_TYPE_ERROR_WRONG_ARGUMENT)
      break;

    else if (rep_type == ANSWER_TYPE_ERROR_WEBSITE_FAILURE)
      break;

    else if (rep_type == ANSWER_TYPE_NEW_QUESTION) {
      cout << endl << endl << "Question:" << rep << endl;
      cout << "INPUT_TYPE_YES:" << INPUT_TYPE_YES << endl;
      cout << "INPUT_TYPE_PROBABLY:" << INPUT_TYPE_PROBABLY << endl;
      cout << "INPUT_TYPE_DO_NOT_KNOW:" << INPUT_TYPE_DO_NOT_KNOW << endl;
      cout << "INPUT_TYPE_PROBABLY_NOT:" << INPUT_TYPE_PROBABLY_NOT
           << endl;
      cout << "INPUT_TYPE_NO:" << INPUT_TYPE_NO << endl;
      cout << "Answer?";
      cin >> player_choice;
      ak.send_player_answer(player_choice);
    }

    else if (rep_type == ANSWER_TYPE_SUGGESTION) {
      cout << endl << endl << "Suggestion:" << rep << endl;
      cout << "INPUT_TYPE_PEOPLE_YES:" << INPUT_TYPE_PEOPLE_YES << endl;
      cout << "INPUT_TYPE_PEOPLE_NO:" << INPUT_TYPE_PEOPLE_NO << endl;
      cout << "Answer?";
      cin >> player_choice;
      ak.send_player_answer(player_choice);
    }

  } // end of while loop
}

void test_skill() {
#if IN_VISMAGGIE
  //CettsSkill* ettsSkill = new CettsSkill(0);
  CettsSkillWrapper etts; //skill wrapper para poder usar la habilidad de manera sencilla sin preocuparnos de escribir en MCP
  etts.activaEttsSkill(); //emite el evento que activa la habilidad
#endif
  CAkinator* ak = new CAkinator(100);
  ak->launch();

  while (1) {
    char line[10];
    std::cin.getline(line, 10);
    if (strlen(line) < 1) // user entered empty line -> quit
      break;
  }

  ak->end();
  delete ak;
}

int main() {
  test_solver();
  //test_string();
  //	test_skill();
}

