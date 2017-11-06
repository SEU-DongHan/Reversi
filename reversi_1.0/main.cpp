#include <iostream>
#include <random>
#include <string>
#include <cstdlib>
#include <ctime>
#include "jsoncpp/json.h"

using namespace std;

// 棋盘类
class board
{
    public:
    int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
    int gridInfo[8][8]; // 先x后y，记录棋盘状态
    int blackPieceCount , whitePieceCount;
    int possiblePos[64][2], posCount;

    board()
    {
        // 初始化棋盘
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
        gridInfo[3][4] = gridInfo[4][3] = 1;  //|白|黑|
        gridInfo[3][3] = gridInfo[4][4] = -1; //|黑|白|
        blackPieceCount =2 ;
        whitePieceCount =2;
        posCount=0;

        currBotColor=1; //默认第一回合黑色

       // cout<<"board has been made."<<endl;
    }
    /*
    测试用，自写复制构造函数
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

 // 在坐标处落子，检查是否合法或模拟落子
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

 // 检查color方有无合法棋步
 bool CheckIfHasValidMove(board &copyboard,int color)
 {
    int x, y;
    for (y = 0; y < 8; y++)
    for (x = 0; x < 8; x++)
    if (copyboard.ProcStep(copyboard,x, y, color, true))
        return true;
    return false;
 }

 //根据系统返回的Json 对游戏复盘
 void chessboard_resuming(Json::Value input, board &bd)
 {
        int x,y;
        int turnID=0;
        // 分析自己收到的输入和自己过往的输出，并恢复状态
        turnID = input["responses"].size();
        bd.currBotColor = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? 1 : -1; // 第一回合收到坐标是-1, -1，说明我是黑方
        for (int i = 0; i < turnID; i++)
        {
            // 根据这些输入输出逐渐恢复状态到当前回合
            x = input["requests"][i]["x"].asInt();
            y = input["requests"][i]["y"].asInt();
            if (x >= 0)
                bd.ProcStep(bd,x, y, -currBotColor); // 模拟对方落子
            x = input["responses"][i]["x"].asInt();
            y = input["responses"][i]["y"].asInt();
            if (x >= 0)
                bd.ProcStep(bd,x, y, currBotColor); // 模拟己方落子
        }
        // 看看自己本回合输入
        x = input["requests"][turnID]["x"].asInt();
        y = input["requests"][turnID]["y"].asInt();

        /* 测试是否接收到第一回输入
        cout<<"***************"<<endl;
        cout<<"x:"<<x<<" "<<"y:"<<y<<endl;
        cout<<"***************"<<endl;
        */
        if (x >= 0)
            bd.ProcStep(bd,x, y, -currBotColor);
    }

    //找出合法落子点
    bool findValidPosition(board &copyboard,int currBotColor1)
    {
        bool result = false;
        //每一次调用之前将合法落子点数组清空
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
    int flag = 0;  //flag =0 结束 1 未结束
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(board.gridInfo[i][j]==0){
                // 存在没下完的位置，且黑棋或白棋有合法位置，则游戏继续，没有结束
                if(board.findValidPosition(board,-1)==true || board.findValidPosition(board,1)==true){
                    return false;
                }

            }
        }
    }

    return true;
}

//产生真随机数
static random_device rd;
int giveUaChoice(int poscount){
    int choice;
    choice = rd() % poscount;
    return choice;
}
//对当前可能的落子点进行模拟，
static float simulationTimes = 0;
static float whiteWinTimes = 0;
static int firstFlag=1;
void makeAdecision(board &board1,int inputX=-2,int inputY=-2)
{
    board copyboard = board1;
    /* board实例中所包含的属性
    int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
    int gridInfo[8][8];
    int blackPieceCount , whitePieceCount;
    int possiblePos[64][2], posCount;

    输出棋盘当前布局情况
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
        //对合法点位进行随机挑选，挑选任意一个进行落子
        if (copyboard.posCount > 0)
        {
            choice = giveUaChoice(copyboard.posCount);
//              输出每一次的选择情况
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
    //更新合法点位集合
    copyboard.currBotColor = -copyboard.currBotColor;
    copyboard.findValidPosition(copyboard,copyboard.currBotColor);
    //判断游戏是否结束
    if( ifGameOver(copyboard))
    {
        //游戏结束则记一次模拟
        simulationTimes ++;
        int bkcount = copyboard.blackPieceCount;
        int wtcount = copyboard.whitePieceCount;
        //cout<<"黑棋个数: "<<bkcount<<","<<"白棋个数: "<<wtcount<<"simulationTimes:"<<simulationTimes<<endl;
        if(wtcount > bkcount){
            //白棋AI赢
            whiteWinTimes ++;
            //cout<<"白棋赢得第"<<whiteWinTimes<<"次胜利"<<endl;
        }
        firstFlag=1;
    }
    else{
        //游戏继续情况下，进行递归，直到游戏结束
        makeAdecision(copyboard);
    }
}




int main(){
    // 读入JSON
    string str;
    getline(cin, str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);

    float winPossibility = 0;
    int x = -1;
    int y = -1;
    board board1;
    //对游戏复盘
    board1.chessboard_resuming(input,board1);
    //找出合法点位
    board1.findValidPosition(board1, -board1.currBotColor);

    //对当前所有合法点位进行模拟下棋，找出胜率最高的点位
    for(int ii=0;ii<board1.posCount;ii++){
        //对每一个合法点位进行若干次模拟，判断该点位的胜率
        int xx = board1.possiblePos[ii][0];
        int yy = board1.possiblePos[ii][1];
        cout<<"x:"<<xx<<",y:"<<yy<<endl;
        // 对每个点位进行 jj次模拟，jj次数越大，结果越趋向真实
        for(int jj=0;jj<100;jj++){
            makeAdecision(board1,xx,yy);
        }
        //cout<< "第"<<ii<<"个点位胜率为："<<whiteWinTimes/simulationTimes<<endl;
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
    // 做出决策（你只需修改以下部分）
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
    // 决策结束，输出结果（你只需修改以上部分）

    Json::Value ret;
    ret["response"]["x"] = resultX;
    ret["response"]["y"] = resultY;
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
    return 0;
}
