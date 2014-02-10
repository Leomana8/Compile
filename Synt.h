#include "Lex.h"

enum ID_errors
{
	 E_UNK_ID,// необъявленный идентификатор
	 E_INV_CONS, // недопустимая последовательность
	 E_NON_ID, // отсутствие идентификатора на переменную
	 E_REDEFINE // переопределение переменной
};
// структура дерева разбора
struct Parse_Tree
{
	Token lex;
	Parse_Tree* next1;
	Parse_Tree* next2; // может быть только у операции
	Parse_Tree* prev;

	Parse_Tree()
	{
		
		next1 = NULL;
		next2 = NULL;
		prev = NULL;
	}
	Parse_Tree(Token l, Parse_Tree* n1, Parse_Tree* n2, Parse_Tree* p)
	{
		lex = l;
		next1 = n1;
		next2 = n2;
		prev = p;
	}
	Parse_Tree(Parse_Tree* n1, Parse_Tree* n2, Parse_Tree* p)
	{
		next1 = n1;
		next2 = n2;
		prev = p;
	}
	
};

// структура созданных переменных
struct Created_Var
{
	string name;
	ID_reserved type;
	int ival;
	double fval;
	friend ostream &operator<< (ostream &stream, Created_Var ob);
};


// вывод объекта типа Created_Var
ostream &operator<< (ostream &stream, Created_Var ob)
{
	stream << " - " << ob.name << " #" << ob.type;
	return stream;
}
class Synt : public Lex
{
	// массив указателей на начало каждого выражения (деревья синтак.разбора)
	// выражаение заканчиваетя ;
	//Parse_Tree** tops_trees;
	vector<Parse_Tree*> tops_trees;
	// на последние элементы
	vector<Parse_Tree*> bottoms_trees;
	//Parse_Tree** bottoms_trees;
	
	// таблица созданных переменных
	vector<Created_Var> var;

	ofstream file;

	int Cur_Reserved(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP);
	int Cur_Variable(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP);
	int Cur_RVariable(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP);
	int Cur_MOV(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP);
	int Cur_MVariable(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP);
	int Cur_MNumber(Parse_Tree* cur, int i,int j, Parse_Tree* wM, Parse_Tree* wOP);
	int Cur_Op(Parse_Tree* cur, int i, int j, Parse_Tree* wM, Parse_Tree* wOP);
	int Cur_OpVarNum(Parse_Tree* cur, int i, int j, Parse_Tree* wM, Parse_Tree* wOP);
	// при ошибке прогнать токены до завершения выражения(;)
	int Go_to_end(int i);
	int Find_name(string st);
	void Del_expres(Parse_Tree*); // удаление целого выражения
	void Del_ob(Parse_Tree* cur); // удаление одного объекта
	// печать одного токена
	void Pr_T(Parse_Tree* cur);
	void Pr_instructions(Parse_Tree* cur); 
	void P(Parse_Tree* c, void (Synt::*send_token)(Parse_Tree*));


public:
	Synt(Lex *Ob);
	void Parser(); 
	void Print(int);
	~Synt();

	// Генерация кода
	bool Code_generation (char *path);
	
};

Synt::Synt(Lex *Ob)
{
	// переходим к анализу только если нет лексических ошибок
	if(!Ob->isError())
	{
		names_variable = Ob->getNV();
		num_const_int = Ob->getNCI();
		num_const_float = Ob->getNCF();
		num_expres = Ob->getNE();
		tokens = Ob->getT();
		Parser();
	}
	
}

