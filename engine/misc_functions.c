#include <stdio.h>

unsigned char ascii_to_keycode(unsigned char key)
{
	if (key >= 'a' && key <= 'z') key -= 'a' - 'A';
	switch (key) {
	case ')':
		key = '0';
		break;
	case '!':
		key = '1';
		break;
	case '@':
		key = '2';
		break;
	case '#':
		key = '3';
		break;
	case '$':
		key = '4';
		break;
	case '%':
		key = '5';
		break;
	case '^':
		key = '6';
		break;
	case '&':
		key = '7';   
		break;
	case '*':
		key = '8';
		break;
	case '(':
		key = '9';
		break;

	case '~':
		key = '`';
		break;
	case '_':
		key = '-';
		break;
	case '+':
		key = '=';
		break;
	case '{':
		key = '[';
		break;
	case '}':
		key = ']';
		break;
	case '|':
		key = '\\';
		break;
	case ':':
		key = ';';
		break;
	case '"':
		key = '\'';
		break;
	case '<':
		key = ',';
		break;
	case '>':
		key = '.';
		break;
	case '?':
		key = '/';
		break;
	}
	return key;
}
