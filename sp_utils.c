/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 Copyright (C) 2011 Alexey Veretennikov (alexey dot veretennikov at gmail.com)
 
 This file is part of libspmatrix.

 libspmatrix is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 libspmatrix is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with libspmatrix.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sp_utils.h"


static int is_from(const char c, const char* from)
{
  if (from && c)
  {
    while (*from)
    {
      if (c == *from)
        return 1;
      from++;
    }
  }
  return 0;
}

/*
 * Case-insensitive ASCII 7bit string comparison
 */
int sp_istrcmp ( const char * str1, const char * str2 )
{
  int result = 0;
  while (*str1 && *str2 )
  {
    result = toupper(*str1) - toupper (*str2);
    if (result)
      return result;
    str1++,str2++;
  } 
  return toupper(*str1) - toupper (*str2);
}

/*
 * Returns the copy of n characters of the string
 * Caller shall free the returned string
 */
char* sp_strndup(const char *s, size_t n)
{
  char* str = malloc(n+1);
  memcpy(str,s,n);
  str[n] = '\0';
  return str;
}



/*
 * Extract file extension from the filename
 */
const char* sp_parse_file_extension(const char* filename)
{
  const char* ptr;
  int len = strlen(filename);
  ptr = filename + len;
  while (len)
  {
    ptr--,len--;
    if ( *ptr == '.' )
      break;
  }
  return len ? ptr + 1 : 0;
}

/*
 * Skip whitespaces
 */
const char* sp_skip_whitespaces(const char* line)
{
  while (*line && (*line == ' ' || *line == '\t'))
    line++;
  return line;
}

/*
 * Skip alphanumeric characters and characters from chars array
 */
const char* sp_skip_alnum(const char* line, const char* chars)
{
  while (*line && (isalnum(*line) || is_from(*line,chars)))
    line++;
  return line;
}

/*
 * Extract word from the string
 * return pointer to the word into the word argument
 * Caller shall free the word
 */
const char* sp_extract_next_word(const char* line, const char** word)
{
  const char* end;
  /* skip whitespaces to the object type */
  line = sp_skip_whitespaces(line);
  /* extract word */
  end = sp_skip_alnum(line,"-");
  *word = sp_strndup(line,end - line);
  return end;
}


/*
 * Read the text file and return buffer with the file's contens
 * returns 0 if unable to read
 */
char* sp_read_text_file(const char* filename)
{
  FILE* file;
  const int block_size = 1024;
  /* contents buffer */
  char* contents = calloc(block_size+1,1);
  /* auxulary counters */
  int read_chunk = 0,read = 0;

  file = fopen(filename,"rt");
  if (!file)
  {
    fprintf(stderr,"Cannot read file %s\n", filename);
    free(contents);
    return 0;
  }
  /* read file contents  */
  while(!feof(file))
  {
    read_chunk = fread(contents+read,1,block_size, file);
    read += read_chunk;
    
    contents = (char*)realloc(contents,read + block_size);
  }
  contents[read] = '\0';
  /* close the file */
  fclose(file);
  return contents;
}

/* Extracts the integer in size bytes of the buffer from */
int sp_extract_positional_int(const char* from, int size)
{
  int result;
  char* buf = malloc(size+1);
  char* ptr = buf;
  int i = 0;
  while (i < size && *from)
  {
    *ptr++ = *from++;
    ++i;
  }
  buf[size] = '\0';
  result = atoi(buf);
  free(buf);
  return result;
}


/*
 * Very simple Parser for FORTRAN IO Format specifiers
 * Fortran IO format grammar BNF specification:
 * <format> --> \(<format-string>\)
 * <format-string> --> <count>?<rest>
 * <count> --> \d+
 * <rest> --> <fixedid>|<intid>|<fltid>|<doubleid>|<generalid>
 * <fixedid> --> F<field-width>\.<digits-after-decimal-point>
 * <intid> --> I<field-width>(\.<min-num-digits>)?
 * <fltid> --> E<field-width>\.<decimal-significand-length>(E<num-digits-in-exponent>)?
 * <doubleid> --> D<field-width>\.<decimal-significand-length>(E<num-digits-in-exponent>)?
 * <generalid> --> G<field-width>\.<decimal-significand-length>(E<num-digits-in-exponent>)?
 * <field-width> --> \d+
 * <min-num-digits> --> \d+  (defaults to zero)
 * <num-digits-in-exponent> --> \d+
 * <decimal-significand-length> --> \d+
 * <digits-after-decimal-point> --> \d+
 */
