/*
Luiji's Tiny BASIC Compiler
Copyright (C) 2012 Entertaining Software, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#ifdef __GNUC__
# define PRINTF(f, va) __attribute__ ((__format__ (__printf__, f, va)))
#else
# define PRINTF(f, va) /* unsupported */
#endif

#if defined (__STDC__) || defined (_AIX) \
    || (defined (__mips) && defined (_SYSTYPE_SVR4)) \
    || defined (WIN32) || defined (__cplusplus)
# define PARAMS(protos) protos
#else
# define PARAMS(protos) (/* unsupported */)
#endif

enum
{
  T_PRINT,
  T_IF,
  T_GOTO,
  T_INPUT,
  T_LET,
  T_GOSUB,
  T_RETURN,
  T_REM,
  T_RUN,
  T_END
};

enum
{
  O_EQUAL,
  O_NOT_EQUAL,
  O_LESS,
  O_MORE,
  O_LESS_OR_EQUAL,
  O_MORE_OR_EQUAL
};

static char        look          = 0;
static int         line          = 1;
static int         use_print_i   = 0;
static int         use_print_s   = 0;
static int         use_print_t   = 0;
static int         use_print_n   = 0;
static int         use_input_i   = 0;
static int         vars_used[26] = { 0 };
static char **     strings       = NULL;
static int         string_count  = 0;
static const char *program_name  = "<?>";
static const char *input_name    = NULL;
static const char *output_name   = NULL;

static void  shutdown      PARAMS ((void));
static void *xmalloc       PARAMS ((size_t size));
static void *xrealloc      PARAMS ((void *original, size_t size));
static void  xfree         PARAMS ((void *pointer));
static void  parse_opts    PARAMS ((int argc, char **argv));
static void  print_help    PARAMS ((void));
static void  print_version PARAMS ((void));
static void  begin         PARAMS ((void));
static void  finish        PARAMS ((void));
static void  error         PARAMS ((const char *format, ...)) PRINTF (1, 2);
static void  eat_blanks    PARAMS ((void));
static void  match         PARAMS ((int what));
static void  get_char      PARAMS ((void));
static char  get_name      PARAMS ((void));
static int   get_num       PARAMS ((void));
static int   get_keyword   PARAMS ((void));
static int   do_line       PARAMS ((void));
static void  do_statement  PARAMS ((void));
static void  do_print      PARAMS ((void));
static void  do_print_item PARAMS ((void));
static void  do_if         PARAMS ((void));
static void  do_goto       PARAMS ((void));
static void  do_input      PARAMS ((void));
static void  do_let        PARAMS ((void));
static void  do_gosub      PARAMS ((void));
static void  do_return     PARAMS ((void));
static void  do_rem        PARAMS ((void));
static void  do_end        PARAMS ((void));
static void  do_expression PARAMS ((void));
static void  do_term       PARAMS ((void));
static void  do_factor     PARAMS ((void));
static void  do_string     PARAMS ((void));

int
main (argc, argv)
     int argc;
     char **argv;
{
  atexit (shutdown);

  parse_opts (argc, argv);

  begin ();
  while (do_line ());
  finish ();

  return EXIT_SUCCESS;
}

static void
shutdown ()
{
  int i;

  for (i = 0; i < string_count; ++i)
    xfree (strings[i]);

  xfree (strings);
}

static void *
xmalloc (size)
     size_t size;
{
  void *data = malloc (size);
  if (!data)
    {
      fprintf (stderr, "%s: failed to allocate memory\n", program_name);
      exit (EXIT_FAILURE);
    }
  return data;
}

static void *
xrealloc (original, size)
     void *original;
     size_t size;
{
  void *data = realloc (original, size);
  if (!data)
    {
      fprintf (stderr, "%s: failed to allocate memory\n", program_name);
      exit (EXIT_FAILURE);
    }
  return data;
}

static void
xfree (pointer)
     void *pointer;
{
  if (pointer)
    free (pointer);
}

