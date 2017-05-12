/*

  Micro Pascal
  Niels A. Moseley
  Moseley Instruments
  www.moseleyinstruments.com

  Licence: GPL v2.0

*/

//#include "functiondefs.h"
#include <iostream>
#include "reader.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    Reader *reader = Reader::create("tests\\tokentest.pas");
    Tokenizer tokenizer;

    std::vector<token_t> tokens;

    tokenizer.process(reader, tokens);

    for(uint32_t i=0; i<tokens.size(); i++)
    {
        std::cout << tokens[i].tokID << " = " << tokens[i].txt << "\n";
    }

    delete reader;
    return 0;
}