int sp_parse_fortran_format(const char* string, fortran_io_format* format)
{
  const char* ptr = sp_skip_whitespaces(string);
  const char* end;
  const char* ptr1;
  memset(format,0,sizeof(fortran_io_format));
  /* <format> --> \(<format-string>\) */
  /* find start \( */
  if (!*ptr || *ptr != '(')
  {
    fprintf(stderr, "Incorrect FORTRAN IO format: %s, no '('\n",string);
    return 0;
  }
  /* find end \) */
  end = ptr;
  while(*end && *end != ')') ++end;
  if (!*end  || *end != ')')
  {
    fprintf(stderr, "Incorrect FORTRAN IO format: %s, no ')'\n",string);
    return 0;
  }
  ptr++;
  /* <format-string> --> <count>?<rest> */
  ptr1 = ptr;
  /* <count> --> \d+ */
  while (isdigit(*ptr1) && ptr1 != end) ++ptr1;
  format->count = ptr1 > ptr ? sp_extract_positional_int(ptr,ptr1-ptr) : 0;
  /* <rest> --> <fixedid>|<intid>|<fltid>|<doubleid>|<generalid> */
  ptr = ptr1;
  if (!is_from(*ptr1,"FIEDG"))
  {
    fprintf(stderr, "Incorrect FORTRAN IO format: %s, %c != [FIEDG]\n",
            string,*ptr1);
    return 0;
  }
  format->type = *ptr;
  ptr1 = ++ptr;
  /* [FIEDG]\d+ */
  while(isdigit(*ptr1) && ptr1 != end) ++ptr1;
  if (ptr1 == ptr)
  {
    fprintf(stderr, "Incorrect FORTRAN IO format: %s, field-width = ''\n",
            string);
    return 0;
  }
  format->width = sp_extract_positional_int(ptr,ptr1-ptr);
  
  /* <intid> --> I<field-width>(\.<min-num-digits>)? */
  if (format->type == 'I')
  {
    if (ptr1 == end)
      return 1;
    if (*ptr1 != '.')
    {
      fprintf(stderr,"Incorrect FORTRAN IO format: %s, type = 'I'\n",
              string);
      return 0;
    }
    ptr1++;
    ptr = ptr1;
    while(isdigit(*ptr1) && ptr1 != end) ++ptr1;
    if (ptr1 == ptr)
    {
      fprintf(stderr, "Incorrect FORTRAN IO format: %s, type = 'I'\n",
              string);
      return 0;
    }
    format->num1 = sp_extract_positional_int(ptr,ptr1-ptr);
  }
  /* <fixedid> --> F<field-width>\.<digits-after-decimal-point> */
  else if (format->type == 'F')
  {
    if (*ptr1 != '.')
    {
      fprintf(stderr,"Incorrect FORTRAN IO format: %s, type = 'F'\n",
              string);
      return 0;
    }
    ptr1++;
    ptr = ptr1;
    while(isdigit(*ptr1) && ptr1 != end) ++ptr1;
    if (ptr1 == ptr)
    {
      fprintf(stderr, "Incorrect FORTRAN IO format: %s, type = 'F'\n",
              string);
      return 0;
    }
    format->num1 = sp_extract_positional_int(ptr,ptr1-ptr);
  }
  else                          /* E,D,G */
  {
    if (*ptr1 != '.')
    {
      fprintf(stderr,"Incorrect FORTRAN IO format: %s, type = '%c'\n",
              string,format->type);
      return 0;
    }
    ptr1++;
    ptr = ptr1;
    while(isdigit(*ptr1) && ptr1 != end) ++ptr1;
    if (ptr1 == ptr)
    {
      fprintf(stderr, "Incorrect FORTRAN IO format: %s, type = '%c'\n",
              string,format->type);
      return 0;
    }
    format->num1 = sp_extract_positional_int(ptr,ptr1-ptr);
    /* (E<num-digits-in-exponent>)? */
    if (ptr1 != end)
    {
      if (*ptr1 != 'E')
      {
        fprintf(stderr, "Incorrect FORTRAN IO format: %s, type = '%c'\n",
                string,format->type);
        return 0;
      }
      ptr1++;
      ptr = ptr1;
      while(isdigit(*ptr1) && ptr1 != end) ++ptr1;
      if (ptr1 == ptr)
      {
        fprintf(stderr, "Incorrect FORTRAN IO format: %s, type = '%c'\n",
                string,format->type);
        return 0;
      }
      format->num2 = sp_extract_positional_int(ptr,ptr1-ptr);
    }
  }
  if ( ptr1 != end)
  {
    fprintf(stderr, "Incorrect FORTRAN IO format: %s, type = '%c'\n",
            string,format->type);
    return 0;
  }
  return 1;
}
