// $Id: main.cpp,v 1.2 2016-08-18 15:13:48-07 - - $

#include <string>
using namespace std;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_set.h"

const string CPP = "/usr/bin/cpp -D__OCLIB_OH__";

/*
void check_options(){

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
    	printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
    	// http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
    	int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
				    &linenr, inputname);
    	if (sscanf_rc == 2) {
    	  printf ("DIRECTIVE: line %d file \"%s\"\n", linenr, inputname);
    	  continue;
    	}
    	char* savepos = NULL;
    	char* bufptr = buffer;
    	for (int tokenct = 1;; ++tokenct) {
    	    char* token = strtok_r (bufptr, " \t\n", &savepos);
    	    bufptr = NULL;
    	    if (token == NULL) break;
    	    printf ("token %d.%d: [%s]\n",
	    	linenr, tokenct, token);
  		}
   		++linenr;
        const string* str = string_set::intern (argv[i]);
    	printf ("intern (\"%s\") returned %p->\"%s\"\n",
	    	argv[i], str, str->c_str());	
  	}
}

int main (int argc, char** argv) {
	/*
  	for (int i = 1; i < argc; ++i) {
    	const string* str = string_set::intern (argv[i]);
    	printf ("intern (\"%s\") returned %p->\"%s\"\n",
	    	argv[i], str, str->c_str());
  	}
  	*/
  	string_set::dump (stdout);
  	return EXIT_SUCCESS;
  	const char* execname = basename (argv[0]);
  	int exit_status = EXIT_SUCCESS;
  	for (int argi = 1; argi < argc; ++argi) {
  	  	char* filename = argv[argi];
  	  	string command = CPP + " " + filename;
  	  	printf ("command=\"%s\"\n", command.c_str());
  	  	FILE* pipe = popen (command.c_str(), "r");
  	  	if (pipe == NULL) {
  	  	  exit_status = EXIT_FAILURE;
  	  	  fprintf (stderr, "%s: %s: %s\n",
			       execname, command.c_str(), strerror (errno));
  	  	}else {
  	  	  cpplines (pipe, filename);
  	  	  int pclose_rc = pclose (pipe);
  	  	  eprint_status (command.c_str(), pclose_rc);
  	  	  if (pclose_rc != 0) exit_status = EXIT_FAILURE;
  	  	}
  	}
  	return exit_status;
}