void Synt::Parser()
{
	int i = 0;
	int j = 0; // номер выражения
	int cond = 0;
	Parse_Tree* waitingMOV = NULL; // указывает на последнюю операцию =
	Parse_Tree* waitingOP = NULL;// указывает на ожидающую операцию 
	
	while (i < tokens.size())
	{
		Parse_Tree* current;
		waitingMOV = NULL; 
		waitingOP = NULL;
		if(errors.size() == 0)
		{
			// текущий указатель на будущий элемент дерева
			current = new Parse_Tree();
			// создаем указатель на начало дерево
			tops_trees.push_back(current) ;
			// указатель на конец
			bottoms_trees.push_back(current) ;
		}
		else
		{
			current = NULL;
		}

		switch (tokens[i].ltype)
		{
		case L_VARIABLE: 
			i = Cur_Variable(current, i, j, waitingMOV, waitingOP);
			break;
		case L_RESERVED:
			i = Cur_Reserved(current, i, j, waitingMOV, waitingOP);
			break;
		default:
			// удаляем ненужную часть дерева
			Del_ob(current);
			if(tokens[i].ltype == L_OPERATION && tokens[i].lvalue == OP_END)				
				break;
			else
			{
				Errorka t;
				t.er = E_INV_CONS;
				t.n_str = tokens[i].n_str;
				t.str = "Incorrect consistency #1";
				errors.push_back(t);
				i = Go_to_end(i);
			}
		}
		
		i++;
		j++;
		
	}// while
	if(!(tokens.back().ltype == L_OPERATION && tokens.back().lvalue == OP_END))
	{
		Errorka t;
		t.er = E_INV_CONS;
		t.n_str = tokens.back().n_str;
		t.str = "Expected ;";
		errors.push_back(t);
	}
	
}

