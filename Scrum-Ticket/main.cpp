#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdio>
#include <string>
#include <conio.h>

#define DEF_EMPTY	00
#define DEF_STORY 	01
#define DEF_DESCRIPTION 02
#define DEF_DEPENDENCY 	03
#define DEF_POINTS 	04

using namespace std;

void startParse();
int read(ifstream &);
void story(string);
void description(string);
void dependency(string);
void points(string);
void crash();
void changeState();
void print();

string _story;
string _description; 
string _dependency;
string _points;

ifstream rFile;
ofstream wFile;



int currState = DEF_EMPTY;

int main()
{
	startParse();
	return 0;
}

void startParse()
{
	rFile.open("tasks.txt");
	wFile.open("tasks.html");
	wFile.clear();
	wFile<<"<html><body>"<<endl;
	if (!rFile)
	{
		cout<<"Unable to open file";
	}	
	else
	{
		read(rFile);	
	}
	wFile<<"</html></body>"<<endl;
	rFile.close();
	wFile.close();
}

int read ( ifstream &inFile)
{
	string line;
	string temp;
	char word;

	if(!inFile)
	{
		return 0;
	}
	while(getline(inFile, line, '\n'))
	{
		line += '\n';
		for (int i=0; i<line.length(); i++)
		{
			temp = line[i];
			word = int(line[i]);

			if((word != '\t' && word != '\n') && (currState == DEF_STORY || currState == DEF_EMPTY))
			{
				story(temp);
			}
			else if((word != '\t' && word != '\n') && (currState == DEF_DESCRIPTION))
			{
				description(temp);
			}
			else if((word != '\t' && word != '\n') && (currState == DEF_DEPENDENCY))
			{
				dependency(temp);	
			}
			else if((word != '\t' && word != '\n') && (currState == DEF_POINTS))
			{
				points(temp);
			}
			else if(word == '\t')
			{
				changeState();
			}
			else if(word == '\n')
			{
				if(currState == DEF_POINTS)
				{currState = DEF_STORY;}
			}
			if(word == '~')
			{
				_story.clear();
				_description.clear();
				_dependency.clear();
				_points.clear();
			}
		}
	}
	return 1;
}

void story( string data)
{
	currState = DEF_STORY;
	_story += data;
}

void description( string data)
{
	currState = DEF_DESCRIPTION;
	_description += data;	
}

void dependency( string data)
{
	currState = DEF_DEPENDENCY;
	_dependency += data;
}

void points( string data)
{
	currState = DEF_POINTS;
	_points += data;
	print();
}

void crash()
{
	cout<<"**************************************************\n";
	exit(0);
}

void changeState()
{
	if(currState == DEF_STORY)
	{
		currState = DEF_DESCRIPTION;
	}
	else if(currState == DEF_DESCRIPTION)
	{
		currState = DEF_DEPENDENCY;	
	}
	else if(currState == DEF_DEPENDENCY)
	{
		currState = DEF_POINTS;
	}
	else
	{
		crash();
	}
}

void print()
{
	wFile<<"<br><br>"<<endl; 
	wFile<<"<div align='center'>"<<endl;
	wFile<<"<table border='4'>"<<endl;
	wFile<<"<tr><th width='500'><div align='left'>" + _story + "</div></th></tr>"<< endl;
	wFile<<"<tr><td height='150'><div align='center'>" + _description + "</div></td></tr>"<<endl;
	wFile<<"<tr><td height='50'><div align='center'>"+ _dependency +" "+ _points +"</div></td></tr>"<<endl;
	wFile<<"</table></div>"<<endl;
	_description.clear();
	_dependency.clear();
	_points.clear();
}
