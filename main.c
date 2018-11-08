//TODO: deal with objects in corners, swapping back and forth. if xdirection and ydirection
			//are not equal to 0, just change the direction to directly away from corners.
			//thats 4 different cases though.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "input_error.h"

typedef struct Robot {
	int currentX;			//start point
	int currentY;
	int finishX;
	int finishY;			//end point
	int XDirection;
	int YDirection;			//current direction to potential position
	int firstPositionX;
	int firstPositionY;		//prev spot before object 1 'hit' robot
	int secondPositionX;
	int secondPositionY;	//prev spot before object 2 'hit' robot
	int flag1;				//means object 1 has 'hit' the robot
	int flag2;				//means object 2 has 'hit' the robot
} robot;

typedef struct Obstacle {
	int currentX;
	int currentY; 
	int speed;
	int XDirection;
	int YDirection;
} obstacle;

typedef struct Vertex {
	int Xcoordinate;
	int Ycoordinate;
	char value;		//this is for what is in the square. (robot, finish, obstacle)
	int number; 	//this is the number in the list of vertices.
	char backup;	//this is to deal with the start and finish which will stay where they are,
					//but if a robot or obstacle lands on it, then we want R or O to show up instead of 
					//the start or finish icon. Then we can replace it afterwards.
	char backup2;
	char backup3;
	
	struct Vertex* LUP;
	struct Vertex* UP;
	struct Vertex* RUP;
	struct Vertex* RIGHT;			//THESE 8 EDGES ARE THE NEIGHBORS THAT EACH VERTEX WILL HAVE.
	struct Vertex* RDOWN;
	struct Vertex* DOWN;
	struct Vertex* LDOWN;
	struct Vertex* LEFT;
	
} vertex;

void loadFile(int, char*, int*, int*, obstacle*, obstacle*, robot*);	//DONE
int validPair(char*, int, int*, int*);							//DONE
int validDirection(char*, int*, int*);							//DONE
void outputGrid(vertex*, int);									//DONE
void moveObstacle(vertex*, robot*, obstacle*, int, int);		//DONE
vertex* buildGraph(int);										//DONE
vertex* findVertexAddress(vertex*, int, int, int);				//DONE
void placeIcons(vertex*, robot*, obstacle*, obstacle*, int);	//DONE
void play(vertex*, robot*, obstacle*, obstacle*, int);			//DONE
void moveWinston(vertex*, robot*, int, obstacle*, obstacle*);	//DONE
int checkSwap(vertex*, int, vertex*, robot*, obstacle*, obstacle*, vertex*);			
int doubleCheck(vertex*, int, vertex*, vertex*, robot*, obstacle*, obstacle*);
vertex* rotateLeft(vertex*, vertex*);							//DONE
vertex* rotateRight(vertex*, vertex*);							//DONE
void getDirection(vertex*, vertex*, int*, int*);


int main(int argc, char** argv)
{
	//create all variables and objects here
	//int* queue= NULL;

	int totalAmount = 0;
	int roomLength = 0;
	robot* winston = (robot*)malloc(sizeof(robot));
	obstacle* firstObstacle = (obstacle*)malloc(sizeof(obstacle));
	obstacle* secondObstacle = (obstacle*)malloc(sizeof(obstacle));
	
	loadFile(argc, argv[1], &roomLength, &totalAmount, firstObstacle, secondObstacle, winston);	//initialize all objects
	vertex* root = buildGraph(roomLength);		//create and initialize all vertices.
	placeIcons(root, winston, firstObstacle, secondObstacle, roomLength);		//places initial positions of everything on the grid
	play(root, winston, firstObstacle, secondObstacle, roomLength);
	free(root);
	free(winston);
	free(firstObstacle);
	free(secondObstacle);
	return 0;
}

/***********************************************************************************/
/***********************************************************************************/

