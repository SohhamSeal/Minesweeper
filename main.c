#include <stdio.h>
#include <stdlib.h>
#include <time.h>//to get the current time and other things
#include <windows.h>

int **a;
char **b;//a stores the actual board values; b holds the opened tiles only, others are by default 'unopened'
int mine,size_x,size_y,more;//mine holds the number of mines. size_x and size_y holds the dimensions of the minefield
int mode;
int tstart,tnow;//starting time and current time resp
int firstX,firstY;

//for scoreboard
char names[5][50];
int scores[5];
char fileNames[3][13]={"BEGINNER.txt","MODERATE.txt","EXPERT.txt"};

//these are the ASCII characters for the vertical and horizontal borders, respectively
char border_v=186,border_h=205,explode='*',deactivate='~',empty=32,unopened=177;//(176,177,178,219)

/*
explode is the mine display character.
deactivate is the mine display character if the player wins and is able to deactivate all the mines (deactivated mines)
empty is the display character when there are no numbers,i.e, the tile does not have any surrounding mines
unopened is the display character for a choose-able tile
*/


int state; //a flag to detect the end of game play. state is 1 if the player has lost; 2 if the player has won
char name[30]; //string to hold name of player

void displayRules();  //to display rules
void scoreboard();    //display scoreboard
void readFile();      //to read from file
int sBChange();       //to read and write to files
void setup();         //to declare the 2d arrays, and set the border in second matrix
void initialize();    //to plant the mines randomly
void adjustVals();    //to set the numbers in each tile
void openUp(int,int); //to select the tiles once an empty tile is chosen
void display();       //to display the minefield
int check();          //to check if the player has won. Returns 0 if not, else 1 and sets state to 2
void play();          //function for continuous input from the user and checking for the end of game play

void SetColor(int ForgC){
     WORD wColor;
     //This handle is needed to get the current background attribute

     HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
     CONSOLE_SCREEN_BUFFER_INFO csbi;
     //csbi is used for wAttributes word

     if(GetConsoleScreenBufferInfo(hStdOut, &csbi))
     {
          //To mask out all but the background attribute, and to add the color
          wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
          SetConsoleTextAttribute(hStdOut, wColor);
     }
     return;
}


//FUNCTION DEFINITIONS

void displayRules()
{
    system("cls");
    printf("                                             ___   _     _     ____  __ \n");
    printf("=========================================== | |_) | | | | |   | |_  ( (` ===========================================\n");
    printf("                                            |_| \\ \\_\\_/ |_|__ |_|__ _)_)\n\n");
    printf("\t\t\t\t\t   (Press enter to read new rule)\n\n");
    getchar();
    printf("(1). The objective of the game is to expose all the tiles on the minefield without hitting any mines.\n");
    getchar();
    printf("\n(2). Choose a tile on the minefield by entering the row and column numbers respectively.\n");
    getchar();
    printf("\n(3). The numbers on the board represent how many bombs are adjacent to a square. \n  For example, if a tile has a \"3\" on it, then there are 3 bombs diagonally or orthogonally surrounding the tile.\n");
    getchar();
    printf("\n(4). Avoid all the bombs and expose all the empty spaces to win Minesweeper.\n");
    getchar();
    printf("\n\nYOU ARE READY!! LET'S GO!!!");
    getchar();
}

void readFile(int m)
{
    FILE *fpr;
    int i;
    fpr=fopen(fileNames[m-1],"r");
    for(i=0;i<5;i++)
    {
        //fgets(names[i],30,fpr);
        fscanf(fpr,"%[^\n]s",names[i]);
        fscanf(fpr,"%d%*c",&scores[i]);
    }
    fclose(fpr);
}

