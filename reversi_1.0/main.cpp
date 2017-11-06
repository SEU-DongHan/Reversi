#include <iostream>
#include <random>
#include <string>
#include <cstdlib>
#include <ctime>
#include "jsoncpp/json.h"

using namespace std;

// ������
class board
{
    public:
    int currBotColor; // ����ִ����ɫ��1Ϊ�ڣ�-1Ϊ�ף�����״̬��ͬ��
    int gridInfo[8][8]; // ��x��y����¼����״̬
    int blackPieceCount , whitePieceCount;
    int possiblePos[64][2], posCount;

    board()
    {
        // ��ʼ������
        for(int i=0;i<8;i++){
            for(int j=0;j<8;j++){
                gridInfo[i][j]= 0;
            }
        }
        for(int i=0;i<64;i++){
            for(int j=0;j<2;j++){
                possiblePos[i][j]= 0;
            }
        }
        gridInfo[3][4] = gridInfo[4][3] = 1;  //|��|��|
        gridInfo[3][3] = gridInfo[4][4] = -1; //|��|��|
        blackPieceCount =2 ;
        whitePieceCount =2;
        posCount=0;

        currBotColor=1; //Ĭ�ϵ�һ�غϺ�ɫ

       // cout<<"board has been made."<<endl;
    }
    /*
    �����ã���д���ƹ��캯��
    board(board &bd)
    {
        currBotColor= bd.currBotColor;
        for(int i=0;i<8;i++){
            for(int j=0;j<8;j++){
                gridInfo[i][j]= bd.gridInfo[i][j];
            }
        }

        blackPieceCount= bd.blackPieceCount ;
        whitePieceCount= bd.whitePieceCount ;
        for(int i=0;i<64;i++){
            for(int j=0;j<2;j++){
                possiblePos[i][j]= bd.possiblePos[i][j];
            }
        }
        posCount= bd.posCount;

        cout<<"board has been copyed."<<endl;
    }
    */

   inline bool MoveStep(int &x, int &y, int Direction)
 {
    if (Direction == 0 || Direction == 6 || Direction == 7)
        x++;
    if (Direction == 0 || Direction == 1 || Direction == 2)
        y++;
    if (Direction == 2 || Direction == 3 || Direction == 4)
        x--;
    if (Direction == 4 || Direction == 5 || Direction == 6)
        y--;
    if (x < 0 || x > 7 || y < 0 || y > 7)
        return false;
    return true;
 }

 // �����괦���ӣ�����Ƿ�Ϸ���ģ������
    bool ProcStep(board &copyboard,int xPos, int yPos, int color, bool checkOnly = false)
 {
    int effectivePoints[8][2]={0};
    int dir, x, y, currCount;
    bool isValidMove = false;
    if (copyboard.gridInfo[xPos][yPos] != 0)
        return false;
    for (dir = 0; dir < 8; dir++)
    {
        x = xPos;
        y = yPos;
        currCount = 0;
        while (1)
        {
            if (!MoveStep(x, y, dir))
            {
                currCount = 0;
                break;
            }
            if (copyboard.gridInfo[x][y] == -color)
            {
                currCount++;
                effectivePoints[currCount][0] = x;
                effectivePoints[currCount][1] = y;
            }
            else if (copyboard.gridInfo[x][y] == 0)
            {
                currCount = 0;
                break;
            }
            else
            {
                break;
            }
        }
        if (currCount != 0)
        {
            isValidMove = true;
            if (checkOnly)
                return true;
            if (color == 1)
            {
                blackPieceCount += currCount;
                whitePieceCount -= currCount;
            }
            else
            {
                whitePieceCount += currCount;
                blackPieceCount -= currCount;
            }
            while (currCount > 0)
            {
                x = effectivePoints[currCount][0];
                y = effectivePoints[currCount][1];
                copyboard.gridInfo[x][y] *= -1;
                currCount--;
            }
        }
    }
    if (isValidMove)
    {
        copyboard.gridInfo[xPos][yPos] = color;
        if (color == 1)
            copyboard.blackPieceCount++;
        else
            copyboard.whitePieceCount++;
        return true;
    }
    else
        return false;
 }

 // ���color�����޺Ϸ��岽
 bool CheckIfHasValidMove(board &copyboard,int color)
 {
    int x, y;
    for (y = 0; y < 8; y++)
    for (x = 0; x < 8; x++)
    if (copyboard.ProcStep(copyboard,x, y, color, true))
        return true;
    return false;
 }

 //����ϵͳ���ص�Json ����Ϸ����
 void chessboard_resuming(Json::Value input, board &bd)
 {
        int x,y;
        int turnID=0;
        // �����Լ��յ���������Լ���������������ָ�״̬
        turnID = input["responses"].size();
        bd.currBotColor = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? 1 : -1; // ��һ�غ��յ�������-1, -1��˵�����Ǻڷ�
        for (int i = 0; i < turnID; i++)
        {
            // ������Щ��������𽥻ָ�״̬����ǰ�غ�
            x = input["requests"][i]["x"].asInt();
            y = input["requests"][i]["y"].asInt();
            if (x >= 0)
                bd.ProcStep(bd,x, y, -currBotColor); // ģ��Է�����
            x = input["responses"][i]["x"].asInt();
            y = input["responses"][i]["y"].asInt();
            if (x >= 0)
                bd.ProcStep(bd,x, y, currBotColor); // ģ�⼺������
        }
        // �����Լ����غ�����
        x = input["requests"][turnID]["x"].asInt();
        y = input["requests"][turnID]["y"].asInt();

        /* �����Ƿ���յ���һ������
        cout<<"***************"<<endl;
        cout<<"x:"<<x<<" "<<"y:"<<y<<endl;
        cout<<"***************"<<endl;
        */
        if (x >= 0)
            bd.ProcStep(bd,x, y, -currBotColor);
    }