static void
parse_opts (argc, argv)
     int argc;
     char **argv;
{
  extern char *optarg;
  extern int optind;
  int getopt PARAMS ((int argc, char *const *argv, const char *optstring));

  int was_error = 0, opt;

  program_name = strrchr (argv[0], '/');
  if (program_name)
    ++program_name;
  else
    program_name = argv[0];

  while ((opt = getopt (argc, argv, "hVo:")) != -1)
    switch (opt)
      {
      case 'h':
	print_help ();
	exit (EXIT_SUCCESS);
      case 'V':
	print_version ();
	exit (EXIT_SUCCESS);
      case 'o':
	output_name = optarg;
	break;
      case '?':
	was_error = 1;
	break;
      default:
	assert (0);
	break;
      }

  if (was_error)
    exit (EXIT_FAILURE);

  if (optind < argc)
    input_name = argv[optind];

  for (++optind; optind < argc; ++optind)
    fprintf (stderr, "%s: ignoring argument '%s'\n", program_name,
	     argv[optind]);

  if (input_name)
    {
      if (!freopen (input_name, "r", stdin))
	{
	  fprintf (stderr, "%s: failed to open for reading\n", input_name);
	  exit (EXIT_FAILURE);
	}
    }
  else
    {
      input_name = "<stdin>";
    }

  if (output_name)
    {
      if (!freopen (output_name, "w", stdout))
	{
	  fprintf (stderr, "%s: failed to open for writing\n", output_name);
	  exit (EXIT_FAILURE);
	}
    }
  else
    {
      output_name = "<stdout>";
    }
}

static void
print_help ()
{
  printf ("Usage: %s [-o OUTPUT] [INPUT]\n", program_name);
  puts ("");
  puts ("Compiles Tiny BASIC programs into Netwide Assembler programs.");
  puts ("");
  puts ("Options:");
  puts ("  -o  set output file name");
  puts ("  -h  print this help message and exit");
  puts ("  -V  print this version message adn exit");
  puts ("");
  puts ("Input and output defaults to stdin and stdout.");
  puts ("");
  puts ("Report bugs to: luiji@users.sourceforge.net");
  puts ("tbc home page: <https://github.com/Luiji/TinyBASIC>");
}

static void
print_version ()
{
  puts ("Luiji's Tiny BASIC Compiler 0.0.0");
  puts ("Copyright (C) 2012 Entertaining Software, Inc.");
  puts ("License GPLv3+: GNU GPL version 3 or later \
<http://gnu.org/licenses/gpl.html>");
  puts ("This is free software: you are free to change and redistribute it.");
  puts ("There is NO WARRANTY, to the extent permitted by law.");
}

static void
begin ()
{
  puts ("; Generated by Luiji's Tiny BASIC Compiler");

  puts ("");
  puts ("section .text");
  puts ("");
  puts ("\torg 0x100");

  get_char ();
}

