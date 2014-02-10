// Compil.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include "Lex.h"
#include "Synt.h"
#include <stdlib.h>
#include <crtdbg.h>


using namespace std;


char * Write_file_to_buf(const char* path);
int Find_name (char *st);


int main(int argc, _TCHAR* argv[])
{
		
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	try
	{	
		char * buf = Write_file_to_buf("kod.txt");
	
		if(buf == NULL)
			return 1;
		int j = 1;
	
		cout << endl<< j << ". ";
		for (int i = 0; buf[i] != '\0'; i++)
		{
			cout << buf[i];
			if(buf[i] == '\n')
			{
				j++;
				cout << j << ". " ;
			}		
		}
		cout << endl;
	
		system("pause");
		// Лексический анализатор
		Lex* lexer = new Lex(buf);
	
			int k;
		lexer->Print_Lex();
	
		system("pause");
	
		Synt* parser = new Synt(lexer);
		//_crtBreakAlloc = 439;
	
		parser->Print(0);
		if(parser->Code_generation(NULL))
			cout << "\nCode generation is completed\n" ;
		else
			cout << "\nCode generation is failed\n" ;
		system("pause");
		delete lexer;
		delete parser;	
		delete buf;
		_CrtDumpMemoryLeaks();
	}
	catch (...)
	{
		cerr << "Oops" << endl;
		system("pause");
		return -1;
	}
	
	return 0;
	
}


// Записать текст файла в массив

char * Write_file_to_buf(const char* path)
{
	
	streamoff size = 0;
	char *buf = NULL;
	ifstream file(path,ios::in );
	if(!file)
	{
		cerr << "File is not open" << endl;
		system("pause");
		return NULL;
	}
	// определим размер файла
	file.seekg (0, std::ios::end);
	size = file.tellg();
	file.seekg (0, std::ios::beg);
	if(size < 1)
	{
		cerr << "File is empty" << endl;
		system("pause");
		file.close();
		return NULL;
	}
	buf = new char[size + 1];
	// считать весь файл в buf
	file.getline(buf,size+1, EOF);
	if (file.gcount() < 1)
	{
		cerr << "File is not read" << endl;
		system("pause");
		file.close();
		return NULL;
	}
	buf[size] = '\0';
	return buf;
}




