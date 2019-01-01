// stack.cpp
//
// this file implements a generic stack which holds
//
//	 * fstream references
//	 * line numbers (for error messaging)
//
// 16-oct-90, property of maz spork - all rights reserved
//

#include <fstream.h>
#include <string.h>
#include "stack.h"

// create a stack

stack::stack (int s) {
	stk = new stacknode [s];
	index = 0;
	}

// destroy a stack

stack::~stack () {
	delete stk;
	}

// push a string onto the stack

void stack::push (ifstream &f, int &l) {
	stk [index].file = f;
	stk [index].line = l;
	index++;
	}

// pop a string off the stack

void stack::pop (ifstream &f, int &l) {
	f = stk [index].file;
	l = stk [index].line;
	}

// get the index of the stack

int stack::getindex (void) {
	return index;
	}
