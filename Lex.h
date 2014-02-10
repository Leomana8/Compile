#include <vector>
#include <cctype>
#include <string>
#include <iostream>


using namespace std;

#pragma once
// �������
enum ID_lexem {
	L_VARIABLE, // ���������
	L_NUMBER_INT, // ����� �������������
	L_NUMBER_FLOAT, // ����� ������������
	L_OPERATION, // ��������
	L_RESERVED, // ����������������� �����
	L_WRONG, // ������������ ������
	L_ // ������
};
// ��������
enum ID_operation
{
	OP_MOV, // =
	OP_PLUS, // +
	OP_MINUS, // -
	OP_MUL, // *
	OP_DIV, // /
	OP_END // ;

};

// ����������������� �����
enum ID_reserved
{
	R_INT,
	R_FLOAT
};


struct Errorka
{
	int er; // ��� ������
	int n_str; // ����� ������ � �������
	string str; // ������ ����, ��� �����������
	friend ostream &operator<< (ostream &stream, Errorka ob);
};
// ����� ������� ���� Errorka
ostream &operator<< (ostream &stream, Errorka ob)
{
	stream << " - " << ob.str << " #";
	return stream;
}
// ������ �������	
struct Token
{
	ID_lexem ltype; // ��� �������
	int lvalue; // �������� �������
	int n_str; // ����� ������
	Token() {ltype = L_; lvalue = -1;}
	Token(ID_lexem a, int b, int c): ltype(a), lvalue(b), n_str(c)
	{ }

};

// ����� ������������ �����������
class Lex
{
protected:
	// ������� ����
	vector<string> names_variable;
	// ������� ������������� �������� ��������
	vector<int> num_const_int; 
	// ������� ������������ �������� ��������
	vector<double> num_const_float; 
	// ������� ����������� ������
	vector<Errorka> errors;
	// ������ �������
	vector<Token> tokens;
	int num_expres; // ���������� ���������



public:
	Lex(const char *buf);
	Lex();
	~Lex();
	void Lex_Analis (const char * buf);
	virtual int Find_name (string st);
	void Print_Lex();

	// �������
	vector<string> getNV() { return names_variable;}
	vector<int> getNCI() { return num_const_int;}
	vector<double> getNCF() { return num_const_float;}
	vector<Token> getT() { return tokens;}
	int getNE() { return num_expres;}
	bool isError(){ return errors.size() == 0 ? false : true;}
};

Lex::Lex(const char *buf)
{
	num_expres = 0; 
	Lex_Analis(buf);
}
Lex::Lex()
{
	num_expres = 0;
}

Lex::~Lex()
{
	names_variable.clear();
	num_const_float.clear();
	num_const_int.clear();
	errors.clear();
	tokens.clear();
	
}
// ����� ������ ������ � ������� �������� ������ st � ������� ����
int Lex::Find_name ( string st)
{
	for(int i = 0; i < names_variable.size(); i++)
	{
		if(names_variable[i] == st)
			return i;
	}
	// �� �����
	return -1;
}