static void
finish ()
{
  int i;

  puts ("");
  puts ("\t; exit to operating system");
  puts ("\t; program falls through to here when there is no explicit end");
  puts ("exit:");
  puts ("\tmov ax, 0x4c00");
  puts ("\tint 0x21");
  puts ("");

  if (use_print_i)
    {
      puts ("\t; print an integer to the terminal");
      puts ("print_i:");
      puts ("\t; number of digits pushed and divisor for ax");
      puts ("\tmov cx, 0");
      puts ("\tmov bx, 10");
      puts ("\t; print out a '-' if the number is negative");
      puts ("\tcmp ax, 0");
      puts ("\tjge print_i_loop_1");
      puts ("\tpush ax");
      puts ("\tmov ah, 0x2");
      puts ("\tmov dl, 0x2d");
      puts ("\tint 0x21");
      puts ("\tpop ax");
      puts ("\tneg ax");
      puts ("\t; push the asciified digits onto the stack");
      puts ("\t; (direct output would be backwards)");
      puts ("print_i_loop_1:");
      puts ("\txor dx, dx");
      puts ("\tdiv bx");
      puts ("\tadd dl, 0x30");
      puts ("\tpush dx");
      puts ("\tinc cx");
      puts ("\tcmp ax, 0");
      puts ("\tjnz print_i_loop_1");
      puts ("\t; output the digits in the correct order");
      puts ("print_i_loop_2:");
      puts ("\tmov ah, 2");
      puts ("\tpop dx");
      puts ("\tint 21h");
      puts ("\tloop print_i_loop_2");
      puts ("\tret");
    }
  else
    {
      puts ("; print_i excluded");
    }
  puts ("");

  if (use_print_s)
    {
      puts ("\t; print string to terminal");
      puts ("print_s:");
      puts ("\tmov dx, ax");
      puts ("\tmov ah, 0x9");
      puts ("\tint 0x21");
      puts ("\tret");
    }
  else
    {
      puts ("; print_s excluded");
    }
  puts ("");

  if (use_print_t)
    {
      puts ("\t; print a tab to the terminal");
      puts ("print_t:");
      puts ("\tmov ah, 0x2");
      puts ("\tmov dl, 0x9");
      puts ("\tint 0x21");
      puts ("\tret");
    }
  else
    {
      puts ("; print_t excluded");
    }
  puts ("");

  if (use_print_n)
    {
      puts ("\t; print a newline to the terminal");
      puts ("print_n:");
      puts ("\tmov ah, 0x2");
      puts ("\tmov dl, 13");
      puts ("\tint 0x21");
      puts ("\tmov ah, 0x2");
      puts ("\tmov dl, 10");
      puts ("\tint 0x21");
      puts ("\tret");
    }
  else
    {
      puts ("; print_n excluded");
    }
  puts ("");

  if (use_input_i)
    {
      puts ("\t; read a number from the terminal");
      puts ("input_i:");
      puts ("\t; return value is stored in the stack; bx = 10 for mul");
      puts ("\tpush 0");
      puts ("\tmov bx, 10");
      puts ("\t; read in the digits");
      puts ("input_i_loop:");
      puts ("\tmov ah, 0x1");
      puts ("\tint 0x21");
      puts ("\tcmp al, 0xd");
      puts ("\tje input_i_done");
      puts ("\tmov ah, 0");
      puts ("\tmov cx, ax");
      puts ("\tpop ax");
      puts ("\txor dx, dx");
      puts ("\tmul bx");
      puts ("\tsub cx, 0x30");
      puts ("\tadd ax, cx");
      puts ("\tpush ax");
      puts ("\tjmp input_i_loop");
      puts ("input_i_done:");
      puts ("\tpop ax");
      puts ("\tret");
    }
  else
    {
      puts ("; input_i excluded");
    }
  puts ("");

  puts ("section .bss");
  puts ("");

  for (i = 0; i < 26; ++i)
    if (vars_used[i])
      printf ("\tv%c: resw 1\n", 'a' + i);

  puts ("");

  puts ("section .text");
  puts ("");

  for (i = 0; i < string_count; ++i)
    printf ("\ts%i: db \"%s\", '$'\n", i, strings[i]);

  puts ("");

  puts ("; Task Completed -- Assemble with NASM or YASM");
}

static void
error (format)
     const char *format;
     /* ... */
{
  va_list args;

  fprintf (stderr, "%s:%i: error: ", input_name, line);

  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);

  fputs ("\n", stderr);

  exit (EXIT_FAILURE);
}

static void
eat_blanks ()
{
  while (look == ' ' || look == '\t')
    get_char ();
}

static void
match (what)
     int what;
{
  if (look == what)
    get_char ();
  else
    error ("expected '%c' got '%c'", what, look);

  eat_blanks ();
}

static void
get_char ()
{
  look = fgetc (stdin);
}

static char
get_name ()
{
  char name = tolower (look);

  if (isalpha (name))
    get_char ();
  else
    error ("expected identifier got '%c'", name);

  vars_used[name - 'a'] = 1;

  eat_blanks ();

  return name;
}

