// stack.h
//
// defines the string stack
//
// (c) maz spork 16-oct-90, all rights reserved


#include <ifstream.h>

// struct "stack node"

struct stacknode {
	istream file;
	int line;
	};

// class "stack" defines a generic stack of strings

class stack {
	stacknode *stk;
	int index;

public:

			stack (int);
		  ~stack (void);
	void  push (istream &, line &);
	void  pop (istream &, line &);
	int   getindex (void);
	};