// ����������� ���������� (���������  ���������� ���������)
void Lex::Lex_Analis (const char * buf)
{
	//tokens = new Token();
	int num_str = 1;
	
	for (int i = 0; buf[i] != '\0'; i++)
	{
		// ������� �� �������
		if(buf[i] >= '�' && buf[i] <= '�')
		{
			Errorka t;
			t.er = 0;				
			t.str = buf[i];		
			tokens.push_back(Token(L_WRONG, errors.size(), num_str));
			errors.push_back(t);
			continue;
		}
		
		// �����
		if (isalpha(buf[i]))
		{

			string s;
			s = buf[i];
			// ���� �� ������� �����
			i++;
			// ������� �� �������
			if(buf[i] >= '�' && buf[i] <= '�')
			{
				Errorka t;
				t.er = 0;				
				t.str = buf[i];		
				tokens.push_back(Token(L_WRONG, errors.size(), num_str));
				errors.push_back(t);
				continue;
			}
			for (; isalpha(buf[i]) && buf[i] != '\0' ; i++)
			{
				s += buf[i];
			}
			i--;
			// �������� �������� �� ����� �����������������
			if (s == "int")
			{
				tokens.push_back(Token (L_RESERVED, R_INT, num_str));
				continue;
			}
			else if(s == "float")
			{
				tokens.push_back(Token (L_RESERVED, R_FLOAT, num_str));
				continue;
			}
			else
			{
				// ���� � ������� ���� ���������
				int f = Find_name(s);
				if (f == -1)
				{
					names_variable.push_back(s);
					f = names_variable.size() - 1;
				}
				tokens.push_back(Token(L_VARIABLE, f, num_str));
			}
		}
		// �����
		else if (isdigit(buf[i]))
		{

			string s;
			s = buf[i];
			int iorf = 0; // ���� ��� float - 2, int - 0, 1 - ������ ��-�� �������� ���� ����� �����
			// ���� �� ������� �����
			for (i++; buf[i] != '\0' ; i++)
			{			
				if(isdigit(buf[i])) 
				{
					if(iorf == 1) 
						iorf = 2;
				}
				else 
				{
					if(buf[i] == '.')
					{
						// ��������� �����
						if(iorf != 0)
							// ��� ��� ������ �����, ������� ������
							break;
						// ����� ���� float
						iorf = 1;						
					}
					// ��� ��� �� �����
					else 
						break;
				}
				s += buf[i];

			}

			i--;
			cout << " num = " << s.c_str();
			if(iorf == 0)
			{
				tokens.push_back(Token(L_NUMBER_INT, num_const_int.size(), num_str));
				num_const_int.push_back(atoi(s.c_str()));
			}
			else if (iorf == 2)
			{
				tokens.push_back(Token(L_NUMBER_FLOAT, num_const_float.size(), num_str));
				cout << atof("3.5");
				num_const_float.push_back(atof(s.c_str()));
			}
			else 
			{
				// ������ 
				Errorka t;
				t.er = 0;
				t.str = ".\0";
				tokens.push_back(Token(L_WRONG, errors.size(), num_str));
				errors.push_back(t);
				i++;
			}
			
		}

		else			
		{
			switch(buf[i])
			{
			case '=':
				tokens.push_back(Token(L_OPERATION, OP_MOV, num_str));
				break;
			case '+':
				tokens.push_back(Token(L_OPERATION, OP_PLUS, num_str));
				break;
			case '-':
				tokens.push_back(Token(L_OPERATION, OP_MINUS, num_str));
				break;
			case '*':
				tokens.push_back(Token(L_OPERATION, OP_MUL, num_str));
				break;
			case '/':
				tokens.push_back(Token(L_OPERATION, OP_DIV, num_str));
				break;
			case ';':
				num_expres++;
				tokens.push_back(Token(L_OPERATION, OP_END, num_str));
				break;
			case ' ':
				break;
			case '\t':
				break;
			case '\n':
				num_str++;
				break;
			default: 
				Errorka t;
				t.er = 0;				
				t.str = buf[i];
				tokens.push_back(Token(L_WRONG, errors.size(), num_str));
				errors.push_back(t);
			}
		}

	}

}

void Lex::Print_Lex()
{
		
		for(int i = 0; i < tokens.size(); i++)
		{
			cout << endl << i << ". " << tokens[i].ltype << "   " << tokens[i].lvalue<< " Line #" << tokens[i].n_str << "   ";
			switch (tokens[i].ltype )
			{
			case L_VARIABLE: 
				cout << names_variable[tokens[i].lvalue] << endl; 
				break;
			case L_RESERVED:
				if(tokens[i].lvalue == R_FLOAT)
					cout << "float" << endl;
				else if (tokens[i].lvalue == R_INT)
					cout << "int" << endl;
				break;
			case L_NUMBER_FLOAT:
				cout << num_const_float[tokens[i].lvalue] << endl;
				break;
			case L_NUMBER_INT:
				cout << num_const_int[tokens[i].lvalue] << endl;
				break;
			case L_OPERATION:
				switch(tokens[i].lvalue)
				{
				case OP_MOV:
					cout << " = " << endl;
					break;
				case OP_PLUS:
					cout << " + " << endl;
					break;
				case OP_MINUS:
					cout << " - " << endl;
					break;
				case OP_MUL:
					cout << " * " << endl;
					break;
				case OP_DIV:
					cout << " / " << endl;
					break;
				case OP_END:
					cout << " ; " << endl;
					break;
				}
				break;
				case L_WRONG:
					cout << " ERROR!!! " << errors[tokens[i].lvalue] << endl;

			}
		
		}
}