int Synt::Cur_Reserved(Parse_Tree* cur, int i,int j, Parse_Tree* wM, Parse_Tree* wOP)
{
	Created_Var v;
	if(errors.size() == 0)
	{
		// нет ошибок , добавляем в таблицу переменных тип предполагаемой переменной		
		switch(tokens[i].lvalue)
		{
		case R_INT: v.type = R_INT; break;
		case R_FLOAT: v.type = R_FLOAT; break;
		}
		v.name = "";
		
	}
	// переходим на следующий токен с проверкой на концовку
	if(++i >= tokens.size())
		return i;
	// ожидаем в следующем токене переменную
	if (tokens[i].ltype == L_VARIABLE)
	{
		if(errors.size() == 0)
			var.push_back(v);
		i = Cur_RVariable(cur, i, j, wM, wOP);
	}
	// ошибка, отсутствует переменная
	else
	{
		// удаляем ненужную часть дерева
		Del_ob(cur);
		Errorka t;
		t.er = E_NON_ID;
		t.n_str = tokens[i].n_str;
		t.str = "Expected variable";
		errors.push_back(t);
		i = Go_to_end(i);
	}
	return i;
}
int Synt::Cur_Variable(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP)
{
	
		if(Find_name(names_variable[tokens[i].lvalue]) != -1) // если переменная объявлена
		{
			// дополняем дерево
			if(errors.size() == 0)
			{
				cur->lex = tokens[i];
				cur->next1 = new Parse_Tree;
				bottoms_trees.back() = cur;
				Parse_Tree* c = cur;
				cur = cur->next1;
				cur->prev = c;
			}
			
		}
		// ошибка,  переменная не была объявлена 
		else
		{	
			if(errors.size() == 0)
			{
				Del_ob(cur);
			}// удаление пустой ветки
				Errorka t;
				t.er = E_UNK_ID;	
				t.n_str = tokens[i].n_str;
				t.str += "Identifier is not defined -";
				t.str += names_variable[tokens[i].lvalue];
				errors.push_back(t);
				i = Go_to_end(i);
				return i;
			
		}	
	
	// переходим на следующий токен с проверкой на концовку
	if(++i >= tokens.size())
		return i;
	// ожидаем знак равенства
	if(tokens[i].ltype == L_OPERATION && tokens[i].lvalue == OP_MOV)
	{
		i = Cur_MOV(cur, i, j, wM, wOP);
	}
	// ошибка, недопустимый следующий токен 
	else
	{
		Del_ob(cur);// удаление пустой ветки
		Errorka t;
		t.er = E_INV_CONS;
		t.n_str = tokens[i].n_str;
		t.str = "Expected \= after ";
		t.str += names_variable[tokens[i-1].lvalue];	
		errors.push_back(t);
		i = Go_to_end(i);
	}
	return i;
}
int Synt::Cur_RVariable(Parse_Tree* cur, int i,int j, Parse_Tree* wM, Parse_Tree* wOP)
{
	
		if(Find_name(names_variable[tokens[i].lvalue]) == -1) // переменная не была объявлена
		{
			// заносим идентификатор в таблицу переменных
			var[var.size() - 1].name = names_variable[tokens[i].lvalue];
			if(errors.size() == 0)
			{
				
				// дополняем дерево			
				cur->lex = tokens[i];
				cur->next1 = new Parse_Tree;
				bottoms_trees.back() = cur;
				Parse_Tree* c = cur;
				cur = cur->next1;
				cur->prev = c;
			}
			
		}
		else // ошибка, пeреопределение переменной
		{
			if(errors.size() == 0)
			{
				Del_ob(cur); 
			}// удаление пустой ветки
				Errorka t;
				t.er = E_REDEFINE;
				t.n_str = tokens[i].n_str;
				t.str += "Redefinition - ";
				t.str += names_variable[tokens[i].lvalue];		
				errors.push_back(t);
				i = Go_to_end(i);
				return i;
			
		}
	
		// переходим на следующий токен с проверкой на концовку
		if(++i >= tokens.size())
			return i;
		// ожидаем знак равенства или ;
		if(tokens[i].ltype == L_OPERATION && tokens[i].lvalue == OP_MOV)
		{
			i = Cur_MOV(cur, i, j, wM, wOP);
		}
		else 
		{
			if(tokens[i].ltype == L_OPERATION && tokens[i].lvalue == OP_END)
			{
				Parse_Tree* c = NULL;
				if(cur != NULL)
					c = cur->prev;
				Del_ob(cur);// удаление пустой ветки
				Del_ob(c); // в дереве одна перменная - ненужна
				return i;
			}
			// ошибка, недопустимый следующий токен 
			else
			{
				Del_ob(cur);// удаление пустой ветки
				Errorka t;
				t.er = E_INV_CONS;
				t.n_str = tokens[i].n_str;
				t.str = "Expected ; after ";
				t.str += names_variable[tokens[i-1].lvalue];	
				errors.push_back(t);
				i = Go_to_end(i);
			}
		}
	return i;
}
int Synt::Cur_MOV(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP)
{
	if(errors.size() == 0)
	{
		// дополняем дерево
		// заносим токен равенства перед предыдущим операндом (обмен токенами)
		
		cur->lex = cur->prev->lex; // cur имеет операнд
		cur->prev->lex = tokens[i]; // = теперь перед операндом
		cur->next1 = new Parse_Tree;
		bottoms_trees.back() = cur;
		Parse_Tree* c = cur;
		cur = cur->next1;
		cur->prev = c;
		// ссылаемся на операнд после знака равенства
		wM = c;
	}
	// переходим на следующий токен с проверкой на концовку
	if(++i >= tokens.size())
		return i;
	// ожидаем переменную или число
	if(tokens[i].ltype == L_VARIABLE )
	{
		i = Cur_MVariable(cur, i, j, wM, wOP);
	}
	else 
	{
		if(tokens[i].ltype == L_NUMBER_FLOAT || tokens[i].ltype == L_NUMBER_INT)
		{
			i = Cur_MNumber(cur, i, j, wM, wOP);
		}
		// ошибка, недопустимый следующий токен 
		else
		{
			Del_ob(cur);// удаление пустой ветки
			Errorka t;
			t.er = E_INV_CONS;
			t.n_str = tokens[i].n_str;
			t.str = "Expected operand";
			errors.push_back(t);
			i = Go_to_end(i);
		}
	}
	return i;
}
int Synt::Cur_MVariable(Parse_Tree* cur, int i,int j,Parse_Tree* wM, Parse_Tree* wOP)
{
	if(errors.size() == 0)
	{
		if(Find_name(names_variable[tokens[i].lvalue]) != -1) // переменная была объявлена
		{

			// дополняем дерево			
			cur->lex = tokens[i];
			cur->next1 = new Parse_Tree;
			bottoms_trees.back() = cur;
			Parse_Tree* c = cur;
			cur = cur->next1;
			cur->prev = c;

		}
		// ошибка,  переменная не была объявлена 
		else
		{
			Del_ob(cur);// удаление пустой ветки
			Errorka t;
			t.er = E_UNK_ID;
			t.n_str = tokens[i].n_str;
			t.str += "Identifier is not defined -  ";
			t.str += names_variable[tokens[i].lvalue];
			errors.push_back(t);
			i = Go_to_end(i);
			return i;
		}
	}
		// переходим на следующий токен с проверкой на концовку
		if(++i >= tokens.size())
			return i;
		// ожидаем  любую операцию
		if(tokens[i].ltype == L_OPERATION )
		{
			switch(tokens[i].lvalue)
			{
			case OP_MOV:
				i = Cur_MOV(cur, i, j, wM, wOP); break;
			case OP_END:
				Del_ob(cur);// удаление пустой ветки
				break;
			default:
				i = Cur_Op(cur, i, j, wM, wOP); 
			}
			
		}
		else 
		{
			// ошибка, недопустимый следующий токен 
			Del_ob(cur);// удаление пустой ветки
			Errorka t;
			t.er = E_INV_CONS;
			t.n_str = tokens[i].n_str;
			t.str = "Expected ; after ";
			t.str += names_variable[tokens[i-1].lvalue];	
			errors.push_back(t);
			i = Go_to_end(i);
		}
	return i;
}
int Synt::Cur_MNumber(Parse_Tree* cur, int i, int j,Parse_Tree* wM, Parse_Tree* wOP)
{
	if(errors.size() == 0)
	{
		// дополняем дерево			

		cur->lex = tokens[i];
		cur->next1 = new Parse_Tree;
		bottoms_trees.back() = cur;
		Parse_Tree* c = cur;
		cur = cur->next1;
		cur->prev = c;
	}
		// переходим на следующий токен с проверкой на концовку
		if(++i >= tokens.size())
			return i;
		// ожидаем знак любой операции
		if(tokens[i].ltype == L_OPERATION )
		{
			switch(tokens[i].lvalue)
			{
			case OP_MOV:
				{
				// ошибка, недопустимый следующий токен 
					Del_ob(cur);// удаление пустой ветки
					Errorka t;
					t.er = E_INV_CONS;
					t.n_str = tokens[i].n_str;
					t.str = "Invalid consistency \"=\"";
					errors.push_back(t);
					i = Go_to_end(i);
					break;
				}
			case OP_END: 
				Del_ob(cur);// удаление пустой ветки
				break;
			default:
				i = Cur_Op(cur, i, j, wM, wOP); 
			}

		}
		else 
		{
			// ошибка, недопустимый следующий токен 
			Del_ob(cur);// удаление пустой ветки
			Errorka t;
			t.er = E_INV_CONS;
			t.n_str = tokens[i].n_str;
			t.str = "Expected ;";
			errors.push_back(t);
			i = Go_to_end(i);
		}
	
	return i;
}
int Synt::Cur_Op(Parse_Tree* cur, int i,int j, Parse_Tree* wM, Parse_Tree* wOP)
{
	if(errors.size() == 0)
	{
		// дополняем дерево
		// заносим токен равенства перед предыдущим операндом
		
		Parse_Tree* c;
		if(tokens[i].lvalue == OP_MINUS ||  tokens[i].lvalue == OP_PLUS)
		{
			wOP = NULL;	
			cur->lex = tokens[i];
			// указатель предыдщего токена для разрыва связи опернда с текущим токеном 
			// при существовании ожидающей операции
			Parse_Tree* pr = cur->prev; 
			c = wM->next1;
			cur->next1 = c;
			cur->prev = wM;
			wM->next1 = cur;
			c->prev = cur;
			if(c->next1 == NULL || c->lex.ltype != L_OPERATION) // если это первая операция в выражении
			{
				
				c->next1 = new Parse_Tree;
				wOP = cur; // текущий токен операции является ожидающей операцией
				cur = c->next1;
				cur->prev = c;
			}
			else
			{
				
				pr->next1 = NULL; // разрывем связь у операнда с текущей операцией
				c->next2 = new Parse_Tree;
				// текущий токен не меняется
				cur = c->next2;
				cur->prev = c;
			}			
		}
		else
		{
			// дополняем дерево
			// заносим токен равенства перед предыдущим операндом (обмен токенами)
		
			cur->lex = cur->prev->lex; // cur имеет операнд
			cur->prev->lex = tokens[i]; 
			wOP = cur->prev;
			bottoms_trees.back() = cur;
			Parse_Tree* c = cur;
				
			if (cur->next1 == NULL)
			{
				cur->next1 = new Parse_Tree;
				cur = cur->next1;
				cur->prev = c;
			}
			else
			{
				cur->next2 = new Parse_Tree;
				cur = cur->next2;
				cur->prev = c;
			}
			
		}
	}
	//Print();
	// переходим на следующий токен с проверкой на концовку
	if(++i >= tokens.size())
		return i;
	// ожидаем переменную или число
	if(tokens[i].ltype == L_VARIABLE )
	{
		i = Cur_MVariable(cur, i, j, wM, wOP);
	}
	else 
	{
		if(tokens[i].ltype == L_NUMBER_FLOAT || tokens[i].ltype == L_NUMBER_INT)
		{
			i = Cur_MNumber(cur, i, j, wM, wOP);
		}
		// ошибка, недопустимый следующий токен 
		else
		{
			Del_ob(cur);// удаление пустой ветки
			Errorka t;
			t.er = E_INV_CONS;
			t.n_str = tokens[i].n_str;
			t.str = "Expected operand";
			errors.push_back(t);
			i = Go_to_end(i);
		}
	}
	return i;
}
int Synt::Cur_OpVarNum(Parse_Tree* cur, int i,int j, Parse_Tree* wM, Parse_Tree* wOP)
{
	return i;
}

