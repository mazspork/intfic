// ifs.h
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// This is the header file for all C++ source files in the IFS project.
// Contains declarations for all classes and data structures,
// prototypes for the most important functions and constant
// data declarations (no #defines).

// constant data

const
	BUFFERSIZE   = 0x500,
	TAB          = 9,
	SPACE        = ' ',
	SEMICOLON    = ';',
	SCREENWIDTH  = 80,
	NEWLINE      = 13,
	TRUE         = 1,
	FALSE        = 0,
	NOTFOUND     = -32768,
	BEFORE_ENTRY = 1,
	AFTER_ENTRY  = 2,


// Normal keyboard codes


			Tab =   9,
			Esc =  27,
		 Break =   3,
	Backspace =   8,


// Extended keyboard codes (buffer holds '\0' + code)

			 Up =  72,
		  Down =  80,
		  Left =  75,
	 CtrlLeft = 115,
		 Right =  77,
	CtrlRight = 116,
			Ins =  82,
			Del =  83,
		  Home =  71,
			End =  79,
		  PgUp =  73,
	 CtrlPgUp = 132,
		  PgDn =  81,
	 CtrlPgDn = 118,
			 F1 =  59,
			 F2 =  60,
			 F3 =  61,
			 F4 =  62,
			 F5 =  63,
			 F6 =  64,
			 F7 =  65,
			 F8 =  66,
			 F9 =  67,
			F10 =  68;

// prototypes for data structures and classes

struct object;
struct vocabulary;
struct event;
struct wordnode;
struct sentence_type;
struct namenode;
struct prcond;
struct obcond;
struct grcom;
struct message;
struct product;

// enumerations (these are global)

// enumeration "word class"
// defines the different classes of words in a sentence: verbs, nouns,
// prepositions and so on.

enum wordclass {
	NO_CLASS, WC_VERB, WC_NOUN, WC_ADJECTIVE, WC_PREPOSITION,
	WC_ARTICLE, WC_ADVERB, WC_ACTOR, WC_LAST
	};

// enumeration "object relation"
// defines relations between objects

enum object_relation {
	OR_ERROR,         // invalid relation
	OR_IN,            // is the object a child of X
	OR_WITH,          // is the object a sister of X
	OR_WITHIN,        // is the object either a child or a sister of X
	OR_WITHINALL,     // is the object either a child, a sister or a parent of X
	OR_NOTIN,         // is the object not a child of X
	OR_NOTWITH,       // is the object not a sister of X
	OR_NOTWITHIN,     // is the object neither a child nor a sister of X
	OR_HIDDEN,        // has the object no parent object
	OR_WASIN,         // was the object a child of X
	OR_WASWITH,       // was the object a sister of X
	OR_WASWITHIN,     // was the object either a child or a sister of X
	OR_WASNOTIN,      // was the object not a child of X
	OR_WASNOTWITH,    // was the object not a sister of X
	OR_WASNOTWITHIN,  // was the object neither a sister nor a child of X
	OR_WASHIDDEN,     // had the object no parent object
	OR_WITHNOPARENT,  // are the objects sisters (no-parent version)
	OR_RELATED			// somehow related in any way
	};

// enumeration "graphic command numbers"
// defines graphics primitives

enum grcomnum {
	GR_INVALID,			// invalid graphic command
	GR_MOVE,				// move pointer to (x,y)
	GR_PLOT,				// plot a point at (x,y)
	GR_DRAW,				// draw a line to (x,y)
	GR_FILL,				// flood fill at (x,y)
	GR_COLOUR,			// change colour to (colour);
	GR_PATTERN,			// change pattern to (pattern);
	GR_THICKNESS		// change thickness of lines to (thickness)
	};

// data structure declarations and C++ objects

// structure "player data"
// defines the player's personal file

struct player_data {
	char firstname [40],
		  lastname [40],
		  nationality [30],
		  country [30],
		  sex [7],
		  age [2],
		  tongue [40];
	};

// structure "sentence type"
// struct sentence defines a normal english sentence
// this has to be slightly modified for other national languages

struct sentence_type {
	int actor;
	int verb;
	int nounlist [0x8];
	int prep;
	int preplist [0x8];
	};

// structure "namenode"
// defines an entry into the vocabulary's symbol table, contatining
// a word number, a word class (noun, verb etc.) and a link

struct namenode {
	char *name;             // pointer to character string
	int number;             // it's symbol table number
	int dual;               // same word defined elsewhere with other class
	wordclass type;         // word class, eg. noun or verb or preposition
	};

// structure "wordnode"
// defines a word (by number) and has a link pointer

struct wordnode {
	int number;
	wordnode *next;
	};

// structure "symbol"
// defines an entry in a symbol table

struct symbol {
	char *name;
	int value;
	};

// structure "product condition type"
// defines a product structure

struct prcond {
	int product, value;              // the product number and its value condition
	char relation;                   // the relation to be met
	prcond *next;                    // next product in this list
	char *errormsg;                  // possible error to print if not met
	int indirect_errormsg;           // and is that error string allocated elsewhere?
	};

