#include "AkinatorDialog.h"

/***************************************************************************//**
 * \file AkinatorDialog.cpp
 *
 * \brief The implementation of AkinatorDialog.h
 *
 * \author Arnaud Ramey ( arnaud.ramey@m4x.org )
 *
 * \date 19/06/2009
 *******************************************************************************/

extern "C" {
#include <errno.h>    //for error handling
#include <iconv.h>
#include <string.h>
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * Execute a command and get its output.
 * Careful though, be aware that this will only grab stdout and not stderr
 * From http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
 * \param cmd
 * \return
 *    the output of that command
 */
inline std::string exec_system_get_output(const char* cmd) {
  //printf("exec_system_get_output('%s')\n", cmd);
  FILE* pipe = popen(cmd, "r");
  if (!pipe) {
    printf("exec_system_get_output('%s'): could not open pipe\n", cmd);
    return "ERROR";
  }
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

/* Initialize the library. */
iconv_t initialize(const char * EUCSET, const char * OUTSET) {
  iconv_t conv_desc;
  conv_desc = iconv_open(OUTSET, EUCSET);
  //if ((int) conv_desc == -1) {
  if (conv_desc == NULL) {
    /* Initialization failure. */
    if (errno == EINVAL) {
      printf("Conversion from '%s' to '%s' is not supported.\n",
             EUCSET, OUTSET);
    } else {
      printf("Initialization failure: %s\n", strerror(errno));
    }
    exit(1);
  }
  return conv_desc;
}

////////////////////////////////////////////////////////////////////////////////

/* Convert EUC into UTF-8 using the iconv library. */
std::string euc2utf8(iconv_t conv_desc, const char * euc) {
  size_t iconv_value;
  char * utf8;
  //unsigned int len;
  //unsigned int utf8len;
  size_t len, utf8len;
  /* The variables with "start" in their name are solely for display
     of what the function is doing. As iconv runs, it alters the
     values of the variables, so these are for keeping track of the
     start points and start lengths. */
  char * utf8start;
  //const char * euc_start;
  //int len_start;
  //int utf8len_start;

  len = strlen(euc);
  if (!len) {
    //printf("Input std::string is empty.");
    return "";
  }
  /* Assign enough space to put the UTF-8. */
  utf8len = 2 * len;
  utf8 = (char*) calloc(utf8len, 1);
  /* Keep track of the variables. */
  utf8start = utf8;
  //len_start = len;
  //utf8len_start = utf8len;
  //euc_start = euc;
  /* Display what is in the variables before calling iconv. */
  //show_values("before", euc_start, len_start, utf8start, utf8len_start);
  iconv_value = iconv(conv_desc, (char**) &euc, &len, &utf8, &utf8len);
  /* Handle failures. */
  if (iconv_value == (size_t) -1) {
    printf("iconv failed: in std::string '%s', length %d, "
           "out std::string '%s', length %d", euc, (int) len, utf8start,
           (int) utf8len);
    switch (errno) {
      /* See "man 3 iconv" for an explanation. */
      case EILSEQ:
        throw std::invalid_argument("Invalid multibyte sequence.\n");
        break;
      case EINVAL:
        throw std::invalid_argument("Incomplete multibyte sequence.\n");
        break;
      case E2BIG:
        throw std::invalid_argument("No more room.\n");
        break;
      default:
        //throw std::invalid_argument("Error: %s.\n", strerror(errno));
        throw std::invalid_argument("Error\n");
    }
    exit(1);
  }
  /* Display what is in the variables after calling iconv. */
  //show_values("after", euc_start, len_start, utf8start, utf8len_start);
  return std::string(utf8start);
}

////////////////////////////////////////////////////////////////////////////////

/* Close the connection with the library. */

void finalize(iconv_t conv_desc) {
  int v = iconv_close(conv_desc);
  if (v != 0) {
    //printf("iconv_close failed: %s\n", strerror(errno));
  }
}

////////////////////////////////////////////////////////////////////////////////

/*! change the encoding of a string
 * \param content the string to change
 * \param EUCSET the initial encoding of the string
 * \param OUTSET the encoding we want in output
 */
void convert_string_encoding(std::string & content,
                             const char * EUCSET, const char * OUTSET) {

  /* Conversion descriptor. */
  iconv_t conv_desc = initialize(EUCSET, OUTSET);

  content = euc2utf8(conv_desc, (char*) content.c_str());
  finalize(conv_desc);

  //  if (out_string)
  //      //printf("Final iconv output: %s\n", out_string);
}

////////////////////////////////////////////////////////////////////////////////

static void find_and_replace( string &source, const string find, string replace ) {
  size_t j;
  for ( ; (j = source.find( find )) != string::npos ; ) {
    source.replace( j, find.length(), replace );
  }
}

////////////////////////////////////////////////////////////////////////////////

/*! change the encoding of a string UTF8 -> ISO-8859-1
 * \param content the string to change
 */
void convert_string_encoding_utf_to_iso(std::string & content) {
  convert_string_encoding(content, "UTF-8", "ISO-8859-1");
}

////////////////////////////////////////////////////////////////////////////////

/*! convert the HTML entities of a string -> UTF-8
 * \param content the string to change
 */
void convert_string_encoding_htlm_to_utf(std::string & content) {
#if 0 // use lib - does not work, give a segfault (buggy lib?)
  RECODE_OUTER outer = recode_new_outer (true);
  RECODE_REQUEST request = recode_new_request (outer);
  const char* code = "HTML_4.0";
  recode_scan_request (request, code);
  char * content_out = new char [content.length()+1];
  std::strcpy (content_out, content.c_str());
  size_t output_allocated, output_length = content.size() + 1;
  recode_string_to_buffer(request, content.c_str(),
                          &content_out, &output_length, &output_allocated);
  content = std::string(content_out);
  delete content_out;
  recode_delete_request (request);
  recode_delete_outer (outer);
#else // make a system call
  find_and_replace(content, "\"", "\\\""); // escape "
  // we need to recode content to iso
  std::string content_iso = content;
  convert_string_encoding_utf_to_iso(content_iso);
  std::ostringstream command;
  // remove final line break with "-n"
  command << "echo -n \"" << content_iso << "\" | recode HTML_4.0..UTF-8";
  content = exec_system_get_output(command.str().c_str());
#endif
}

/*!
 * \brief   constructor
 */
AkinatorDialog::AkinatorDialog() {
  //URL_BASE = "http://es.akinator.com/ws/";
  URL_BASE = "http://api-es3.akinator.com/ws/";
  //URL_BASE = "http://www.akinator.com/aki_fr/ws/";
  //  URL_BASE = "http://fr.akinator.com/ws/";
  session = -1;
  game_status = GAME_STATUS_NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   destructor
 */
AkinatorDialog::~AkinatorDialog() {
  if (game_status != GAME_STATUS_NOT_RUNNING)
    stop_game();
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   ping the server of Akinator
 * \return  true if the server is responding
 */
bool AkinatorDialog::ping() {
  debugInfo("ping()");

  int base = 0;

  ostringstream url;
  url << URL_BASE << "ping.php?";
  url << "base=" << base;

  string url_content;
  WS_get_url_content(url.str(), &url_content, 3);

  bool serv_KO = WS_return_code_is_KO(&url_content);
  debugInfo("Ping answer:" + is_KO);
  return serv_KO;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   start a new session
 *
 * \return  true if successful
 */
void AkinatorDialog::start_game() {
  debugInfo("start_game()");
  bool was_launch_OK = WS_new_session();
  if (!was_launch_OK) {
    game_status = GAME_STATUS_NOT_RUNNING;
    return;
  }

  game_status = GAME_STATUS_RUNNING;
  // reset variables
  step = 0;
  false_suggestions_number = 0;
  progression = 0;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   end this new session
 */
void AkinatorDialog::stop_game() {
  debugInfo("stop_game()");
  WS_cancelling();
  game_status = GAME_STATUS_NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   return the number of tries (questions + errors)
 */
int AkinatorDialog::get_number_of_tries() {
  return step + false_suggestions_number;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   a routine to send an answer
 */
void AkinatorDialog::send_player_answer(int ans_code) {
  if (ans_code == INPUT_TYPE_YES || ans_code == INPUT_TYPE_PROBABLY
      || ans_code == INPUT_TYPE_DO_NOT_KNOW || ans_code
      == INPUT_TYPE_PROBABLY_NOT || ans_code == INPUT_TYPE_NO)
    return WS_answer_something(ans_code);

  if (ans_code == INPUT_TYPE_PEOPLE_YES) {
    game_status = GAME_STATUS_WON_BY_AKINATOR;
    return WS_choice(0);
  }

  if (ans_code == INPUT_TYPE_PEOPLE_NO) {
    false_suggestions_number++;
    return WS_exclusion();
  }

  debugInfo("answer_type = ANSWER_TYPE_ERROR_WRONG_ARGUMENT");
  answer_type = ANSWER_TYPE_ERROR_WRONG_ARGUMENT;
  return;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   a routine to find the next move
 */
void AkinatorDialog::find_next_move() {
  debugInfo("find_next_move()");

  // game not running => do nothing
  if (game_status != GAME_STATUS_RUNNING)
    return;

  // if errors => do nothing
  if (answer_type == ANSWER_TYPE_ERROR_WEBSITE_FAILURE)
    return;

  // if errors => do nothing
  if (answer_type == ANSWER_TYPE_ERROR_WRONG_ARGUMENT)
    return;

  cout << "step:" << step << endl;
  cout << "false_suggestions_number:" << false_suggestions_number << endl;

  if (step + false_suggestions_number >= NUMBER_OF_TRY_MAX) {
    game_status = GAME_STATUS_LOST_BY_AKINATOR;
    return;
  }

  // if debug => display some possibilities
  if (DEBUG && step > 0)
    WS_list();

  // high proba of finding or last chance => answer the first one
  if (progression > THRES_TRY_SUGGESTION || step + false_suggestions_number
      == NUMBER_OF_TRY_MAX - 1) {
    WS_list(1);
    return answer_first_in_list();
  }

  // low proba of finding => answer a new question_from_server
  if (progression < THRES_TRY_SUGGESTION) {
    return answer_question();
  }
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   copy the computed answers into the given args
 *
 * \param   type the type of answer :
 * \param   ans the answer itself, respectively :
 */
void AkinatorDialog::get_answer(int* type, string* ans) {
  debugInfo("get_answer()");

  *type = answer_type;
  *ans = answer;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   fetch the content of an URL and copy it in a string
 *
 * \param   url the url
 * \param   rep a pointer toward the answer
 * \param   verbose_level 1 : not much information, 2:medium, 3:much
 */
void AkinatorDialog::WS_get_url_content(string url, string* rep,
                                        int verbose_level /*=1*/) {
  /* create the file */
  ostringstream order;
  order << "touch " << WEB_FILENAME;
  system(order.str().c_str());

  /* connect to the site using wget and downloaf the rep */
  order.str("");
  order << "wget \"" << url << "\" --output-document=" << WEB_FILENAME;
  //// verbose level
  if (!DEBUG)
    order << " -- quiet"; // NOT MUCH
  else {
    order << (verbose_level == 1 ? " --no-verbose" : ""); // NOT MUCH
    order << (verbose_level == 2 ? " --verbose" : ""); // MEDIUM
    order << (verbose_level == 3 ? " --debug" : ""); // MUCH
  }

  debugInfo(order.str());
  system(order.str().c_str());

  /* copy the results in a stream, removing the spaces */
  ostringstream rep_stream;
  ifstream infile(WEB_FILENAME);
  string line;

  while (infile.good()) {
    std::getline(infile, line);
    string::size_type begin_without_space = line.find_first_not_of(" ");
    //    cout << "line:'" << line << "'" << endl;
    //    cout << "begin_without_space:" << begin_without_space << endl;
    if (begin_without_space != string::npos)
      rep_stream << line.substr(begin_without_space);
  }

  infile.close();

  /* remove the file */
  //~ order.str("");
  //~ order << "rm " << WEB_FILENAME;
  //~ system(order.str().c_str());

  /* copy in the rep */
  *rep = rep_stream.str();
  if (verbose_level > 1)
    debugInfo("Rep:" + *rep);
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   check if there is an error in the line
 */
bool AkinatorDialog::WS_return_code_is_KO(string* line) {
  if (line->length() == 0) {
    answer_type = ANSWER_TYPE_ERROR_BAD_HTTP;
    is_KO = "Bad HTTP";
    debugInfo("!!!!!! Bad HTTP !");
    answer = is_KO;
    return true;
  }

  get_value_from_fieldname_in_string(line, &is_KO, "COMPLETION");
  if (is_KO == "OK")
    return false;
  else {
    if (is_KO == "KO - TECHNICAL ERROR") {
      answer_type = ANSWER_TYPE_ERROR_WEBSITE_FAILURE;
    } else {
      answer_type = ANSWER_TYPE_ERROR_WRONG_ARGUMENT;
    }
    debugInfo("!!!!!! Error !" + is_KO);
    answer = is_KO;
    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   start a new session
 *
 * \return  true if successful
 */
bool AkinatorDialog::WS_new_session() {
  debugInfo("WS_new_session()");

  /* compute elements */
  base = 0;
  partner = 201;

  /* create url */
  ostringstream url;
  url << URL_BASE << "new_session.php?";
  url << "base=" << base;
  url << "&partner=" << partner;
  url << "&player=" << PLAYER_NAME;

  /* send url and get data */
  string url_content;
  WS_get_url_content(url.str(), &url_content, 2); // a bit verbose
  if (WS_return_code_is_KO(&url_content))
    return false;

  /* extract values */
  get_int_value_from_fieldname_in_string(&url_content, &channel, "CHANNEL");
  get_int_value_from_fieldname_in_string(&url_content, &session, "SESSION");
  get_int_value_from_fieldname_in_string(&url_content, &step, "STEP");
  get_value_from_fieldname_in_string(&url_content, &signature, "SIGNATURE");
  get_value_from_fieldname_in_string(&url_content, &question_from_server,
                                     "QUESTION");

  answer_type = ANSWER_TYPE_NEW_QUESTION;
  answer = question_from_server;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   Cancel a non-terminated session
 */
void AkinatorDialog::WS_cancelling() {
  debugInfo("WS_cancelling()");

  /* create url */
  ostringstream url;
  url << URL_BASE << "cancelling.php?";
  url << "channel=" << channel;
  url << "&base=" << base;
  url << "&session=" << session;
  url << "&signature=" << signature;

  /* send url and get data */
  string url_content;
  WS_get_url_content(url.str(), &url_content);
  if (WS_return_code_is_KO(&url_content))
    return;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   answer something to the server
 * \param   rep the answer
 */
void AkinatorDialog::WS_answer_something(int rep) {
  debugInfo("WS_answer_something()", rep);

  /* compute elements */
  int ans_idx = -1;
  (rep == INPUT_TYPE_YES ? ans_idx = 0 : 0);
  (rep == INPUT_TYPE_NO ? ans_idx = 1 : 0);
  (rep == INPUT_TYPE_DO_NOT_KNOW ? ans_idx = 2 : 0);
  (rep == INPUT_TYPE_PROBABLY ? ans_idx = 3 : 0);
  (rep == INPUT_TYPE_PROBABLY_NOT ? ans_idx = 4 : 0);

  /* create url */
  ostringstream url;
  url << URL_BASE << "answer.php?";
  url << "channel=" << channel;
  url << "&base=" << base;
  url << "&session=" << session;
  url << "&signature=" << signature;
  url << "&step=" << step;
  url << "&answer=" << ans_idx;

  /* send url and get data */
  string url_content;
  WS_get_url_content(url.str(), &url_content);
  if (WS_return_code_is_KO(&url_content))
    return;

  /* extract values */
  get_int_value_from_fieldname_in_string(&url_content, &step, "STEP");
  get_double_value_from_fieldname_in_string(&url_content, &progression,
                                            "PROGRESSION");
  get_value_from_fieldname_in_string(&url_content, &question_from_server,
                                     "QUESTION");

  debugInfo("progression", progression);
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   Obtain the WS_list of the most fitted elements
 */
void AkinatorDialog::WS_list(int nb_ans /*= 10*/) {
  debugInfo("WS_list()", nb_ans);
  /* compute elements */

  /* create url */
  ostringstream url;
  url << URL_BASE << "list.php?";
  url << "channel=" << channel;
  url << "&base=" << base;
  url << "&session=" << session;
  url << "&signature=" << signature;
  url << "&step=" << step;
  url << "&size=" << nb_ans;

  /* send url and get data */
  string url_content;
  WS_get_url_content(url.str(), &url_content);
  if (WS_return_code_is_KO(&url_content))
    return;

  /* extract values */
  suggestions.clear();
  int position_in_string = 0;
  for (int people_id = 0; people_id < nb_ans; ++people_id) {
    People p;
    position_in_string = p.parse_line(&url_content, position_in_string);
    if (position_in_string < 0)
      break;
    debugInfo("Pushing back: " + p.toString());
    suggestions.push_back(p);
  }
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   Choice of one of the proposed elements
 * \param   order_in_list the rank of the People in the WS_list
 */
void AkinatorDialog::WS_choice(int order_in_list) {
  debugInfo("WS_choice()", order_in_list);

  /* compute elements */
  int element_id = suggestions.at(order_in_list).id;

  /* create url */
  ostringstream url;
  url << URL_BASE << "choice.php?";
  url << "channel=" << channel;
  url << "&base=" << base;
  url << "&session=" << session;
  url << "&signature=" << signature;
  url << "&element=" << element_id;

  /* send url and get data */
  string url_content;
  WS_get_url_content(url.str(), &url_content);
  if (WS_return_code_is_KO(&url_content))
    return;

  /* extract values */
  get_int_value_from_fieldname_in_string(&url_content, &times_selected,
                                         "TIMES_SELECTED");
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   exclude the proposed elements in order they can not be proposed again
 */
void AkinatorDialog::WS_exclusion() {
  debugInfo("WS_exclusion()");

  /* compute elements */

  /* create url */
  ostringstream url;
  url << URL_BASE << "exclusion.php?";
  url << "channel=" << channel;
  url << "&base=" << base;
  url << "&session=" << session;
  url << "&signature=" << signature;
  url << "&step=" << step;

  /* send url and get data */
  string url_content;
  WS_get_url_content(url.str(), &url_content);
  if (WS_return_code_is_KO(&url_content))
    return;

  /* extract values */
  // No returned information
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   returns as answer the first People of the WS_list
 */
void AkinatorDialog::answer_first_in_list() {
  debugInfo("answer_first_in_list()");

  answer_type = ANSWER_TYPE_SUGGESTION;
  answer = suggestions.front().name;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   returns as answer the question_from_server computed by
 */
void AkinatorDialog::answer_question() {
  debugInfo("answer_question()");

  answer_type = ANSWER_TYPE_NEW_QUESTION;
  answer = question_from_server;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   parse the string to extract a value
 *
 * \param   line a pointer to the line ( a string )
 * \param   rep a pointer to a string, it will contain the answer
 * \param   field the value to extract
 * \param   begin an optionnal index for the beginning of the search
 *
 * \return  the value in string
 */
int AkinatorDialog::get_value_from_fieldname_in_string(string* line,
                                                       string* rep, string field, int begin /*=0*/) {
  string field_begin = "<" + field + ">";
  string field_end = "</" + field + ">";

  string::size_type index_begin = line->find(field_begin, begin);
  if (index_begin == string::npos) {
    cout << "Impossible to find the field " << field_begin << endl;
    return -1;
  }
  index_begin += field_begin.size(); // adding the length of the word + the "<" and ">"

  string::size_type index_end = line->find(field_end, index_begin);
  if (index_end == string::npos) {
    cout << "Impossible to find the field " << field_end << endl;
    return -1;
  }
  index_end--; // we want the end of the word

  *rep = line->substr(index_begin, index_end - index_begin + 1);
  convert_string_encoding_htlm_to_utf(*rep);
  return index_end;

}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   cf get_value_from_fieldname_in_string()
 */
int AkinatorDialog::get_int_value_from_fieldname_in_string(string* line,
                                                           int* rep, string field, int begin /*= 0*/) {
  string rep_string;
  int return_value = get_value_from_fieldname_in_string(line, &rep_string,
                                                        field, begin);
  *rep = atoi(rep_string.c_str());
  return return_value;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   cf get_value_from_fieldname_in_string()
 */
int AkinatorDialog::get_double_value_from_fieldname_in_string(string* line,
                                                              double* rep, string field, int begin /*= 0*/) {
  string rep_string;
  int return_value = get_value_from_fieldname_in_string(line, &rep_string,
                                                        field, begin);
  *rep = strtod(rep_string.c_str(), NULL);
  return return_value;
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   parse a line to get the fields of this object
 *
 * \param   line the string containing the informations
 * \param   begin the beginning index in the line
 * \return  the end position of the parser
 */
int AkinatorDialog::People::parse_line(string* line, int begin /*=0*/) {
  if (AkinatorDialog::get_int_value_from_fieldname_in_string
      (line, &id, "ID", begin) < 0)
    return -1;
  if (AkinatorDialog::get_value_from_fieldname_in_string
      (line, &name, "NAME", begin) < 0)
    return -1;
  if (AkinatorDialog::get_value_from_fieldname_in_string
      (line, &picture_path, "PICTURE_PATH", begin) < 0)
    return -1;
  return AkinatorDialog::get_value_from_fieldname_in_string
      (line, &id_base, "ID_BASE", begin);
}

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief   returns a string representing the information
 */
string AkinatorDialog::People::toString() {
  ostringstream m;
  m << name << " (" << id << " - " << id_base << " - " << picture_path << ")";
  return m.str();
}