void loadFile(int numCommandArgs, char* fileString, int* roomLength, int* amountNodes, obstacle* firstObstacle, obstacle* secondObstacle,
					robot* winston)
{
	if(numCommandArgs != 2)
	{
        printf("Incorrect number of command line arguments");
        exit(INCORRECT_NUMBER_OF_COMMAND_LINE_ARGUMENTS);
    }
    
    FILE* inputFile = fopen(fileString, "r");
    if(inputFile == NULL)
    {
        printf("File failed to open\n");
        exit(FILE_FAILED_TO_OPEN);
    }
        
    //DECLARE VARIABLES
    size_t limit = 15;                    //this is for getline function. Idk what this is tbh.
    char* numberStr = (char*)malloc(sizeof(char) * 15);         //temporary string to hold lines from file
    int result=-1;      //used to check validity of each line
    int first=0;        //holds the number before the comma on each line
    int second=0;        //holds the number after the comma on each line
    int tempSpeed=-1;
        
        
        
    //BEGIN THE FUN
    if(getline(&numberStr, &limit, inputFile)!=-1)
    {
    	*roomLength = atoi(numberStr);
        *amountNodes = *roomLength * *roomLength;		//squares the number received from first line of input file.
    }
    
    if(*amountNodes == 0)
	{
		printf("Parsing error line 1");
        exit(PARSING_ERROR_INVALID_FORMAT);
    }
    else if(*amountNodes == -1)
    {
    	printf("empty file");
        exit(PARSING_ERROR_EMPTY_FILE);
    }
    //check validity and store information where it's needed.
    int lineNum=2;
    for(lineNum=2;getline(&numberStr, &limit, inputFile)!=-1; lineNum++)
    {
    	int i=0;
        for(i=0; numberStr[i] != '\n' && numberStr[i] != EOF; i++);
        numberStr[i] = '\0';
        if(numberStr[0] != '\0')
        {
            //check validity here and build graph/map
            //with every index, we are on a new line so I use a switch statement to deal with
            //different lines. we've already got the first line and stored it in roomLength above
            //so there are only 8 lines left in the file. if it gets to the default case,
            //then that means the input file is not formatted properly so we exit with error.
			switch(lineNum)
			{
				case 2:		//robot starting location
					result = validPair(numberStr, *roomLength, &first, &second);
					if(result)
					{
						winston->currentX = first;
						winston->currentY = second;
					}
					else
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					
					break;
				case 3:		//robot exit location
					result = validPair(numberStr, *roomLength, &first, &second);
					if(result)
					{
						winston->finishX = first;
						winston->finishY = second;
						winston->flag1 = 0;
						winston->flag2 = 0;
					}
					else
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					
					break;
				case 4:		//first obstacle starting location
					result = validPair(numberStr, *roomLength, &first, &second);
					if(result)
					{
						firstObstacle->currentX = first;
						firstObstacle->currentY = second;
					}
					else
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					
					
					break;
				case 5:		//first obstacle speed
					tempSpeed = atoi(numberStr);
	
					if(tempSpeed == -1)
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					else
						firstObstacle->speed = tempSpeed;
					
					tempSpeed = -1;		//reset tempSpeed for next obstacle speed
					break;
				case 6:		//first obstacle direction
					result = validDirection(numberStr, &first, &second);
					if(result)
					{
						firstObstacle->XDirection = first;
						firstObstacle->YDirection = second;
					}
					else
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					
					break;
				case 7:		//second obstacle starting location
					result = validPair(numberStr, *roomLength, &first, &second);
					if(result)
					{
						secondObstacle->currentX = first;
						secondObstacle->currentY = second;
					}
					else
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					
					break;
				case 8:		//second obstacle speed
					tempSpeed = atoi(numberStr);
	
					if(tempSpeed == -1)
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					else
						secondObstacle->speed = tempSpeed;
					
					break;
				case 9:		//second obstacle direction
					result = validDirection(numberStr, &first, &second);
					if(result)
					{
						secondObstacle->XDirection = first;
						secondObstacle->YDirection = second;
					}
					else
					{
						printf("Parsing error line %d", lineNum);
						exit(PARSING_ERROR_INVALID_FORMAT);
					}
					
					break;
				default:
					printf("Parsing error default");
					exit(PARSING_ERROR_INVALID_FORMAT);
					break;
        	}
        }
        else
        	exit(PARSING_ERROR_INVALID_FORMAT);		//this means its an empty line
    }
    fclose(inputFile);
}