int Synt::Go_to_end(int i)	
{
	while(i < tokens.size() - 1 && !(tokens[i].ltype == L_OPERATION && tokens[i].lvalue == OP_END) )
		i++;
	return i;
}

void Synt::Print(int ch)
{
	int i; 
	void (Synt::*send_token)(Parse_Tree*); // указатель на функцию
	if(ch != 0)
		send_token = &Synt::Pr_instructions;
		
	else
	{
		send_token = &Synt::Pr_T;
		cout << "\nErrors:\n";
		for(i = 0; i < errors.size();i++)
		{
			cout << i+1 << ". " << errors[i] ;
			cout <<" on line #" << errors[i].n_str << endl;
		}
		cout << "\nVariables:\n";
		for(i = 0; i < var.size();i++)
		{
			cout << i+1 << ". " << var[i] << endl;
		}		
	cout << "\nTree:\n";
	}
	// для того чтобы не зациклится при наличии ветки next2 и пройти ветку только один раз
	Parse_Tree* n2 = NULL; 
	int ne2 = 0;
	for(i = 0; i < tops_trees.size(); i++)
	{
		cout << endl << i+1 <<". \n" << endl;
		P(tops_trees[i],send_token);
	}
}

void Synt::P(Parse_Tree* c, void (Synt::*send_token)(Parse_Tree*))
{
	if(c->next1 != NULL)
	{
		P(c->next1,send_token);
		(this->*send_token)(c);	
	}
	else
		(this->*send_token)(c);	
	if(c->next2 != NULL)
	{
		cout << "   ";
		P(c->next2,send_token);
	}

}

