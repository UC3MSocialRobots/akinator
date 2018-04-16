#ifndef AKINATORDIALOG_H
#define AKINATORDIALOG_H

/***************************************************************************//**
 * \class AkinatorDialog
 *
 * \brief this is a C++ interface to the website akinator.com
 *
 * It uses the web services of the site.
 * More information on the page :
 * http://www.akinator.com/aki_fr/ws/doc_ws.html
 *
 * \author Arnaud Ramey ( arnaud.ramey@m4x.org )
 *
 * \date 19/06/2009
 *******************************************************************************/


#define WEB_FILENAME "/tmp/web_content.txt"
#define PLAYER_NAME "Maggie-UC3M"

#define THRES_TRY_SUGGESTION 90
#define NUMBER_OF_TRY_MAX 20

///// global constants
#define GAME_STATUS_NOT_RUNNING 10
#define GAME_STATUS_RUNNING 11
#define GAME_STATUS_WON_BY_AKINATOR 12
#define GAME_STATUS_LOST_BY_AKINATOR 13


#define INPUT_TYPE_YES 21
#define INPUT_TYPE_PROBABLY 22
#define INPUT_TYPE_DO_NOT_KNOW 23
#define INPUT_TYPE_PROBABLY_NOT 24
#define INPUT_TYPE_NO 25

#define INPUT_TYPE_PEOPLE_YES 26
#define INPUT_TYPE_PEOPLE_NO 27


#define ANSWER_TYPE_ERROR_BAD_HTTP 100
#define ANSWER_TYPE_ERROR_WEBSITE_FAILURE 101
#define ANSWER_TYPE_ERROR_WRONG_ARGUMENT 102
#define ANSWER_TYPE_NEW_QUESTION 103
#define ANSWER_TYPE_SUGGESTION 104


///// STL imports
#include <stdio.h>
#include <stdlib.h>     // for exit, executing system commands
#include <iostream>     // for cin, cout
#include <string>     // for strings
#include <sstream>      // for std::sstreams
#include <fstream>      // for file reading
#include <vector>     // for vectors
#include <deque>      // for deque
using namespace std;


class AkinatorDialog {
public:
  AkinatorDialog();
  virtual ~AkinatorDialog();

  //! the path for the webservice of Maggie
  string URL_BASE;
  //! display DEBUG infos
  bool DEBUG;

  //! the current status of the game
  int game_status;

  bool ping();
  void start_game();
  void stop_game();
  void send_player_answer(int ans_code);
  void find_next_move();
  void get_answer(int* type, string* answer);
  int get_number_of_tries();

private:
  /**
   * \class AkinatorDialog::People
   * \brief the descriptor of an individual suggested as
   * an answer by the website Akinator
   */
  class People {
  public :
    //! ID of the element
    int id;
    //! Name of the element
    string name;
    //! Path of the picture of the element
    string picture_path;
    //! Static ID of the element in the database
    string id_base;

    int parse_line(string* line, int begin = 0);
    string toString();
  };

  /////
  ///// session variables
  /////
  //! id of the base - 0 = people, 1= objects
  int base;
  //! id of the partner
  int partner;
  //! number identifying the channel responsible of the session
  int channel;
  //! session id
  int session;
  //! step in the session
  int step;
  //! signature authenticating the session
  string signature;
  //! percentage indicating wether the engine is close to find
  double progression;
  //! false suggestions number
  int false_suggestions_number;


  /////
  ///// answers
  /////
  void answer_first_in_list();
  void answer_question();
  //! the current type of answer
  int answer_type;
  //! the current answer
  string answer;
  //! the suggestion of people
  deque<People> suggestions;

  /////
  ///// communication with the website
  /////
  void WS_get_url_content(string url, string* rep, int verbose_level = 1);
  bool WS_return_code_is_KO(string* line);
  //! the answer of the server
  string is_KO;

  bool WS_new_session();
  string question_from_server;

  void WS_cancelling();

  void WS_answer_something(int rep);

  void WS_list(int nb_ans = 10);

  void WS_choice(int order_in_list);
  //! number of times the element has been choosen
  int times_selected;

  void WS_exclusion();

  //////
  ////// static functions
  //////

  /** displays a text */
  inline void displayInfo(string txt, double v)  { cout << txt << " = " << v << endl; }
  inline void displayInfo(string txt)       { cout << txt << endl;}

  /** displays a text in DEBUG mode */
  inline void debugInfo(string txt)           { if (DEBUG) {cout << "AkinatorDialog:" << txt << endl;}  }
  inline void debugInfo(string txt, double v)   { if (DEBUG) {cout << "AkinatorDialog:" << txt << " = " << v << endl;}  }

  ///// parsing of the answers
  static int get_value_from_fieldname_in_string(string* line, string* rep, string field, int begin = 0);
  static int get_int_value_from_fieldname_in_string(string* line, int* rep, string field, int begin = 0);
  static int get_double_value_from_fieldname_in_string(string* line, double* rep, string field, int begin = 0);
};

#endif // AKINATORDIALOG_H
