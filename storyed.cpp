// storyed.cpp
//
// a story script editor
//
// this file will as of now convert the ASCII script into an easier
// translatable binary file




enum token_type {

	VOCABULARY,
		ACTORS,
		VERBS,
		NOUNS,
		ADJECTIVES,
		PREPOSISIONS,
		ARTICLES,
		ADVERBS,

	MESSAGES,
		MSG,

	OBJECTS,
		OBJECT,
		BRIEF1,
		BRIEF2,
		VERBOSE,
		PARENT,
		EXITS,
		GRAPHIC,

	PRODUCTS,
		PRODUCT,

	EVENTS,
		EVENT,
		CONDITIONS,
		ACTIONS,
		MESSAGE,
		INPUT,

	SYSTEM,
		INITIAL,
		LOOK,
		INVENTORY,
		GO,
		QUIT,
		GET,
		PUT,
		HELP,
		WAIT,
		GIVE,
		AND,
		OR,
		ALL,
		IT,
		WHAT,
		YOU,
		IN,
		OUT

	};

class field {
	token_type token;

	};

class vocabulary_field : field {

	struct {
		char *word;
		int number;
		} *table;

	};

class message_field : field {

	char **messages;

	};

class object_field : field {

	struct {


	};

class product_field : field {

	int *product;
	int *timers;
	int *randoms;

	};

class event_field : field {



	};

class system_field : field {



	};




main () {

	field *story;			// all the different fields;


	}