void Synt::Pr_T(Parse_Tree* c)
{
	cout << "\t";
	switch (c->lex.ltype )
	{
	case L_VARIABLE: 
		cout << names_variable[c->lex.lvalue] << endl; 
		break;
	case L_NUMBER_FLOAT:
		cout << num_const_float[c->lex.lvalue] << endl;
		break;
	case L_NUMBER_INT:
		cout << num_const_int[c->lex.lvalue] << endl;
		break;
	case L_OPERATION:
		switch(c->lex.lvalue)
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
	}
}

int Synt::Find_name(string st)
{
	for(int i = 0; i < var.size(); i++)
	{
		if(var[i].name == st)
			return i;
	}
	// не нашли
	return -1;
}

Synt:: ~Synt()
{
	Parse_Tree * c ;
	for(int i = 0; i < tops_trees.size(); i++)
	{
		c = tops_trees[i];
		Del_expres(c);		
	}
	tops_trees.clear();
	bottoms_trees.clear();
	var.clear();
}

void Synt::Del_expres(Parse_Tree* head)
{
	Parse_Tree * c ;
	if(head != NULL)
	{
		while (head->next1 != NULL)
		{
			c = head->next1;
			if(head->next2 != NULL)
			{
				Del_expres(head->next2 );
			}
			delete head;
			head = NULL;
			head = c;
			head->prev = NULL;
		}

		delete head;
		head = NULL;
	}
}

