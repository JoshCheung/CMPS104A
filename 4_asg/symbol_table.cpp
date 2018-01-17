#include <string>
#include <unordered_map>
#include <vector>
#include <bitset>

#include "lyutils.h"
#include "astree.h"
#include <string.h>
#include "symbol_table.h"
#include "lyutils.h"
#include "astree.h"

int next_block = 1;
FILE* sym_outfile;

vector<symbol_table*> symbol_stack;
vector <int> blocks;

symbol* new_symbol (astree* node) {
   if (symbol_stack.empty() ){
      symbol_stack.push_back (new symbol_table);
      symbol_stack.push_back (nullptr);
   }
   if (blocks.empty()) {
      blocks.push_back (0);
   }

   symbol* n_symbol = new symbol();
   n_symbol->attr = node->attr;
   n_symbol->fields = nullptr;
   n_symbol->filenr = node->lloc.filenr;
   n_symbol->linenr = node->lloc.linenr;
   n_symbol->offset = node->lloc.offset;
   n_symbol->block_nr = blocks.back();
   n_symbol->parameters = nullptr;
   return n_symbol; 
}

const char* get_attr(astree* node) {

   string str = "";
   attr_bitset attr = node->attr;

   if (attr[ATTR_void] == 1 ){ 
      str += "void ";     
   }
   if (attr[ATTR_int] == 1){ 
      str += "int " ;     
   }
   if (attr[ATTR_null] == 1) { 
      str += "null ";     
   }
   if (attr[ATTR_string] == 1) { 
      str += "string ";   
   }
   if (attr[ATTR_struct] == 1) { 
      str += "struct ";   
   }
   if (attr[ATTR_array] == 1) { 
      str += "array ";    
   }
   if (attr[ATTR_function] == 1) { 
      str += "function "; 
   }
   if (attr[ATTR_variable] == 1) { 
      str += "variable "; 
   }
   if (attr[ATTR_field] == 1) { 
      str += "field ";    
   }
   if (attr[ATTR_typeid] == 1) { 
      str += "typeid ";   
   }
   if (attr[ATTR_param] == 1) { 
      str += "param ";    
   }  
   if (attr[ATTR_lval] == 1) { 
      str += "lval ";     
   }
   if (attr[ATTR_const] == 1) { 
      str += "const ";    
   }
   if (attr[ATTR_vreg] == 1) { 
      str += "vreg ";     
   }
   if (attr[ATTR_vaddr] == 1) { 
      str += "vaddr ";    
   }
   //fprintf(sym_outfile, "%s ", str.c_str());

   return strdup(str.c_str());
}

void check_type(astree* node) {
   switch(node-> symbol) {
      case TOK_VOID:
            node->attr[ATTR_void] = true;
            if (node->children.size() > 0) {
               node->children[0]->attr[ATTR_void] = true;
            }
            break;

      case TOK_INT:
            node->attr[ATTR_int] = true;
            node->attr[ATTR_lval] = true;
            if (node->children.size() > 0) {
               node->children[0]->attr[ATTR_int] = true;
            }
            break;

      case TOK_NULL:
            node->attr[ATTR_null] = true;
            node->attr[ATTR_const] = true;
            break;

      case TOK_STRING:
            node->attr[ATTR_string] = true;
            node->attr[ATTR_lval] = true;
            if (node->children.size() > 0) {
               node->children[0]->attr[ATTR_string] = true;
            }
            break;

      case TOK_STRUCT:
            node->attr[ATTR_struct] = true;
            node->attr[ATTR_typeid] = true;
            node->children[0]->attr[ATTR_struct] = true;

            node->attr[ATTR_variable] = false;
            node->attr[ATTR_lval] = false;

            node->children[0]->attr[ATTR_variable] = false;
            node->children[0]->attr[ATTR_lval] = false;
            break;

      case TOK_ARRAY:
            node->attr[ATTR_array] = true;
            break;

      case TOK_FUNCTION:
            node->attr[ATTR_function] = true;
            node->children[0]->children[0]->attr[ATTR_function] = true;

            node->attr[ATTR_variable] = false;
            node->attr[ATTR_lval] = false;   

            node->children[0]->children[0]->attr[ATTR_variable] = false;
            node->children[0]->children[0]->attr[ATTR_lval] = false;
            break;

      case TOK_PROTOTYPE:
            node->children[0]->children[0]->attr[ATTR_variable] = false;
            node->children[0]->children[0]->attr[ATTR_lval] = false;
            break;

      case TOK_PARAMLIST:
            for (unsigned int i = 0; i < node->children.size(); ++i) {
               node->children[i]->children[0]->attr[ATTR_param] = true;
            }
            break;

      case TOK_STRINGCON:
            node->attr[ATTR_string] = true;
            node->attr[ATTR_const] = true;
            node->attr[ATTR_lval] = true;
            break;

      case TOK_CHARCON:
            node->attr[ATTR_const] = true;
            node->attr[ATTR_lval] = true;
            break;
 
      case TOK_INTCON:
            node->attr[ATTR_int] = true;
            node->attr[ATTR_const] = true;
            node->attr[ATTR_lval] = true;
            break;
  
      case TOK_FIELD:
            node->attr[ATTR_field] = true;
            node->attr[ATTR_struct] = true;
            break;

      case TOK_TYPEID:
            node->attr[ATTR_typeid] = true;
            for(unsigned int i = 0; i < node->children.size(); ++i){
               node->children[i]->attr[ATTR_typeid] = true;
               node->children[i]->attr[ATTR_struct] = true;
               break;
            }

      case TOK_IDENT:
            node->attr[ATTR_lval] = true;
            node->attr[ATTR_variable] = true;
            break;

      case TOK_VARDECL:
            node->attr[ATTR_variable] = true;
            break;
      
      case TOK_DECLID:
            node->attr[ATTR_lval] = true;
            node->attr[ATTR_variable] = true;
            break;

      default:
            break;
   }
}

 

