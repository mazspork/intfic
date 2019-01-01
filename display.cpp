// get next whitespace or zero-terminated string from p.
// p is a reference to the string pointer, so it will
// change the original pointer in the calling function.
// returns the address of the word found.

char *getnext (char *&p) {
	static char buffer [0x80];				// max identifier length for word
	char *q = buffer;

	while (isspace (*p)) p++;				// skip white space
	while (*p && !isspace (*p)) *q++ = toupper (*p++);
	*q = '\0';
	while (isspace (*p)) p++;				// skip white space
	if (!*p) p = NULL;

	return buffer;
	}

// print string in a way such that words are not split across two lines
// this function is not 100% correct !

void prettyprint (char *s) {
	static int c = 0;
	int i;
	char *p;

	while (*s) {
		p = s;
		i = 0;
		while (*p && *p != SPACE && *p != '\n') {
			p++;
			i++;
			}
		if (i + c >= SCREENWIDTH) {
			cout << '\n';
			c = 0;
			}
		else c += i + 1;
		for (; i; i--) cout << *s++;
		if (*p == NEWLINE) c = 0;
		if (*p) cout << *p++;
		s = p;
		}
	}

// gets next sentence in input buffer (possibly waiting there through a
// multi-sentence construct)

sentence_type *getsentence () {
	static sentence_type sentence;		// must stay after return
	char buffer [0x80];
	int i, j, k;
	wordnode *n;
	char ch, *p, *q;

	for (k = 0; k < 0x10; k++) sentence.wordlist [k] = 0;

	cin.width (sizeof (buffer));				// avoid overflow in streambuf

	if (debug) cout << "Reading user input\n";

	cout << "? ";

	cin.get (buffer, 0x80);
	cin.get (ch);

	if (debug) cout << "Processing user input\n";

	strupr (buffer);
	q = buffer;

	for (j = 0; q;) {
		p = getnext (q);
		i = vocab.getnumber (p);

		if (i) {								// did the vocabulary contain the word?
			sentence.wordlist [j++] = i;
			}
		}

	if (debug) cout << "User input processed\n";

	return &sentence;
	}

