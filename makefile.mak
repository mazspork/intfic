# Makefile for the Interactive Fiction System

# This file compiles all changed source files and links them
# with existing object files into an executeable image

ifs.exe:  event.cpp main.cpp object.cpp product.cpp graphics.cpp\
			 user.cpp trans.cpp display.cpp vocab.cpp

			 tlink event