    //�ҳ��Ϸ����ӵ�
    bool findValidPosition(board &copyboard,int currBotColor1)
    {
        bool result = false;
        //ÿһ�ε���֮ǰ���Ϸ����ӵ��������
        for (int j=0;j <2;j++){
            for(int i=0;i<64;i++){
                copyboard.possiblePos[i][j]=0;
            }
        }
        copyboard.posCount = 0;


        for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
        if (copyboard.ProcStep(copyboard,x, y, currBotColor1, true))
        {
            copyboard.possiblePos[posCount][0] = x;
            copyboard.possiblePos[posCount++][1] = y;
            result = true;
        }
        return result;

    }
};

bool ifGameOver(board board)
{
    int flag = 0;  //flag =0 ���� 1 δ����
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(board.gridInfo[i][j]==0){
                // ����û�����λ�ã��Һ��������кϷ�λ�ã�����Ϸ������û�н���
                if(board.findValidPosition(board,-1)==true || board.findValidPosition(board,1)==true){
                    return false;
                }

            }
        }
    }

    return true;
}

//�����������
static random_device rd;
int giveUaChoice(int poscount){
    int choice;
    choice = rd() % poscount;
    return choice;
}
//�Ե�ǰ���ܵ����ӵ����ģ�⣬
static float simulationTimes = 0;
static float whiteWinTimes = 0;
static int firstFlag=1;
void makeAdecision(board &board1,int inputX=-2,int inputY=-2)
{
    board copyboard = board1;
    /* boardʵ����������������
    int currBotColor; // ����ִ����ɫ��1Ϊ�ڣ�-1Ϊ�ף�����״̬��ͬ��
    int gridInfo[8][8];
    int blackPieceCount , whitePieceCount;
    int possiblePos[64][2], posCount;

    ������̵�ǰ�������
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            cout<<copyboard.gridInfo[i][j]<<" ";
        }
        cout<<endl;
    }
*/
    int resultX, resultY, choice;
    if(firstFlag==1){
            resultX = inputX;
            resultY = inputY;
            firstFlag =0;
    }
    else
    {
        //�ԺϷ���λ���������ѡ����ѡ����һ����������
        if (copyboard.posCount > 0)
        {
            choice = giveUaChoice(copyboard.posCount);
//              ���ÿһ�ε�ѡ�����
//            cout<<"poscount:"<<copyboard.posCount<<" choice is "<<choice<<endl;
            resultX = copyboard.possiblePos[choice][0];
            resultY = copyboard.possiblePos[choice][1];
        }
        else
        {
            resultX = -1;
            resultY = -1;
        }

    }
    copyboard.ProcStep(copyboard,resultX,resultY,copyboard.currBotColor,false);
    //���ºϷ���λ����
    copyboard.currBotColor = -copyboard.currBotColor;
    copyboard.findValidPosition(copyboard,copyboard.currBotColor);
    //�ж���Ϸ�Ƿ����
    if( ifGameOver(copyboard))
    {
        //��Ϸ�������һ��ģ��
        simulationTimes ++;
        int bkcount = copyboard.blackPieceCount;
        int wtcount = copyboard.whitePieceCount;
        //cout<<"�������: "<<bkcount<<","<<"�������: "<<wtcount<<"simulationTimes:"<<simulationTimes<<endl;
        if(wtcount > bkcount){
            //����AIӮ
            whiteWinTimes ++;
            //cout<<"����Ӯ�õ�"<<whiteWinTimes<<"��ʤ��"<<endl;
        }
        firstFlag=1;
    }
    else{
        //��Ϸ��������£����еݹ飬ֱ����Ϸ����
        makeAdecision(copyboard);
    }
}




int main(){
    // ����JSON
    string str;
    getline(cin, str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);

    float winPossibility = 0;
    int x = -1;
    int y = -1;
    board board1;
    //����Ϸ����
    board1.chessboard_resuming(input,board1);
    //�ҳ��Ϸ���λ
    board1.findValidPosition(board1, -board1.currBotColor);

    //�Ե�ǰ���кϷ���λ����ģ�����壬�ҳ�ʤ����ߵĵ�λ
    for(int ii=0;ii<board1.posCount;ii++){
        //��ÿһ���Ϸ���λ�������ɴ�ģ�⣬�жϸõ�λ��ʤ��
        int xx = board1.possiblePos[ii][0];
        int yy = board1.possiblePos[ii][1];
        cout<<"x:"<<xx<<",y:"<<yy<<endl;
        // ��ÿ����λ���� jj��ģ�⣬jj����Խ�󣬽��Խ������ʵ
        for(int jj=0;jj<100;jj++){
            makeAdecision(board1,xx,yy);
        }
        //cout<< "��"<<ii<<"����λʤ��Ϊ��"<<whiteWinTimes/simulationTimes<<endl;
        float tempoary = whiteWinTimes/simulationTimes;
        if(tempoary > winPossibility){
                x = xx;
                y = yy;
                winPossibility = tempoary;
        }
        whiteWinTimes=0;
        simulationTimes=0;
    }

    //cout<<"the decision with the highest winning possibility is  X:"<<x<<" Y:"<<y<<endl;
    // �������ߣ���ֻ���޸����²��֣�
    //***************************************

    int resultX, resultY,choice;
    if (board1.posCount > 0)
    {
        resultX = x;
        resultY = y;
    }
    else
    {
        resultX = -1;
        resultY = -1;
    }



     //***************************************
    // ���߽���������������ֻ���޸����ϲ��֣�

    Json::Value ret;
    ret["response"]["x"] = resultX;
    ret["response"]["y"] = resultY;
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
    return 0;
}