/******************************************************************************/
/******************************************************************************/

int validPair(char* line, int roomLength, int* first, int* second)
{
    int index=0;
    char* number = (char*)malloc(sizeof(char) * 10);
    number[0]='\0';
    int loops=0;
    char check='.';

    //first loop deals with '(' and the first number.
    //second loop deals with ',' and the second number.
    //third loop deals with ')'.
    for(loops=0; loops < 3; loops++)
    {
        switch(loops)
        {
            case 0:
                check = '(';
                break;
            case 1:
                check = ',';
                break;
            case 2:
                check = ')';  
                break;    
        }
        //parentheses and comma check
        if(line[index] != check)
            return 0;

        if(loops != 2)
        {
            //first and second vertex
            if(!isdigit(line[++index]))
                return 0;
            else
                number[0] = line[index];
            int i;
            for(i=0;isdigit(line[++index]) && i<6;i++)
                number[i+1] = line[index];
            
            if(i>5)     //UINT-MAX is 5 digits, so if i reaches more than 5 digits, 
                return 0;   //then we have a number that's too big
            else
                number[i+1]='\0';


			//store numbers into variables that are used back in loadFile
            if(loops==0)
                *first = atoi(number);
            if(loops==1)
                *second = atoi(number);
            //valid vertex number?
            if(loops<2 && (atoi(number) > roomLength || atoi(number) < 1))
        	{
        		printf("Invalid vertex coordinate\n");
        		exit(INTEGER_IS_NOT_A_VERTEX);
        	}
            
        }
    }
    
    free(number);
    
    if(line[++index] != '\0' && line[index] != EOF)
        return 0;
  
    return 1;
}

/******************************************************************************/
/******************************************************************************/

int validDirection(char* line, int* first, int* second)
{
	int index=0;
    int number=-2;
    int loops=0;
    char check='.';
    

    //first loop deals with '(' and the first number.
    //second loop deals with ',' and the second number.
    //third loop deals with ')'
    for(loops=0; loops < 3; loops++)
    {
        switch(loops)
        {
            case 0:
                check = '(';
                break;
            case 1:
                check = ',';
                break;
            case 2:
                check = ')';  
                break;    
        }
        //parentheses and comma check
        if(line[index] != check)
            return 0;

        if(loops != 2)
        {
            //first and second number
                
                
            switch(line[++index])
            {
            	case '0':
            		number = 0;
            		break;
            	case '+':
            		if(line[++index] != '1')
            			return 0;
            		else
            			number = 1;
            		break;
            	case '-':
            		if(line[++index] != '1')
            			return 0;
            		else
            			number = -1;
            		break;
            	default:
            		return 0;
            }

			index++;
			//STORE NUMBERS WHERE THEY NEED TO GO
            if(loops==0)
                *first = number;
            if(loops==1)
                *second = number;
            
        }
    }
    
    if(line[++index] != '\0' && line[index] != EOF)
        return 0;
  
    return 1;
}

/*********************************************************************************/
/*********************************************************************************/

