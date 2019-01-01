.AUTODEPEND

#		*Translator Definitions*
CC = bcc +IFS.CFG
TASM = TASM
TLINK = tlink


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
  event.obj \
  graphics.obj \
  main.obj \
  message.obj \
  object.obj \
  product.obj \
  screens.obj \
  trans.obj \
  user.obj \
  vocab.obj \
  cgaf.obj \
  egavgaf.obj \
  gothf.obj \
  hercf.obj \
  ibm8514f.obj \
  sansf.obj \
  tripf.obj

#		*Explicit Rules*
ifs.exe: ifs.cfg $(EXE_dependencies)
  $(TLINK) /v/x/c/P-/LC:\BORLANDC\LIB @&&|
c0h.obj+
event.obj+
graphics.obj+
main.obj+
message.obj+
object.obj+
product.obj+
screens.obj+
trans.obj+
user.obj+
vocab.obj+
cgaf.obj+
egavgaf.obj+
gothf.obj+
hercf.obj+
ibm8514f.obj+
sansf.obj+
tripf.obj
ifs
		# no map file
emu.lib+
mathh.lib+
ch.lib+
graphics.lib
|


#		*Individual File Dependencies*
event.obj: event.cpp 

graphics.obj: graphics.cpp 

main.obj: main.cpp 

message.obj: message.cpp 

object.obj: object.cpp 

product.obj: product.cpp 

screens.obj: screens.cpp 

trans.obj: trans.cpp 

user.obj: user.cpp 

vocab.obj: vocab.cpp 

#		*Compiler Configuration File*
ifs.cfg: ifs.mak
  copy &&|
-mh
-2
-K
-v
-G
-Z
-H=IFS.SYM
-IC:\BORLANDC\INCLUDE
-LC:\BORLANDC\LIB
| ifs.cfg