void insert_sym (symbol_table* table, astree* node) {
   symbol* sym = new_symbol(node);
   //symbol_stack.push_back(table);
   if (node != NULL && table != NULL) {
      //table->insert (symbol_entry ((string*) node->lexinfo, sym));
      table->insert (make_pair ((string*)node->lexinfo, sym));
   }
   for (unsigned int i = 1; i < blocks.size(); ++i) {
      fprintf (sym_outfile, "   ");
   }

   fprintf (sym_outfile, "%s (%zu.%zu.%zu) {%zu} %s\n",
            ((const string*) node->lexinfo)->c_str(),
            node->lloc.filenr, 
            node->lloc.linenr, 
            node->lloc.offset, 
            node->block_nr,
            get_attr(node));
   //fprintf (sym_outfile, "node->lexinfo %s\n", 
   //       ((const string*) node->lexinfo)->c_str());
//fprintf (sym_outfile, "node->lloc.filenr: %lu\n", node->lloc.filenr);
//fprintf (sym_outfile, "node->lloc.linenr: %lu\n", node->lloc.linenr);
//fprintf (sym_outfile, "node->lloc.offset: %lu\n", node->lloc.offset);
   //fprintf (sym_outfile, "node->block_nr: %lu\n", node->block_nr);
   //fprintf (sym_outfile, "get_attr(node)): %s\n", get_attr(node));
            
}

void count_block (astree* root) {
   blocks.push_back (next_block);
   next_block++;
   symbol_stack[blocks.back()] = new symbol_table;
   symbol_stack.push_back (nullptr);
   traversing_tree (root);
   blocks.pop_back();
}

void struct_symbol (astree* root) {
   symbol* sym = new_symbol (root->children[0]);
   sym->fields = new symbol_table;
   //sym->fields = symbol_table* fields;
   insert_sym (symbol_stack[0], root->children[0]);
   for (unsigned int i = 1; i < root->children.size(); ++i) {
      if(root->children[i]->children[0] == NULL) {
         break;
      }

      astree* field = root->children[i]->children[0];
      fprintf(sym_outfile, "  ");
      insert_sym (sym->fields, field);
   }
}

void function_symbol (astree* root) {
   astree* funct = root->children[0]->children[0];
   symbol* sym = new_symbol (funct);
   vector <symbol*> params;
   sym->parameters = &params;
   insert_sym (symbol_stack[0], funct);
   astree* param_list = root->children[1];

   for (unsigned int i = 0; i < param_list->children.size(); ++i) {
      astree* param = param_list->children[i]->children[0];
      symbol* sym = new_symbol (param);
      ++sym->block_nr;
      params.push_back (sym);
      fprintf (sym_outfile, "    ");      
      insert_sym (symbol_stack[0], param);
   }
   count_block (root->children[2]);
}

void prototype_symbol (astree* root) {
   astree* prototype = root->children[0]->children[0];
   symbol* sym = new_symbol (prototype);
   //symbol_stack.push_back(sym);
   vector <symbol*> params;
   sym->parameters = &params;
   insert_sym (symbol_stack[0], prototype); 
   astree* param_ls = root->children[1];
   for (unsigned int i = 0; i < param_ls->children.size(); ++i) {
      astree* param = param_ls->children[i]->children[0];
      sym = new_symbol (param);
      ++sym->block_nr;
      params.push_back(sym);
   }
}

//void block_symbol (astree* root) {
//   astree* block = root->chidlren

void vardecl_symbol (astree* root) {
   astree* var = root->children[0]->children[0];
   //symbol* sym = new_symbol (var);
   insert_sym (symbol_stack[blocks.back()], var);
}

void recursive_check (astree* root) {
   //printf("root: %s\n", (string*) root->lexinfo->c_str());
//printf("children: %s\n", (string*)root->children[0]->lexinfo->c_str());
   
   astree* new_root = root;
   if (new_root == NULL){
      return;
   }
   unsigned int i = 0;
   while (i < root->children.size()) {
      //(unsigned int i = 0; i < root->children.size(); ++i) {
      recursive_check (root->children[i]); 
      ++i;
   }
   check_type(root);
}

void traversing_tree(astree* n_root) {
   astree* root = n_root;
   if (root == NULL){
       return;
   }

   for (unsigned int i = 0; i < root->children.size(); ++i) {
      int sym = root->children[i]->symbol;
      switch(sym) {
         case TOK_STRUCT:
            struct_symbol (root->children[i]);
            fprintf (sym_outfile, "\n");
            break;

         case TOK_PROTOTYPE:
            prototype_symbol (root->children[i]);
            fprintf (sym_outfile, "\n");
            break;

         case TOK_FUNCTION:
            function_symbol (root->children[i]);
            fprintf (sym_outfile, "\n");
            break;
         
         case TOK_VARDECL:
            vardecl_symbol (root->children[i]);
            //fprintf (sym_outfile, "\n");
            break;
         case TOK_BLOCK:
           // block_symbol (root->children [i]);
            break;
         
         case TOK_IF:
            count_block (root->children[i]->children[1]);
            break;

         case TOK_IFELSE:
            count_block (root->children[i]->children[1]);
            count_block (root->children[i]->children[2]);
            break;
 
         default:
          
            break;
      }
   }
}