// structure "object condition type" defines an object structure

struct obcond {
	object *a, *b;                   // pointers to the two objects to compare
	object_relation relation;			// how to compare (IN or WITH or ...)
	obcond *next;                    // next object in this condition list
	char *errormsg;                  // possible error to print if not met
	int indirect_errormsg;           // and it that error string allocated elsewhere?
	};

// structure "object node" defines a node in a linked list of object pointers

struct objnode {
	object *ptr;
	objnode *next;
	};

// structure "query node" defines a node in a linked list of queries

struct qnode {
	char *prompt;
	int product;
	qnode *next;
	};

// structure "graphics command"
// defines the graphics primitives as single entities:-
//
//   <COMMAND>,<X>,<Y>,<COLOUR | RADIUS | PATTERN | BORDER>
//
// (refer to enum grcomnum for a list of valid commands)
//
// These commands are in IBM EGA form, and are translated to lower
// level graphic systems by a colour translation table, which
// selects other grey level patterns or colour palettes on demand.

struct grcom {
	grcomnum command;	// graphics command to execute
	int x, y;			// the coordinates of the object
	union {
		int radius;		// the radius of a circle, or
		int colour;		// the colour of an object, or
		int pattern;	// the pattern of an object
		int border;		// the border of a flood fill
		int thickness;	// the thickness of a line
		};
	};

// class "object"
// holds the world as bound entities. this structure is linked together
// in two ways, by contents (associative) and by entry (sequential):
//
//    A-----------E        Here, A and E have "root" parents. A's sister
//    |           |        pointer refers to E. A's child pointer points
//    B---C---D   F---G    to B, whose sister leads to C, and in turn to
//        |       |        D. B, C and D all have parent pointers to A,
//        |       |        as F and G has to E.
//        H       I        H and I have no sister objects.
//
//
//    A--B--C--D--E--F     Here, the nodes are just connected through a
//                   |     singly linked list, starting at the private
//             I--H--G     static "root". Just for faster reference.
//
//
// It is very important that the first object defined is called "root", and
// that no "root" objects are defined afterwards. Otherwise, cross reference
// and, more importantly, deallocation, will fail.
// The static data and function declaration is put down first, and the
// automatics later.

struct object {
	static object *root;						  // only 1 of this
	static int count;							  // counts objects

	static struct pic_t {                 // graphic command structure (internal)
		int size;                          // size of graphic command list
		grcom *list;                       // list of graphic commands
		} picture;

	static object *search (int *, object *);  // search on a list of words
	static object *search (char *);		  		// on name
	static object *search (int, object *);		// and on single word
	static void   ready (void);			  		// get ready for new pass

	struct {                              // different on/off flags
		unsigned moved    : 1;             // has the object just moved?
		unsigned takeable : 1;             // can the object be carried?
		unsigned actor    : 1;				  // is it an actor object?
		} flags;

	int       id;								  // identifier
	char      *name;                      // name of object
	char      *brief, *brief2, *verbose;  // descriptions
	object    *parent, *sister, *child;   // links to parent, sister and child
	object    *next;							  // links to next in global linkage
	wordnode  *wordlist;						  // list of words
	object	 *lastparent;                // last parent object
	objnode   *exitlist;                  // list of exit objects
	char      *grfile;						  // graphic filename

				 object (char *);            // constructs objects
				~object (void);              // destructs obejcts
	void      setname (char *);           // set name of object
	char      *getname (void);            // get name of object
	int       setbrief  (char *);         // set brief description (a-form)
	int       setbrief2 (char *);         // set brief description (the-form)
	char      *getbrief  (void);          // return object's brief description (a-form)
	char      *getbrief2 (void);          // return object's brief description (the-form)
	void      setverbose (char *);        // set long description
	char      *getverbose (void);         // return long description
	int       setparent (char *);         // set parent object (string version)
	void      setparent (object *);       // set parent object (pointer version)
	int       setsister (char *);         // set sister object (string version)
	void      setsister (object *);       // set sister object (pointer version)
	void      setchild (object *);        // set child object
	object    *getparent (void);          // return parent object of this (if any)
	object    *getchild (void);           // return 1st child object (if any)
	object    *getsister (void);          // return 1st sister object (if any)
	int       issister (object *);        // is a certain object sister to this
	int       ischild (object *);         // is a certain object child to this
	void      listchildren (void);        // give all children (inventory)
	void      listsisters (void);         // give all sisters (look)
	void      listparentverbose (void);   // write out parent info (look)
	void      setgrfile (char *);         // set graphic filename
	int       grload (int);				     // read graphics information
	void      grdraw (void);              // draw object's graphic on screen
	void      cleanup (void);             // remove redundant information etc.
	void      describe (void);            // list parent verbose and all sisters
	int       setexit (char *);			  // set exit object
	int       isexit (object *);          // is certain object a valid exit?
	void      checkmovements (void);      // check for people moving relative to this
	int       rel (object_relation, object *); // check relation
	};

