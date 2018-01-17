#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <string>
#include <vector>
#include <bitset>
using namespace std;

#include "auxlib.h"
#include "lyutils.h"
#include "astree.h"


extern int next_block;
//extern size_t block_level;
using symbol_entry = pair<const string*,symbol*>;

struct symbol {
    attr_bitset attr; 
    symbol_table* fields; 
    size_t filenr; 
    size_t linenr; 
    size_t offset; 
    size_t block_nr; 
    vector<symbol*>* parameters; 
};


void recursive_check (astree* root);
void traversing_tree (astree* root);
#endif
