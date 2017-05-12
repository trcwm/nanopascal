/*

  Source code reader for the Micro Pascal language
  Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

*/

#include <stdio.h>
#include "reader.h"

Reader::Reader()
{
    m_curpos.line = 0;
    m_curpos.offset = 0;
    m_curpos.pos = 0;
}

Reader::~Reader()
{
}

Reader* Reader::create(const char *filename)
{
    Reader *reader = new Reader();

    FILE *fin = fopen(filename, "rt");
    if (fin == 0)
    {
        return NULL;
    }

    fseek(fin, 0, SEEK_END);
    size_t bytes = ftell(fin);
    rewind(fin);

    reader->m_source.resize(bytes);
    fread(&(reader->m_source[0]), 1, bytes, fin);
    fclose(fin);

    return reader;
}

char Reader::peek()
{
    if (m_curpos.offset < m_source.size())
    {
        return m_source[m_curpos.offset];
    }
    else
        return 0;
}

char Reader::accept()
{
    char c = peek();
    if (c != 0)
    {
        // read succesful!
        m_curpos.offset++;
        m_curpos.pos++;
        if (c == 10)
        {
            m_curpos.line++;
            m_curpos.pos = 0;
        }
        return c;
    }
    return 0;
}