void outputGrid(vertex* root, int roomLength)		//might need the root sent here to print their values.
{
	if(roomLength > 20)
	{}
	else
	{
		int index=0;
		int number=0;
		//for the number of rows (which is equal to roomLength)
		for(index=0; index<roomLength; index++)
		{ 
			//print separator line before each row.
			int i=0;
			for(i=0; i<roomLength; i++)
				printf("----");
			printf("-\n");
		
			for(i=0; i<roomLength; i++)
			{
				printf("| %c ", (root+number)->value);		//replace this with the value of that node (robot, obstacle, etc.)
				number++;									//make sure there are 3 characters after '|' so the separator row lines up.
			}
			printf("|\n");
		}
		//print last separator line.
		for(index=0; index<roomLength; index++)
			printf("----");
		printf("-\n\n*****************************************************************************************************\n\n");
	}
}


/*********************************************************************************/
/*********************************************************************************/

vertex* buildGraph(int roomLength)
{
	//malloc all vertices and connect neighbors.
	//return root of list.
	int totalAmount = roomLength * roomLength;
	vertex* root = (vertex*)malloc(sizeof(vertex) * totalAmount);
	int Xcoordinate = 1;
	int Ycoordinate = 1;
	
	int index=0;
	for(index=0; index<totalAmount; index++)
	{
		(root+index)->Xcoordinate = Xcoordinate;
		(root+index)->Ycoordinate = Ycoordinate;
		(root+index)->value = ' ';
		(root+index)->backup = ' ';
		(root+index)->backup2 = ' ';
		(root+index)->backup3 = ' ';
		(root+index)->number = index+1;
		
		Xcoordinate++;
		if(Xcoordinate>roomLength)
		{
			Xcoordinate = 1;
			Ycoordinate++;
		}
	}
	
	Xcoordinate = 1;
	Ycoordinate = 1;
	for(index=0; index<totalAmount; index++)
	{	
		//LUP
		if(Xcoordinate == 1 || Ycoordinate == 1)
			(root+index)->LUP = NULL;
		else
			(root+index)->LUP = findVertexAddress(root, Xcoordinate-1, Ycoordinate-1, roomLength);
		
		
		//UP
		if(Ycoordinate == 1)
			(root+index)->UP = NULL;
		else
			(root+index)->UP = findVertexAddress(root, Xcoordinate, Ycoordinate-1, roomLength);
		
		
		//RUP
		if(Xcoordinate == roomLength || Ycoordinate == 1)
			(root+index)->RUP = NULL;
		else
			(root+index)->RUP = findVertexAddress(root, Xcoordinate+1, Ycoordinate-1, roomLength);
		
		
		//RIGHT
		if(Xcoordinate == roomLength)
			(root+index)->RIGHT = NULL;
		else
			(root+index)->RIGHT = findVertexAddress(root, Xcoordinate+1, Ycoordinate, roomLength);
		
		
		//RDOWN
		if(Xcoordinate == roomLength|| Ycoordinate == roomLength)
			(root+index)->RDOWN = NULL;
		else
			(root+index)->RDOWN = findVertexAddress(root, Xcoordinate+1, Ycoordinate+1, roomLength);
		
		
		//DOWN
		if(Ycoordinate == roomLength)
			(root+index)->DOWN = NULL;
		else
			(root+index)->DOWN = findVertexAddress(root, Xcoordinate, Ycoordinate+1, roomLength);
		
		
		//LDOWN
		if(Xcoordinate == 1 || Ycoordinate == roomLength)
			(root+index)->LDOWN = NULL;
		else
			(root+index)->LDOWN = findVertexAddress(root, Xcoordinate-1, Ycoordinate+1, roomLength);
		
		
		//LEFT
		if(Xcoordinate == 1)
			(root+index)->LEFT = NULL;
		else
			(root+index)->LEFT = findVertexAddress(root, Xcoordinate-1, Ycoordinate, roomLength);
		
		Xcoordinate++;
		if(Xcoordinate>roomLength)
		{
			Xcoordinate = 1;
			Ycoordinate++;
		}
		
		
	}
	
	return root;
}

/*********************************************************************************/
/*********************************************************************************/

