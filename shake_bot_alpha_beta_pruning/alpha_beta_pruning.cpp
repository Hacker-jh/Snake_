#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<list>
#include<string>
#include<ctime>
#include"jsoncpp/json.h"
#include<cmath>
using namespace std;
int n, m;
const int maxn = 25;
const int WIN = 100000000;
const int dx[4] = { -1,0,1,0 };
const int dy[4] = { 0,1,0,-1 };
bool invalid[maxn][maxn];

int dist_value[300];//效益数组，越后走到的点效益值越低
int disteps[300];//存放不增加长度的回合数，用于求蛇身上的点在第几回合后才能空出，进行潜在评估
int sum = 0, DEPTH;
//--------------------------------------------
const int HASH_SIZE = 1 << 20;//1左移二十位
int hash_move[HASH_SIZE];
//------------------------------------------
int timeout = (int)((0.9689) * (double)CLOCKS_PER_SEC); //时间限制

int NEXT_STEP, Time;

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



Json::Value ret;
void output(int d)
{

	ret["response"]["direction"] = d;

	ret["response"]["myinformatin"] = sum;




	Json::FastWriter writer;

	cout << writer.write(ret) << endl;
	system("pause");
}



list<point> snake[2]; //存放两条蛇


bool whetherGrow(int num)  //本回合是否增长
{
	if (num <= 9) return true;
	if ((num - 9) % 3 == 0) return true;
	return false;
}


void deleteEnd(int id)     //去尾
{
	snake[id].pop_back();
}



void move(int id, int dire, int num)  //移动
{
	point p = *(snake[id].begin());
	int x = p.x + dx[dire];
	int y = p.y + dy[dire];
	snake[id].push_front(point(x, y));
	if (!whetherGrow(num))
		deleteEnd(id);
}

