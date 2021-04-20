#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<list>
#include<string>
#include<ctime>
#include"jsoncpp/json.h"
#include<math.h>
using namespace std;
int n, m;
const int maxn = 25;
const int dx[4] = { -1,0,1,0 };
const int dy[4] = { 0,1,0,-1 };
bool invalid[maxn][maxn];
int dist[maxn][maxn];

struct point
{
	int x, y;
	point(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
	point() {};
};


list<point> snake[2]; // 0表示自己的蛇，1表示对方的蛇
int possibleDire[10];
int posCount;

bool whetherGrow(int num)  //本回合是否生长
{
	if (num <= 9) return true;
	if ((num - 9) % 3 == 0) return true;
	return false;
}

void deleteEnd(int id)     //删除蛇尾
{
	snake[id].pop_back();
}

void move(int id, int dire, int num)  //编号为id的蛇朝向dire方向移动一步
{
	point p = *(snake[id].begin());
	int x = p.x + dx[dire];
	int y = p.y + dy[dire];
	snake[id].push_front(point(x, y));
	if (!whetherGrow(num))
		deleteEnd(id);
}
void outputSnakeBody(int id)    //调试语句
{
	cout << "Snake No." << id << endl;
	for (list<point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
		cout << iter->x << " " << iter->y << endl;
	cout << endl;
}

bool isInBody(int x, int y)   //判断(x,y)位置是否有蛇
{
	for (int id = 0; id <= 1; id++)
		for (list<point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
			if (x == iter->x && y == iter->y)
				return true;
	return false;
}

bool validDirection(int id, int k)  //判断当前移动方向的下一格是否合法
{
	point p = *(snake[id].begin());
	int x = p.x + dx[k];
	int y = p.y + dy[k];
	if (x > n || y > m || x < 1 || y < 1) return false;
	if (invalid[x][y]) return false;
	if (isInBody(x, y)) return false;
	return true;
}

int Rand(int p)   //随机生成一个0到p的数字
{
	return rand() * rand() * rand() % p;
}

void general_map(int map[25][25])//根据当前形势生成障碍物地图（对方蛇体，我方蛇体，障碍物均为障碍）,n棋盘高度 m为当前棋盘的宽度
{
	for (int i = 0; i <= n + 1; i++)
		for (int j = 0; j <= m + 1; j++)
			map[i][j] = 1;

	//************边界*************
	for (int i = 1; i <= n; i++)
		for (int j = 1; j <= m; j++)
			map[i][j] = invalid[i][j];
	//************复制障碍物**********

	list<point>::iterator it = snake[0].begin();
	list<point>::iterator it1 = snake[1].begin();
	for (; it != snake[0].end(); it++)
	{
		map[it->x][it->y] = 1;

	}

	for (; it1 != snake[1].end(); it1++)
	{
		map[it1->x][it1->y] = 1;
	}
	//*****************复制蛇身***************
}


double calc_value(point & my_head, point & op_head, int map[][25]) //map存放障碍 蛇身也是障碍
{
	int visit[25][25];	//BFS中用于标记有没有没访问过
	
	memset(visit,0,sizeof(visit));
	
	int Territory[25][25]; //标记蛇的区域  0表示我方蛇 1表示对方蛇
	
	memset(Territory,-1,sizeof(Territory));
	

	memset(dist,0,sizeof(dist));

	Territory[my_head.x][my_head.y] = 0;
	Territory[op_head.x][op_head.y] = 1;

	//visit[my_head.x][my_head.y] = 1;
	//visit[op_head.x][op_head.y] = 1;//??

	deque<point> now_try;
	now_try.push_back(op_head);//先入队对方的节点
	now_try.push_back(my_head);//入队我方节点
	
	

	while (!now_try.empty()) 	//BFS
	{
		point new_p(*now_try.begin());
		now_try.pop_front();
		for (int d = 0; d < 4; d++)
		{
			point p1;
			p1.x = new_p.x + dx[d];
			p1.y = new_p.y + dy[d];
			//向某方向走一步，now_p存新方向的点
			if (map[p1.x][p1.y] != 1 && visit[p1.x][p1.y] == 0)//新的待入队节点合法
			{
				now_try.push_back(p1);	//加入下一个节点
				dist[p1.x][p1.y] = dist[new_p.x][new_p.y] + 1;
				Territory[p1.x][p1.y] = Territory[new_p.x][new_p.y];//同一区域
				visit[p1.x][p1.y] = 1;
			}
		}

	}
	double value = 0.0;
	for (int i = 1; i <= n; i++)
		for (int j = 1; j <= m; j++)
		{
			if (map[i][j] != 1 && dist[i][j] != 0 && Territory[i][j] == 0)//我方走过的区域
				value += 1.0 / sqrt(dist[i][j]);
			if (map[i][j] != 1 && dist[i][j] != 0 && Territory[i][j] == 1)//对方走过的区域
				value -= 1.0 / sqrt(dist[i][j]);

		}
	return value;
}



int one_step_judge()//从当前节点
{
	int map[25][25];
	general_map(map);
	double max = -1e9 ;
	int max_dir = 0;
	point p(*snake[0].begin());
	for (int i = 0; i < 4; i++)
	{
		point p1;
		double temp = 0.0;
		p1.x = p.x + dx[i];
		p1.y = p.y + dy[i];
		if (!map[p1.x][p1.y])
		{
			map[p1.x][p1.y]=1;
			temp = calc_value(p1, *(snake[1].begin()), map);
			if (temp > max)
			{
				max = temp;
				max_dir = i;//mark

			}
			map[p1.x][p1.y]=0;
		}

	}

	return max_dir;// 0,1,2,3 走哪一步
}



int main()
{
	memset(invalid, 0, sizeof(invalid));
	string str;
	string temp;
	while (getline(cin, temp))
		str += temp;

	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);

	n = input["requests"][(Json::Value::UInt) 0]["height"].asInt();  //棋盘高度
	m = input["requests"][(Json::Value::UInt) 0]["width"].asInt();   //棋盘宽度

	int x = input["requests"][(Json::Value::UInt) 0]["x"].asInt();  //读蛇初始化的信息
	if (x == 1)
	{
		snake[0].push_front(point(1, 1));
		snake[1].push_front(point(n, m));
	}
	else
	{
		snake[1].push_front(point(1, 1));
		snake[0].push_front(point(n, m));
	}
	//处理地图中的障碍物
	int obsCount = input["requests"][(Json::Value::UInt) 0]["obstacle"].size();

	for (int i = 0; i < obsCount; i++)
	{
		int ox = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["x"].asInt();
		int oy = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["y"].asInt();
		invalid[ox][oy] = 1;
	}

	//根据历史信息恢复现场
	int total = input["responses"].size();

	int dire;
	for (int i = 0; i < total; i++)
	{
		dire = input["responses"][i]["direction"].asInt();
		move(0, dire, i);

		dire = input["requests"][i + 1]["direction"].asInt();
		move(1, dire, i);
	}

	if (!whetherGrow(total)) // 本回合两条蛇生长
	{
		deleteEnd(0);
		deleteEnd(1);
	}

	srand((unsigned)time(0) + total);



	//随机做出一个决策
	Json::Value ret;
	ret["response"]["direction"] = one_step_judge();

	Json::FastWriter writer;
	cout << writer.write(ret) << endl;

	return 0;
}