void Synt::Del_ob(Parse_Tree* cur)
{
	if(cur != NULL)
	{
		if(tops_trees.back() == cur)
			tops_trees.pop_back();
		if(bottoms_trees.back() == cur)
			bottoms_trees.pop_back();
		Parse_Tree* q = cur->prev;
		if(q != NULL) 
		{
			q->next1 = NULL;
			q->next2 = NULL;
		}
		delete cur; // удаление пустой ветки
		cur = NULL;
	}
}


bool Synt::Code_generation (char *path)
{
	if (errors.size() != 0)
		return false;
	file.open("!myfile.asm",ios::out);
	if(!file)
	{
		cerr << "File is not open" << endl;
		system("pause");
		return false;
	}
	file << ";===[ Начало сегмента кода ]==== \n\
			MYCODE: segment .code \n\
			START:	;---[ Точка старта ]----\n\
			push eax\n\
			push ebp\n\
			mov ebp, esp\n";
	Print(1);

	file << "\n;---[ Стандартное завершение программы ]---\n\
		mov AX, 4C00h\n\
		int 21h\n";
	if(num_const_float.size() > 0)
	{
		file << ";===[ Начало сегмента данных ]===\n\
		; ...\n\
		align 16, db 90h\n\
		db '=MY='\n\
		segment .dat\n\
		MYDAT:\n";
	}
	for(int i = 0; i < num_const_float.size(); i++)
	{
		file << "\n F" << i << ": dd " << num_const_float[i];
	}
	return true;
}

void Synt::Pr_instructions(Parse_Tree* c)
{
	static int id_op;
	switch (c->lex.ltype )
	{
	case L_VARIABLE: 
		id_op = c->lex.lvalue;
		file << "\n fld dword[ebp - " << (id_op + 1)*4 << "]";
		break;
	case L_NUMBER_FLOAT:
		file << "\n fld dword[F" << c->lex.lvalue << "]";
		break;
	case L_NUMBER_INT:
		file << "\n mov eax, " << num_const_int[c->lex.lvalue];
		file << "\n mov [ebp - 200], eax" ;
		file << "\n fild dword[ebp - 200]";
		break;
	case L_OPERATION:
		switch(c->lex.lvalue)
		{
		case OP_MOV:
			file << "\n fxch";
			if(var[id_op].type == R_FLOAT)
				file << "\n fstp dword[ebp - " << (id_op + 1)*4 << "]";
			else
				file << "\n fistp dword[ebp - " << (id_op + 1)*4 << "]";
			break;
		case OP_PLUS:
			file << "\n fadd";
			break;
		case OP_MINUS:
			file << "\n fsub";
			break;
		case OP_MUL:
			file << "\n fmul";
			break;
		case OP_DIV:
			file << "\n fdiv";
			break;
		}
		break;
	}
}