static int
get_num ()
{
  int result = 0;

  if (isdigit (look))
    for (; isdigit (look); get_char ())
      {
	result *= 10;
	result += look - '0';
      }
  else
    error ("expected integer got '%c'", look);

  eat_blanks ();

  return result;
}

static int
get_keyword ()
{
  int i;
  char token[7];

  if (!isalpha (look))
    error ("expected keyword got '%c'", look);

  for (i = 0; i < (int) sizeof (token) - 1 && isalpha (look); ++i)
    {
      token[i] = tolower (look);
      get_char ();
    }

  token[i] = 0;

  if (!strcmp (token, "print"))
    i = T_PRINT;
  else if (!strcmp (token, "if"))
    i = T_IF;
  else if (!strcmp (token, "goto"))
    i = T_GOTO;
  else if (!strcmp (token, "input"))
    i = T_INPUT;
  else if (!strcmp (token, "let"))
    i = T_LET;
  else if (!strcmp (token, "gosub"))
    i = T_GOSUB;
  else if (!strcmp (token, "return"))
    i = T_RETURN;
  else if (!strcmp (token, "rem"))
    i = T_REM;
  else if (!strcmp (token, "run"))
    i = T_RUN;
  else if (!strcmp (token, "end"))
    i = T_END;
  else
    error ("expected keyword got '%s'", token);

  eat_blanks ();

  return i;
}

static int
do_line ()
{
  while (look == '\n')
    {
      ++line;
      get_char ();
    }

  if (feof (stdin))
    return 0;

  if (tolower (look) == 'r')
    {
      get_char ();
      if (tolower (look) == 'u')
	{
	  get_char ();
	  if (tolower (look) == 'n')
	    return 0;
	  else
	    error ("expected statement or 'run'");
	}

      ungetc (look, stdin);
      look = 'r';
    }

  printf ("\n\t; debug line %i\n", line);

  do_statement ();

  if (look != '\n')
    error ("expected newline got '%c'", look);
  else
    get_char ();

  ++line;

  return 1;
}

static void
do_statement ()
{
  if (isdigit (look))
    printf ("l%i:\n", get_num ());

  switch (get_keyword ())
    {
    case T_PRINT:  do_print ();  break;
    case T_IF:     do_if ();     break;
    case T_GOTO:   do_goto ();   break;
    case T_INPUT:  do_input ();  break;
    case T_LET:    do_let ();    break;
    case T_GOSUB:  do_gosub ();  break;
    case T_RETURN: do_return (); break;
    case T_REM:    do_rem ();    break;
    case T_END:    do_end ();    break;
    default:       assert (0);   break;
    }
}

static void
do_print ()
{
  use_print_n = 1;

  do_print_item ();

  for (;;)
    {
      if (look == ',')
	{
	  match (',');
	  use_print_t = 1;
	  puts ("\tcall print_t");
	}
      else if (look == ';')
	{
	  match (';');
	}
      else
	{
	  break;
	}

      do_print_item ();
    }

  puts ("\tcall print_n");
}

static void
do_print_item ()
{
  if (look == '"')
    {
      use_print_s = 1;
      do_string ();
      puts ("\tcall print_s");
    }
  else
    {
      use_print_i = 1;
      do_expression ();
      puts ("\tcall print_i");
    }
}