int move1(int id, int dire, int total)
{
	point p = *(snake[id].begin());
	int x = p.x + dx[dire];
	int y = p.y + dy[dire];
	snake[id].push_front(point(x, y));
	if (id == 1) {
		total++;
	}
	return total;
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



void general_map(int map[25][25])//根据当前形势生成障碍物地图（对方蛇体，我方蛇体，障碍物均为障碍）,n棋盘高度 m为当前棋盘的宽度
{
	for (int i = 0; i <= n + 1; i++)//-1é’æ¿†îé–?
		for (int j = 0; j <= m + 1; j++)
			map[i][j] = 1;

	//************边界*************
	for (int i = 1; i <= n; i++)
		for (int j = 1; j <= m; j++)
			map[i][j] = invalid[i][j];//障碍为1
	//************复制障碍物**********
	list<point>::iterator it = snake[0].begin();
	list<point>::iterator it1 = snake[1].begin();
	for (it; it != snake[0].end(); it++)
	{
		map[it->x][it->y] = 2;

	}

	for (it1; it1 != snake[1].end(); it1++)
	{
		map[it1->x][it1->y] = 3;
	}
	//*****************复制蛇身***************
}

bool canmove(int id) {
	bool flag;
	int i;
	for (i = 0; i < 4; i++) {
		if (validDirection(id, i)) {
			return true;
		}
	}
}


int evaluate(int total)
{
	int dist1[25][25] = { 0 }, dist2[25][25] = { 0 }, temp_map[25][25];
	int st[25][25] = { 0 };

	for (int i = 0; i < 25; i++)
		for (int j = 0; j < 25; j++)
		{
			dist1[i][j] = 9999999;
			dist2[i][j] = 9999999;
			st[i][j] = 0;
		}

	list<point>::iterator it0 = snake[0].end();
	list<point>::iterator it1 = snake[1].end();
	it0--;
	it1--;
	int k = 0;
	for (k = total - snake[0].size() + 1; k <= total; k++)
	{

		st[it0->x][it0->y] = disteps[k] - total;
		st[it1->x][it1->y] = disteps[k] - total;
		if (it0 != snake[0].begin())
			it0--;
		if (it1 != snake[1].begin())
			it1--;

	}


	general_map(temp_map);

	for (int id = 0; id <= 1; id++)
	{
		point head(snake[id].front());

		int visit[25][25] = { 0 };

		visit[head.x][head.y] = 1;
		if (id == 0)
			dist1[snake[id].front().x][snake[id].front().y] = 0;
		else
			dist2[snake[id].front().x][snake[id].front().y] = 0;

		deque<point> now_try;
		now_try.push_back(head);


		if (id == 0)
		{
			while (!now_try.empty())
			{
				point new_p(*now_try.begin());
				now_try.pop_front();
				for (int d = 0; d < 4; d++)
				{
					point p1;
					p1.x = new_p.x + dx[d];
					p1.y = new_p.y + dy[d];

					if (dist1[p1.x][p1.y] > dist1[new_p.x][new_p.y] + 1 && temp_map[p1.x][p1.y] != 1 && st[p1.x][p1.y] <= dist1[new_p.x][new_p.y] + 1)//æ–°çš„å¾…å…¥é˜ŸèŠ‚ç‚¹åˆæ³•
					{
						now_try.push_back(p1);
						dist1[p1.x][p1.y] = dist1[new_p.x][new_p.y] + 1;

					}
				}

			}
		}

		if (id == 1)
		{
			while (!now_try.empty())
			{
				point new_p(*now_try.begin());
				now_try.pop_front();
				for (int d = 0; d < 4; d++)
				{
					point p1;
					p1.x = new_p.x + dx[d];
					p1.y = new_p.y + dy[d];
					//向某方向走一步，new_p存新方向的点
					if (dist2[p1.x][p1.y] > dist2[new_p.x][new_p.y] + 1 && temp_map[p1.x][p1.y] != 1 && st[p1.x][p1.y] <= dist2[new_p.x][new_p.y] + 1)//æ–°çš„å¾…å…¥é˜ŸèŠ‚ç‚¹åˆæ³•
					{
						now_try.push_back(p1);
						dist2[p1.x][p1.y] = dist2[new_p.x][new_p.y] + 1;

					}
				}

			}
		}


	}





	int value1 = 0, value2 = 0;
	for (int i = 1; i <= n; i++)
		for (int j = 1; j <= m; j++)
		{
			if (dist1[i][j] < dist2[i][j])
				value1 += dist_value[dist1[i][j]];
			else if (dist2[i][j] < dist1[i][j])
				value2 += dist_value[dist2[i][j]];

		}
	return value1 - value2;




};

int noplace(int id)
{
	if (id == 0)
	{
		if (canmove(1))
		{
			return -WIN;
		}
		return 0;//双方都不能走了
	}
	else if (id == 1)
	{
		point p0 = *(snake[0].begin());
		point p1 = *(snake[1].begin());

		if (abs(p0.x - p1.x) + abs(p0.y - p1.y) == 1)
		{
			return 10;
		}
		return -WIN;
	}

}
int returnvalue;
int alpha_beta(int depth, int id, int alpha, int beta, int total, bool firstpoint, int hash_value) {
	ret["response"]["total"] = total;

	if ((clock() - Time) >= timeout) 
	{

		output(returnvalue);
		exit(0);

	}

	int now = -1000000000, bestmove = 0;
	int i, j;
	bool isdeletetail = 0;
	if (depth == 0) {
		return evaluate(total);
	}
	//----------------------------------------------
	int best = hash_move[hash_value & (HASH_SIZE - 1)];

	/////

	point p0, p1;
	if (!whetherGrow(total)) {//尾部是否消失
		p0 = snake[0].back();//
		p1 = snake[1].back();//保存双方的尾部
	}

	int map[25][25];
	general_map(map);
	bool endflag = 1;//如果最后还是一，代表无路可走，就结束
	if (!id)
	{

		if (whetherGrow(total) == 0)
		{
			snake[0].pop_back();
			snake[1].pop_back();
		}
	}
	for (int i = best, j = 0; j < 4; i = (i + 1) & 3, j++)
	{
		point p = *(snake[id].begin());//读取最近的位置

		int snakex = p.x + dx[i];
		int snakey = p.y + dy[i];

		if (map[snakex][snakey] != 0) continue;//障碍

		total = move1(id, i, total);//移动
		int idnext = (id == 1 ? 0 : 1);
		endflag = 0;//还有路可以走

		int score = -alpha_beta(depth - 1, idnext, -beta, -alpha, total, 0, hash_value * 20011 + i);//递归

		ret["response"]["score"] = score;


		if (id == 1) {//恢复移动
			total--;
		}
		snake[id].pop_front();

		if (score > now) {
			bestmove = i;
			now = score;
			ret["response"]["now"] = now;
			if (alpha < score) {//剪枝
				alpha = score;
				if (firstpoint == 1) {
					returnvalue = i;
				}
			}
			if (score > beta) {//剪枝
				break;
			}
		}
	}
	if (endflag) {
		now = noplace(id);//无路可走\成功处理
	}

	if (!id) {//恢复尾部

		if (whetherGrow(total) == 0)
		{
			snake[0].push_back(p0);
			snake[1].push_back(p1);
		}
	}

	hash_move[hash_value & (HASH_SIZE - 1)] = bestmove;//hash
	return now;
}


int mtd(int alpha, int beta, int depth, int test,int total) {
	int best_value;
	do {
		// 进行零宽窗口试探
		best_value = alpha_beta(depth, 0, test - 1, test, total,1, 0);
		// 如果是alpha节点
		if (best_value < test) {
			// 更新估值上限，并将此做为新的试探值
			test = beta = best_value;
			// 否则是beta节点
		}
		else {
			// 更新估值下限
			alpha = best_value;
			// 新的试探值
			test = best_value + 1;
		}
	} while (alpha < beta);
	return best_value;
}



int main()
{
	memset(invalid, 0, sizeof(invalid));
	string str;
	//string temp;
	//while (getline(cin, temp))
	//	str += temp;
	cin >> str;
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
	dist_value[0] = 10000;
	for (int i = 1; i < 300; i++)
		dist_value[i] = dist_value[i - 1] * 0.93;//加权得到不同距离的权值

	for (int i = 0, j = 0; i < 300; i++)
		if (whetherGrow(i) == 0)
			disteps[j++] = i + 1;
	Time = clock();//记录开始时间

	
	int test = -WIN;
	int depth = 6;
	while (1)
	{

		test = mtd(-WIN, WIN, depth, -WIN, total);

		depth += 2;
		DEPTH = depth / 2;
	}

	return 0;
}