vertex* findVertexAddress(vertex* root, int Xcoordinate, int Ycoordinate, int roomLength)
{
	vertex* temp = root;
	int desired = ((Ycoordinate-1) * roomLength) + Xcoordinate;
	while(temp != NULL && temp->number != desired)
		temp++;			//this was root=root->next;

	return temp;
}

/*********************************************************************************/
/*********************************************************************************/

void placeIcons(vertex* root, robot* winston, obstacle* firstObstacle, obstacle* secondObstacle, int roomLength)
{
	vertex* temp = NULL;
	temp = findVertexAddress(root, winston->currentX, winston->currentY, roomLength);
	temp->value = 'R';
	temp->backup = 'F';
	temp = findVertexAddress(root, winston->finishX, winston->finishY, roomLength);
	temp->value = 'L';
	temp = findVertexAddress(root, firstObstacle->currentX, firstObstacle->currentY, roomLength);
	if(temp->value != ' ')
		temp->backup = temp->value;
	temp->value = 'O';
	temp = findVertexAddress(root, secondObstacle->currentX, secondObstacle->currentY, roomLength);
	if(temp->value != ' ')
	{
		temp->backup2 = temp->backup;
		temp->backup = temp->value;
	}
	temp->value = 'O';
	outputGrid(root, roomLength);
	
	if(firstObstacle->XDirection != 0 && firstObstacle->YDirection != 0)
	{
		if(firstObstacle->currentX == 1 && firstObstacle->currentY == 1)
		{
			firstObstacle->XDirection = 1;
			firstObstacle->YDirection = 1;
		}
		else if(firstObstacle->currentX == 1 && firstObstacle->currentY == roomLength)
		{
			firstObstacle->XDirection = 1;
			firstObstacle->YDirection = -1;
		}
		else if(firstObstacle->currentX == roomLength && firstObstacle->currentY == 1)
		{
			firstObstacle->XDirection = -1;
			firstObstacle->YDirection = 1;
		}
		else if(firstObstacle->currentX == roomLength && firstObstacle->currentY == roomLength)
		{
			firstObstacle->XDirection = -1;
			firstObstacle->YDirection = -1;
		}
	}
	//second
	if(secondObstacle->XDirection != 0 && secondObstacle->YDirection != 0)
	{
		if(secondObstacle->currentX == 1 && secondObstacle->currentY == 1)
		{
			secondObstacle->XDirection = 1;
			secondObstacle->YDirection = 1;
		}
		else if(secondObstacle->currentX == 1 && secondObstacle->currentY == roomLength)
		{
			secondObstacle->XDirection = 1;
			secondObstacle->YDirection = -1;
		}
		else if(secondObstacle->currentX == roomLength && secondObstacle->currentY == 1)
		{
			secondObstacle->XDirection = -1;
			secondObstacle->YDirection = 1;
		}
		else if(secondObstacle->currentX == roomLength && secondObstacle->currentY == roomLength)
		{
			secondObstacle->XDirection = -1;
			secondObstacle->YDirection = -1;
		}
	}
}

/*********************************************************************************/
/*********************************************************************************/

void play(vertex* root, robot* winston, obstacle* firstObstacle, obstacle* secondObstacle, int roomLength) 
{
	while(winston->currentX != winston->finishX || winston->currentY != winston->finishY)
	{
		moveObstacle(root, winston, firstObstacle, roomLength, 1);
		moveObstacle(root, winston, secondObstacle, roomLength, 2);
		moveWinston(root, winston, roomLength, firstObstacle, secondObstacle);
		winston->flag1 = 0;
		winston->flag2 = 0;
		if(winston->currentX == firstObstacle->currentX && winston->currentY == firstObstacle->currentY)
		{
			outputGrid(root, roomLength);
			printf("\n\nYou lose!\n\n");
			return;
		}
		else if(winston->currentX == secondObstacle->currentX && winston->currentY == secondObstacle->currentY)
		{
			outputGrid(root, roomLength);
			printf("\n\nYou lose!\n\n");
			return;
		}
		
		outputGrid(root, roomLength);
	}
	printf("\n\n\nYOU WIN!!!!!!!!!!!\n\n");
}

