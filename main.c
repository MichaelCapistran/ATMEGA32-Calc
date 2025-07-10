/*
 * main.c
 *
 * Created: 4/29/2025 11:07:50 PM
 *  Author: wolfg
 */ 

#include <xc.h>
#include "avr.h"
#include "lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define MAX 100

const char *expr;
int pos;
char inArray[MAX];
int curInput;
unsigned char inOperators = 0;
unsigned char divZeroError = 0;
unsigned char syntaxError = 0;
double prevAns = 0;
char numpadToChar(int row, int col);
char charToNum(char c);
void handleInput(char iput);
char getNumPadInput();
char numToChar(unsigned char c);
double parse_expression();




int main(void)
{
	lcd_init();
	char iput;
	DDRC = 0x0F;
	PORTC = 0xF0;
	lcd_clr();
	lcd_pos(0,0);
	char buf[20];
	
	while(1){
		iput = getNumPadInput();
		handleInput(iput);
		lcd_clr();
		lcd_pos(0,0);
		for (int i = (curInput < 16) ? 0 : curInput - 16; i < curInput; i++){
			lcd_put(inArray[i]);
		}
		if(inOperators){
			lcd_pos(1,0);
			lcd_puts1("A+,B-,C/,1(,2)");
		}
		else if(divZeroError){
			lcd_pos(1,0);
			lcd_puts1("ERR:Div by Zero");
		}
		else if(syntaxError){
			lcd_pos(1,0);
			lcd_puts1("ERR:Syntax");
		}
		else{
			lcd_pos(1,0);
			sprintf(buf, "%f", prevAns);
			lcd_puts2(buf);
		}
		avr_wait(200);
	}
}

void skip_spaces() {
	while (isspace(expr[pos])) pos++;
}

// Get current character
char peek() {
	skip_spaces();
	return expr[pos];
}

// Advance and return the current character
char get() {
	skip_spaces();
	return expr[pos++];
}

// Parse a number (integer or floating point)
double parse_number() {
	skip_spaces();
	double result = 0.0;
	int start = pos;

	while (isdigit(expr[pos]) || expr[pos] == '.') pos++;

	if (start == pos) {
		syntaxError = 1;
		return 0;
	}

	char buffer[64];
	int len = pos - start;
	if (len >= sizeof(buffer)) {
		syntaxError = 1;
		return 0;
	}

	strncpy(buffer, expr + start, len);
	buffer[len] = '\0';

	result = strtod(buffer, NULL);
	return result;
}


// Parse a factor: number, (expression), or unary minus
double parse_factor() {
	skip_spaces();
	char c = peek();

	if (c == '-') {
		get(); // consume '-'
		return -parse_factor(); // unary minus
	} 
	else if (c == '(') {
		get(); // consume '('
		double result = parse_expression();
		if (peek() != ')') {
			syntaxError = 1;
			return 0;
		}
		get(); // consume ')'
		return result;
	} 
	else {
		return parse_number();
	}
}

// Parse term: factor ((* or /) factor)*
double parse_term() {
	double result = parse_factor();

	while (1) {
		char c = peek();
		if (c == '*') {
			get();
			result *= parse_factor();
			} else if (c == '/') {
			get();
			double divisor = parse_factor();
			if (divisor == 0.0) {
				divZeroError = 1;
				return 0;
			}
			result /= divisor;
			} else {
			break;
		}
	}

	return result;
}

// Parse expression: term ((+ or -) term)*
double parse_expression() {
	double result = parse_term();

	while (1) {
		char c = peek();
		if (c == '+') {
			get();
			result += parse_term();
			} else if (c == '-') {
			get();
			result -= parse_term();
			} else {
			break;
		}
	}

	return result;
}

// Main evaluation function
double evaluate(const char *input) {
	expr = input;
	pos = 0;
	double result = parse_expression();

	if (peek() != '\0') {
		syntaxError = 1;
		return 0;
	}

	return result;
}



void handleInput(char iput){
	char pass = '\0';
	if (inOperators){
		if (iput == '0'){
			inOperators = 0;
		}
		else if (iput == 'a'){
			pass = '+';
		}
		else if (iput == 'b'){
			pass = '-';
		}
		else if (iput == 'c'){
			pass = '/';
		}
		else  if (iput == '1'){
			pass = '(';
		}
		else if (iput == '2'){
			pass = ')';
		}
	}
	else if (iput == 'a'){
		inOperators = 1;
	}
	else if (iput == '#'){
		pass = '.';
	}
	else if(charToNum(iput) != 10 || iput == '*'){
		pass = iput;
	}
	else if(iput == 'b'){
		curInput = (curInput == 0) ? 0 : curInput - 1;
		return;
	}
	else if(iput == 'c'){
		inArray[curInput] = '\0';
		syntaxError = 0;
		divZeroError = 0;
		prevAns = evaluate(inArray);
		curInput = 0;
		return;
	}
	
	if (pass && curInput < MAX - 1){
		inArray[curInput] = pass;
		curInput++;
	}
}

char getNumPadInput(){
	for (int i = 0; i < 4; i++){
		PORTC |= 0x0F;
		CLR_BIT(PORTC, i);
		for (int j = 0; j < 4; j++){
			if(!GET_BIT(PINC, j + 4)){
				while(!GET_BIT(PINC, j+4));
				return numpadToChar(i,j);
			}
		}
	}
	return '\0';
}

char numpadToChar(int row, int col){
	if(row == 0){
		if (col == 0){
			return '1';
		}
		else if(col == 1){
			return '2';
		}
		else if(col == 2){
			return '3';
		}
		else{
			return 'a';
		}
	}
	else if (row == 1){
		if (col == 0){
				return '4';
			}
			else if(col == 1){
				return '5';
			}
			else if(col == 2){
				return '6';
			}
			else{
				return 'b';
			}
	}
	else if (row == 2){
		if (col == 0){
			return '7';
		}
		else if(col == 1){
			return '8';
		}
		else if(col == 2){
			return '9';
		}
		else{
			return 'c';
		}
	}
	else{
		if (col == 0){
			return '*';
		}
		else if(col == 1){
			return '0';
		}
		else if(col == 2){
			return '#';
		}
		else{
			return 'd';
		}
	}
}




char numToChar(unsigned char c){
	return c + '0';
}

char charToNum(char c){
	if(c < '0' || c > '9'){
		return 10;
	}
	else{
		return c - '0';
	}
}