void scoreboard()
{
	int i,j;
    system("cls");
    printf("\t   ___     ___     ___     ___     ___     ___     ___     ___     ___     ___\n");
    printf("\t  / __|   / __|   / _ \\   | _ \\   | __|   | _ )   / _ \\   /   \\   | _ \\   |   \\\n");
    printf("\t  \\__ \\  | (__   | (_) |  |   /   | _|    | _ \\  | (_) |  | - |   |   /   | |) |\n");
    printf("\t  |___/   \\___|   \\___/   |_|_\\   |___|   |___/   \\___/   |_|_|   |_|_\\   |___/\n");
    printf("\t_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"| \n");
    printf("\t\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\n");
    printf("  ____________________________________________________________________________________________\n");
    printf("  ============================================================================================\n\n");
    for(i=0;i<3;i++)
    {
        readFile(i+1);
        printf("\n  --------------------------------------------------------------------------------------------\n");
        printf("\t\t\t\t\t%s\b\b\b\b    ",fileNames[i]);
        printf("\n  --------------------------------------------------------------------------------------------\n\n");
        printf("\tRANK\t\tTIME TAKEN\t\tNAME\n\n");
        for(j=0;j<5;j++)
            printf("\t(%d)\t\t%5d sec\t\t%s\n",(j+1),scores[j],names[j]);
        printf("\n");
    }
    printf("  ____________________________________________________________________________________________\n");
    printf("  ============================================================================================");
    getchar();
}

int sBChange()//returns 1 if the new score is in the first 5, otherwise 0
{
    FILE *fpw;
    readFile(mode);
    if(scores[4]<tnow)
        return 0;
    int i=4,j;
    while(i>=0 && scores[i]>tnow)
        i--;
    i++;
    for(j=4;j>=i+1;j--)
    {
        scores[j]=scores[j-1];
        strcpy(names[j],names[j-1]);
    }
    scores[i]=tnow;
    strcpy(names[i],name);
    fpw=fopen(fileNames[mode-1],"w");
    for(i=0;i<5;i++)
        fprintf(fpw,"%s\n%d\n",names[i],scores[i]);
    fclose(fpw);
    return 1;
}

void setup()
{
    state=0;
    tstart=-1;
    int i,j;
    //declaring the array sizes
    a=(int**)calloc(size_x,sizeof(int*));
    for(i=0;i<size_x;i++)
        a[i]=(int*)calloc(size_y,sizeof(int));
    b=(char**)calloc(size_x+2,sizeof(char*));
    for(i=0;i<size_x+2;i++)
        b[i]=(char*)calloc(size_y+2,sizeof(char));
    //initializing default values to b
    more=((2*mine)>(size_x*size_y))?1:0;
    if(more)
    {
        for(i=0;i<size_x;i++)
            for(j=0;j<size_y;j++)
                a[i][j]=-1;
    }
    for(i=1;i<size_x+1;i++)
        for(j=1;j<size_y+1;j++)
            b[i][j]=unopened;
    for(i=0;i<size_x+2;i++)
    {
        b[i][0]=border_v;
        b[i][size_y+1]=border_v;
    }
    for(i=0;i<size_y+2;i++)
    {
        b[0][i]=border_h;
        b[size_x+1][i]=border_h;
    }
    b[0][0]=201;
    b[0][size_y+1]=187;
    b[size_x+1][0]=200;
    b[size_x+1][size_y+1]=188;
}

void initialize()
{
    int random_x,random_y;
    int counter=0;
    srand(time(0));
    if(more)
    {
        a[firstX][firstY]=0;
        counter++;
        while(counter!=(size_x*size_y-mine))
        {
            random_x=rand()%size_x;
            random_y=rand()%size_y;
            if(a[random_x][random_y]!=0)
            {
                a[random_x][random_y]=0;
                counter++;
            }
        }
    }
    //counter!=(int)abs(size_x*size_y*more-mine)
    else
    {
        while(counter!=mine)
        {
            random_x=rand()%size_x;
            random_y=rand()%size_y;
            if(a[random_x][random_y]!=-1 && random_x!=firstX && random_y!=firstY)
            {
                a[random_x][random_y]=-1;
                counter++;
            }
        }
    }
    adjustVals();
}

void adjustVals()
{
    int i,j,count;
    for(i=0;i<size_x;i++)
        for(j=0;j<size_y;j++)
        {
            count=0;
            if(a[i][j]!=-1)
            {
                if((i-1)>=0 && (j-1)>=0 && a[i-1][j-1]==-1)
                    count++;
                if((i-1)>=0 && a[i-1][j]==-1)
                    count++;
                if((i-1)>=0 && (j+1)<size_y && a[i-1][j+1]==-1)
                    count++;
                if((j-1)>=0 && a[i][j-1]==-1)
                    count++;
                if((j+1)<size_y && a[i][j+1]==-1)
                    count++;
                if((i+1)<size_x && (j-1)>=0 && a[i+1][j-1]==-1)
                    count++;
                if((i+1)<size_x && a[i+1][j]==-1)
                    count++;
                if((i+1)<size_x && (j+1)<size_y && a[i+1][j+1]==-1)
                    count++;
                a[i][j]=count;
            }
        }
    openUp(firstX,firstY);
}

void openUp(int i, int j)
{
    if(i>=0 && i<size_x && j>=0 && j<size_y && b[i+1][j+1]!=empty)
    {
        if(a[i][j]==0)
        {
            b[i+1][j+1]=empty;
            openUp(i-1,j-1);
            openUp(i-1,j);
            openUp(i-1,j+1);
            openUp(i,j-1);
            openUp(i,j+1);
            openUp(i+1,j-1);
            openUp(i+1,j);
            openUp(i+1,j+1);
        }
        else
            b[i+1][j+1]=a[i][j]+48;
    }
}

void display()
{
	int i,j;
    system("cls");
    if(state)
    {
        for(i=0;i<size_x;i++)
            for(j=0;j<size_y;j++)
            {
                if(a[i][j]==-1 && state==1)
                    b[i+1][j+1]=explode;
                else if(a[i][j]==-1 && state==2)
                    b[i+1][j+1]=deactivate;
            }
    }
    printf("\n");
    /* to display internal matrix
    for(int i=0;i<size_x;i++)
    {
        for(int j=0;j<size_y;j++)
            printf("%d\t",a[i][j]);
        printf("\n");
    }*/
    
    int count=0;
    printf("\t\t");
    for(i=0;i<size_y;i++)
        printf("\t(%d)",(i+1));
    printf("\n");
    for(i=0;i<size_x+2;i++)
    {
        if(i!=0 && i!=size_x+1)
            printf("\t(%d)",(i));
        else
            printf("\t ");
        for(j=0;j<size_y+2;j++)
        {
            if(b[i][j]==unopened)
                count++;
            if(i==0 || j==0 || i==size_x+1 || j==size_y+1)
                SetColor(14);
            else if(b[i][j]==deactivate)
                SetColor(2);
            else if(b[i][j]==explode)
                SetColor(4);
            printf("\t%c",b[i][j]);
            SetColor(255);
        }
        printf("\n\n");
    }
    if(!state)
        printf("\t\t   Non-Mine Tiles remaining: %d\t\tTotal mines in the field:%d\n\n",(count-mine),mine);
}

int check()
{
	int i,j;
    for(i=1;i<size_x+2;i++)
        for(j=1;j<size_y+2;j++)
            if(b[i][j]==unopened && a[i-1][j-1]!=-1)
                return 0;
    state=2;
    display();
    return 1;
}

void play()
{
    int first=1;
    do
    {
        int x,y;
        display();
        //timer
        if(tstart==-1)
            tstart=time(0);
        printf("Enter Row and Column:: (in X Y format) :: ");
        scanf("%d %d",&x,&y);
        x--;
        y--;
        if(first)
        {
            first=0;
            firstX=x;
            firstY=y;
            initialize();
        }
        else
        {
            if(x>=0 && x<size_x && y>=0 && y<size_y)
            {
                if(b[x+1][y+1]!=unopened)
                {
                    printf("Already selected! Choose again\n");
                    getch();
                }
                else
                {
                    if(a[x][y]==-1)
                    {
                        state=1;
                        display();
                        SetColor(4);
                        printf("\n\a\t\t     )   )                  (         (               (      ____\n");
                        printf("\t\t  ( /(( /(            (     )\\ )      )\\ )       (    )\\ )  |   /\n");
                        printf("\t\t  )\\())\\())    (      )\\   (()/((    (()/(  (    )\\  (()/(  |  /\n");
                        printf("\t\t ((_)((_)\\     )\\  ((((_)(  /(_))\\    /(_)) )\\((((_)( /(_)) | /\n");
                        printf("\t\t__ ((_)((_) _ ((_)  )\\ _ )\\(_))((_)  (_))_ ((_))\\ _ )(_))_  |/\n");
                        printf("\t\t\\ \\ / / _ \\| | | |  (_)_\\(_| _ | __|  |   \\| __(_)_\\(_|   \\(|\n");
                        printf("\t\t \\ V | (_) | |_| |   / _ \\ |   | _|   | |) | _| / _ \\ | |) )\\\n");
                        printf("\t\t  |_| \\___/ \\___/   /_/ \\_\\|_|_|___|  |___/|___/_/ \\_\\|___((_)\n");
                        SetColor(255);
                        float percentage=(1-(mine/(float)(size_x*size_y)))*100;
                        if(percentage<40)
                            printf("\n\t\t\t\tWell you had only %.2f chance to win!\n\t\t\t\tBetter Luck Next time",percentage);
                        else
                            printf("\n\t\t\tYou should have won this! :/\n\t\t\t\tYou had %.2f chance of winning.\n",percentage);
                        return;//to exit the function, but not the entire program
                    }
                    else if(a[x][y]==0)
                    {
                        openUp(x,y);
                        display();
                    }
                    else
                        b[x+1][y+1]=a[x][y]+'0';
                }
            }
            else
            {
                printf("Wrong selection! :( Choose again\n");
                getch();
            }
        }
    }while(!check() && !state);
    //timer
    tnow=time(0)-tstart;
    display();
    SetColor(2);
    printf("\n\n\a\t\t\t   _, _, ,  ,  _,  ,_  _  ___,  , ,  _  ___,___,  _, ,  , _,!\n");
    printf("\t\t\t  /  / \\,|\\ | / _  |_)'|\\' | |  | | '|\\' | ' |   / \\,|\\ |(_, \n");
    printf("\t\t\t '\\_'\\_/ |'\\|'\\_|`'| \\ |-\\ |'\\__|'|__|-\\ |  _|_,'\\_/ |'\\| _) \n");
    printf("\t\t\t    `'   '  `  _|  '  `'  `'    `   ''  `' '     '   '  `'   \n");
    printf("\t\t\t              '                                              \n");
    SetColor(7);
    printf("\n\t\t\t\t\t  %s, You have won!!\n",name);
    printf("\n\t\t\t\t\t  Time taken: %dsec\n",tnow);
    getchar();
    if(mode!=4 && sBChange())
    {
        printf("\n\t\t\t   WoW!! Great Game!! Press Enter to see your place on the scoreboard\n");
        getch();
        scoreboard();
    }
    else
        printf("\n\t\t\t   Alas! You did not make it to the scoreboard! :(\n\t\t No Problem! Just play again! :)");
}

int main()
{
    int goOn;
    int rules;
    int keepgoing=0;
    SetColor(6);
    printf("\n\t=====================================================================================================================================================================================================\n");/*printf(" _______ _______ _______ _______ _______ ________ _______ _______ ______ _______ ______");
    printf("\n|   |   |_     _|    |  |    ___|     __|  |  |  |    ___|    ___|   __ \\    ___|   __ \\ ");
    printf("\n|       |_|   |_|       |    ___|__     |  |  |  |    ___|    ___|    __/    ___|      <");
    printf("\n|__|_|__|_______|__|____|_______|_______|________|_______|_______|___|  |_______|___|__|\n");
    */
    SetColor(2);
    printf("\n\t\t  _____   _   _ U _____ u         __  __             _   _   U _____ u____                 U _____ uU _____ u  ____   U _____ u   ____              ____      _      __  __  U _____ u\n");
    printf("\t\t |_ \" _| |'| |'|\\| ___\"|/       U|' \\/ '|u  ___     | \\ |\"|  \\| ___\"|/ __\"| u  __        __\\| ___\"|/\\| ___\"|/U|  _\"\\ u\\| ___\"|/U |  _\"\\ u        U /\"___|uU  /\"\\  uU|' \\/ '|u\\| ___\"|/\n");
    printf("\t\t   | |  /| |_| |\\|  _|\"         \\| |\\/| |/ |_\"_|   <|  \\| |>  |  _|\"<\\___ \\/   \\\"\\      /\"/ |  _|\"   |  _|\"  \\| |_) |/ |  _|\"   \\| |_) |/        \\| |  _ / \\/ _ \\/ \\| |\\/| |/ |  _|\"\n");
    printf("\t\t  /| |\\ U|  _  |u| |___          | |  | |   | |    U| |\\  |u  | |___ u___) |   /\\ \\ /\\ / /\\ | |___   | |___   |  __/   | |___    |  _ <           | |_| |  / ___ \\  | |  | |  | |___\n");
    printf("\t\t u |_|U  |_| |_| |_____|         |_|  |_| U/| |\\u   |_| \\_|   |_____||____/>> U  \\ V  V /  U|_____|  |_____|  |_|      |_____|   |_| \\_\\           \\____| /_/   \\_\\ |_|  |_|  |_____|\n");
    printf("\t\t _// \\\\_ //   \\\\ <<   >>        <<,-,,-.-,_|___|_,-.||   \\\\,-.<<   >> )(  (__).-,_\\ /\\ /_,-.<<   >>  <<   >>  ||>>_    <<   >>   //   \\\\_          _)(|_   \\\\    >><<,-,,-.   <<   >>\n");
    printf("\t\t(__) (__|_\") (\"_|__) (__)        (./  \\.)_)-' '-(_/ (_\")  (_/(__) (__|__)      \\_)-'  '-(_/(__) (__)(__) (__)(__)__)  (__) (__) (__)  (__)        (__)__) (__)  (__)(./  \\.) (__) (__)\n");
    SetColor(6);
    printf("\n\t=====================================================================================================================================================================================================\n");
    SetColor(255);
    printf("\n\t\t\t\t\t\t\t\t\t\t\t_   _______.____   ___ __________  _");
    printf("\n\t\t\t\t\t\t\t\t\t\t\t|   |___ | '[__    |__]|___| __||\\ |");
    printf("\n\t\t\t\t\t\t\t\t\t\t\t|___|___ |  ___]   |__]|___|__]|| \\|");
    SetColor(2);
    printf("\n\n\t\t\t\t\t\t\t\t\t\t\t\t   (Press Enter)");
    SetColor(255);
    getch();
    system("cls");
    printf("Do you want to get a quick view of the game rules?(1/0): ");
    scanf("%d",&rules);
    if(rules==1)
        displayRules();
    else if(rules!=0)
    {
        printf("\n\nYou had to enter either 1 or 0. :(\nAnyways, you still get to see the rules. :)");
        getch();
        displayRules();
    }
    system("cls");
    do
    {
        system("cls");
        if(!keepgoing)
        {
            printf("Enter your name: ");
            fflush(stdin);
            //getchar();
            gets(name);
            system("cls");
            printf("Welcome %s!\n",name);
        }
        else
            keepgoing=0;
        printf("\n<================MENU================>\n");
        printf("\n1.  Beginner Mode ::  8x8  :: 10 mines\n");
        printf("2.  Moderate Mode :: 18x18 :: 40 mines\n");
        printf("3.  Expert Mode   :: 22x22 :: 50 mines\n");
        printf("4.  Custom Mode\n");
        printf("5.  Revisit the rules\n");
        printf("6.  View the scoreboard\n");
        printf("Enter the mode you wanna play in (1-6: anything else to exit game): ");
        scanf("%d",&mode);
        switch(mode)
        {
            case 1: mine=10;
                    size_x=8;
                    size_y=8;
                    break;
            case 2: mine=40;
                    size_x=18;
                    size_y=18;
                    break;
            case 3: mine=50;
                    size_x=22;
                    size_y=22;
                    break;
            case 4: printf("\nEnter board sizes::(Try to keep it within 22x23)::\n\nNumber of rows: ");
                    scanf("%d",&size_x);
                    printf("Number of columns: ");
                    scanf("%d",&size_y);
                    int okay=0;
                    while(!okay)
                    {
                        printf("\nEnter Number of mines you want in the field: ");
                        scanf("%d",&mine);
                        if(mine<size_x*size_y-1 && mine>0)
                            okay=1;
                        else
                            printf("Number of mines should be in the range of 1 to (m*n-1)! :(\n");
                    }
                    break;
            case 5: displayRules();
                    keepgoing=1;
                    break;
            case 6: scoreboard();
                    keepgoing=1;
                    break;
            default: printf("\nThanks for playing! :)\n");
                     exit(0);
        }
        if(!keepgoing)
        {
            setup();
            play();
            getch();
            system("cls");
            printf("Wanna play again?(1/0: anything else to exit): ");
        }
        else
        {
            getch();
            system("cls");
            printf("Go back to main menu?(1/0: anything else to exit): ");
        }
        scanf("%d",&goOn);
    }while(goOn==1);
    printf("\nThanks for playing! :)\n");
    return 0;
}