/*********************************************************************************/
/*********************************************************************************/

void moveObstacle(vertex* root, robot* winston, obstacle* object, int roomLength, int obstacle)
{
	vertex* next=NULL;
	int found = 0;
	vertex* current = findVertexAddress(root, object->currentX, object->currentY, roomLength);
	int move = 0;
	for(move=0; move<object->speed; move++)
	{
		while(!found)
		{
			//Get pointer for current direction
			if(object->XDirection == -1 && object->YDirection == -1)
				next = current->LUP;
			else if(object->XDirection == 0 && object->YDirection == -1)
				next = current->UP;
			else if(object->XDirection == 1 && object->YDirection == -1)
				next = current->RUP;
			else if(object->XDirection == 1 && object->YDirection == 0)
				next = current->RIGHT;
			else if(object->XDirection == 1 && object->YDirection == 1)
				next = current->RDOWN;
			else if(object->XDirection == 0 && object->YDirection == 1)
				next = current->DOWN;
			else if(object->XDirection == -1 && object->YDirection == 1)
				next = current->LDOWN;
			else if(object->XDirection == -1 && object->YDirection == 0)
				next = current->LEFT;
		
			if(next == NULL)
			{
				//check for wall bounces.
				if(object->currentX == 1 || object->currentX == roomLength)
					object->XDirection = object->XDirection * (-1);
				 
				if(object->currentY == 1 || object->currentY == roomLength)
						object->YDirection = object->YDirection * (-1);
			}
			else
			{
				//place O icon where it needs to go.
				if(next->value == 'R' && move == object->speed - 1)
				{
					next->backup3 = next->backup2;
					next->backup2 = next->backup;
					next->backup = current->value;
					if(obstacle == 1)
						winston->flag1 = 1;
					else
						winston->flag2 = 1;
						
					if(obstacle == 1)
					{
						winston->firstPositionX = current->Xcoordinate;
						winston->firstPositionY = current->Ycoordinate;
					}
					else
					{
						winston->secondPositionX = current->Xcoordinate;
						winston->secondPositionY = current->Ycoordinate;
				}
				}
				else
				{
					next->backup3 = next->backup2;
					next->backup2 = next->backup;
					next->backup = next->value;
					next->value = current->value;
				}
				current->value = current->backup;
				current->backup = current->backup2;
				current->backup2 = current->backup3;
				current->backup3 = ' ';
				object->currentX = next->Xcoordinate;
				object->currentY = next->Ycoordinate;
				current = next;
				next = NULL;
				found=1;
			}
		}
		
		found=0;
		
	}
}

/*********************************************************************************/
/*********************************************************************************/

