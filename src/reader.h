/*

  Source code reader for the Micro Pascal language
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#ifndef reader_h
#define reader_h

#include <stdint.h>
#include <vector>

/** The reader object reads a source file into memory and provides
    an interface to the characters that supports roll-backs.
*/

class Reader
{
public:
  virtual ~Reader();

  struct position_info
  {
    size_t  offset;     // offset into m_source
    size_t  line;       // the line number
    size_t  pos;        // the position within the line
  };

  /** Create a reader object using source code in a QString
      NULL is returned when an error occured. */
  static Reader* create(const char *filename);

  /** Get the charater at the current read position.
        The read position is not advanced.
        When there are no characters to read,
        it returns 0.
    */
  char peek();

  /** Read the character at the current read position.
      The read position is advanced one character.
      When there are no characters to read,
      it returns 0.
    */
  char accept();

  /** Get the current read position */
  position_info getPos() const
  {
    return m_curpos;
  }

protected:
  /** Hide the constructor so the user can only get
      a Reader object by using 'create'.
    */
  Reader();

  std::vector<char>   m_source;               // the source code
  position_info   m_curpos;                   // the current read position.
};

#endif