// class "event"
// - works like this:
// when all conditions in the linked list "condition" are met, ie. when
// the "flag" equals the "value", then the "message" is printed out and
// subsequently, the flags listed in the linked list "changes" are modified
// according to the "value" - the value is added to the flag

struct event {
	static event *root;						  // root event for linking
	static int affect (int);

	char      *name;							  // name of event (not important)
	wordnode  *wordlist;						  // basic sentence to meet condition
	prcond	 *prod_conditions;			  // flags to meet condition
	obcond	 *obj_conditions;				  // objects to meet
	char      *message_text;				  // message to print if met
	prcond	 *prod_changes;				  // flags to affect if met
	obcond	 *obj_changes;					  // objects to affect if met
	event     *next;							  // pointer to next event
	qnode     *qlist;							  // list of queries if event fires
	int       indirect_message_text;		  // if message is allocated outside event

				 event (char *);
				~event (void);
	char      *getname (void);
	void      condition (char *);
	void      condition (object *, object_relation, object *, char *);
	void      condition (int, char, int, char *);
	void      change (object *, object_relation, object *);
	void      change (int, char, int);
	void      set_message (char *);
	int       match (int);
	int       fire (void);
	event     *getnextevent (void);
	void      query (int, char*);
	void      query (void);
	};

// class "vocabulary" defines the set of strings, the types of words they
// define, as well as the methods used for manipulating them.

struct vocabulary {
	namenode  *dictionary;					  			// the dictionary (names, types, ...)
	int       number;							  			// the current word number
	wordclass type;							  			// the current word class
	int       wordcount;						  			// used during pass 1

				 vocabulary (void);
				~vocabulary (void);
	void      enter (char *, wordclass);         // add a word by name and class
	void      synonym (char *, char *);				// add a synonym by name and name
	void      synonym (char *);						// add a synonym by name to current word
	int       getnumber (char *);						// get the internal number of a given word
	wordclass gettype (char *);						// get the actual type of a given word
	wordclass gettype (int);							// get the type of a known internal word number
	void      addword (int, wordclass, char *);	// add a word by full-criteria parameter list (no fuss)
	void      addwordcount (void);					// add to the counter during initialisation
	void      initialize (void);						// initialise internal structure before allocation
	int       getother (int);							// return a possible "mirror image" of a word
	};

// class "product" - defines the product symbol table

struct product {
	char      **namelist;                 // list of product names (temporary)
	int       entries;                    // total nos. of products
	int       timers;                     // of these, the no. of timer prods.
	int       randoms;                    // and no. of random prods.
	int       *timerlist;                 // list of pointers to timer prods.
	int       *randlist;                  // list of pointers to random prods.
	long      *list;                      // and the actual symbol table array
	int       global_timer;					  // the "real time" timer
	int		 game_running;					  // as long as the game is running

				 product (void);
				~product (void);
	void      entry (char *, int);		  // add word and value to database
	void      entry (int);					  // add value to database
	void      entry (char *);				  // add word to database
	int       match (int, char, int);	  // check for matching product
	void      affect (int, char, int);	  // change state of product
	void      cleanup (void);				  // clean up buffers
	void      initialize (void);			  // allocate buffers
	int       getindex (char *);			  // get index of certain product
	void      addcount (char *);			  // add word-count for value
	void      elapse (long);				  // update all timer products
	};

// class "message" defines the message symbol table

struct message {
	char **namelist;							  // the actual messages
	int entries;								  // the number of entries

				 message (void);
				~message (void);
	void      entry (int, char *);		  // insert a new message
	void      addcount (int);				  // add to workspace
	char*     getmsg (int);		           // get a message from table
	void      initialize (void);          // initialize the tables
	};


// the most important function prototypes

void loadworld   (char *);               // initialize from a definition file
char *getnext    (char *&);              // get the next word from a list
void getsentence (int);                  // get a sentence from the player
void error       (char *, char *, int);  // signal a definition error
void draw        (grcom *, int);         // draw a picture
void printout    (unsigned char *);      // graphical print-a-string
void printout    (unsigned char);        // graphical print-a-character
void printreset  (void);                 // reset printing subsystem
int  ginit       (void);                 // initialize graphic system
void gclose      (void);                 // close down graphic system
void animate     (void);                 // animate current scene
void specialtext (void);                 // display text in special format
void normaltext  (void);                 // display text in normal format
void stub_anim   (void);                 // stub animation handler (no-anim)
void catastrophy (char *);               // when everything else fails...
int  input       (char *, int);          // input from keyboard
void draw_header (void);                 // draw the Intl.Devel. header

char mytolower   (char);
char mytoupper   (char);
int  myisascii   (char);
int  myisalnum   (char);

void prettyprint (char *s, char *s1 =0, char *s2 =0, char*s3 =0, char *s4 =0, char *s5 =0);
