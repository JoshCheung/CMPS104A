// $Id: main.cpp,v 1.1 2017-10-30 01:03:20-07 - - $
#include <string>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <assert.h>

#include "assert.h"
#include "astree.h"
#include "lyutils.h"
#include "auxlib.h"
#include "string_set.h"

const string CPP = "/usr/bin/cpp -nostdinc";
constexpr size_t LINESIZE = 1024;
astree* yyparse_astree;
string cpp_d_option = " ";
FILE* str_outfile;
FILE* tok_outfile;
FILE* ast_outfile;

// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

char* change_suffix(string filename, string suffix){
   string outfile;
   string parse = filename.substr(0, filename.find(".")).c_str();
   return strdup((char*)(parse+suffix).c_str()); 
}

void open_files(string file_in){
   char* str_open = change_suffix(file_in, ".str");
   char* tok_open = change_suffix(file_in, ".tok");
   char* ast_open = change_suffix(file_in, ".ast");
   str_outfile = fopen(str_open, "w");
   tok_outfile = fopen(tok_open, "w");
   ast_outfile = fopen(ast_open, "w");

   if(str_outfile == NULL || tok_outfile == NULL || 
      ast_outfile == NULL) 
   {
      fprintf(stderr, "FILE NOT FOUND");
   }
   free(str_open);
   free(tok_open);
   free(ast_open);

}
/*
// Print the meaning of a signal.
static void eprint_signal (const char* kind, int signal) {
   fprintf (stderr, ", %s %d", kind, signal);
   const char* sigstr = strsignal (signal);
   if (sigstr != NULL) fprintf (stderr, " %s", sigstr);
}

// Print the status returned from a subprocess.
void eprint_status (const char* command, int status) {
   if (status == 0) return; 
      fprintf (stderr, "%s: status 0x%04X", command, status);
   if (WIFEXITED (status)) {
      fprintf (stderr, ", exit %d", WEXITSTATUS (status));
   }
   if (WIFSIGNALED (status)) {
      eprint_signal ("Terminated", WTERMSIG (status));
      #ifdef WCOREDUMP
   if (WCOREDUMP (status)) fprintf (stderr, ", core dumped");
      #endif
   }
   if (WIFSTOPPED (status)) {
      eprint_signal ("Stopped", WSTOPSIG (status));
   }
   if (WIFCONTINUED (status)) {
      fprintf (stderr, ", Continued");
   }
   fprintf (stderr, "\n");
}
*/

void cpplines (FILE* pipe, const char* filename) {
    int linenr = 1;
     char inputname[LINESIZE];
     strcpy (inputname, filename);
     for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
               &linenr, inputname);
      if (sscanf_rc == 2) {
         continue;
      }

      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;

         if (token == NULL) break;
         string_set::intern (token);
        }
         ++linenr;
     }
}


int scan_opt(int argc, char** argv ) {
   int flags;
   yydebug = 0;
   yy_flex_debug = 0;
 
   while ((flags = getopt(argc, argv, "@lyD:")) != -1 ) {
      string d_fl;
      string at_fl;
           if(flags == EOF)
         break;
      switch (flags) {
         case '@' : 
            at_fl = optarg;
            break;
         case 'l' :
            yy_flex_debug = 1; 
            break;
         case 'y' :
            yydebug = 1; 
            break;
         case 'D' : 
            cpp_d_option = "-D" + std::string(optarg);
            break;
         default:  fprintf (
            stderr, 
            "Unknown option`\\%s'.\n",
            optarg
         );
      }
   }
   return optind;
}

void return_token(const char* cmd_Line) {
   yyin = popen(cmd_Line, "r");
   if( yyin == NULL){
      fprintf(stderr, "error running CPP");
      return;
   }
   else {
      yyparse();
      pclose(yyin);
      astree::print(ast_outfile, yyparse_astree, 0);
   }
}



int main (int argc, char** argv) {
   int args = scan_opt(argc, argv);
   const char* execname = basename (argv[0]); 
   exec::execname = basename (argv[0]);  
   int exit_status = EXIT_SUCCESS;

   for (int argi = args; argi < argc; ++argi) {
      char* filename = argv[argi];
      if(!strstr(filename, ".oc")){
         fprintf(
            stderr, 
            "Not an oc file found %s instead\n", 
            filename
         );
      }

      string command = CPP + cpp_d_option + filename;
      FILE* PIPE = popen (command.c_str(), "r");
      
      if (PIPE == NULL) {
         exit_status = EXIT_FAILURE;
         fprintf (stderr, "%s: %s: %s\n",
               execname, command.c_str(), strerror (errno));
      } else {
         string con = argv[argc-1];
         open_files(con);
         cpplines (PIPE, filename);

         int pclose_rc = pclose (PIPE);

         return_token(command.c_str());

         string_set::dump(str_outfile);
         yylex_destroy();

         fclose(str_outfile);
         fclose(tok_outfile);
         fclose(ast_outfile);
         eprint_status (command.c_str(), pclose_rc);
         if (pclose_rc != 0) exit_status = EXIT_FAILURE;
      }
   }
   return exit_status;
}
