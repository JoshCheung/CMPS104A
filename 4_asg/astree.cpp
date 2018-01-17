// $Id: astree.cpp,v 1.1 2017-10-30 01:03:20-07 - - $

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"
#include "symbol_table.h"


astree::astree (int symbol_, const location& lloc_, const char* info) {
   //astree* tree = new astree(symbol_, lloc_, info);
   
   symbol = symbol_;
   block_nr = 0;
   attr = 0;
   lloc = lloc_;
   lexinfo = string_set::intern (info);
   // vector defaults to empty -- no children
}

/*astree* astree (int symbol_, int filenr, int linenr,
                int offset, const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol_;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = string_set::intern (lexinfo);
   tree->attr  = 0;
   tree->block_nr = 0;
   return tree;
}

astree* function_def ( astree* first, astree* 
*/



astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
   symbol = symbol_;
   return adopt (child);
}
astree* astree::swap (int symbol) {
   symbol = symbol; 
   return this; 
}

void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
            this, parser::get_tname (symbol),
            lloc.filenr, lloc.linenr, lloc.offset,
            lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (nullptr);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   if (tree) {
      if (depth != 0) {
         for (int i = 0; i < depth; ++i) { 
            fprintf (outfile, "|  "); 
         }
      }
      char* remtok; 
      if (strstr((char*)parser::get_tname(tree->symbol),"TOK") != NULL){
         remtok = (char*) parser::get_tname (tree->symbol) + 4;   
      }
      else { remtok = (char*) parser::get_tname (tree->symbol); }
      fprintf (outfile, "%s \"%s\" %zd.%zd.%zd\n",
               remtok, tree->lexinfo->c_str(),
               tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset);
      for (astree* child: tree->children) {
         astree::print (outfile, child, depth + 1);
      } 
   }
}

void destroy (astree* tree1, astree* tree2) {
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr),
              lloc.linenr,
              lloc.offset,
              buffer);
}