void moveWinston(vertex* root, robot* winston, int roomLength, obstacle* firstObstacle, obstacle* secondObstacle)
{
	//make a decision
		//check for obstacle and NULL
		//check for collisions
			//if we need to dodge, compare differences in x and y to dodge in closer direction.
		//check for walls
		//replace char in backup (start icon)
	vertex* next = NULL;
	vertex* current = findVertexAddress(root, winston->currentX, winston->currentY, roomLength);
	winston->XDirection = 0;
	winston->YDirection = 0;
	
	//initially set direction
		if(winston->currentX > winston->finishX)
			winston->XDirection = -1;
		else if(winston->currentX < winston->finishX)
			winston->XDirection = 1;
		else
			winston->XDirection = 0;
		
		if(winston->currentY > winston->finishY)
			winston->YDirection = -1;
		else if(winston->currentY < winston->finishY)
			winston->YDirection = 1;
		else
			winston->YDirection = 0;
		
	//first choice, check straight towards finish.	
	if(winston->XDirection == -1 && winston->YDirection == -1)
		next = current->LUP;
	else if(winston->XDirection == 0 && winston->YDirection == -1)
		next = current->UP;
	else if(winston->XDirection == 1 && winston->YDirection == -1)
		next = current->RUP;
	else if(winston->XDirection == 1 && winston->YDirection == 0)
		next = current->RIGHT;
	else if(winston->XDirection == 1 && winston->YDirection == 1)
		next = current->RDOWN;
	else if(winston->XDirection == 0 && winston->YDirection == 1)
		next = current->DOWN;
	else if(winston->XDirection == -1 && winston->YDirection == 1)
		next = current->LDOWN;
	else if(winston->XDirection == -1 && winston->YDirection == 0)
		next = current->LEFT;
	
	int found = doubleCheck(root, roomLength, next, current, winston, firstObstacle, secondObstacle);
	
	vertex* temp=NULL;
	//rotate once to either side, then try rotating twice to either side.
	if(found)
		return;
	else
	{	
		if(winston->XDirection == 0 || winston->YDirection == 0)
		{
			temp = rotateLeft(next, current);
			if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
				return;
			temp = rotateRight(next, current);
			if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
				return;
			
			//rotate twice away from initial
			temp = rotateLeft(next, current);
			temp = rotateLeft(temp, current);
			if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
				return;
			temp = rotateRight(next, current);
			temp = rotateRight(temp, current);
			if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
				return;
		}
		//this is handling which way do I rotate first?
		else if(abs(winston->currentX - winston->finishX) < abs(winston->currentY - winston->finishY))
		{
			if(winston->XDirection == winston->YDirection)
			{
				temp = rotateRight(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				temp = rotateLeft(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				
				temp = rotateRight(next, current);
				temp = rotateRight(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				temp = rotateLeft(next, current);
				temp = rotateLeft(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
			}
			else
			{
				temp = rotateLeft(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				temp = rotateRight(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
					
				temp = rotateLeft(next, current);
				temp = rotateLeft(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				temp = rotateRight(next, current);
				temp = rotateRight(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
			}
		}
		else
		{
			if(winston->XDirection == winston->YDirection)
			{
				temp = rotateLeft(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				temp = rotateRight(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
					
				temp = rotateLeft(next, current);
				temp = rotateLeft(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				temp = rotateRight(next, current);
				temp = rotateRight(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
			}
			else
			{
				temp = rotateRight(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				temp = rotateLeft(next, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
					
				temp = rotateRight(next, current);
				temp = rotateRight(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
					
				temp = rotateLeft(next, current);
				temp = rotateLeft(temp, current);
				if(doubleCheck(root, roomLength, temp, current, winston, firstObstacle, secondObstacle))
					return;
				
			}
		}
	}
	
	
	
}

/*********************************************************************************/
/*********************************************************************************/

int checkSwap(vertex* root, int roomLength, vertex* current, robot* winston, obstacle* firstObstacle, obstacle* secondObstacle, vertex* potential)
{
	int objx=0;
	int objy=0;
	int robx=0;
	int roby=0;
	vertex* old = NULL;
	
	if(potential->Xcoordinate == winston->firstPositionX && potential->Ycoordinate == winston->firstPositionY)
	{
		if(winston->currentX == firstObstacle->currentX && winston->currentY == firstObstacle->currentY)
			return 1;
	}
	else if(potential->Xcoordinate == winston->secondPositionX && potential->Ycoordinate == winston->secondPositionY)
	{
		if(winston->currentX == secondObstacle->currentX && winston->currentY == secondObstacle->currentY)
			return 1;
	}
	
	
	getDirection(potential, current, &robx, &roby);
	//deal with object chasing robot.
	if(winston->flag1)
	{
		//IF SAME DIRECTION OR OPPOSITE, DODGE
		//direction of object
		old = findVertexAddress(root, winston->firstPositionX, winston->firstPositionY, roomLength);
		getDirection(current, old, &objx, &objy);		//this is current for robot, next for object
		if(robx == objx && roby == objy)													
		{
			//IF SPEED IS GREATER ON OBJECT, THEN return 1;
			if(firstObstacle->speed > 1)
				return 1;
		}
		else if(robx * (-1) == objx && roby * (-1) == objy)									
		{
			return 1;
		}
	}
	
	if(winston->flag2)
	{
		//IF SAME DIRECTION OR OPPOSITE
		old = findVertexAddress(root, winston->secondPositionX, winston->secondPositionY, roomLength);
		getDirection(current, old, &objx, &objy);		//this is current for robot, next for object
		if(robx == objx && roby == objy)													
		{
			//IF SPEED IS GREATER ON OBJECT, THEN return 1;
			if(secondObstacle->speed > 1)
				return 1;
		}
		else if(robx * (-1) == objx && roby * (-1) == objy)												
		{
			return 1;
		}
	}
	return 0;
}

/*********************************************************************************/
/*********************************************************************************/

int doubleCheck(vertex* root, int roomLength, vertex* next, vertex* current, robot* winston, obstacle* firstObstacle, obstacle* secondObstacle)
{
	//CHECK FOR OFF MAP AND OBJECT BEING IN THE SQUARE
	if(next != NULL && next->value != 'O')
	{
		//CHECK FOR COLLISIONS
		if(!checkSwap(root, roomLength, current, winston, firstObstacle, secondObstacle, next))
		{
			next->backup3 = next->backup2;
			next->backup2 = next->backup;
			next->backup = next->value;
			next->value = 'R';
			current->value = current->backup;
			current->backup = current->backup2;
			current->backup2 = current->backup3;
			current->backup3 = ' ';
			winston->currentX = next->Xcoordinate;
			winston->currentY = next->Ycoordinate;
			current = next;
			next = NULL;
			return 1;
		}
	}
	return 0;
}

/*********************************************************************************/
/*********************************************************************************/

vertex* rotateLeft(vertex* next, vertex* current)
{
	//ROTATE POINTER LEFT
	if(next == current->LUP)
		return current->LEFT;
	else if(next == current->LEFT)
		return current->LDOWN;
	else if(next == current->LDOWN)
		return current->DOWN;
	else if(next == current->DOWN)
		return current->RDOWN;
	else if(next == current->RDOWN)
		return current->RIGHT;
	else if(next == current->RIGHT)
		return current->RUP;
	else if(next == current->RUP)
		return current->UP;
	else if(next == current->UP)
		return current->LUP;
	else 
		return NULL;
		
}

/*********************************************************************************/
/*********************************************************************************/

vertex* rotateRight(vertex* next, vertex* current)
{
	//ROTATE POINTER RIGHT
	if(next == current->LUP)
		return current->UP;
	else if(next == current->LEFT)
		return current->LUP;
	else if(next == current->LDOWN)
		return current->LEFT;
	else if(next == current->DOWN)
		return current->LDOWN;
	else if(next == current->RDOWN)
		return current->DOWN;
	else if(next == current->RIGHT)
		return current->RDOWN;
	else if(next == current->RUP)
		return current->RIGHT;
	else if(next == current->UP)
		return current->RUP;
	else 
		return NULL;
}

/*********************************************************************************/
/*********************************************************************************/

void getDirection(vertex* next, vertex* current, int* x, int* y)
{
	//TURN POINTER INTO DIRECTION
	if(next == current->LUP)
	{ *x = -1; *y = -1; }
	else if(next == current->LEFT)
	{ *x = -1; *y = 0; }
	else if(next == current->LDOWN)
	{ *x = -1; *y = 1; }
	else if(next == current->DOWN)
	{ *x = 0; *y = 1; }
	else if(next == current->RDOWN)
	{ *x = 1; *y = 1; }
	else if(next == current->RIGHT)
	{ *x = 1; *y = 0; }
	else if(next == current->RUP)
	{ *x = 1; *y = -1; }
	else if(next == current->UP)
	{ *x = 0; *y = -1; }
}