static void
do_if ()
{
  int op;

  do_expression ();
  puts ("\tmov dx, ax");

  if (look == '=')
    {
      match ('=');
      if (look == '<')
	{
	  match ('<');
	  op = O_LESS_OR_EQUAL;
	}
      else if (look == '>')
	{
	  match ('>');
	  op = O_MORE_OR_EQUAL;
	}
      else
	{
	  op = O_EQUAL;
	}
    }
  else if (look == '<')
    {
      match ('<');
      if (look == '=')
	{
	  match ('=');
	  op = O_LESS_OR_EQUAL;
	}
      else if (look == '>')
	{
	  match ('>');
	  op = O_NOT_EQUAL;
	}
      else
	{
	  op = O_LESS;
	}
    }
  else if (look == '>')
    {
      match ('>');
      if (look == '=')
	{
	  match ('=');
	  op = O_MORE_OR_EQUAL;
	}
      else if (look == '<')
	{
	  match ('<');
	  op = O_NOT_EQUAL;
	}
      else
	{
	  op = O_MORE;
	}
    }
  else
    {
      error ("expected =, <>, ><, <=, =<, >= or => got '%c'", look);
    }

  do_expression ();

  fputs ("\tcmp dx, ax\n\tj", stdout);

  /* note inversal of operators */
  if (op == O_EQUAL)
    fputs ("ne", stdout);
  else if (op == O_NOT_EQUAL)
    fputs ("e", stdout);
  else if (op == O_LESS)
    fputs ("ge", stdout);
  else if (op == O_MORE)
    fputs ("le", stdout);
  else if (op == O_LESS_OR_EQUAL)
    fputs ("g", stdout);
  else if (op == O_MORE_OR_EQUAL)
    fputs ("l", stdout);

  printf (" i%i\n", line);

  do_statement ();

  printf ("i%i:\n", line);
}

static void
do_goto ()
{
  printf ("\tjmp l%i\n", get_num ());
}

static void
do_input ()
{
  use_input_i = 1;
  do
    printf ("\tcall input_i\n\tmov [v%c], ax\n", get_name ());
  while (look == ',');
}

static void
do_let ()
{
  char name = get_name ();
  match ('=');
  do_expression ();
  printf ("\tmov [v%c], ax\n", name);
}

static void
do_gosub ()
{
  printf ("\tcall l%i\n", get_num ());
}

static void
do_return ()
{
  puts ("\tret");
}

static void
do_rem ()
{
  while (look != '\n')
    get_char ();
}

static void
do_end ()
{
  puts ("\tjmp exit");
}

static void
do_expression ()
{
  if (look == '+' || look == '-')
    puts ("\txor ax, ax");
  else
    do_term ();

  while (look == '+' || look == '-')
    {
      int op = look;
      puts ("\tpush ax");
      match (look);
      do_term ();
      puts ("\tpop bx");
      if (op == '+')
	puts ("\tadd ax, bx");
      else if (op == '-')
	puts ("\tsub ax, bx\n\tneg ax");
    }
}

static void
do_term ()
{
  do_factor ();

  while (look == '*' || look == '/')
    {
      int op = look;
      puts ("\tpush ax");
      match (look);
      do_factor ();
      puts ("\tpop bx");
      puts ("\txor dx, dx");
      if (op == '*')
	puts ("\timul bx");
      else if (op == '/')
	puts ("\txchg ax, bx\n\tidiv bx");
    }
}

static void
do_factor ()
{
  if (look == '(')
    {
      match ('(');
      do_expression ();
      match (')');
    }
  else if (isalpha (look))
    {
      printf ("\tmov ax, [v%c]\n", get_name ());
    }
  else
    {
      printf ("\tmov ax, %i\n", get_num ());
    }
}

static void
do_string ()
{
  int i;
  size_t string_size = 0;
  char *string;

  string = xmalloc (1);
  string[0] = 0;

  for (get_char (); look != '"'; get_char ())
    {
      ++string_size;
      string = xrealloc (string, string_size + 1);
      string[string_size - 1] = look;
      string[string_size] = 0;
    }

  get_char ();

  for (i = 0; i < string_count; ++i)
    if (!strcmp (strings[i], string))
      {
	printf ("\tmov ax, s%i\n", i);
	return;
      }

  ++string_count;
  strings = xrealloc (strings, string_count * sizeof (char *));
  strings[string_count - 1] = string;

  printf ("\tmov ax, s%i\n", i);
}

/* vim:set ts=8 sts=2 sw=2 noet: */
